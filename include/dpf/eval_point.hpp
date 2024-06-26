/// @file dpf/eval_point.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_EVAL_POINT_HPP__
#define LIBDPF_INCLUDE_DPF_EVAL_POINT_HPP__

#include <portable-snippets/builtin/builtin.h>
#include <hedley/hedley.h>

#include <cstddef>
#include <tuple>

#include "dpf/dpf_key.hpp"
#include "dpf/eval_common.hpp"
#include "dpf/path_memoizer.hpp"

namespace dpf
{

namespace internal
{

template <typename DpfKey,
          typename InputT,
          typename PathMemoizer>
inline auto eval_point_interior(const DpfKey & dpf, InputT && x, PathMemoizer && path)
{
    using dpf_type = DpfKey;

    auto level_index = path.assign_x(dpf, x);

    DPF_UNROLL_LOOP
    for (auto mask = dpf.msb_mask>>(level_index-1);
        level_index <= dpf.depth; ++level_index, mask>>=1)
    {
        bool bit = !!(mask & x);
        auto cw = dpf.correction_word(level_index-1, bit);
        path[level_index] = dpf_type::traverse_interior(path[level_index-1], cw, bit);
    }
}

template <std::size_t I,
          typename DpfKey,
          typename PathMemoizer>
inline auto eval_point_exterior(const DpfKey & dpf, PathMemoizer && path)
{
    assert_not_wildcard_output<I>(dpf);

    auto interior = path[dpf.depth];
    return dpf.template traverse_exterior<I>(interior);
}

template <std::size_t I,
          typename DpfKey,
          typename InputT,
          typename PathMemoizer>
HEDLEY_ALWAYS_INLINE
auto eval_point(const DpfKey & dpf, InputT && x, PathMemoizer && path)
{
    utils::flip_msb_if_signed_integral(x);
    internal::eval_point_interior(dpf, x, path);
    return internal::eval_point_exterior<I>(dpf, path);
}

}  // namespace internal

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          typename PathMemoizer = dpf::nonmemoizing_path_memoizer<DpfKey>>
HEDLEY_ALWAYS_INLINE
auto eval_point(const DpfKey & dpf, InputT && x, PathMemoizer && path = PathMemoizer{})
{
    assert_not_wildcard_output<I>(dpf);
    using output_type = typename DpfKey::concrete_output_type<I>;

    auto tx = dpf.offset_x(x);
    return make_dpf_output<output_type>(internal::eval_point<I>(dpf, tx, path), tx);
}

template <std::size_t I0,
          std::size_t I1,
          std::size_t ...Is,
          typename DpfKey,
          typename InputT,
          typename PathMemoizer = dpf::basic_path_memoizer<DpfKey>>
HEDLEY_ALWAYS_INLINE
auto eval_point(const DpfKey & dpf, InputT && x, PathMemoizer && path = PathMemoizer{})
{
    return std::make_tuple(
        *eval_point<I0>(dpf, x, path),
        *eval_point<I1>(dpf, x, path),
        *eval_point<Is>(dpf, x, path)...);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_POINT_HPP__
