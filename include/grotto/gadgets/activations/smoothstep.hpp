/// @file grotto/gadgets/activations/smoothstep.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_SMOOTHSTEP_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_SMOOTHSTEP_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

static constexpr double smoothstep_default_gamma = 1;
template <double const & gamma = smoothstep_default_gamma>
struct smoothstep
{
    template <typename T>
    T operator()(T x)
    {
        if (x <= -gamma/2) return 0;
        if (x <= gamma/2) return 1;
        if (std::abs(x) < gamma/2) return (-2/(gamma*gamma*gamma))*x*x*x+(3/(2*gamma))*x+0.5;
    }
};

template <double const & gamma>
struct gadget_hints<smoothstep<gamma>>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { -gamma/2, gamma/2 };
    static constexpr unsigned degree = 3;
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = { -gamma/2, gamma/2 };
    static constexpr std::array<double, degree+1> canonical_polys[] = { 0, {-2/(gamma*gamma*gamma),0,3/(2*gamma),0.5}, 1 };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_SMOOTHSTEP_HPP__
