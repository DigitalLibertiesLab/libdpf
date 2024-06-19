/// @file grotto/gadgets/elementary/approx.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_APPROX_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_APPROX_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

static constexpr double approx_default_target = 0;
static constexpr unsigned approx_default_ULPs = 1;

template <double const & target  = approx_default_target,
          unsigned const & ULPs = approx_default_ULPs>
struct approx
{
    template <typename T>
    T operator()(T x) { return std::abs(x - target) <= ULPs * ulp_of(target) ? 1 : 0; }
};

template <double const & target,
          unsigned const & ULPs>
struct gadget_hints<approx<target, ULPs>>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr unsigned degree = 0;
    static constexpr double poles[] = { };
    static constexpr bool has_canonical_representation = true;
    static constexpr double interesting_points[] = { target - ULPs * ulp_of(target), target + ULPs * ulp_of(target) };
    static constexpr double canonical_bounds[] = { target - ULPs * ulp_of(target), target + ULPs * ulp_of(target), target + ULPs * ulp_of(target) };
    static constexpr std::array<double, degree+1> canonical_polys[] = { 0, 1, 1, 0 };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_APPROX_HPP__
