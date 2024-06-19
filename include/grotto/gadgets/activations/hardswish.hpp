/// @file grotto/gadgets/activations/hardswish.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDSWISH_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDSWISH_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct hardswish
{
    template <typename T>
    T operator()(T x)
    {
        if (x <= -3) return 0;
        if (x >= 3) return x;
        return x*(x+3)/6;
    }
};

template <>
struct gadget_hints<hardswish>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { -3, 3 };
    static constexpr unsigned degree = 2;
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = { -3, 3 };
    static constexpr std::array<double, degree+1> canonical_polys[] = { {0}, {0,0.5,1/6.0}, {0,1,0} };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDSWISH_HPP__
