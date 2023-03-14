/// @file dpf/interval_memoizer.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_INTERVAL_MEMOIZER_HPP__
#define LIBDPF_INCLUDE_DPF_INTERVAL_MEMOIZER_HPP__

#include "dpf/dpf_key.hpp"

namespace dpf
{

template <typename DpfKey>
struct interval_memoizer
{
  public:
    using dpf_type = DpfKey;
    using node_type = typename DpfKey::interior_node_t;
    static constexpr auto depth = dpf_type::depth;

    explicit interval_memoizer(const dpf_type & dpf, std::size_t output_len)
      : dpf_{std::cref(dpf)},
        from_{std::nullopt},
        to_{std::nullopt},
        output_length{output_len},
        level_index{0}
    { }

    explicit interval_memoizer(std::size_t output_len)
      : dpf_{std::nullopt},
        from_{std::nullopt},
        to_{std::nullopt},
        output_length{output_len},
        level_index{0}
    { }

    // level -1 should access the root
    // level goes up to (but not including) depth
    virtual node_type * operator[](std::size_t) const noexcept = 0;

    virtual std::size_t assign_interval(const dpf_type & dpf, std::size_t new_from, std::size_t new_to)
    {
        static constexpr auto complement_of = std::bit_not{};
        if (std::addressof(dpf_->get()) != std::addressof(dpf)
            || from_.value_or(complement_of(new_from)) != new_from
            || to_.value_or(complement_of(new_to)) != new_to)
        {
            if (new_to - new_from > output_length)
            {
                throw std::length_error("size of new interval is too large for memoizer");
            }

            this->operator[](-1)[0] = dpf.root;
            dpf_ = std::cref(dpf);
            from_ = new_from;
            to_ = new_to;
            level_index = 0;
        }

        return level_index;
    }

    std::size_t advance_level()
    {
        return ++level_index;
    }

    std::size_t get_nodes_at_level(std::size_t level)
    {
        // if (from_.has_value() == false || to_.has_value() == false)
        // {
        //     throw std::runtime_error("from_node or to_node not properly set");
        // }
        return get_nodes_at_level(level, from_.value(), to_.value());
    }

    static std::size_t get_nodes_at_level(std::size_t level, std::size_t from_node, std::size_t to_node)
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

  protected:
    const std::size_t output_length;

  private:
    std::optional<std::reference_wrapper<const dpf_type>> dpf_;
    std::optional<std::size_t> from_;
    std::optional<std::size_t> to_;
    std::size_t level_index;
};

template <typename DpfKey,
          typename Allocator = aligned_allocator<typename DpfKey::interior_node_t>>
struct basic_interval_memoizer : public interval_memoizer<DpfKey>
{
  public:
    using dpf_type = DpfKey;
    using node_type = typename DpfKey::interior_node_t;
    using unique_ptr = typename Allocator::unique_ptr;
    using interval_memoizer<dpf_type>::depth;
    using interval_memoizer<dpf_type>::output_length;

    explicit basic_interval_memoizer(const dpf_type & dpf, std::size_t output_len, Allocator alloc = Allocator{})
      : interval_memoizer<DpfKey>(dpf, output_len),
        pivot{(dpf::utils::msb_of_v<std::size_t> >> clz(output_len))/2},
        length{std::max(3*pivot, output_len)},
        buf{alloc.allocate_unique_ptr(length * sizeof(node_type))}
    {
        // TODO: is this needed / correct
        // if (pivot < 32)  // check alignment (pivot is 0 or some power of 2)
        // {
        //     throw std::domain_error("output_len must be at least 64");
        // }
    }

    explicit basic_interval_memoizer(std::size_t output_len, Allocator alloc = Allocator{})
      : interval_memoizer<DpfKey>(output_len),
        pivot{(dpf::utils::msb_of_v<std::size_t> >> clz(output_len))/2},
        length{std::max(3*pivot, output_len)},
        buf{alloc.allocate_unique_ptr(length * sizeof(node_type))}
    { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    node_type * operator[](std::size_t level) const noexcept override
    {
        auto b = !((depth ^ level) & 1);
        return Allocator::assume_aligned(&buf[b*pivot]);
    }

  private:
    static constexpr auto clz = utils::countl_zero<std::size_t>{};
    const std::size_t pivot;
  public:
    const std::size_t length;
    unique_ptr buf;
};

template <typename DpfKey,
          typename Allocator = aligned_allocator<typename DpfKey::interior_node_t>>
struct full_tree_interval_memoizer : public interval_memoizer<DpfKey>
{
  public:
    using dpf_type = DpfKey;
    using node_type = typename DpfKey::interior_node_t;
    using unique_ptr = typename Allocator::unique_ptr;
    using interval_memoizer<dpf_type>::depth;
    using interval_memoizer<dpf_type>::output_length;

    explicit full_tree_interval_memoizer(const dpf_type & dpf, std::size_t output_len, Allocator alloc = Allocator{})
      : interval_memoizer<DpfKey>(dpf, output_len),
        idxs{new std::size_t[depth+1]},
        length{calc_length()},
        buf{alloc.allocate_unique_ptr(length)}
    { }

    explicit full_tree_interval_memoizer(std::size_t output_len, Allocator alloc = Allocator{})
      : interval_memoizer<DpfKey>(output_len),
        idxs{new std::size_t[depth+1]},
        length{calc_length()},
        buf{alloc.allocate_unique_ptr(length)}
    { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    node_type * operator[](std::size_t level) const noexcept override
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

    std::unique_ptr<std::size_t[]> idxs;
  public:
    const std::size_t length;
    unique_ptr buf;
};

template <typename DpfKey,
          typename InputT>
auto make_basic_interval_memoizer(const DpfKey & dpf, InputT from = 0,
    InputT to = std::numeric_limits<InputT>::max())
{
    if (from > to)
    {
        throw std::domain_error("from cannot be greater than to");
    }

    std::size_t from_node = utils::quotient_floor(from, (InputT)DpfKey::outputs_per_leaf), to_node = utils::quotient_ceiling(to, (InputT)DpfKey::outputs_per_leaf);
    auto nodes_in_interval = to_node - from_node;

    if (nodes_in_interval == 0)
    {
        throw std::out_of_range("attempting to evaluate 0 nodes, possibly caused by overflow");
    }

    return basic_interval_memoizer<DpfKey>(dpf, nodes_in_interval);
}

template <typename DpfKey,
          typename InputT>
auto make_full_tree_interval_memoizer(const DpfKey & dpf, InputT from = 0,
    InputT to = std::numeric_limits<InputT>::max())
{
    if (from > to)
    {
        throw std::domain_error("from cannot be greater than to");
    }

    std::size_t from_node = utils::quotient_floor(from, (InputT)DpfKey::outputs_per_leaf), to_node = utils::quotient_ceiling(to, (InputT)DpfKey::outputs_per_leaf);
    auto nodes_in_interval = to_node - from_node;

    if (nodes_in_interval == 0)
    {
        throw std::domain_error("attempting to evaluate 0 nodes, possibly caused by overflow");
    }

    return full_tree_interval_memoizer<DpfKey>(dpf, nodes_in_interval);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_INTERVAL_MEMOIZER_HPP__
