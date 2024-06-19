/// @file grotto/gadgets/elementary/step.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_STEP_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_STEP_HPP__

#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

static constexpr double step_default_before = 0;
static constexpr double step_default_after  = 1;

template <double const & at,
          double const & before = step_default_before,
          double const & after  = step_default_after>
struct step
{
    template <typename T>
    T operator()(T x) { return x < at ? before : after; }
};


template <double const & at,
          double const & before,
          double const & after>
struct gadget_hints<step<at, before, after>>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr unsigned degree = 0;
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { at };
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = { at };
    static constexpr std::array<double, degree+1> canonical_polys[] = { before, after };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_STEP_HPP__
