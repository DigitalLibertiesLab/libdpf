/// @file grotto/gadgets/elementary/gt.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_GT_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_GT_HPP__

#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

template <double const & target>
struct gt
{
    template <typename T>
    T operator()(T x) { return x > target; }
};


template <double const & target>
struct gadget_hints<gt<target>>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr unsigned degree = 0;
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { target };
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = { target, target };
    static constexpr std::array<double, degree+1> canonical_polys[] = { 0, 0, 1 };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_GT_HPP__
