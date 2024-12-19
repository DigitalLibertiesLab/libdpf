/// @file grotto/gadgets/trigonometric/deg2rad.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_TRIGONOMETRY_DEG2RAD_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_TRIGONOMETRY_DEG2RAD_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "../../gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct deg2rad
{
    template <typename T>
    T operator()(T x) { 
        double X = std::fmod(static_cast<double>(x), 360);
        if (X < 0) X += 360;
        return X*180/M_PI;
    }
};

template <>
struct gadget_hints<deg2rad>
{
    static constexpr double min = -180;
    static constexpr double max = 180;
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { };
    static constexpr unsigned degree = 1;
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = { 0 };
    static constexpr std::array<double, degree+1> canonical_polys[] = { { 180/M_PI, 0 } };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_TRIGONOMETRY_DEG2RAD_HPP__
