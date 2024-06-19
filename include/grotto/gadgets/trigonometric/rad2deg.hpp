/// @file grotto/gadgets/trigonometric/rad2deg.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_TRIGONOMETRY_RAD2DEG_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_TRIGONOMETRY_RAD2DEG_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "../../gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct rad2deg
{
    template <typename T>
    T operator()(T x) { 
        double X = std::fmod(static_cast<double>(x), 2*M_PI);
        if (X < 0) X += 2*M_PI;
        return X * M_PI / 180;
    }
};

template <>
struct gadget_hints<rad2deg>
{
    static constexpr double min = -M_PI;
    static constexpr double max = M_PI;
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { };
    static constexpr unsigned degree = 1;
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = { 0 };
    static constexpr std::array<double, degree+1> canonical_polys[] = { { M_PI/180, 0 } };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_TRIGONOMETRY_RAD2DEG_HPP__



// radians = degrees / 180 * M_PI

// degrees = radians / M_PI * 180