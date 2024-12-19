/// @file grotto/gadgets/powers/iqtrt.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_POWERS_IQTRT_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_POWERS_IQTRT_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct iqtrt
{
    template <typename T>
    T operator()(T x)
    {
        if (x <= 0) return 0; // TODO
        return std::min(1.0 / std::pow(static_cast<double>(x), 0.25), std::numeric_limits<T>::max());
    }
};

template <>
struct gadget_hints<iqtrt>
{
    static constexpr double min = std::exp2(-63);
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double poles[] = { 0 };
    static constexpr double interesting_points[] = { 0 };
    static constexpr unsigned degree = 3;
    static constexpr bool has_canonical_representation = false;
    static constexpr double canonical_bounds[] = { };
    static constexpr std::array<double, degree+1> canonical_polys[] = { };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_POWERS_IQTRT_HPP__
