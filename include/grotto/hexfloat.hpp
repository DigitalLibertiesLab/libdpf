/// @file grotto/hexfloat.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @details
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_HEXFLOAT_HPP__
#define LIBDPF_INCLUDE_GROTTO_HEXFLOAT_HPP__

#include <cmath>
#include <string>
#include <cstring>

namespace grotto
{

template <class CharT = char,
          class Traits = std::char_traits<CharT>,
          class Allocator = std::allocator<CharT>>
std::basic_string<CharT, Traits, Allocator> to_hexfloat(double x)
{
    static constexpr auto base_indicator = "0x";
    if (std::isnan(x))
    {
        return "NaN";
    }

    if (std::isinf(x))
    {
        return std::string(std::signbit(x) ? "-" : "+") + "Infinity";
    }

    if (iszero(x))
    {
        return std::string(std::signbit(x) ? "-" : "+")
            + "0x0.0000000000000p+0000";
    }

    uint32_t X[2];
    std::memcpy(&X[0], &x, sizeof(X));
    const int32_t sign_mask = 0x7fffffff;
    const int32_t exponent_mask  = 0x7ff00000;
    const unsigned hi = 1, lo = 1 - hi;

    // extract and clear the exponent
    int32_t exponent = ((X[hi] & exponent_mask) >> 20) - 0x3ff;
    if (exponent == -1023)
    {
        x *= std::exp2(1023);
        std::memcpy(&X[0], &x, sizeof(X));
        exponent += ((X[hi] & exponent_mask) >> 20) - 0x3ff;
    }
    X[hi] = (X[hi] & ~exponent_mask) ^ 0x3ff00000;

    // extract and clear the sign bit
    std::string sign_prefix = !!(X[hi] & ~sign_mask) ? "-" : "+";
    X[hi] &= sign_mask;

    uint64_t mantissa = X[lo] | (uint64_t(X[hi]) & 0xfffff) << 32;
    char buf[13+1];
    snprintf(buf, 13+1, "%013lx", mantissa);
    std::string hex_mantissa = std::string("1.") + std::string(buf);

    std::string expsign = exponent < 0 ? "p-" : "p+";
    exponent = std::abs(exponent);
    std::string expnum = std::to_string(exponent);
    std::string exponent_suffix = expsign + std::string(4 - std::min(size_t(4), expnum.length()), '0') + expnum;

    return sign_prefix + base_indicator + hex_mantissa + exponent_suffix;
}

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_HEXFLOAT_HPP__
