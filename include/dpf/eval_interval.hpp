/// @file dpf/eval_interval.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
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
#include "dpf/wildcard_input_iterable.hpp"

namespace dpf
{

namespace internal
{

template <typename DpfKey,
          typename IntervalMemoizer,
          typename IntegralT = typename DpfKey::integral_type>
inline auto eval_interval_interior(const DpfKey & dpf, IntegralT from_node,
    IntegralT to_node, IntervalMemoizer & memoizer,  // NOLINT(runtime/references)
    bool wrap, std::size_t to_level = DpfKey::depth)
{
    using dpf_type = DpfKey;
    using integral_type = typename DpfKey::integral_type;
    using node_type = typename DpfKey::interior_node;

    // perform normal eval_full
    if (wrap == true && (from_node <= to_node))
    {
        from_node = 0;
        to_node = integral_type{1} << dpf_type::depth;
        wrap = false;
    }

    // level_index represents the current level being built
    // level_index = 0 => root
    // level_index = depth => last layer of interior nodes
    std::size_t level_index = memoizer.assign_interval(dpf, from_node, to_node, wrap);
    std::size_t nodes_at_level = memoizer.get_nodes_at_level();
    integral_type mask = utils::get_node_mask<dpf_type>(dpf.msb_mask, level_index);
    integral_type mod = integral_type{1} << (level_index-1);

    for (; level_index <= to_level; level_index = memoizer.advance_level(), nodes_at_level = memoizer.get_nodes_at_level(), mask>>=1, mod<<=1)
    {
        std::size_t i = 0, j = 0;
        bool from_offset = mask & from_node,
             to_offset = from_offset ^ (nodes_at_level & 1);
        const node_type cw[2] = {
            dpf.correction_word(level_index-1, 0),
            dpf.correction_word(level_index-1, 1)
        };

        // process node which only requires a right traversal
        if (from_offset == true)
        {
            memoizer[level_index][i++] = dpf_type::traverse_interior(memoizer[level_index-1][j++%mod], cw[1], 1);
        }
        // process all nodes which require both a left traversal and a right traversal
        DPF_UNROLL_LOOP
        for (; i < nodes_at_level - to_offset;)
        {
            auto cur_node = memoizer[level_index-1][j++%mod];
            memoizer[level_index][i++] = dpf_type::traverse_interior(cur_node, cw[0], 0);
            memoizer[level_index][i++] = dpf_type::traverse_interior(cur_node, cw[1], 1);
        }
        // process node which only requires a left traversal
        if (to_offset == true)
        {
            memoizer[level_index][i] = dpf_type::traverse_interior(memoizer[level_index-1][j%mod], cw[0], 0);
        }
    }
}

template <std::size_t I,
          typename DpfKey,
          typename OutputBuffer,
          typename IntervalMemoizer,
          typename IntegralT = typename DpfKey::integral_type>
inline auto eval_interval_exterior(const DpfKey & dpf, IntegralT from_node,
    IntegralT to_node, OutputBuffer && outbuf, IntervalMemoizer && memoizer, bool wrap)
{
    assert_not_wildcard_output<I>(dpf);

    using dpf_type = DpfKey;
    using integral_type = typename DpfKey::integral_type;
    using output_type = typename DpfKey::concrete_output_type<I>;
    using exterior_node_type = typename DpfKey::exterior_node;
    constexpr integral_type mod = integral_type{1} << dpf_type::depth;

    std::size_t nodes_in_interval = memoizer.get_nodes_at_level(dpf_type::depth, from_node, to_node, wrap);

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    auto rawbuf = reinterpret_cast<exterior_node_type *>(utils::data(outbuf));
    DPF_UNROLL_LOOP
    for (std::size_t j = 0, k = memoizer.get_exterior_start(from_node); j < nodes_in_interval; ++j, ++k)
    {
        auto leaf = dpf.template traverse_exterior<I>(memoizer[dpf_type::depth][k%mod]);
        if constexpr (std::is_same_v<output_type, dpf::bit>)
        {
            std::memcpy(&rawbuf[j], &leaf, sizeof(leaf));
        }
        else
        {
            std::memcpy(&outbuf[j*dpf_type::outputs_per_leaf], &leaf, sizeof(output_type)*dpf_type::outputs_per_leaf);
        }
    }

    if (wrap == true && from_node < to_node)
    {
        rawbuf[nodes_in_interval] = rawbuf[0];
    }
HEDLEY_PRAGMA(GCC diagnostic pop)
}

template <std::size_t ...Is,
          typename DpfKey,
          typename InputT,
          typename OutputBuffers,
          typename IntervalMemoizer,
          std::size_t ...IIs>
auto eval_interval_impl(const DpfKey & dpf, InputT from, InputT to,
    OutputBuffers && outbufs, IntervalMemoizer && memoizer,
    std::index_sequence<IIs...>)
{
    using dpf_type = DpfKey;
    using integral_type = typename DpfKey::integral_type;

    bool wrap = to < from;

    utils::flip_msb_if_signed_integral(from);
    utils::flip_msb_if_signed_integral(to);

    integral_type from_node = utils::get_from_node<dpf_type>(from),
        to_node = utils::get_to_node<dpf_type>(to);

    internal::eval_interval_interior(dpf, from_node, to_node, memoizer, wrap);
    (internal::eval_interval_exterior<Is>(dpf, from_node, to_node, utils::get<IIs>(outbufs), memoizer, wrap), ...);
}

template <std::size_t ...Is,
          typename DpfKey,
          typename InputT,
          typename OutputBuffers,
          typename IntervalMemoizer,
          std::size_t ...IIs>
auto eval_interval_wildcard_input(const DpfKey & dpf, InputT from, InputT to,
    OutputBuffers && outbufs, IntervalMemoizer && memoizer,
    std::index_sequence<IIs...>)
{
    using dpf_type = DpfKey;
    using integral_type = typename DpfKey::integral_type;
    constexpr auto mod_pow_2 = utils::mod_pow_2<InputT>{};
    constexpr auto to_integral_t = utils::to_integral_type<InputT>{};

    using input_type = typename DpfKey::input_type;

    auto min = std::numeric_limits<input_type>::min(),
         max = std::numeric_limits<input_type>::max();

    eval_interval_impl<Is...>(dpf, min, max, outbufs, memoizer, std::make_index_sequence<sizeof...(Is)>());

    return utils::make_tuple(wildcard_input_iterable(dpf, std::begin(utils::get<IIs>(outbufs)), std::next(std::begin(utils::get<IIs>(outbufs)), integral_type{1} << utils::bitlength_of_v<InputT>), utils::get_leafnodes_in_output_interval<dpf_type>(from, to), to_integral_t(from), to_integral_t(to), mod_pow_2(from, dpf_type::lg_outputs_per_leaf), dpf_type::outputs_per_leaf)...);
}

template <typename DpfKey,
          typename InputT,
          typename IntervalMemoizer>
auto eval_interval_wildcard_output(const DpfKey & dpf, InputT from, InputT to,
    IntervalMemoizer & memoizer)
{
    using dpf_type = DpfKey;
    using integral_type = typename DpfKey::integral_type;

    auto tfrom = dpf.offset_x(from),
         tto = dpf.offset_x(to);

    bool wrap = tto < tfrom;

    utils::flip_msb_if_signed_integral(tfrom);
    utils::flip_msb_if_signed_integral(tto);

    integral_type from_node = utils::get_from_node<dpf_type>(tfrom),
        to_node = utils::get_to_node<dpf_type>(tto);

    internal::eval_interval_interior(dpf, from_node, to_node, memoizer, wrap);
}

template <typename DpfKey,
          typename IntervalMemoizer>
auto eval_interval_wildcard_input_output(const DpfKey & dpf, IntervalMemoizer & memoizer)
{
    using integral_type = typename DpfKey::integral_type;
    constexpr auto last_node = integral_type{1} << dpf.depth;

    internal::eval_interval_interior(dpf, integral_type{0}, last_node, memoizer, false);
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
    constexpr auto mod_pow_2 = utils::mod_pow_2<InputT>{};
    constexpr auto to_integral_t = utils::to_integral_type<InputT>{};

    auto tfrom = dpf.offset_x(from),
         tto = dpf.offset_x(to);

    eval_interval_impl<Is...>(dpf, tfrom, tto, outbufs, memoizer, std::make_index_sequence<sizeof...(Is)>());

    // TODO: where to put this
    utils::flip_msb_if_signed_integral(from);
    utils::flip_msb_if_signed_integral(to);

    return utils::make_tuple(subinterval_iterable(std::begin(utils::get<IIs>(outbufs)), utils::size(utils::get<IIs>(outbufs)), to_integral_t(from), to_integral_t(to), mod_pow_2(tfrom, dpf_type::lg_outputs_per_leaf), dpf_type::outputs_per_leaf)...);
}

}  // namespace internal

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename InputT,
          typename OutputBuffers,
          typename IntervalMemoizer>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to,
    OutputBuffers & outbufs, IntervalMemoizer && memoizer, wildcard_input_tag_)
{
    assert_not_wildcard_output<I, Is...>(dpf);

    return internal::eval_interval_wildcard_input<I, Is...>(dpf, from, to, outbufs, memoizer, std::make_index_sequence<1+sizeof...(Is)>());
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename InputT,
          typename OutputBuffers,
          std::enable_if_t<!std::is_base_of_v<dpf::interval_memoizer_base<DpfKey>,
              std::decay_t<OutputBuffers>>, bool> = true>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to, OutputBuffers & outbufs, wildcard_input_tag_)
{
    using input_type = typename DpfKey::input_type;

    return eval_interval<I, Is...>(dpf, from, to, outbufs,
        dpf::make_basic_interval_memoizer<DpfKey>(
            std::numeric_limits<input_type>::min(),
            std::numeric_limits<input_type>::max()),
    wildcard_input_tag);
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename InputT,
          typename IntervalMemoizer,
          std::enable_if_t<std::is_base_of_v<dpf::interval_memoizer_base<DpfKey>,
              std::decay_t<IntervalMemoizer>>, bool> = true>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to,
    IntervalMemoizer && memoizer, wildcard_input_tag_)
{
    using input_type = typename DpfKey::input_type;

    auto min = std::numeric_limits<input_type>::min(),
         max = std::numeric_limits<input_type>::max();
    auto outbufs = utils::make_tuple(
        make_output_buffer_for_interval<I>(dpf, min, max),
        make_output_buffer_for_interval<Is>(dpf, min, max)...);

    auto iterable = eval_interval<I, Is...>(dpf, from, to, outbufs, memoizer, wildcard_input_tag);
    return std::make_pair(std::move(outbufs), std::move(iterable));
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename InputT>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to, wildcard_input_tag_)
{
    using input_type = typename DpfKey::input_type;

    return eval_interval<I, Is...>(dpf, from, to,
    dpf::make_basic_interval_memoizer<DpfKey>(
        std::numeric_limits<input_type>::min(),
        std::numeric_limits<input_type>::max()),
    wildcard_input_tag);
}

