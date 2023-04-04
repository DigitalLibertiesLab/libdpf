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
#include "dpf/subinterval_iterable.hpp"

namespace dpf
{

namespace internal
{

template <typename DpfKey,
          class IntervalMemoizer,
          typename IntegralT = typename DpfKey::integral_type>
inline auto eval_interval_interior(const DpfKey & dpf, IntegralT from_node, IntegralT to_node,
    IntervalMemoizer & memoizer, std::size_t to_level = DpfKey::depth)
{
    using dpf_type = DpfKey;
    using input_type = typename DpfKey::input_type;
    // using integral_type = typename DpfKey::integral_type;
    using node_type = typename DpfKey::interior_node;

    // level_index represents the current level being built
    // level_index = 0 => root
    // level_index = depth => last layer of interior nodes
    std::size_t level_index = memoizer.assign_interval(dpf, from_node, to_node);
    input_type mask = dpf.msb_mask >> (level_index-1 + dpf_type::lg_outputs_per_leaf);
    std::size_t nodes_at_level = memoizer.get_nodes_at_level();

    for (; level_index <= to_level; level_index = memoizer.advance_level(), nodes_at_level = memoizer.get_nodes_at_level(), mask>>=1)
    {
        std::size_t i = 0, j = 0;
        bool from_offset = mask & from_node,
             to_offset = from_offset ^ (nodes_at_level & 1);
        const node_type cw[2] = {
            set_lo_bit(dpf.correction_words[level_index-1], dpf.correction_advice[level_index-1]&1),
            set_lo_bit(dpf.correction_words[level_index-1], (dpf.correction_advice[level_index-1]>>1)&1)
        };

        // process node which only requires a right traversal
        if (from_offset == true)
        {
            memoizer[level_index][i++] = dpf_type::traverse_interior(memoizer[level_index-1][j++], cw[1], 1);
        }
        // process all nodes which require both a left traversal and a right traversal
        DPF_UNROLL_LOOP
        for (; i < nodes_at_level - to_offset;)
        {
            auto cur_node = memoizer[level_index-1][j++];
            memoizer[level_index][i++] = dpf_type::traverse_interior(cur_node, cw[0], 0);
            memoizer[level_index][i++] = dpf_type::traverse_interior(cur_node, cw[1], 1);
        }
        // process node which only requires a left traversal
        if (to_offset == true)
        {
            memoizer[level_index][i] = dpf_type::traverse_interior(memoizer[level_index-1][j], cw[0], 0);
        }
    }
}

template <std::size_t I = 0,
          typename DpfKey,
          class OutputBuffer,
          class IntervalMemoizer,
          typename IntegralT = typename DpfKey::integral_type>
inline auto eval_interval_exterior(const DpfKey & dpf, IntegralT from_node, IntegralT to_node,
    OutputBuffer & outbuf, IntervalMemoizer & memoizer)
{
    assert_not_wildcard<I>(dpf);

    using dpf_type = DpfKey;
    using output_type = std::tuple_element_t<I, typename DpfKey::concrete_outputs_tuple>;
    using exterior_node_type = typename DpfKey::exterior_node;

    std::size_t nodes_in_interval = to_node - from_node;

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    auto cw = dpf.template exterior_cw<I>();
    auto rawbuf = reinterpret_cast<exterior_node_type *>(utils::data(outbuf));
    DPF_UNROLL_LOOP
    for (std::size_t j = 0, k = 0; j < nodes_in_interval; ++j,
        k += block_length_of_leaf_v<output_type, exterior_node_type>)
    {
        auto leaf = dpf_type::template traverse_exterior<I>(memoizer[dpf_type::depth][j],
            get_if_lo_bit(cw, memoizer[dpf_type::depth][j]));
        std::memcpy(&rawbuf[k], &leaf, sizeof(leaf));
    }
HEDLEY_PRAGMA(GCC diagnostic pop)
}

}  // namespace internal

template <std::size_t I = 0,
          typename DpfKey,
          class OutputBuffer,
          class IntervalMemoizer,
          typename InputT>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to,
    OutputBuffer && outbuf, IntervalMemoizer & memoizer)
{
    assert_not_wildcard<I>(dpf);

    using dpf_type = DpfKey;
    using integral_type = typename DpfKey::integral_type;
    using output_type = std::tuple_element_t<I, typename DpfKey::concrete_outputs_tuple>;

    integral_type from_node = utils::get_from_node<dpf_type>(from),
        to_node = utils::get_to_node<dpf_type>(to);
    std::size_t nodes_in_interval = utils::get_nodes_in_interval_impl(from_node, to_node);

    internal::eval_interval_interior(dpf, from_node, to_node, memoizer);
    internal::eval_interval_exterior<I>(dpf, from_node, to_node, outbuf, memoizer);

    return subinterval_iterable<output_type>(utils::data(outbuf), nodes_in_interval*dpf_type::outputs_per_leaf, from % dpf_type::outputs_per_leaf,
        dpf_type::outputs_per_leaf - (to % dpf_type::outputs_per_leaf));
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          typename OutputBuffer>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to, OutputBuffer && outbuf)
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
    auto outbuf = make_output_buffer_for_interval<I>(dpf, from, to);
    auto iterable = eval_interval<I>(dpf, from, to, outbuf, memoizer);
    return std::make_tuple(std::move(outbuf), std::move(iterable));
}

template <std::size_t I = 0,
          typename DpfKey,
          class OutputBuffer,
          class IntervalMemoizer>
auto eval_full(const DpfKey & dpf, OutputBuffer && outbuf, IntervalMemoizer & memoizer)
{
    using input_type = typename DpfKey::input_type;
    return eval_interval<I>(dpf, input_type(0), std::numeric_limits<input_type>::max(), outbuf, memoizer);
}

template <std::size_t I = 0,
          typename DpfKey,
          class OutputBuffer>
auto eval_full(const DpfKey & dpf, OutputBuffer && outbuf)
{
    using input_type = typename DpfKey::input_type;
    auto memoizer = make_basic_interval_memoizer(dpf, input_type(0), std::numeric_limits<input_type>::max());
    return eval_interval<I>(dpf, input_type(0), std::numeric_limits<input_type>::max(), outbuf, memoizer);
}

template <std::size_t I = 0,
          typename DpfKey>
auto eval_full(const DpfKey & dpf)
{
    using input_type = typename DpfKey::input_type;
    return eval_interval<I>(dpf, input_type(0), std::numeric_limits<input_type>::max());
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_INTERVAL_HPP__
