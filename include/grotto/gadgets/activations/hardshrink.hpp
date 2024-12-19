/// @file grotto/gadgets/activations/hardshrink.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDSHRINK_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDSHRINK_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

static constexpr double hardshrink_default_lambda = 0.5;
template <double const & lambda = hardshrink_default_lambda>
struct hardshrink
{
    template <typename T>
    T operator()(T x)
    {
        if (std::abs(x) <= lambda) return 0;
        return x;
    }
};

template <double const & lambda>
struct gadget_hints<hardshrink<lambda>>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { -lambda, lambda };
    static constexpr unsigned degree = 1;
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = { -lambda, lambda };
    static constexpr std::array<double, degree+1> canonical_polys[] = { {0,1}, {0}, {0,1} };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDSHRINK_HPP__
