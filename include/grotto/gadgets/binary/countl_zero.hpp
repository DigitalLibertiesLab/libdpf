/// @file grotto/gadgets/binary/countl_zero.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief gadgetized form of `std::countl_zero`
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_BINARY_COUNTL_ZERO_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_BINARY_COUNTL_ZERO_HPP__

#include <array>
#include <limits>
#include <portable-snippets/builtin/builtin.h>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct countl_zero
{
    template <typename T>
    T operator()(T x)
    {
        if (x == 0) return std::numeric_limits<T>::bits();
        return psnip_builtin_clz64(static_cast<uint64_t>(static_cast<double>(x)));
    }
};

template <>
struct gadget_hints<countl_zero>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = { 0 };
    static constexpr unsigned degree = 0;
    static constexpr bool has_canonical_representation = false;
    static constexpr double canonical_bounds[] = { };
    static constexpr std::array<double, degree+1> hint_canonical_polys[] = { };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_BINARY_COUNTL_ZERO_HPP__