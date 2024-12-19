/// @file grotto/gadgets/activations/hardtanh.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDTANH_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDTANH_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct hardtanh
{
    template <typename T>
    T operator()(T x)
    {
        if (x <= -1) return -1;
        if (x >= 1) return 1;
        return x;
    }
};

template <>
struct gadget_hints<hardtanh>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { -1, 1 };
    static constexpr unsigned degree = 1;
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = { -1, 1 };
    static constexpr std::array<double, degree+1> canonical_polys[] = { {-1}, {0,1}, {1} };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_HARDTANH_HPP__
