/// @file dpf/interval_memoizer.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_INTERVAL_MEMOIZER_HPP__
#define LIBDPF_INCLUDE_DPF_INTERVAL_MEMOIZER_HPP__

#include <cstddef>
#include <cstring>
#include <type_traits>
#include <functional>
#include <algorithm>
#include <new>
#include <limits>
#include <stdexcept>
#include <optional>
#include <array>
#include <functional>

#include "dpf/dpf_key.hpp"

namespace dpf
{

template <typename DpfKey,
          typename ReturnT = typename DpfKey::interior_node *>
struct interval_memoizer_base
{
  public:
    using dpf_type = DpfKey;
    using integral_type = typename DpfKey::integral_type;
    using return_type = ReturnT;
    using iterator_type = return_type;
    using node_type = typename DpfKey::interior_node;

    // level 0 should access the root
    // level goes up to (and including) depth
    virtual return_type operator[](std::size_t) const noexcept = 0;

    // iterators should access most recently completed level
    virtual return_type begin() const noexcept = 0;
    virtual return_type end() const noexcept = 0;

    virtual std::size_t assign_interval(const dpf_type & dpf, integral_type new_from, integral_type new_to, bool new_wrap)
    {
        // If the interior is not fully evaluated (level_index != depth+1), and
        //   the to and from nodes do not match, force full recalculation.
        //
        // See recalculate() documentation below for explanation.
        //   Determines if recalculation is needed based on currently computed nodes
        if (dpf_.has_value() == false
            || std::memcmp(&dpf_root_, &dpf.root(), sizeof(node_type)) != 0
            || std::memcmp(&dpf_common_part_hash_, &dpf.common_part_hash(), sizeof(digest_type)) != 0
            || (level_index != depth+1 && (from_ != new_from || to_ != new_to))
            || recalculate(new_from, new_to, new_wrap))
        {
            this->operator[](0)[0] = dpf.root();
            dpf_ = std::cref(dpf);
            dpf_root_ = dpf.root();
            dpf_common_part_hash_ = dpf.common_part_hash();
            from_ = new_from;
            to_ = new_to;
            wrap_ = new_wrap;
            level_index = 1;
            get_nodes_at_level_function_ = wrap_ ?
                interval_memoizer_base::get_nodes_at_level_wrap :
                interval_memoizer_base::get_nodes_at_level_no_wrap;
            if (get_nodes_at_level_function_(depth, from_, to_) > output_length)
            {
                throw std::length_error("size of new interval is too large for memoizer");
            }
        }

        return level_index;
    }

    std::size_t advance_level()
    {
        return ++level_index;
    }

    std::size_t get_nodes_at_level() const
    {
        return get_nodes_at_level_function_(level_index, from_, to_);
    }

    std::size_t get_nodes_at_level(std::size_t level) const
    {
        return get_nodes_at_level_function_(level, from_, to_);
    }

    std::size_t get_nodes_at_level(std::size_t level, integral_type from_node, integral_type to_node)
    {
        return get_nodes_at_level_function_(level, from_node, to_node);
    }

    static std::size_t get_nodes_at_level(std::size_t level, integral_type from_node, integral_type to_node, bool wrap)
    {
        if (wrap == true)
        {
            return get_nodes_at_level_wrap(level, from_node, to_node);
        }
        return get_nodes_at_level_no_wrap(level, from_node, to_node);
    }

    static std::size_t get_nodes_at_level_no_wrap(std::size_t level, integral_type from_node, integral_type to_node)
    {
        // Algorithm explanation:
        //   Input:
        //     * offset - (derived from depth and level, note that level of 0 represents the root of the tree)
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
        return ((to_node - 1) >> offset) - (from_node >> offset) + 1;
    }

    static std::size_t get_nodes_at_level_wrap(std::size_t level, integral_type from_node, integral_type to_node)
    {
        // Algorithm explanation:
        //   See above for general idea behind derivation.
        //
        //   Main difference here is that the known wrapping changes observation 1. The relevant calculation for
        //   a given level and a known [from, to) range becomes (((1 << level) - from) + to). ((1 << level) - from)
        //   gives the number of nodes in the range [from, 2^level), while (to) gives the number of nodes in the
        //   range [0, to).
        //
        //   Observation 2 still applies as normal.
        //
        //   Performing the calculation directly can cause specific nodes to be computed twice. To rectify this,
        //   simply return the minimum of the direct calculation with the maximum possible nodes for that level.

        std::size_t offset = depth - level;
        integral_type max_nodes_at_level = integral_type(1) << level;
        return std::min(max_nodes_at_level - (from_node >> offset) + ((to_node - 1) >> offset) + 1, max_nodes_at_level);
    }

    std::size_t get_exterior_start(integral_type from_node)
    {
        return from_node - from_ + (from_ > from_node ? max_nodes_at_depth : 0);
    }

