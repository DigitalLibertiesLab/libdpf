/// @file grotto/gadgets/activations/hardsigmoid.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDSIGMOID_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDSIGMOID_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct hardsigmoid
{
    template <typename T>
    T operator()(T x)
    {
        if (x <= -3) return 0;
        if (x >= 3) return 1;
        return (x+3)/6;
    }
};

template <>
struct gadget_hints<hardsigmoid>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { -3, 3 };
    static constexpr unsigned degree = 1;
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = { -3, 3 };
    static constexpr std::array<double, degree+1> canonical_polys[] = { {0}, {0.5,1/6.0}, {1} };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDSIGMOID_HPP__
