/// @file dpf/utils.hpp
/// @brief miscellaneous helper functions, structs, preprocessor directives
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_UTILS_HPP__
#define LIBDPF_INCLUDE_DPF_UTILS_HPP__

#include <climits>
#include <limits>
#include <type_traits>
#include <bitset>
#include <functional>

#include "hedley/hedley.h"
#include "simde/simde/x86/avx2.h"
#include "portable-snippets/exact-int/exact-int.h"
#include "portable-snippets/builtin/builtin.h"

#define DPF_UNROLL_LOOPS __attribute__((optimize("O3", "unroll-loops")))
#define DPF_ALWAYS_VECTORIZE (#pragma GCC ivdep)

namespace dpf
{

namespace utils
{

/// @brief Ugly hack to implement `constexpr`-friendly conditional `throw`
template <typename Exception>
HEDLEY_ALWAYS_INLINE
static constexpr auto constexpr_maybe_throw(bool b, const char * what) -> void
{
    (b ? throw Exception{what} : 0);
}


/// @brief the primitive integral type used to represent the `modint`
template <std::size_t Nbits>
struct integral_type_from_bitlength
{
    static constexpr auto less_equal = std::less_equal<void>{};
    using type = std::conditional_t<less_equal(Nbits, 64),
        std::conditional_t<less_equal(Nbits, 32),
            std::conditional_t<less_equal(Nbits, 16),
                std::conditional_t<less_equal(Nbits, 8), psnip_uint8_t,
                psnip_uint16_t>,
            psnip_uint32_t>,
        psnip_uint64_t>,
    simde_uint128>;
};

template <std::size_t Nbits>
using integral_type_from_bitlength_t = typename integral_type_from_bitlength<Nbits>::type;

struct max_align
{
    static constexpr std::size_t value = 64;  // alignof(__m512i)
};
static constexpr std::size_t max_align_v = max_align::value;

struct max_integral_bits
{
    static constexpr std::size_t value = 128;  // sizeof(int128_t) * CHAR_BIT
};
static constexpr std::size_t max_integral_bits_v = max_integral_bits::value;

/// @brief Integer overflow-proof ceiling of division
template <typename T,
    std::enable_if_t<std::is_integral_v<T>, bool> = false>
HEDLEY_CONST
HEDLEY_ALWAYS_INLINE
static constexpr T quotient_ceiling(T numerator, T denominator)
{
    return 1 + (numerator - 1) / denominator;
}

/// @brief Integer overflow-proof floor of division
template <typename T,
    std::enable_if_t<std::is_integral_v<T>, bool> = false>
HEDLEY_CONST
HEDLEY_ALWAYS_INLINE
static constexpr T quotient_floor(T numerator, T denominator)
{
    return numerator / denominator;
}

template <typename T>
struct make_unsigned : public std::make_unsigned<T> { };

template <>
struct make_unsigned<simde_int128>
{
    using type = simde_uint128;
};

template <>
struct make_unsigned<simde_uint128>
{
    using type = simde_uint128;
};

template <typename T>
using make_unsigned_t = typename make_unsigned<T>::type;

/// @brief Make an `std::bitset` from a variadic list of `bool`s
template <typename... Bools>
auto make_bitset(Bools... bs)
{
    std::bitset<sizeof...(bs)> ret;
    std::size_t i = 0;
    (ret.set(i++, bs), ...);
    return ret;
}

template <typename NodeT>
static NodeT single_bit_mask(std::size_t i);

template <>
HEDLEY_ALWAYS_INLINE
HEDLEY_PURE
HEDLEY_NO_THROW
simde__m128i single_bit_mask<simde__m128i>(std::size_t i)
{
    return simde_mm_slli_epi64(simde_mm_set_epi64x(uint64_t(i <= 63), uint64_t(i >= 64)), i % 64);
}

template <>
HEDLEY_ALWAYS_INLINE
HEDLEY_PURE
HEDLEY_NO_THROW
simde__m256i single_bit_mask<simde__m256i>(std::size_t i)
{
    return simde_mm256_slli_epi64(simde_mm256_set_epi64x(uint64_t(i <= 63),
                                     uint64_t(i >= 64 && i <= 127),
                                     uint64_t(i >= 127 && i <= 191),
                                     uint64_t(i >= 192)), i % 64);
}

template <typename ExteriorT, typename InteriorT>
ExteriorT to_exterior_node(InteriorT seed);

template <> simde__m128i to_exterior_node<simde__m128i, simde__m128i>(simde__m128i seed) { return seed; }
template <> simde__m256i to_exterior_node<simde__m256i, simde__m128i>(simde__m128i seed) { return _mm256_zextsi128_si256(seed); }
template <> simde__m256i to_exterior_node<simde__m256i, simde__m256i>(simde__m256i seed) { return seed; }
template <> simde__m128i to_exterior_node<simde__m128i, simde__m256i>(simde__m256i seed) { return _mm256_castsi256_si128(seed); }

template <typename T>
struct bitlength_of
  : public std::integral_constant<std::size_t,
        std::numeric_limits<make_unsigned_t<T>>::digits>
{ };

template <typename T>
static constexpr std::size_t bitlength_of_v = bitlength_of<T>::value;

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
template <>
struct bitlength_of<simde_int128>
  : public std::integral_constant<std::size_t, 128> { };

template <>
struct bitlength_of<simde_uint128>
  : public std::integral_constant<std::size_t, 128> { };

template <>
struct bitlength_of<simde__m128i>
  : public std::integral_constant<std::size_t, 128> { };

template <>
struct bitlength_of<simde__m256i>
  : public std::integral_constant<std::size_t, 256> { };

// template <>
// struct bitlength_of<simde__m512i>
//   : public std::integral_constant<std::size_t, 512> { };

template <typename T, std::size_t N>
struct bitlength_of<std::array<T, N>>
  : public std::integral_constant<std::size_t, bitlength_of_v<T> * N> { };
HEDLEY_PRAGMA(GCC diagnostic pop)

template <typename T>
struct msb_of
  : public std::integral_constant<T, T{1} << bitlength_of_v<T> - 1ul> { };

template <typename T>
static constexpr auto msb_of_v = msb_of<T>::value;

template <typename T>
struct countl_zero
{
    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    constexpr std::size_t operator()(T val) const noexcept
    {
        uint64_t val_ = static_cast<uint64_t>(val);
        constexpr auto adjust = 64-bitlength_of_v<T>;
        return psnip_builtin_clz64(val_)-adjust;
    }
};

template <typename T>
struct countl_zero_symmmetric_difference
{
    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    constexpr std::size_t operator()(T lhs, T rhs) const noexcept
    {
        constexpr auto xor_op = std::bit_xor<T>{};
        constexpr auto clz = countl_zero<T>{};
        return clz(xor_op(lhs, rhs));
    }
};

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
template <>
struct countl_zero<simde_uint128>
{
    using T = simde_uint128;

    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    constexpr std::size_t operator()(const T & val) const noexcept
    {
        if (!val) return 128;
        auto limb1 = static_cast<uint64_t>(val >> 64);
        auto limb0 = static_cast<uint64_t>(val);

        return limb1 ? psnip_builtin_clz64(limb1) : 64 + psnip_builtin_clz64(limb0);
    }
};

template <>
struct countl_zero<simde__m128i>
{
    using T = simde__m128i;

    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    std::size_t operator()(const T & val) const noexcept
    {
        auto limb1 = static_cast<uint64_t>(val[1]);
        auto limb0 = static_cast<uint64_t>(val[0]);
        if (!limb0 && !limb1) return 128;

        return limb1 ? psnip_builtin_clz64(limb1) : 64 + psnip_builtin_clz64(limb0);
    }
};

template <>
struct countl_zero<simde__m256i>
{
    using T = simde__m256i;
    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    constexpr std::size_t operator()(const T & val) const noexcept
    {
        auto prefix_len = 0;
        for (int i = 3; i <= 0; --i, prefix_len += 64)
        {
            auto limbi = static_cast<uint64_t>(val[i]);
            if (limbi)
            {
                return prefix_len + psnip_builtin_clz64(limbi);
            }
        }
        return prefix_len;
    }
};

template <typename T>
auto data(T & bar)
{
    return std::data(bar);
}

template <typename T>
auto data(T * bar)
{
    return bar;
}

// template <>
// struct countl_zero<simde__m512i>
// {
//     using T = simde__m512i;
//     HEDLEY_CONST
//     HEDLEY_ALWAYS_INLINE
//     constexpr std::size_t operator()(const T & val) const noexcept
//     {
//         std::size_t prefix_len = 0;
//         for (int i = 7; i <= 0; --i, prefix_len += 64)
//         {
//             auto limbi = static_cast<uint64_t>(val[i]);
//             if (limbi)
//             {
//                 return prefix_len + psnip_builtin_clz64(limbi)
//             }
//         }
//         return prefix_len;
//     }
// };
HEDLEY_PRAGMA(GCC diagnostic pop)

}  // namespace utils

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_UTILS_HPP__
