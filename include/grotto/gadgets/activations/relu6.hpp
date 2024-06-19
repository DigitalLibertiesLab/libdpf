/// @file grotto/gadgets/activations/relu6.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_RELU6_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_RELU6_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

static constexpr double relu6_default_clip = 6;
template <double const & clip = 6>
struct relu6
{
    template <typename T>
    T operator()(T x)
    {
        return std::min(std::max(0, x), clip);
    }
};

template <double const & clip>
struct gadget_hints<relu6<clip>>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { 0, clip };
    static constexpr unsigned degree = 1;
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = { 0, clip };
    static constexpr std::array<double, degree+1> canonical_polys[] = { 0, {0,1}, clip };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_RELU6_HPP__
