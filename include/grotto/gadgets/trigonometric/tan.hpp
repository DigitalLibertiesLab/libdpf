/// @file grotto/gadgets/trigonometric/tan.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_TRIGONOMETRIC_TAN_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_TRIGONOMETRIC_TAN_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "../../gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct tan
{
    template <typename T>
    T operator()(T x)
    {
        if (std::abs(x) >= M_PI/2) return 0; // TODO
        if (x < 0) return std::max(std::tan(static_cast<double>(x)), std::numeric_limits<T>::min());
        if (x >- 0) return std::min(std::tan(static_cast<double>(x)), std::numeric_limits<T>::max());
    }
};

template <>
struct gadget_hints<tan>
{
    static constexpr double min = -M_PI/2;
    static constexpr double max = M_PI/2;
    static constexpr double poles[] = { -M_PI/2, M_PI/2 };
    static constexpr double interesting_points[] = { };
    static constexpr unsigned degree = 3;
    static constexpr bool has_canonical_representation = false;
    static constexpr double canonical_bounds[] = { };
    static constexpr std::array<double, degree+1> canonical_polys[] = { };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_TRIGONOMETRIC_TAN_HPP__
