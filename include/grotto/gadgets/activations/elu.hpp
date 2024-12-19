/// @file grotto/gadgets/activations/elu.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_ELU_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_ELU_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

static constexpr double elu_default_alpha = 1;
template <double const & alpha = elu_default_alpha>
struct elu
{
    template <typename T>
    T operator()(T x)
    {
        if (x <= 0) return alpha * std::expm1(x);
        if (x > 0) return std::min(x, std::numeric_limits<T>::max());
    }
};

template <double const & alpha>
struct gadget_hints<elu<alpha>>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { };
    static constexpr unsigned degree = 3;
    static constexpr bool has_canonical_representation = false;
    static constexpr double canonical_bounds[] = { };
    static constexpr std::array<double, degree+1> canonical_polys[] = { };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_ELU_HPP__
