/// @file grotto/gadgets/hyperbolic/acosh.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_HYPERBOLIC_ACOSH_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_HYPERBOLIC_ACOSH_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

// Returns the inverse hyperbolic cosine of a number.
// The number must be greater than or equal to 1.
struct acosh
{
    template <typename T>
    T operator()(T x) { 
        if (x < 1) return -1; // TODO
        return std::acosh(static_cast<double>(x)); }
};

template <>
struct gadget_hints<acosh>
{
    static constexpr double min = 1;
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { };
    static constexpr unsigned degree = 3;
    static constexpr bool has_canonical_representation = false;
    static constexpr double canonical_bounds[] = { };
    static constexpr std::array<double, degree+1> canonical_polys[] = { };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_HYPERBOLIC_ACOSH_HPP__
