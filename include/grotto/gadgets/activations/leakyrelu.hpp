/// @file grotto/gadgets/activations/leakyrelu.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_LEAKYRELU_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_LEAKYRELU_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

static constexpr double leakyrelu_default_negative_slope = 0.01;
static constexpr double leakyrelu_zero_negative_slope = 0.0;
template <double const & negative_slope = leakyrelu_default_negative_slope>
struct leakyrelu
{
    template <typename T>
    T operator()(T x)
    {
        if (x >= 0) return x;
        return negative_slope * x;
    }
};

template <double const & negative_slope>
struct gadget_hints<leakyrelu<negative_slope>>
{
    inline static constexpr double min = std::numeric_limits<double>::lowest();
    inline static constexpr double max = std::numeric_limits<double>::max();
    inline static constexpr double poles[] = { };
    inline static constexpr double interesting_points[] = { 0 };
    inline static constexpr unsigned degree = 1;
    inline static constexpr bool has_canonical_representation = true;
    inline static constexpr double canonical_bounds[] = { 0 };
    inline static constexpr std::array<double, degree+1> canonical_polys[] = { {0,negative_slope}, {0,1} };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_LEAKYRELU_HPP__
