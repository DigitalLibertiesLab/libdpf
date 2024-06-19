/// @file grotto/gadgets/trigonometric/cot.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_TRIGONOMETRIC_COT_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_TRIGONOMETRIC_COT_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "../../gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct csc
{
    template <typename T>
    T operator()(T x)
    {
        if (x == M_PI) return 0; // TODO
        if (x == 0) return 0; // TODO
        if (x < M_PI/2) return std::min(std::cos(static_cast<double>(x))/std::sin(static_cast<double>(x)), std::numeric_limits<T>::max());
        if (x <= M_PI/2) return std::max(std::cos(static_cast<double>(x))/std::sin(static_cast<double>(x)), std::numeric_limits<T>::min());
    }
};

template <>
struct gadget_hints<csc>
{
    static constexpr double min = 0;
    static constexpr double max = M_PI;
    static constexpr double poles[] = { 0, M_PI };
    static constexpr double interesting_points[] = { };
    static constexpr unsigned degree = 3;
    static constexpr bool has_canonical_representation = false;
    static constexpr double canonical_bounds[] = { };
    static constexpr std::array<double, degree+1> canonical_polys[] = { };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_TRIGONOMETRIC_COT_HPP__