  protected:
    static constexpr auto depth = dpf_type::depth;
    static constexpr auto max_nodes_at_depth = integral_type{1} << depth;
    std::size_t output_length;
    std::size_t level_index;  // indicates current level being built

    explicit interval_memoizer_base(std::size_t output_len)
      : dpf_{std::nullopt},
        from_{0},
        to_{0},
        wrap_{false},
        output_length{output_len},
        level_index{0}
    { }

  private:
    std::optional<std::reference_wrapper<const dpf_type>> dpf_;
    node_type dpf_root_;
    digest_type dpf_common_part_hash_;
    integral_type from_;
    integral_type to_;
    bool wrap_;
    std::function<std::size_t(std::size_t, integral_type, integral_type)> get_nodes_at_level_function_;

    bool recalculate(integral_type new_from, integral_type new_to, bool new_wrap)
    {
        // If wrap == false, from_ == 0, to_ == 2^depth,
        //   then no recalculation is needed as this was the result of an unshifted eval_full,
        //   otherwise recalculation is determined based on the following cases:
        //
        // 1: wrap_ == false, new_wrap == true
        //    guaranteed recalculation
        //
        // 2: wrap_ == false, new_wrap == false
        //    if from_ <= new_from && new_to <= to_, no recalculation needed
        //
        // 3: wrap_ == true, new_wrap == false
        //    if either from_ <= new_from or new_to <= to_, no recalculation is needed
        //
        // 4: wrap_ == true, new_wrap == true
        //    if from_ <= new_from && new_to <= to_, no recalculation needed
        //
        // Strictly speaking, new_from < new_to should also be checked for the cases where
        //   new_wrap == false, but since new_wrap == false, this check is guaranteed to pass

        // do not recalculate as current calculation is an eval_full
        if (wrap_ == false && from_ == 0 && to_ == max_nodes_at_depth)
        {
            return false;
        }

        if ((wrap_ == false && new_wrap == true)                                               // case 1
            || (wrap_ == new_wrap && !(from_ <= new_from && new_to <= to_))                    // case 2 and 4
            || (wrap_ == true && new_wrap == false && !(from_ <= new_from || new_to <= to_)))  // case 3
        {
            return true;
        }

        return false;
    }

};

template <typename DpfKey,
          typename Allocator = aligned_allocator<typename DpfKey::interior_node>>
struct basic_interval_memoizer final : public interval_memoizer_base<DpfKey>
{
  private:
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using parent = interval_memoizer_base<DpfKey>;
HEDLEY_PRAGMA(GCC diagnostic pop)
  public:
    using unique_ptr = typename Allocator::unique_ptr;
    using return_type = typename DpfKey::interior_node *;
    using parent::depth;
    using parent::level_index;
    using parent::get_nodes_at_level;

