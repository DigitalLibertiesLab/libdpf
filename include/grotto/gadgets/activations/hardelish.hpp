/// @file grotto/gadgets/activations/hardelish.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDELISH_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDELISH_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct hardelish
{
    template <typename T>
    T operator()(T x)
    {
        if (x < 0) return std::expm1(x)*std::max(0, std::min(1,(x+1)/2));
        if (x >= 0) return std::min(x*std::max(0,std::min(1,(x+1)/2)), std::numeric_limits<T>::max());
    }
};

template <>
struct gadget_hints<hardelish>
{
    static constexpr double min = std::numeric_limits<double>::min();
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

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDELISH_HPP__