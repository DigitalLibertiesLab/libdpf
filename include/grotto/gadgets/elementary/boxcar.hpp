/// @file grotto/gadgets/elementary/boxcar.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_BOXCAR_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_BOXCAR_HPP__

#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

static constexpr double boxcar_default_outside = 0;
static constexpr double boxcar_default_inside  = 1;

template <double const & from,
          double const & to,
          double const & outside = boxcar_default_outside,
          double const & inside  = boxcar_default_inside>
struct boxcar
{
    template <typename T>
    T operator()(T x) { return (x < from || x > to) ? outside : inside; }
};


template <double const & from,
          double const & to,
          double const & outside,
          double const & inside>
struct gadget_hints<boxcar<from, to, inside, outside>>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr unsigned degree = 0;
    static constexpr double poles[] = { };
    static constexpr bool has_canonical_representation = true;
    static constexpr double interesting_points[] = { from, to };
    static constexpr double canonical_bounds[] = { from, to };
    static constexpr std::array<double, degree+1> canonical_polys[] = { outside, inside, outside };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_BOXCAR_HPP__
