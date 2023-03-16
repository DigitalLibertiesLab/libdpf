/// @file dpf/interval_memoizer.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_INTERVAL_MEMOIZER_HPP__
#define LIBDPF_INCLUDE_DPF_INTERVAL_MEMOIZER_HPP__

#include "dpf/dpf_key.hpp"

namespace dpf
{

template <typename DpfKey>
struct interval_memoizer_base
{
  public:
    using dpf_type = DpfKey;
    using node_type = typename DpfKey::interior_node_t;

    // level 0 should access the root
    // level goes up to (and including) depth
    virtual node_type * operator[](std::size_t) const noexcept = 0;

    virtual std::size_t assign_interval(const dpf_type & dpf, std::size_t new_from, std::size_t new_to)
    {
        static constexpr auto complement_of = std::bit_not{};
        if (dpf_.has_value() == false
            || std::addressof(dpf_->get()) != std::addressof(dpf)
            || from_.value_or(complement_of(new_from)) != new_from
            || to_.value_or(complement_of(new_to)) != new_to)
        {
            if (new_to - new_from > output_length)
            {
                throw std::length_error("size of new interval is too large for memoizer");
            }

            this->operator[](0)[0] = dpf.root;
            dpf_ = std::cref(dpf);
            from_ = new_from;
            to_ = new_to;
            level_index = 1;
        }

        return level_index;
    }

    std::size_t advance_level()
    {
        return ++level_index;
    }

    std::size_t get_nodes_at_level(std::size_t level) const
    {
        return get_nodes_at_level(level, from_.value_or(0), to_.value_or(0));
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

        std::size_t offset = depth - level;
        return (to_node - 1 >> offset) - (from_node >> offset) + 1;
    }

  protected:
    static constexpr auto depth = dpf_type::depth;
    const std::size_t output_length;

    explicit interval_memoizer_base(std::size_t output_len)
      : dpf_{std::nullopt},
        from_{std::nullopt},
        to_{std::nullopt},
        output_length{output_len},
        level_index{0}
    { }

  private:
    std::optional<std::reference_wrapper<const dpf_type>> dpf_;
    std::optional<std::size_t> from_;
    std::optional<std::size_t> to_;
    std::size_t level_index;
};

template <typename DpfKey,
          typename Allocator = aligned_allocator<typename DpfKey::interior_node_t>>
struct basic_interval_memoizer final : public interval_memoizer_base<DpfKey>
{
  private:
    using parent = interval_memoizer_base<DpfKey>;
  public:
    using dpf_type = DpfKey;
    using node_type = typename DpfKey::interior_node_t;
    using unique_ptr = typename Allocator::unique_ptr;
    using parent::depth;
    using parent::output_length;

    explicit basic_interval_memoizer(std::size_t output_len, Allocator alloc = Allocator{})
      : parent::interval_memoizer_base(output_len),
        pivot{(dpf::utils::msb_of_v<std::size_t> >> clz(output_len))/2},
        length{std::max(3*pivot, output_len)},
        buf{alloc.allocate_unique_ptr(length)}
    {
        // check that `pivot` will be 32-byte aligned, as its asserted as such
        // (for optimization reasons) in `operator[]`
        if (pivot * sizeof(node_type) < dpf::utils::max_align_v)
        {
            throw std::domain_error("output must be at least 64 bytes");
        }
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    node_type * operator[](std::size_t level) const noexcept override
    {
        bool b = (depth ^ level) & 1;
        return Allocator::assume_aligned(&buf[b*pivot]);
    }

  private:
    static constexpr auto clz = utils::countl_zero<std::size_t>{};
    const std::size_t pivot;
    const std::size_t length;
    unique_ptr buf;
};

template <typename DpfKey,
          typename Allocator = aligned_allocator<typename DpfKey::interior_node_t>>
struct full_tree_interval_memoizer final : public interval_memoizer_base<DpfKey>
{
  private:
    using parent = interval_memoizer_base<DpfKey>;
  public:
    using dpf_type = DpfKey;
    using node_type = typename DpfKey::interior_node_t;
    using unique_ptr = typename Allocator::unique_ptr;
    using parent::depth;
    using parent::output_length;

    explicit full_tree_interval_memoizer(std::size_t output_len,
        Allocator alloc = Allocator{})
      : parent::interval_memoizer_base(output_len),
        level_endpoints{initialize_endpoints()},
        buf{alloc.allocate_unique_ptr(level_endpoints[depth] + output_len)}
    { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    node_type * operator[](std::size_t level) const noexcept override
    {
        return Allocator::assume_aligned(&buf[level_endpoints[level]]);
    }

  private:
    const std::array<std::size_t, depth+1> level_endpoints;
    unique_ptr buf;

    constexpr auto initialize_endpoints()
    {
        std::array<std::size_t, depth+1> level_endpoints{0};
        for (std::size_t level=depth, len=output_length; level > 0; --level)
        {
            len = std::min(len/2 + 1, std::size_t(1) << level-1);
            level_endpoints[level] = len;
        }

        for (std::size_t level = 0; level < depth; ++level)
        {
            level_endpoints[level+1] = level_endpoints[level] + level_endpoints[level+1];
        }

        return level_endpoints;
    }
};

namespace detail
{

template <typename DpfKey,
          typename MemoizerT,
          typename InputT>
HEDLEY_ALWAYS_INLINE
auto make_interval_memoizer(InputT from, InputT to)
{
    using dpf_type = DpfKey;
    using input_type = InputT;

    if (from > to)
    {
        throw std::domain_error("from cannot be greater than to");
    }

    std::size_t from_node = utils::quotient_floor(from, (input_type)dpf_type::outputs_per_leaf),
        to_node = utils::quotient_ceiling((input_type)(to+1), (input_type)dpf_type::outputs_per_leaf);
    auto nodes_in_interval = to_node - from_node;

    if (nodes_in_interval*sizeof(typename dpf_type::interior_node_t) < 64)
    {
        throw std::out_of_range("intervals must span at least 64 bytes");
    }

    return MemoizerT(nodes_in_interval);
}

}  // namespace dpf::detail

template <typename DpfKey,
          typename InputT = typename DpfKey::input_type>
auto make_basic_interval_memoizer(const DpfKey &, InputT from = 0,
    InputT to = std::numeric_limits<InputT>::max())
{
    return detail::make_interval_memoizer<DpfKey, basic_interval_memoizer<DpfKey>, InputT>(from, to);
}

template <typename DpfKey,
          typename InputT = typename DpfKey::input_type>
auto make_full_tree_interval_memoizer(const DpfKey &, InputT from = 0,
    InputT to = std::numeric_limits<InputT>::max())
{
    return detail::make_interval_memoizer<DpfKey, full_tree_interval_memoizer<DpfKey>, InputT>(from, to);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_INTERVAL_MEMOIZER_HPP__
