/// @file dpf/eval_point.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_EVAL_POINT_HPP__
#define LIBDPF_INCLUDE_DPF_EVAL_POINT_HPP__

#include <iomanip>

#include <functional>
#include <memory>
#include <limits>
#include <tuple>
#include <algorithm>

#include <portable-snippets/builtin/builtin.h>
#include <hedley/hedley.h>

#include "dpf/dpf_key.hpp"
#include "dpf/eval_common.hpp"
#include "dpf/path_memoizer.hpp"

namespace dpf
{

namespace internal
{

template <typename DpfKey,
          typename InputT,
          class PathMemoizer>
inline auto eval_point_interior(const DpfKey & dpf, InputT x, PathMemoizer & path)
{
    using dpf_type = DpfKey;
    using input_type = InputT;

    std::size_t level_index = path.assign_x(dpf, x);
    DPF_UNROLL_LOOP
    for (input_type mask = dpf.msb_mask>>(level_index-1);
        level_index <= dpf.depth; ++level_index, mask>>=1)
    {
        bool bit = !!(mask & x);
        auto cw = set_lo_bit(dpf.correction_words[level_index-1],
            dpf.correction_advice[level_index-1]>>bit);
        path[level_index] = dpf_type::traverse_interior(path[level_index-1], cw, bit);
    }
}

template <std::size_t I = 0,
          typename DpfKey,
          class PathMemoizer>
inline auto eval_point_exterior(const DpfKey & dpf, PathMemoizer & path)
{
    assert_not_wildcard<I>(dpf);

    auto interior = path[dpf.depth];
    auto ext = dpf.template leaf<I>();
    return DpfKey::template traverse_exterior<I>(interior, ext);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          class PathMemoizer>
HEDLEY_ALWAYS_INLINE
auto eval_point(const DpfKey & dpf, InputT x, PathMemoizer & path)
{
    assert_not_wildcard<I>(dpf);
    internal::eval_point_interior(dpf, x, path);
    return internal::eval_point_exterior<I>(dpf, path);
}

}  // namespace internal

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          class PathMemoizer>
auto eval_point(const DpfKey & dpf, InputT x, PathMemoizer & path)
{
    using output_type = std::tuple_element_t<I, typename DpfKey::concrete_outputs_tuple>;
    return make_dpf_output<output_type>(internal::eval_point<I>(dpf, x, path), x);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT>
auto eval_point(const DpfKey & dpf, InputT x)
{
    auto path = make_nonmemoizing_path_memoizer(dpf);
    return eval_point<I>(dpf, x, path);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_POINT_HPP__
