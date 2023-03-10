/// @file dpf/memoization.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief miscellaneous helper functions, structs, preprocessor directives
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_EVAL_INTERVAL_HPP__
#define LIBDPF_INCLUDE_DPF_EVAL_INTERVAL_HPP__

#include <functional>
#include <memory>
#include <limits>
#include <tuple>
#include <algorithm>

#include <portable-snippets/builtin/builtin.h>
#include <hedley/hedley.h>

#include "dpf/dpf_key.hpp"

namespace dpf
{

template <typename DpfKey,
          class IntervalMemoizer>
DPF_UNROLL_LOOPS
inline auto eval_interval_interior(const DpfKey & dpf, std::size_t from_node, std::size_t to_node,
    IntervalMemoizer & memoizer, std::size_t to_level = DpfKey::tree_depth)
{
    using node_t = typename DpfKey::interior_node_t;
    auto nodes_in_interval = to_node - from_node;

    typename DpfKey::input_type mask = dpf.msb_mask >> (memoizer.level_index + DpfKey::lg_outputs_per_leaf);
    std::size_t nodes_at_level = IntervalMemoizer::get_nodes_at_level(dpf.tree_depth, memoizer.level_index-1, from_node, to_node);
    // TODO: should this be level_index-1 instead of level_index?

    if (memoizer.level_index == 0)
    {
        memoizer[-1][0] = dpf.root;
        // TODO: is this fixed by level_index-1 above?
        // nodes_at_level = 1;
    }

    for (; memoizer.level_index < to_level; ++memoizer.level_index, mask>>=1)
    {
        std::size_t i = !!(mask & from_node), j = i;
        const node_t cw[2] = {
            set_lo_bit(dpf.interior_cws[memoizer.level_index], dpf.correction_advice[memoizer.level_index]&1),
            set_lo_bit(dpf.interior_cws[memoizer.level_index], (dpf.correction_advice[memoizer.level_index]>>1)&1)
        };

        if (i == 1)
        {
            memoizer[memoizer.level_index][0] = DpfKey::traverse_interior(memoizer[memoizer.level_index-1][0], cw[1], 1);
        }
        for (; j < nodes_at_level-1; ++j)
        {
            memoizer[memoizer.level_index][i++] = DpfKey::traverse_interior(memoizer[memoizer.level_index-1][j], cw[0], 0);
            memoizer[memoizer.level_index][i++] = DpfKey::traverse_interior(memoizer[memoizer.level_index-1][j], cw[1], 1);
        }
        nodes_at_level = IntervalMemoizer::get_nodes_at_level(dpf.tree_depth, memoizer.level_index, from_node, to_node);
        memoizer[memoizer.level_index][i++] = DpfKey::traverse_interior(memoizer[memoizer.level_index-1][j], cw[0], 0);
        if (i < nodes_at_level)
        {
            memoizer[memoizer.level_index][i++] = DpfKey::traverse_interior(memoizer[memoizer.level_index-1][j], cw[1], 1);
        }
    }
}

template <std::size_t I = 0,
          typename DpfKey,
          class OutputBuffer,
          class IntervalMemoizer>
DPF_UNROLL_LOOPS
inline auto eval_interval_exterior(const DpfKey & dpf, std::size_t from_node, std::size_t to_node,
    OutputBuffer & outbuf, IntervalMemoizer & memoizer)
{
    assert_not_wildcard<I>(dpf);

    using exterior_node_t = typename DpfKey::exterior_node_t;
    using output_t = std::tuple_element_t<I, typename DpfKey::outputs_t>;

    auto nodes_in_interval = to_node - from_node;

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    auto cw = dpf.template exterior_cw<I>();
    auto rawbuf = reinterpret_cast<exterior_node_t *>(std::data(outbuf));
    for (std::size_t j = 0, k = 0; j < nodes_in_interval; ++j,
        k += dpf::block_length_of_leaf_v<output_t, exterior_node_t>)
    {
        auto leaf = DpfKey::template traverse_exterior<I>(memoizer[DpfKey::tree_depth-1][j],
            dpf::get_if_lo_bit(cw, memoizer[DpfKey::tree_depth-1][j]));
        std::memcpy(&rawbuf[k], &leaf, sizeof(leaf));
    }
HEDLEY_PRAGMA(GCC diagnostic pop)
}

template <std::size_t I = 0,
          typename DpfKey,
          class OutputBuffer,
          class IntervalMemoizer,
          typename InputT>
DPF_UNROLL_LOOPS
auto eval_interval(const DpfKey & dpf, InputT from, InputT to,
    OutputBuffer & outbuf, IntervalMemoizer & memoizer)
{
    assert_not_wildcard<I>(dpf);

    std::size_t from_node = utils::quotient_floor(from, (InputT)DpfKey::outputs_per_leaf), to_node = utils::quotient_ceiling(to, (InputT)DpfKey::outputs_per_leaf);

    eval_interval_interior(dpf, from_node, to_node, memoizer);
    eval_interval_exterior<I>(dpf, from_node, to_node, outbuf, memoizer);

    return dpf::clipped_iterable<OutputBuffer>(outbuf, from % DpfKey::outputs_per_leaf,
        DpfKey::outputs_per_leaf - (to % DpfKey::outputs_per_leaf));
}

struct interval_memoizer
{
  public:
    explicit interval_memoizer()
      : level_index{0}
    { }

