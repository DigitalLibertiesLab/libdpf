/// @file grotto/gadgets/elemwntary/clip.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_CLIP_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_CLIP_HPP__

#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

template <double const & lower,
          double const & upper>
struct clip
{
    static_assert(lower <= upper);
    template <typename T>
    T operator()(T x) { return std::max(std::min(x, upper), lower); }
};


template <double const & lower,
          double const & upper>
struct gadget_hints<clip<lower, upper>>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr unsigned degree = 1;
    static constexpr double poles[] = { };
    static constexpr bool has_canonical_representation = true;
    static constexpr double interesting_points[] = { lower, upper };
    static constexpr double canonical_bounds[] = { lower, upper };
    static constexpr std::array<double, degree+1> canonical_polys[] = { { lower, 0 }, { 0, 1 }, { upper, 0 } };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_CLIP_HPP__
