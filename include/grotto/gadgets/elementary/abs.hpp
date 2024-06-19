/// @file grotto/gadgets/elementary/abs.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_ABS_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_ABS_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct abs
{
    template <typename T>
    T operator()(T x) { return std::abs(static_cast<double>(x)); }
};

template <>
struct gadget_hints<abs>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { 0 };
    static constexpr unsigned degree = 1;
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = { 0 };
    static constexpr std::array<double, degree+1> canonical_polys[] = { { -1, 0 }, { 1, 0 } };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_ABS_HPP__
