/// @file dpf/eval_interval.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
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
#include "dpf/interval_memoizer.hpp"

namespace dpf
{

template <typename DpfKey,
          class IntervalMemoizer>
DPF_UNROLL_LOOPS
inline auto eval_interval_interior(const DpfKey & dpf, std::size_t from_node, std::size_t to_node,
    IntervalMemoizer & memoizer, std::size_t to_level = DpfKey::depth)
{
    using node_t = typename DpfKey::interior_node_t;
    auto nodes_in_interval = to_node - from_node;

    std::size_t level = memoizer.assign_interval(dpf, from_node, to_node);
    typename DpfKey::input_type mask = dpf.msb_mask >> (level + DpfKey::lg_outputs_per_leaf);
    std::size_t nodes_at_level = memoizer.get_nodes_at_level(level-1);

    for (; level < to_level; level = memoizer.advance_level(), mask>>=1)
    {
        std::size_t i = !!(mask & from_node), j = i;
        const node_t cw[2] = {
            set_lo_bit(dpf.interior_cws[level], dpf.correction_advice[level]&1),
            set_lo_bit(dpf.interior_cws[level], (dpf.correction_advice[level]>>1)&1)
        };

        if (i == 1)
        {
            memoizer[level][0] = DpfKey::traverse_interior(memoizer[level-1][0], cw[1], 1);
        }
        for (; j < nodes_at_level-1; ++j)
        {
            memoizer[level][i++] = DpfKey::traverse_interior(memoizer[level-1][j], cw[0], 0);
            memoizer[level][i++] = DpfKey::traverse_interior(memoizer[level-1][j], cw[1], 1);
        }
        nodes_at_level = memoizer.get_nodes_at_level(level);
        memoizer[level][i++] = DpfKey::traverse_interior(memoizer[level-1][j], cw[0], 0);
        if (i < nodes_at_level)
        {
            memoizer[level][i++] = DpfKey::traverse_interior(memoizer[level-1][j], cw[1], 1);
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
        k += block_length_of_leaf_v<output_t, exterior_node_t>)
    {
        auto leaf = DpfKey::template traverse_exterior<I>(memoizer[DpfKey::depth-1][j],
            get_if_lo_bit(cw, memoizer[DpfKey::depth-1][j]));
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

    return clipped_iterable<OutputBuffer>(&outbuf, from % DpfKey::outputs_per_leaf,
        DpfKey::outputs_per_leaf - (to % DpfKey::outputs_per_leaf));
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          typename OutputBuffer>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to, OutputBuffer & outbuf)
{
    auto memoizer = make_basic_interval_memoizer(dpf, from, to);
    return eval_interval<I>(dpf, from, to, outbuf, memoizer);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to)
{
    auto memoizer = make_basic_interval_memoizer(dpf, from, to);
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
    auto memoizer = make_basic_interval_memoizer(dpf, input_t(0), std::numeric_limits<input_t>::max());
    return eval_interval<I>(dpf, input_t(0), std::numeric_limits<input_t>::max(), outbuf, memoizer);
}

template <std::size_t I = 0,
          typename DpfKey>
auto eval_full(const DpfKey & dpf)
{
    using input_t = typename DpfKey::input_type;
    return eval_interval<I>(dpf, input_t(0), std::numeric_limits<input_t>::max());
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_INTERVAL_HPP__
