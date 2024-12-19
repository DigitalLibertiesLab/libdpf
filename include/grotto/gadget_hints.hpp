/// @file grotto/gadget_hints.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @details
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.


#ifndef LIBDPF_INCLUDE_GROTTO_GADGET_HINTS_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGET_HINTS_HPP__

#include <cmath>
#include <array>
#include <limits>

namespace grotto
{

namespace gadgets
{

template <typename T>
struct gadget_hints
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr unsigned degree = 3;
    static constexpr double * poles = nullptr;
    static constexpr double * interesting_points = nullptr;
    static constexpr bool has_canonical_representation = false;
    static constexpr double * canonical_bounds = nullptr;
    static constexpr std::array<double, degree+1> * canonical_polys = nullptr;
};

constexpr double ulp_of(double x)
{
    return std::max(
            std::abs(std::nexttoward(x, std::numeric_limits<double>::infinity())-x),
            std::abs(std::nexttoward(x, x-std::numeric_limits<double>::infinity())));
}

template <typename T, std::size_t N>
struct gadget_domain
{
    static constexpr T min()
    {
        
        double fmin = std::max(gadget_hints<T>::min,
            -std::exp2(std::numeric_limits<T>::bits-N));
        T tmin = static_cast<T>(fmin);
        if (static_cast<double>(tmin) < fmin) tmin += std::exp2(-N);
        return tmin;
    }
    static constexpr T max()
    {
        double fmax = std::max(gadget_hints<T>::max,
            std::exp2(std::numeric_limits<T>::bits-N) - std::exp2(-N));
        T tmax = static_cast<T>(fmax);
        if (static_cast<double>(tmax) > fmax) tmax -= std::exp2(-N);
        return tmax;
    }
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGET_HINTS_HPP__
