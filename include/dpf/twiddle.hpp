/// @file dpf/twiddle.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_TWIDDLE_HPP__
#define LIBDPF_INCLUDE_DPF_TWIDDLE_HPP__

#include "hedley/hedley.h"
#include "simde/simde/x86/avx2.h"

namespace dpf
{

static constexpr simde__m128i lo_bit128{int64_t{1}, int64_t{0}};
static constexpr simde__m128i lo_2bits128{int64_t{3}, int64_t{0}};

HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
simde__m128i mask_from_lo_bit(simde__m128i a)
{
    a = simde_mm_cmpeq_epi32(lo_bit128, simde_mm_and_si128(lo_bit128, a));
    return simde_mm_shuffle_epi32(a, _MM_SHUFFLE(0, 0, 0, 0));
}

HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
uint_fast8_t get_lo_bit(simde__m128i a)
{
    return !simde_mm_testz_si128(simde_mm_and_si128(lo_bit128, a), lo_bit128);
}

HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
simde__m128i set_lo_bit(simde__m128i a, uint_fast8_t b = 1)
{
     return b&1 ? simde_mm_or_si128(lo_bit128, a)
                : simde_mm_andnot_si128(lo_bit128, a);
}

HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
simde__m128i unset_lo_bit(simde__m128i a)
{
    return simde_mm_andnot_si128(lo_bit128, a);
}

HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
auto get_if_lo_bit(simde__m128i a, simde__m128i b)
{
    return simde_mm_and_si128(a, dpf::mask_from_lo_bit(b));
}

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
template <std::size_t N>
HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
auto get_if_lo_bit(std::array<simde__m128i, N> a, simde__m128i b)
{
    auto mask = dpf::mask_from_lo_bit(b);
    std::transform(std::begin(a), std::end(a), std::begin(a), [mask](simde__m128i & a){return simde_mm_and_si128(a, mask);});
    return a;
}

HEDLEY_PRAGMA(GCC diagnostic pop)

HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
simde__m128i xor_if_lo_bit(simde__m128i a, simde__m128i b, simde__m128i c)
{
    return simde_mm_xor_si128(a, dpf::get_if_lo_bit(b, c));
}




HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
uint_fast8_t get_lo_2bits(simde__m128i a)
{
    a = simde_mm_and_si128(a, lo_2bits128);
    return simde_mm_extract_epi64(a, 0);
}

HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
simde__m128i unset_lo_2bits(simde__m128i a)
{
    return simde_mm_andnot_si128(lo_2bits128, a);
}

HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
uint_fast8_t get_lo_bit_and_clear_lo_2bits(simde__m128i & a)
{
    uint_fast8_t lo_bit = get_lo_bit(a);
    a = unset_lo_2bits(a);
    return lo_bit;
}

HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
simde__m128i set_lo_2bits(simde__m128i a, uint_fast8_t i)
{
    static constexpr simde__m128i lo2[4] = {
        {int64_t(0), int64_t(0)},
        {int64_t(1), int64_t(0)},
        {int64_t(2), int64_t(0)},
        {int64_t(3), int64_t(0)}
    };
    return simde_mm_or_si128(dpf::unset_lo_2bits(a), lo2[i&3]);
}

HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
simde__m128i get_if(simde__m128i c, bool b)
{
    static constexpr simde__m128i mask[] = {
        {int64_t(0), int64_t(0)},
        {~int64_t(0), ~int64_t(0)}
    };
    return simde_mm_and_si128(c, mask[b]);
}

HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
simde__m128i xor_if(simde__m128i a, simde__m128i c, bool b)
{
    return simde_mm_xor_si128(a, get_if(c, b));
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_TWIDDLE_HPP__