    virtual void reset() = 0;

    // level -1 should access the root
    // level goes up to (but not including) tree_depth
    // TODO: figure out best way to force an operator[]
    // virtual auto operator[](std::size_t) = 0;

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
struct basic_interval_memoizer : public interval_memoizer
{
  public:
    using unique_ptr = typename Allocator::unique_ptr;

    explicit basic_interval_memoizer(std::size_t output_len, std::size_t depth, Allocator alloc = Allocator{})
      : pivot{(dpf::utils::msb_of_v<std::size_t> >> clz(output_len))/2},
        tree_depth{depth},
        length{std::max(3*pivot, output_len)},
        buf{alloc.allocate_unique_ptr(length * sizeof(NodeT))}
    {
        // TODO: is this needed / correct
        // if (pivot < 32)  // check alignment (pivot is 0 or some power of 2)
        // {
        //     throw std::domain_error("output_len must be at least 64");
        // }
    }

    void reset()
    {
        level_index = 0;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    auto operator[](std::size_t level) const noexcept
    {
        auto b = !((tree_depth ^ level) & 1);
        return Allocator::assume_aligned(&buf[b*pivot]);
    }

  private:
    static constexpr auto clz = utils::countl_zero<std::size_t>{};
    const std::size_t pivot;
    const std::size_t tree_depth;
  public:
    const std::size_t length;
    unique_ptr buf;
};

template <typename NodeT,
          typename Allocator = detail::aligned_allocator<NodeT>>
struct full_tree_interval_memoizer : public interval_memoizer
{
  public:
    using unique_ptr = typename Allocator::unique_ptr;

    explicit full_tree_interval_memoizer(std::size_t output_len, std::size_t depth, Allocator alloc = Allocator{})
      : tree_depth{depth},
        output_length{output_len},
        idxs{new std::size_t[depth+1]},
        length{calc_length()},
        buf{alloc.allocate_unique_ptr(length * sizeof(NodeT))}
    { }

    void reset()
    {
        level_index = 0;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    auto operator[](std::size_t level) const noexcept
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

        for (std::size_t level = tree_depth-1; level > 0; --level)
        {
            len = std::min(len / 2 + 1, static_cast<std::size_t>(1) << (level));
            ret += len;
            idxs[level+1] = len;
        }

        for (std::size_t i = 0; i < tree_depth; ++i)
        {
            idxs[i+1] = idxs[i] + idxs[i+1];
        }

        return ret;
    }

    const std::size_t tree_depth;
    const std::size_t output_length;
    std::unique_ptr<std::size_t[]> idxs;
  public:
    const std::size_t length;
    unique_ptr buf;
};

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          typename OutputBuffer>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to, OutputBuffer & outbuf)
{
    auto memoizer = make_interval_memoizer(dpf, from, to);
    return eval_interval<I>(dpf, from, to, outbuf, memoizer);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to)
{
    auto memoizer = make_interval_memoizer(dpf, from, to);
    auto outbuf = make_output_buffer_for_interval(dpf, from, to);
    auto clipped_iterable = eval_interval<I>(dpf, from, to, outbuf, memoizer);
    return std::make_tuple(std::move(outbuf), std::move(clipped_iterable));
}

template <std::size_t I = 0,
          typename DpfKey,
          class OutputBuffer,
          class IntervalMemoizer>
auto eval_full(const DpfKey & dpf, OutputBuffer & outbuf, IntervalMemoizer & memoizer)
{
    using input_t = typename DpfKey::input_type;
    return eval_interval<I>(dpf, input_t(0), std::numeric_limits<input_t>::max(), outbuf, memoizer);
}

template <std::size_t I = 0,
          typename DpfKey,
          class OutputBuffer>
auto eval_full(const DpfKey & dpf, OutputBuffer & outbuf)
{
    using input_t = typename DpfKey::input_type;
    auto memoizer = make_interval_memoizer(dpf, input_t(0), std::numeric_limits<input_t>::max());
    return eval_interval<I>(dpf, input_t(0), std::numeric_limits<input_t>::max(), outbuf, memoizer);
}

template <std::size_t I = 0,
          typename DpfKey>
auto eval_full(const DpfKey & dpf)
{
    using input_t = typename DpfKey::input_type;
    return eval_interval<I>(dpf, input_t(0), std::numeric_limits<input_t>::max());
}

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

    return MemoizerT(nodes_in_interval, DpfKey::tree_depth);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_INTERVAL_HPP__
