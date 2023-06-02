/// @file dpf/eval_interval.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_EVAL_INTERVAL_HPP__
#define LIBDPF_INCLUDE_DPF_EVAL_INTERVAL_HPP__

#include <portable-snippets/builtin/builtin.h>
#include <hedley/hedley.h>

#include <cstddef>
#include <cstring>
#include <type_traits>
#include <iterator>
#include <utility>

#include "dpf/dpf_key.hpp"
#include "dpf/eval_common.hpp"
#include "dpf/output_buffer.hpp"
#include "dpf/interval_memoizer.hpp"
#include "dpf/subinterval_iterable.hpp"
#include "dpf/eval_interval.hpp"

namespace dpf
{

namespace internal
{

template <typename DpfKey,
          typename IntervalMemoizer,
          typename IntegralT = typename DpfKey::integral_type>
inline auto eval_interval_interior(const DpfKey & dpf, IntegralT from_node,
    IntegralT to_node, IntervalMemoizer & memoizer,  // NOLINT(runtime/references)
    std::size_t to_level = DpfKey::depth)
{
    using dpf_type = DpfKey;
    using integral_type = typename DpfKey::integral_type;
    using node_type = typename DpfKey::interior_node;

    // level_index represents the current level being built
    // level_index = 0 => root
    // level_index = depth => last layer of interior nodes
    std::size_t level_index = memoizer.assign_interval(dpf, from_node, to_node);
    std::size_t nodes_at_level = memoizer.get_nodes_at_level();
    integral_type mask = utils::get_node_mask<dpf_type>(dpf.msb_mask, level_index);

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

template <std::size_t I,
          typename DpfKey,
          typename OutputBuffer,
          typename IntervalMemoizer,
          typename IntegralT = typename DpfKey::integral_type>
inline auto eval_interval_exterior(const DpfKey & dpf, IntegralT from_node,
    IntegralT to_node, OutputBuffer && outbuf, IntervalMemoizer && memoizer)
{
    assert_not_wildcard<I>(dpf);

    using dpf_type = DpfKey;
    using output_type = typename DpfKey::concrete_output_type<I>;
    using exterior_node_type = typename DpfKey::exterior_node;

    std::size_t nodes_in_interval = to_node - from_node;

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    auto cw = dpf.template leaf<I>();
    auto rawbuf = reinterpret_cast<exterior_node_type *>(utils::data(outbuf));
    DPF_UNROLL_LOOP
    for (std::size_t j = 0; j < nodes_in_interval; ++j)
    {
        auto leaf = dpf_type::template traverse_exterior<I>(memoizer[dpf_type::depth][j],
            get_if_lo_bit(cw, memoizer[dpf_type::depth][j]));
        if constexpr (std::is_same_v<output_type, dpf::bit>)
        {
            std::memcpy(&rawbuf[j], &leaf, sizeof(leaf));
        }
        else
        {
            std::memcpy(&outbuf[j*dpf_type::outputs_per_leaf], &leaf, sizeof(output_type)*dpf_type::outputs_per_leaf);
        }
    }
HEDLEY_PRAGMA(GCC diagnostic pop)
}

template <std::size_t ...Is,
          typename DpfKey,
          typename InputT,
          typename OutputBuffers,
          typename IntervalMemoizer,
          std::size_t ...IIs>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to,
    OutputBuffers && outbufs, IntervalMemoizer && memoizer,
    std::index_sequence<IIs...>)
{
    using dpf_type = DpfKey;
    using integral_type = typename DpfKey::integral_type;
    constexpr auto mod = utils::mod_pow_2<InputT>{};

    integral_type from_node = utils::get_from_node<dpf_type>(from),
        to_node = utils::get_to_node<dpf_type>(to);
    std::size_t nodes_in_interval = utils::get_nodes_in_interval_impl(from_node, to_node);

    internal::eval_interval_interior(dpf, from_node, to_node, memoizer);
    (internal::eval_interval_exterior<Is>(dpf, from_node, to_node, utils::get<IIs>(outbufs), memoizer), ...);

    return utils::make_tuple(
        subinterval_iterable(std::begin(utils::get<IIs>(outbufs)),
        nodes_in_interval*dpf_type::outputs_per_leaf,
        mod(from, dpf_type::lg_outputs_per_leaf),
        dpf_type::outputs_per_leaf - mod(to, dpf_type::lg_outputs_per_leaf))...);
}

}  // namespace internal

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename InputT,
          typename OutputBuffers,
          typename IntervalMemoizer = dpf::basic_interval_memoizer<DpfKey>>
HEDLEY_ALWAYS_INLINE
auto eval_interval(const DpfKey & dpf, InputT from, InputT to,
    OutputBuffers & outbufs, IntervalMemoizer && memoizer)  // NOLINT(runtime/references)
{
    assert_not_wildcard<I, Is...>(dpf);

    return internal::eval_interval<I, Is...>(dpf, from, to, outbufs, memoizer, std::make_index_sequence<1+sizeof...(Is)>());
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename InputT,
          typename OutputBuffers,
          std::enable_if_t<!std::is_base_of_v<dpf::interval_memoizer_base<DpfKey>,
              std::remove_reference_t<OutputBuffers>>, bool> = true>
HEDLEY_ALWAYS_INLINE
auto eval_interval(const DpfKey & dpf, InputT from, InputT to,
    OutputBuffers & outbufs)  // NOLINT(runtime/references)
{
    return eval_interval<I, Is...>(dpf, from, to, outbufs,
        dpf::make_basic_interval_memoizer<DpfKey>(from, to));
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename InputT,
          typename IntervalMemoizer,
          std::enable_if_t<std::is_base_of_v<dpf::interval_memoizer_base<DpfKey>,
              std::remove_reference_t<IntervalMemoizer>>, bool> = true>
HEDLEY_ALWAYS_INLINE
auto eval_interval(const DpfKey & dpf, InputT from, InputT to,
    IntervalMemoizer && memoizer)
{
    auto outbufs = utils::make_tuple(
        make_output_buffer_for_interval<I>(dpf, from, to),
        make_output_buffer_for_interval<Is>(dpf, from, to)...);

    // moving `outbufs` is allowed as the `outbufs` are `std::vectors`
    //   the underlying data remains on the heap
    //   and thus the data the iterable refers to is still valid
    auto iterable = eval_interval<I, Is...>(dpf, from, to, outbufs, memoizer);
    return std::make_pair(std::move(outbufs), std::move(iterable));
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename InputT>
HEDLEY_ALWAYS_INLINE
auto eval_interval(const DpfKey & dpf, InputT from, InputT to)
{
    return eval_interval<I, Is...>(dpf, from, to,
        dpf::make_basic_interval_memoizer<DpfKey>(from, to));
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_INTERVAL_HPP__
