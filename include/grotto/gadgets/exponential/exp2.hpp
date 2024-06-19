/// @file grotto/gadgets/exponential/exp2.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief gadgetized form of `std::exp2`
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_EXPONENTIAL_EXP2_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_EXPONENTIAL_EXP2_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct exp2
{
    template <typename T>
    T operator()(T x) { return std::min(std::exp2(static_cast<double>(x)), std::numeric_limits<T>::max()); }
};

template <>
struct gadget_hints<exp2>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double * poles = nullptr;
    static constexpr double * interesting_points = nullptr;
    static constexpr unsigned degree = 3;
    static constexpr bool has_canonical_representation = false;
    static constexpr double * canonical_bounds = nullptr;
    static constexpr std::array<double, degree+1> * canonical_polys = nullptr;
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_EXPONENTIAL_EXP2_HPP__