    // See comment for full_tree_interval_memoizer::initialize_endpoints() for
    //   general explanation of derivation for "nodes at previous level".
    // When creating the final level of interior nodes from the previous level,
    //   care must be taken not to overwrite the previous level until the relevant
    //   nodes have been used to generate the new level. This means the pivot must
    //   be selected to push the previous level as far to the end of the buffer as
    //   possible.
    // For n nodes in the final level:
    //     n odd => (n+1)/2 nodes on previous level
    //           => pivot = n-(n+1)/2 = (n-1)/2 = n/2-1/2 = floor(n/2)
    //     n even => n/2 OR (n+2)/2 nodes on previous level
    //            => pivot = n-(n+2)/2 = (n-2)/2 = n/2-1
    //     unified => floor(n/2)-1+(n%2) = (n>>1)+(n&1)-1
    // In general, each previous level has roughly one half the nodes, but this is
    //   not true for some small n, which can stay constant up to the root.
    //   To handle this, take the maximum between the unified calculation shown
    //   and the number of nodes two levels up from the final level.
    // For n nodes in the final level:
    //     at most ((n+2)/2+2)/2 = n+6>>2 nodes two levels up
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    explicit basic_interval_memoizer(std::size_t output_len, Allocator alloc = Allocator{})
      : parent::interval_memoizer_base(output_len),
        pivot{std::max((output_len>>1)+(output_len&1)-1, output_len+6>>2)},
        buf{alloc.allocate_unique_ptr(pivot+((output_len+2)>>1))}
    {
        if (HEDLEY_UNLIKELY(buf == nullptr)) throw std::bad_alloc{};
    }
HEDLEY_PRAGMA(GCC diagnostic pop)

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type operator[](std::size_t level) const noexcept override
    {
        bool b = (depth ^ level) & 1;
        return Allocator::assume_aligned(&buf[b*pivot]);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type begin() const noexcept override
    {
        return this->operator[](level_index - 1);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type end() const noexcept override
    {
        return this->operator[](level_index - 1) + get_nodes_at_level(level_index - 1);
    }

  private:
    static constexpr auto clz = utils::countl_zero<std::size_t>{};
    std::size_t pivot;
    unique_ptr buf;
};

template <typename DpfKey,
          typename Allocator = aligned_allocator<typename DpfKey::interior_node>>
struct full_tree_interval_memoizer final : public interval_memoizer_base<DpfKey>
{
  private:
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using parent = interval_memoizer_base<DpfKey>;
HEDLEY_PRAGMA(GCC diagnostic pop)
  public:
    using node_type = typename DpfKey::interior_node;
    using unique_ptr = typename Allocator::unique_ptr;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using return_type = std::add_pointer_t<node_type>;
HEDLEY_PRAGMA(GCC diagnostic pop)
    using integral_type = typename DpfKey::integral_type;
    using parent::depth;
    using parent::level_index;
    using parent::get_nodes_at_level;

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    explicit full_tree_interval_memoizer(std::size_t output_len,
        Allocator alloc = Allocator{})
      : parent::interval_memoizer_base(output_len),
        level_endpoints{initialize_endpoints(output_len)},
        buf{alloc.allocate_unique_ptr(level_endpoints[depth] + output_len)}
    {
        if (HEDLEY_UNLIKELY(buf == nullptr)) throw std::bad_alloc{};
    }
HEDLEY_PRAGMA(GCC diagnostic pop)

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type operator[](std::size_t level) const noexcept override
    {
        return Allocator::assume_aligned(&buf[level_endpoints[level]]);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type begin() const noexcept override
    {
        return this->operator[](level_index - 1);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type end() const noexcept override
    {
        return this->operator[](level_index - 1) + get_nodes_at_level(level_index - 1);
    }

  private:
    const std::array<std::size_t, depth+1> level_endpoints;
    unique_ptr buf;

    // For n nodes on a given level, there are the following cases:
    //     n odd => (n+1)/2 nodes on previous level
    //         ex. 5 nodes on current level grouped as
    //             |..|..|.| or |.|..|..|
    //             where both give 3 nodes on previous level
    //     n even => n/2 OR (n+2)/2 nodes on previous level
    //         ex. 6 nodes on current level grouped as
    //             |..|..|..| or |.|..|..|.|
    //             gives either 3 or 4 nodes on previous level
    // Clearly (n+2)/2 is the worst case, so this is used in the derivation
    //   for the number of nodes on each level.
    // Also note that at depth (from the root) i, there can't be more than 2^i
    //   nodes hence the `min()` function call.
    static constexpr auto initialize_endpoints(integral_type len)
    {
        std::array<std::size_t, depth+1> level_endpoints{0};
        for (std::size_t level=depth; level > 0; --level)
        {
            len = std::min(len+2 >> 1, integral_type(1) << level-1);
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

    if (from > to)
    {
        throw std::domain_error("from cannot be greater than to");
    }

    utils::flip_msb_if_signed_integral(from);
    utils::flip_msb_if_signed_integral(to);

    std::size_t nodes_in_interval = utils::get_leafnodes_in_output_interval<dpf_type>(from, to);

    return MemoizerT(nodes_in_interval);
}

}  // namespace detail

template <typename DpfKey,
          typename InputT>
inline auto make_basic_interval_memoizer(InputT from, InputT to)
{
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    return detail::make_interval_memoizer<DpfKey, basic_interval_memoizer<DpfKey>, InputT>(from, to);
HEDLEY_PRAGMA(GCC diagnostic pop)
}

template <typename DpfKey,
          typename InputT>
inline auto make_basic_interval_memoizer(const DpfKey &, InputT from, InputT to)
{
    return make_basic_interval_memoizer<DpfKey>(from, to);
}

template <typename DpfKey>
inline auto make_basic_full_memoizer()
{
    using input_type = typename DpfKey::input_type;

    return make_basic_interval_memoizer<DpfKey>(
        std::numeric_limits<input_type>::min(),
        std::numeric_limits<input_type>::max());
}

template <typename DpfKey>
inline auto make_basic_full_memoizer(const DpfKey &)
{
    return make_basic_full_memoizer<DpfKey>();
}

template <typename DpfKey,
          typename InputT>
inline auto make_full_tree_interval_memoizer(InputT from, InputT to)
{
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    return detail::make_interval_memoizer<DpfKey, full_tree_interval_memoizer<DpfKey>, InputT>(from, to);
HEDLEY_PRAGMA(GCC diagnostic pop)
}

template <typename DpfKey,
          typename InputT>
inline auto make_full_tree_interval_memoizer(const DpfKey &, InputT from, InputT to)
{
    return make_full_tree_interval_memoizer<DpfKey>(from, to);
}

template <typename DpfKey>
inline auto make_full_tree_full_memoizer()
{
    using input_type = typename DpfKey::input_type;

    return make_full_tree_interval_memoizer<DpfKey>(
        std::numeric_limits<input_type>::min(),
        std::numeric_limits<input_type>::max());
}

template <typename DpfKey>
inline auto make_full_tree_full_memoizer(const DpfKey &)
{
    return make_full_tree_full_memoizer<DpfKey>();
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_INTERVAL_MEMOIZER_HPP__
