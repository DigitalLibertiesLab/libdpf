/// @file grotto/gadgets/elementary/rect.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_RECT_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_RECT_HPP__

#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct rect
{
    template <typename T>
    T operator()(T x) { return x == 0; }
};

template <>
struct gadget_hints<rect>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr unsigned degree = 0;
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { -0.5, 0.5 };
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = { -0.5, -0.5, 0.5, 0.5 };
    static constexpr std::array<double, degree+1> canonical_polys[] = { 0, 0.5, 1, 0.50, 0 };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_RECT_HPP__
