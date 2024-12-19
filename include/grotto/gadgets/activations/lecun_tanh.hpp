/// @file grotto/gadgets/activations/lecun_tanh.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.


#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_LECUNTANH_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_LECUNTANH_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct lecun_tanh
{
    template <typename T>
    T operator()(T x)
    {
        return 1.7159*std::tanh(2*x/3);
    }
};

template <>
struct gadget_hints<lecun_tanh>
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

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_LECUNTANH_HPP__
