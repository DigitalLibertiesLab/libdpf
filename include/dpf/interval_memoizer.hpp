/// @file dpf/path_memoizer.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief 
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_INTERVAL_MEMOIZER_HPP__
#define LIBDPF_INCLUDE_DPF_INTERVAL_MEMOIZER_HPP__

namespace dpf
{

template <typename NodeT>
struct interval_memoizer
{
  public:
    explicit interval_memoizer()
      : level_index{0}
    { }

    virtual void reset()
    {
        level_index = 0;
    }

    // level -1 should access the root
    // level goes up to (but not including) depth
    virtual NodeT* operator[](std::size_t) const noexcept = 0;

    static std::size_t get_nodes_at_level(std::size_t depth, std::size_t level, std::size_t from_node, std::size_t to_node)
    {
        // Algorithm explanation:
        //   Input:
        //     * offset - (derived from depth and level, note that level of -1 represents the root of the tree)
        //     * range of nodes - [from_node, to_node)
        //
        //   Observation 1:
        //     For any level, knowing the range [from, to) allows one to calculate the number of nodes at that level
        //     as (to - from).
        //
        //   Observation 2:
        //     If the range were stated as [from_0, to_0] for an offset 0, then [from_n, to_n] = [from_0 >> n, to_0 >> n]
        //     where >> is the bitshift operator. This is because the bits representing a node also represent the path
        //     taken in a binary tree to get to that node. Since from_0 and to_0 are both inclusive bounds, then their
        //     parent nodes must also be inclusive bounds for the next level up. These nodes can be found by simply removing
        //     the LSB from from_0 and to_0. The same can be done for parents further up the tree.
        //
        //   Putting it together:
        //     * to_node-1 converts an excluded node to an included node
        //     * bit shifting as explained in observation 2
        //     * add 1 since observation 1 is for an excluded end point whereas now both end points are included

        std::size_t offset = depth - level - 1;
        return (to_node - 1 >> offset) - (from_node >> offset) + 1;
    }

    std::size_t level_index;
};

template <typename NodeT,
          typename Allocator = detail::aligned_allocator<NodeT>>
struct basic_interval_memoizer : public interval_memoizer<NodeT>
{
  public:
    using unique_ptr = typename Allocator::unique_ptr;

    explicit basic_interval_memoizer(std::size_t output_len, std::size_t depth, Allocator alloc = Allocator{})
      : pivot{(dpf::utils::msb_of_v<std::size_t> >> clz(output_len))/2},
        depth{depth},
        length{std::max(3*pivot, output_len)},
        buf{alloc.allocate_unique_ptr(length * sizeof(NodeT))}
    {
        // TODO: is this needed / correct
        // if (pivot < 32)  // check alignment (pivot is 0 or some power of 2)
        // {
        //     throw std::domain_error("output_len must be at least 64");
        // }
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    NodeT* operator[](std::size_t level) const noexcept override
    {
        auto b = !((depth ^ level) & 1);
        return Allocator::assume_aligned(&buf[b*pivot]);
    }

  private:
    static constexpr auto clz = utils::countl_zero<std::size_t>{};
    const std::size_t pivot;
    const std::size_t depth;
  public:
    const std::size_t length;
    unique_ptr buf;
};

template <typename NodeT,
          typename Allocator = detail::aligned_allocator<NodeT>>
struct full_tree_interval_memoizer : public interval_memoizer<NodeT>
{
  public:
    using unique_ptr = typename Allocator::unique_ptr;

    explicit full_tree_interval_memoizer(std::size_t output_len, std::size_t depth, Allocator alloc = Allocator{})
      : depth{depth},
        output_length{output_len},
        idxs{new std::size_t[depth+1]},
        length{calc_length()},
        buf{alloc.allocate_unique_ptr(length)}
    { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    NodeT* operator[](std::size_t level) const noexcept override
    {
        return Allocator::assume_aligned(&buf[idxs[level+1]]);
    }

  private:
    constexpr std::size_t calc_length()
    {
        std::size_t len = output_length;
        std::size_t ret = 1 + len;  // account for root and leaves
        idxs[0] = 0;
        idxs[1] = 1;

        for (std::size_t level = depth-1; level > 0; --level)
        {
            len = std::min(len / 2 + 1, static_cast<std::size_t>(1) << (level));
            ret += len;
            idxs[level+1] = len;
        }

        for (std::size_t i = 0; i < depth; ++i)
        {
            idxs[i+1] = idxs[i] + idxs[i+1];
        }

        return ret;
    }

    const std::size_t depth;
    const std::size_t output_length;
    std::unique_ptr<std::size_t[]> idxs;
  public:
    const std::size_t length;
    unique_ptr buf;
};

// TODO: find better way to specify which memoizer is being used
template <typename MemoizerT = basic_interval_memoizer<dpf::prg::aes128::block_t>,
          typename DpfKey,
          typename InputT>
auto make_interval_memoizer(const DpfKey &, InputT from = 0,
    InputT to = std::numeric_limits<InputT>::max())
{
    // TODO: throw exceptions when based on following checks:
    // check from <= to
    // check if 0

    auto from_node = from/DpfKey::outputs_per_leaf, to_node = 1+to/DpfKey::outputs_per_leaf;
    auto nodes_in_interval = to_node - from_node;

    return MemoizerT(nodes_in_interval, DpfKey::depth);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_INTERVAL_MEMOIZER_HPP__