template <typename DpfKey,
          typename InputT,
          typename IntervalMemoizer>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to,
    IntervalMemoizer & memoizer, wildcard_output_tag_)
{
    assert_not_wildcard_input(dpf);

    internal::eval_interval_wildcard_output(dpf, from, to, memoizer);
}

template <typename DpfKey,
          typename InputT>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to, wildcard_output_tag_)
{
    auto memoizer = dpf::make_basic_interval_memoizer<DpfKey>(from, to);
    eval_interval(dpf, from, to, memoizer, wildcard_output_tag);
    return memoizer;
}

template <typename DpfKey,
          typename IntervalMemoizer>
auto eval_interval(const DpfKey & dpf, IntervalMemoizer & memoizer, wildcard_input_output_tag_)
{
    internal::eval_interval_wildcard_input_output(dpf, memoizer);
}

template <typename DpfKey>
auto eval_interval(const DpfKey & dpf, wildcard_input_output_tag_)
{
    using input_type = typename DpfKey::input_type;

    auto memoizer = dpf::make_basic_interval_memoizer<DpfKey>(
        std::numeric_limits<input_type>::min(),
        std::numeric_limits<input_type>::max());
    eval_interval(dpf, memoizer, wildcard_input_output_tag);
    return memoizer;
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename InputT,
          typename OutputBuffers,
          typename IntervalMemoizer>
HEDLEY_ALWAYS_INLINE
auto eval_interval(const DpfKey & dpf, InputT from, InputT to,
    OutputBuffers && outbufs, IntervalMemoizer && memoizer)  // NOLINT(runtime/references)
{
    assert_not_wildcard_input(dpf);
    assert_not_wildcard_output<I, Is...>(dpf);

    return internal::eval_interval<I, Is...>(dpf, from, to, outbufs, memoizer, std::make_index_sequence<1+sizeof...(Is)>());
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename InputT,
          typename OutputBuffers,
          std::enable_if_t<!std::is_base_of_v<dpf::interval_memoizer_base<DpfKey>,
              std::decay_t<OutputBuffers>>, bool> = true>
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
              std::decay_t<IntervalMemoizer>>, bool> = true>
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
