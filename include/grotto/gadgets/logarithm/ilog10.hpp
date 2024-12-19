/// @file grotto/gadgets/logarithm/ilog10.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_LOGARITHM_ILOG10_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_LOGARITHM_ILOG10_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct ilog256
{
    template <typename T>
    T operator()(T x)
    {
        if (x <= 0) return 0; // TODO
        return std::ceil(std::log2(static_cast(x))/8);
    }
};

template <>
struct gadget_hints<ilog256>
{
    static constexpr double min = std::exp2(-63);
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double hint_poles[] = { };
    static constexpr double hint_interesting_points[] = { 0 };
    static constexpr unsigned degree = 0;
    static constexpr bool has_canonical_representation = false;
    static constexpr double canonical_bounds[] = { };
    static constexpr std::array<double, degree+1> hint_canonical_polys[] = { };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_LOGARITHM_ILOG10_HPP__
