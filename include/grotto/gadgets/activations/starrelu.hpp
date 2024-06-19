/// @file grotto/gadgets/activations/starrelu.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_STARRELU_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_STARRELU_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

static constexpr double starrelu_default_scale = 0.8944;
static constexpr double starrelu_default_shift = -0.4472;
template<double const & scale = starrelu_default_scale,
         double const & shift = starrelu_default_shift>
struct starrelu
{
    template <typename T>
    T operator()(T x)
    {
        if (x < 0) return 0;
        return scale*x*x+shift;
    }
};

template <double const & scale, double const & shift>
struct gadget_hints<starrelu<shift, scale>>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { 0 };
    static constexpr unsigned degree = 2;
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = { 0 };
    static constexpr std::array<double, degree+1> canonical_polys[] = { {0}, {0,0,1} };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_STARRELU_HPP__
