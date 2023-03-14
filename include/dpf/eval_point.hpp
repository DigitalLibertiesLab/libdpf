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
#include "dpf/path_memoizer.hpp"

namespace dpf
{

namespace internal
{

template <typename DpfKey,
          typename InputT,
          class PathMemoizer>
DPF_UNROLL_LOOPS
inline auto eval_point_interior(const DpfKey & dpf, InputT x, PathMemoizer & path)
{
    std::size_t level_index = path.assign_x(dpf, x);
    for (InputT mask = dpf.msb_mask>>level_index;
        level_index < dpf.depth; ++level_index, mask>>=1)
    {
        bool bit = !!(mask & x);
        auto cw = set_lo_bit(dpf.interior_cws[level_index],
            dpf.correction_advice[level_index]>>bit);
        path[level_index+1] = DpfKey::traverse_interior(path[level_index], cw, bit);
    }
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          class PathMemoizer>
inline auto eval_point_exterior(const DpfKey & dpf, InputT x, const PathMemoizer & path)
{
    assert_not_wildcard<I>(dpf);

    using output_t = std::tuple_element_t<I, typename DpfKey::outputs_t>;

    auto interior = path[dpf.depth];
    auto ext = dpf.template exterior_cw<I>();
    return make_dpf_output<output_t>(
        DpfKey::template traverse_exterior<I>(interior, ext), x);
}

}  // namespace internal

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          class PathMemoizer>
auto eval_point(const DpfKey & dpf, InputT x, PathMemoizer & path)
{
    assert_not_wildcard<I>(dpf);
    internal::eval_point_interior(dpf, x, path);
    return internal::eval_point_exterior<I>(dpf, x, path);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT>
auto eval_point(const DpfKey & dpf, InputT x)
{
    auto memoizer = make_nonmemoizing_path_memoizer(dpf);
    return eval_point<I>(dpf, x, memoizer);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_POINT_HPP__
