/// @file dpf/eval_full.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

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
auto eval_full(const DpfKey & dpf, OutputBuffers & outbufs,
    IntervalMemoizer && memoizer, std::index_sequence<IIs...>)
{
    using dpf_type = DpfKey;
    using input_type = typename dpf_type::input_type;

    if (memoizer.max_size() == 0)
    {
        std::size_t nodes_in_interval= utils::get_nodes_in_interval<dpf_type>(
            input_type(0), std::numeric_limits<input_type>::max());
        memoizer.initialize(nodes_in_interval);
    }

    return eval_interval<Is...>(dpf, input_type(0),
            std::numeric_limits<input_type>::max(), outbufs, memoizer);
}

}  // namespace dpf::internal

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename OutputBuffers,
          typename IntervalMemoizer = dpf::basic_interval_memoizer<DpfKey>,
          std::enable_if_t<!std::is_base_of_v<dpf::interval_memoizer_base<DpfKey>, OutputBuffers>, bool> = true>
HEDLEY_ALWAYS_INLINE
auto eval_full(const DpfKey & dpf, OutputBuffers & outbufs,
    IntervalMemoizer && memoizer = IntervalMemoizer{})
{
    assert_not_wildcard<I, Is...>(dpf);

    return utils::remove_tuple_if_trivial(
        internal::eval_full<I, Is...>(dpf, outbufs, memoizer, std::make_index_sequence<1+sizeof...(Is)>()));
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename IntervalMemoizer = dpf::basic_interval_memoizer<DpfKey>,
          std::enable_if_t<std::is_base_of_v<dpf::interval_memoizer_base<DpfKey>, IntervalMemoizer>, bool> = true>
HEDLEY_ALWAYS_INLINE
auto eval_full(const DpfKey & dpf,
    IntervalMemoizer && memoizer = IntervalMemoizer{})
{
    assert_not_wildcard<I, Is...>(dpf);

    using input_type = typename DpfKey::input_type;
    auto outbufs = std::make_tuple(
        make_output_buffer_for_full<I>(dpf),
        make_output_buffer_for_full<Is>(dpf)...);

    auto iterable = eval_full<I, Is...>(dpf, outbufs, memoizer);

    return std::make_pair(
        std::move(iterable),
        utils::remove_tuple_if_trivial(std::move(outbufs)));
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_FULL_HPP__
