/// @file dpf/eval_full.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_EVAL_FULL_HPP__
#define LIBDPF_INCLUDE_DPF_EVAL_FULL_HPP__

#include <portable-snippets/builtin/builtin.h>
#include <hedley/hedley.h>

#include <cstddef>
#include <type_traits>
#include <utility>
#include <limits>

#include "dpf/dpf_key.hpp"
#include "dpf/eval_common.hpp"
#include "dpf/output_buffer.hpp"
#include "dpf/interval_memoizer.hpp"
#include "dpf/rotation_iterable.hpp"

namespace dpf
{

namespace internal
{

template <std::size_t ...Is,
          typename DpfKey,
          typename OutputBuffers,
          typename IntervalMemoizer,
          std::size_t ...IIs>
auto eval_full_wildcard_input(const DpfKey & dpf, OutputBuffers && outbufs,
    IntervalMemoizer && memoizer, std::index_sequence<IIs...>)
{
    using input_type = typename DpfKey::input_type;

    return eval_interval_wildcard_input<Is...>(dpf,
        std::numeric_limits<input_type>::min(),
        std::numeric_limits<input_type>::max(),
        outbufs, memoizer, std::make_index_sequence<sizeof...(Is)>());
}

template <typename DpfKey,
          typename IntervalMemoizer>
auto eval_full_wildcard_output(const DpfKey & dpf, IntervalMemoizer & memoizer)
{
    using input_type = typename DpfKey::input_type;

    eval_interval_wildcard_output(dpf,
        std::numeric_limits<input_type>::min(),
        std::numeric_limits<input_type>::max(),
        memoizer);
}

template <typename DpfKey,
          typename IntervalMemoizer>
auto eval_full_wildcard_input_output(const DpfKey & dpf, IntervalMemoizer & memoizer)
{
    internal::eval_interval_wildcard_input_output(dpf, memoizer);
}

template <std::size_t ...Is,
          typename DpfKey,
          typename OutputBuffers,
          typename IntervalMemoizer,
          std::size_t ...IIs>
auto eval_full(const DpfKey & dpf, OutputBuffers && outbufs,
    IntervalMemoizer && memoizer, std::index_sequence<IIs...>)
{
    using input_type = typename DpfKey::input_type;

    assert_not_wildcard_input(dpf);

    return dpf::internal::eval_interval<Is...>(dpf,
            std::numeric_limits<input_type>::min(),
            std::numeric_limits<input_type>::max(),
            outbufs, memoizer, std::make_index_sequence<sizeof...(Is)>());
}

}  // namespace internal

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename OutputBuffers,
          typename IntervalMemoizer>
auto eval_full(const DpfKey & dpf, OutputBuffers && outbufs,
    IntervalMemoizer && memoizer, wildcard_input_tag_)
{
    assert_not_wildcard_output<I, Is...>(dpf);

    return internal::eval_full_wildcard_input<I, Is...>(dpf, outbufs, memoizer, std::make_index_sequence<1+sizeof...(Is)>());
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename OutputBuffers,
          std::enable_if_t<!std::is_base_of_v<dpf::interval_memoizer_base<DpfKey>,
              std::decay_t<OutputBuffers>>, bool> = true>
auto eval_full(const DpfKey & dpf, OutputBuffers & outbufs, wildcard_input_tag_)
{
    using input_type = typename DpfKey::input_type;

    return eval_full<I, Is...>(dpf, outbufs,
        dpf::make_basic_interval_memoizer<DpfKey>(
            std::numeric_limits<input_type>::min(),
            std::numeric_limits<input_type>::max()),
    wildcard_input_tag);
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename IntervalMemoizer,
          std::enable_if_t<std::is_base_of_v<dpf::interval_memoizer_base<DpfKey>,
              std::decay_t<IntervalMemoizer>>, bool> = true>
auto eval_full(const DpfKey & dpf, IntervalMemoizer && memoizer, wildcard_input_tag_)
{
    using input_type = typename DpfKey::input_type;

    auto min = std::numeric_limits<input_type>::min(),
         max = std::numeric_limits<input_type>::max();
    auto outbufs = utils::make_tuple(
        make_output_buffer_for_interval<I>(dpf, min, max),
        make_output_buffer_for_interval<Is>(dpf, min, max)...);

    auto iterable = eval_full<I, Is...>(dpf, outbufs, memoizer, wildcard_input_tag);
    return std::make_pair(std::move(outbufs), std::move(iterable));
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey>
auto eval_full(const DpfKey & dpf, wildcard_input_tag_)
{
    using input_type = typename DpfKey::input_type;

    return eval_full<I, Is...>(dpf,
    dpf::make_basic_interval_memoizer<DpfKey>(
        std::numeric_limits<input_type>::min(),
        std::numeric_limits<input_type>::max()),
    wildcard_input_tag);
}

template <typename DpfKey,
          typename IntervalMemoizer>
auto eval_full(const DpfKey & dpf, IntervalMemoizer & memoizer, wildcard_output_tag_)
{
    assert_not_wildcard_input(dpf);

    internal::eval_full_wildcard_output(dpf, memoizer);
}

template <typename DpfKey>
auto eval_full(const DpfKey & dpf, wildcard_output_tag_)
{
    using input_type = typename DpfKey::input_type;

    auto memoizer = dpf::make_basic_interval_memoizer<DpfKey>(
        std::numeric_limits<input_type>::min(),
        std::numeric_limits<input_type>::max());
    eval_full(dpf, memoizer, wildcard_output_tag);
    return memoizer;
}

template <typename DpfKey,
          typename IntervalMemoizer>
auto eval_full(const DpfKey & dpf, IntervalMemoizer & memoizer, wildcard_input_output_tag_)
{
    internal::eval_full_wildcard_input_output(dpf, memoizer);
}

template <typename DpfKey>
auto eval_full(const DpfKey & dpf, wildcard_input_output_tag_)
{
    using input_type = typename DpfKey::input_type;

    auto memoizer = dpf::make_basic_interval_memoizer<DpfKey>(
        std::numeric_limits<input_type>::min(),
        std::numeric_limits<input_type>::max());
    eval_full(dpf, memoizer, wildcard_input_output_tag);
    return memoizer;
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename OutputBuffers,
          typename IntervalMemoizer>
HEDLEY_ALWAYS_INLINE
auto eval_full(const DpfKey & dpf, OutputBuffers && outbufs,
    IntervalMemoizer && memoizer)
{
    assert_not_wildcard_output<I, Is...>(dpf);

    return internal::eval_full<I, Is...>(dpf, outbufs, memoizer, std::make_index_sequence<1+sizeof...(Is)>());
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename OutputBuffers,
          std::enable_if_t<!std::is_base_of_v<dpf::interval_memoizer_base<DpfKey>,
              std::decay_t<OutputBuffers>>, bool> = true>
HEDLEY_ALWAYS_INLINE
auto eval_full(const DpfKey & dpf, OutputBuffers & outbufs)  // NOLINT(runtime/references)
{
    using input_type = typename DpfKey::input_type;
    return eval_full<I, Is...>(dpf, outbufs,
        dpf::make_basic_full_memoizer(dpf));
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename IntervalMemoizer,
          std::enable_if_t<std::is_base_of_v<dpf::interval_memoizer_base<DpfKey>,
              std::decay_t<IntervalMemoizer>>, bool> = true>
HEDLEY_ALWAYS_INLINE
auto eval_full(const DpfKey & dpf, IntervalMemoizer && memoizer)
{
    auto outbufs = utils::make_tuple(
        make_output_buffer_for_full<I>(dpf),
        make_output_buffer_for_full<Is>(dpf)...);

    // moving `outbufs` is allowed as the `outbufs` are `std::vectors`
    //   the underlying data remains on the heap
    //   and thus the data the iterable refers to is still valid
    auto iterable = eval_full<I, Is...>(dpf, outbufs, memoizer);
    return std::make_pair(std::move(outbufs), std::move(iterable));
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey>
HEDLEY_ALWAYS_INLINE
auto eval_full(const DpfKey & dpf)
{
    using input_type = typename DpfKey::input_type;
    return eval_full<I, Is...>(dpf,
        dpf::make_basic_full_memoizer(dpf));
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_FULL_HPP__
