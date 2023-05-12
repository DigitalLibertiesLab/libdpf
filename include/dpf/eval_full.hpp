/// @file dpf/eval_full.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_EVAL_FULL_HPP__
#define LIBDPF_INCLUDE_DPF_EVAL_FULL_HPP__

#include <functional>
#include <memory>
#include <limits>
#include <tuple>
#include <algorithm>

#include <portable-snippets/builtin/builtin.h>
#include <hedley/hedley.h>

#include "dpf/dpf_key.hpp"
#include "dpf/eval_common.hpp"
#include "dpf/output_buffer.hpp"
#include "dpf/interval_memoizer.hpp"
#include "dpf/subinterval_iterable.hpp"

namespace dpf
{

namespace internal
{

template <std::size_t ...Is,
          typename DpfKey,
          typename OutputBuffers,
          typename IntervalMemoizer,
          std::size_t ...IIs>
auto eval_full(const DpfKey & dpf, OutputBuffers && outbufs,
    IntervalMemoizer && memoizer, std::index_sequence<IIs...>)
{
    using dpf_type = DpfKey;
    using input_type = typename dpf_type::input_type;

    return eval_interval<Is...>(dpf,
            std::numeric_limits<input_type>::min(),
            std::numeric_limits<input_type>::max(), outbufs, memoizer);
}

}  // namespace dpf::internal

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename OutputBuffers,
          typename IntervalMemoizer>
HEDLEY_ALWAYS_INLINE
auto eval_full(const DpfKey & dpf, OutputBuffers && outbufs,
    IntervalMemoizer && memoizer)
{
    assert_not_wildcard<I, Is...>(dpf);

    return internal::eval_full<I, Is...>(dpf, outbufs, memoizer, std::make_index_sequence<1+sizeof...(Is)>());
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename OutputBuffers,
          std::enable_if_t<!std::is_base_of_v<dpf::interval_memoizer_base<DpfKey>,
              std::remove_reference_t<OutputBuffers>>, bool> = true>
HEDLEY_ALWAYS_INLINE
auto eval_full(const DpfKey & dpf,
    OutputBuffers & outbufs)
{
    using input_type = typename DpfKey::input_type;
    return eval_full<I, Is...>(dpf, outbufs,
        dpf::make_basic_interval_memoizer(dpf,
            std::numeric_limits<input_type>::min(),
            std::numeric_limits<input_type>::max()));
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename IntervalMemoizer,
          std::enable_if_t<std::is_base_of_v<dpf::interval_memoizer_base<DpfKey>,
              std::remove_reference_t<IntervalMemoizer>>, bool> = true>
HEDLEY_ALWAYS_INLINE
auto eval_full(const DpfKey & dpf,
    IntervalMemoizer && memoizer)
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
        dpf::make_basic_interval_memoizer(dpf,
            std::numeric_limits<input_type>::min(),
            std::numeric_limits<input_type>::max()));
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_FULL_HPP__
