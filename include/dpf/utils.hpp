/// @file dpf/utils.hpp
/// @brief miscellaneous helper functions, structs, preprocessor directives
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_UTILS_HPP__
#define LIBDPF_INCLUDE_DPF_UTILS_HPP__

#include <cstddef>
#include <type_traits>
#include <string_view>
#include <iterator>
#include <limits>
#include <tuple>
#include <utility>
#include <algorithm>
#include <functional>
#include <bitset>
#include <array>

#include "hedley/hedley.h"
#include "simde/simde/x86/avx2.h"
#include "portable-snippets/exact-int/exact-int.h"
#include "portable-snippets/builtin/builtin.h"
#include "portable-snippets/endian/endian.h"
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Waggressive-loop-optimizations")
#include "hash-library/sha256.cpp"  // NOLINT(build/include)
HEDLEY_PRAGMA(GCC diagnostic pop)

#include "uint256_t/uint256_t.hpp"

#define DPF_UNROLL_LOOP_N(N) HEDLEY_PRAGMA(GCC unroll N)
#define DPF_UNROLL_LOOP DPF_UNROLL_LOOP_N(16)
#define DPF_ALWAYS_VECTORIZE (#pragma GCC ivdep)

namespace std
{
/// @details specializes `std::numeric_limits` for `uint128_t`
template<>
class numeric_limits<::uint128_t>
{
  public:
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = false;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr std::float_denorm_style has_denorm = std::denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr std::float_round_style round_style = std::round_toward_zero;
    static constexpr bool is_iec559 = true;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = true;
    static constexpr int digits = 128;
    static constexpr int digits10 = 128 * std::log10(2);
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int max_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool traps = false;
    static constexpr bool tinyness_before = false;

    static constexpr uint128_t min() noexcept { return uint128_t{0ul, 0ul}; }
    static constexpr uint128_t lowest() noexcept { return uint128_t{0ul, 0ul}; }
    static constexpr uint128_t max() noexcept { return uint128_t{-1ul, -1ul}; }
    static constexpr uint128_t epsilon() noexcept { return 0; }
    static constexpr uint128_t round_error() noexcept { return 0; }
    static constexpr uint128_t infinity() noexcept { return 0; }
    static constexpr uint128_t quiet_NaN() noexcept { return 0; }
    static constexpr uint128_t signaling_NaN() noexcept { return 0; }
    static constexpr uint128_t denorm_min() noexcept { return 0; }
};

/// @details specializes `std::numeric_limits` for `uint128_t const`
template<>
class numeric_limits<uint128_t const>
  : public numeric_limits<uint128_t> {};

/// @details specializes `std::numeric_limits` for
///          `uint128_t volatile`
template<>
class numeric_limits<uint128_t volatile>
  : public numeric_limits<uint128_t> {};

/// @details specializes `std::numeric_limits` for
///          `uint128_t const volatile`
template<>
class numeric_limits<uint128_t const volatile>
  : public numeric_limits<uint128_t> {};


/// @details specializes `std::numeric_limits` for `uint256_t`
template<>
class numeric_limits<::uint256_t>
{
  public:
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = false;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr std::float_denorm_style has_denorm = std::denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr std::float_round_style round_style = std::round_toward_zero;
    static constexpr bool is_iec559 = true;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = true;
    static constexpr int digits = 256;
    static constexpr int digits10 = 77;//256 * std::log10(2);  //< correct if `Nbits<129`
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int max_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool traps = false;
    static constexpr bool tinyness_before = false;

    static constexpr uint256_t min() noexcept { return uint256_t{uint128_t{0ul, 0ul}, uint128_t{0ul, 0ul}}; }
    static constexpr uint256_t lowest() noexcept { return uint256_t{uint128_t{0ul, 0ul}, uint128_t{0ul, 0ul}}; }
    static constexpr uint256_t max() noexcept { return uint256_t{uint128_t{-1ul, -1ul}, uint128_t{-1ul, -1ul}}; }
    static constexpr uint256_t epsilon() noexcept { return 0; }
    static constexpr uint256_t round_error() noexcept { return 0; }
    static constexpr uint256_t infinity() noexcept { return 0; }
    static constexpr uint256_t quiet_NaN() noexcept { return 0; }
    static constexpr uint256_t signaling_NaN() noexcept { return 0; }
    static constexpr uint256_t denorm_min() noexcept { return 0; }
};

/// @details specializes `std::numeric_limits` for `uint256_t const`
template<>
class numeric_limits<uint256_t const>
  : public numeric_limits<uint256_t> {};

/// @details specializes `std::numeric_limits` for
///          `uint256_t volatile`
template<>
class numeric_limits<uint256_t volatile>
  : public numeric_limits<uint256_t> {};

/// @details specializes `std::numeric_limits` for
///          `uint256_t const volatile`
template<>
class numeric_limits<uint256_t const volatile>
  : public numeric_limits<uint256_t> {};

}

namespace dpf
{

using digest_type = std::array<psnip_uint8_t, 32>;

namespace utils
{

/// @brief Ugly hack to implement `constexpr`-frien`dly conditional `throw`
template <typename Exception>
HEDLEY_ALWAYS_INLINE
static constexpr auto constexpr_maybe_throw(bool b, std::string_view what) -> void
{
    (b ? throw Exception{std::data(what)} : 0);
}

struct max_align
{
    static constexpr std::size_t value = 64;  // alignof(__m512i)
};
static constexpr std::size_t max_align_v = max_align::value;

struct max_integral_bits
{
    static constexpr std::size_t value = 256;  // sizeof(uint256_t) * CHAR_BIT
};
static constexpr std::size_t max_integral_bits_v = max_integral_bits::value;

/// @brief Integer overflow-proof ceiling of division
template <typename T,
    std::enable_if_t<std::is_integral_v<T>, bool> = false>
HEDLEY_CONST
HEDLEY_ALWAYS_INLINE
static constexpr T quotient_ceiling(T numerator, T denominator)
{
    return 1 + static_cast<T>(numerator - 1) / denominator;
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
struct is_signed_integral
  : public std::conjunction<std::is_integral<T>, std::is_signed<T>> { };

template <typename T>
static constexpr bool is_signed_integral_v = is_signed_integral<T>::value;

template <typename T>
struct make_unsigned
  : std::conditional<is_signed_integral_v<T>, std::make_unsigned_t<T>, T> { };

template <> struct make_unsigned<simde_uint128>{ using type = simde_uint128; };
template <> struct make_unsigned<simde_int128>{  using type = simde_uint128; };
template <> struct make_unsigned<uint128_t>{ using type = uint128_t; };
template <> struct make_unsigned<uint256_t>{ using type = uint256_t; };

template <typename T>
using make_unsigned_t = typename make_unsigned<T>::type;

/// @brief Make an `std::bitset` from a variadic list of `bool`s
template <typename ...Bools>
auto make_bitset(Bools ...bs)
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
    return simde_mm_slli_epi64(simde_mm_set_epi64x(i >= 64, i <= 63), i % 64);
}

template <>
HEDLEY_ALWAYS_INLINE
HEDLEY_PURE
HEDLEY_NO_THROW
simde__m256i single_bit_mask<simde__m256i>(std::size_t i)
{
    return simde_mm256_slli_epi64(simde_mm256_set_epi64x(i >= 192,
                                     i >= 128 && i <= 191,
                                     i >= 64 && i <= 127,
                                     i <= 63), i % 64);
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
        std::is_integral_v<T> ?
            // we cannot leverage `std::make_unsigned_t` here since `T` might
            // not be an integral type at all (i.e., this ternary might take
            // the other path); thus, we resort to a bit of a hack.
            // If `T==bool`, then `digits==1`; otherwise, if `T` is a signed
            // integral type, then `digits==bitlength-1`; otherwise if, `T` is
            // an unsigned integral type, then `digits==bitlength`; otherwise,
            // we take the other branch.
            //
            // We want each of these to return the `bitlength`. So we add
            // `CHAR_BIT-1` so that `bool` maps to `8`, signed types with
            // `8l-1` digits map to `8l+6`, and unsigned types with `8l`
            // digits map to `8l+7`. We then use integer division and
            // multiplication to round down to the nearest multiple of `8`.
            ((static_cast<unsigned int>(std::numeric_limits<T>::digits)+CHAR_BIT-1)/CHAR_BIT)*CHAR_BIT
          : CHAR_BIT * sizeof(T)> { };

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

template <typename OutputT,
          typename NodeT>
struct bitlength_of_output
  : public std::integral_constant<std::size_t,  // NOLINT(whitespace/operators) <- false positive
        sizeof(OutputT) <= sizeof(NodeT) ?
            // if sizeof OutputT is less than or equal to sizeof NodeT then
            // return power of 2 greater than or equal to sizeof OutputT
            // with a minimum value of CHAR_BIT
            //
            // else return multiple of sizeof NodeT greater than or equal to
            // sizeof OutputT
            std::size_t(1) << static_cast<std::size_t>(std::ceil(std::log2(sizeof(OutputT) * CHAR_BIT)))
          : quotient_ceiling(sizeof(OutputT), sizeof(NodeT)) * sizeof(NodeT) * CHAR_BIT> { };

template <typename OutputT,
          typename NodeT>
static constexpr std::size_t bitlength_of_output_v = bitlength_of_output<OutputT, NodeT>::value;

/// @brief the primitive integral type used to represent non integral types
template <std::size_t Nbits,
          std::size_t MinBits = Nbits,
          std::size_t MaxBits = std::max(Nbits, MinBits)>
struct integral_type_from_bitlength
{
    static_assert(MinBits <= MaxBits);
    static constexpr auto effective_Nbits = std::min(std::max(Nbits, MinBits), MaxBits);
    static constexpr auto less_equal = std::less_equal<void>{};
    using type = std::conditional_t<less_equal(effective_Nbits, 256),
        std::conditional_t<less_equal(effective_Nbits, 128),
            std::conditional_t<less_equal(effective_Nbits, 64),
                std::conditional_t<less_equal(effective_Nbits, 32),
                    std::conditional_t<less_equal(effective_Nbits, 16),
                        std::conditional_t<less_equal(effective_Nbits, 8), psnip_uint8_t,
                        psnip_uint16_t>,
                    psnip_uint32_t>,
                psnip_uint64_t>,
            simde_uint128>,
        uint256_t>,
    void>;
};

template <std::size_t Nbits,
          std::size_t MinBits = Nbits,
          std::size_t MaxBits = std::max(Nbits, MinBits)>
using integral_type_from_bitlength_t = typename integral_type_from_bitlength<Nbits, MinBits, MaxBits>::type;

/// @brief the primitive integral type used to represent non integral types
template <std::size_t Nbits,
          std::size_t MinBits = Nbits,
          std::size_t MaxBits  = std::max(std::size_t(256), MinBits)>
struct nonvoid_integral_type_from_bitlength : public integral_type_from_bitlength<Nbits, MinBits, MaxBits>
{
    static_assert(Nbits && Nbits <= MaxBits, "representation must fit in 256 bits");
};

template <std::size_t Nbits,
          std::size_t MinBits = Nbits,
          std::size_t MaxBits = std::max(std::size_t(256), MinBits)>
using nonvoid_integral_type_from_bitlength_t = typename nonvoid_integral_type_from_bitlength<Nbits, MinBits, MaxBits>::type;

template <typename T>
struct to_integral_type_base
{
    static constexpr std::size_t bits = bitlength_of_v<T>;
    // Select integer type larger than or equal to size of std::size_t
    using integral_type = nonvoid_integral_type_from_bitlength_t<bits, bitlength_of_v<std::size_t>>;
};

template <typename T>
struct to_integral_type : public to_integral_type_base<T>
{
    using parent = to_integral_type_base<T>;
    using parent::bits;
    using typename parent::integral_type;

    using T_integral_type = integral_type_from_bitlength_t<bits>;

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr integral_type operator()(const T & input) const noexcept
    {
        return static_cast<integral_type>(static_cast<T_integral_type>(input));
    }
};

template <typename T>
struct make_from_integral_value
{
    using T_integral_type = integral_type_from_bitlength_t<bitlength_of_v<T>>;
    using S_integral_type = std::conditional_t<std::is_void_v<T_integral_type>, simde_uint128, T_integral_type>;
    using integral_type = std::conditional_t<std::is_signed_v<T>, std::make_signed_t<S_integral_type>, S_integral_type>;
    constexpr T operator()(integral_type val) const noexcept
    {
        return T{val};
    }
};

template <typename T>
struct make_default
{
    static constexpr T value = make_from_integral_value<T>{}(1);
};

template <typename T>
static constexpr T make_default_v = make_default<T>::value;

template <typename DpfKey,
          typename InputT = typename DpfKey::input_type,
          typename IntegralT = typename DpfKey::integral_type>
static constexpr IntegralT get_node_mask(InputT mask, std::size_t level_index)
{
    using dpf_type = DpfKey;
    constexpr auto to_int = to_integral_type<InputT>{};
    return static_cast<IntegralT>(to_int(mask) >> (level_index-1 + dpf_type::lg_outputs_per_leaf));
}

template <typename DpfKey,
          typename InputT = typename DpfKey::input_type,
          typename IntegralT = typename DpfKey::integral_type>
static constexpr IntegralT get_from_node(InputT from)
{
    constexpr auto to_int = to_integral_type<InputT>{};
    return quotient_floor(to_int(from),
        static_cast<IntegralT>(DpfKey::outputs_per_leaf));
}

template <typename DpfKey,
          typename InputT = typename DpfKey::input_t,
          typename IntegralT = typename DpfKey::integral_type>
static constexpr IntegralT get_to_node(InputT to)
{
    constexpr auto to_int = to_integral_type<InputT>{};
    return quotient_ceiling(to_int(to)+1,
        static_cast<IntegralT>(DpfKey::outputs_per_leaf));
}

template <typename IntegralT>
static constexpr std::size_t get_leafnodes_in_node_interval(IntegralT from_node, IntegralT to_node)
{
    return static_cast<std::size_t>(to_node - from_node);
}

template <typename DpfKey,
          typename InputT = typename DpfKey::input_t,
          typename IntegralT = typename DpfKey::integral_type>
static constexpr std::size_t get_leafnodes_in_output_interval(InputT from, InputT to)
{
    return get_leafnodes_in_node_interval(get_from_node<DpfKey, InputT, IntegralT>(from),
        get_to_node<DpfKey, InputT, IntegralT>(to));
}

template <typename T>
struct mod_pow_2
{
    std::size_t operator()(T val, std::size_t n) const noexcept
    {
        if (n == 0)
        {
            return 0;
        }

        const uint64_t modulo_mask = static_cast<uint64_t>(~uint64_t{0}) >> bitlength_of_v<uint64_t> - n;
        return static_cast<std::size_t>(val & modulo_mask);
    }
};

template <typename T, typename Enable = void>
struct msb_of
  : public std::integral_constant<T, T{1} << bitlength_of_v<T> - 1ul> { };

template <typename T>
struct msb_of<T,
    std::enable_if_t<is_signed_integral_v<T>, void>>
  : public msb_of<std::make_unsigned_t<T>> { };

template <typename T>
static constexpr auto msb_of_v = msb_of<T>::value;

template <typename T>
struct countl_zero
{
    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    constexpr std::size_t operator()(T val) const noexcept
    {
        constexpr auto to_int = to_integral_type<T>{};
        psnip_uint64_t val_ = static_cast<psnip_uint64_t>(to_int(val));
        constexpr auto adjust = 64-bitlength_of_v<T>;
        return psnip_builtin_clz64(val_)-adjust;
    }
};

template <typename T>
struct countr_zero
{
    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    constexpr std::size_t operator()(T val) const noexcept
    {
        uint64_t val_ = static_cast<uint64_t>(val);
        constexpr auto adjust = 64-bitlength_of_v<T>;
        return psnip_builtin_ctz64(val_)-adjust;
    }
};

template <typename T>
struct countl_zero_symmetric_difference
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
struct countl_zero<simde_int128>
{
    using T = simde_int128;

    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    constexpr std::size_t operator()(const T & val) const noexcept
    {
        if (!val) return 128;
        auto limb1 = static_cast<psnip_uint64_t>(val >> 64);
        auto limb0 = static_cast<psnip_uint64_t>(val);

        return limb1 ? psnip_builtin_clz64(limb1) : 64 + psnip_builtin_clz64(limb0);
    }
};

template <typename T>
struct has_characteristic_two : public std::false_type {};

template <typename T> static constexpr auto has_characteristic_two_v = has_characteristic_two<T>::value;

template <>
struct countl_zero<simde_uint128>
{
    using T = simde_uint128;

    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    constexpr std::size_t operator()(const T & val) const noexcept
    {
        if (!val) return 128;
        auto limb1 = static_cast<psnip_uint64_t>(val >> 64);
        auto limb0 = static_cast<psnip_uint64_t>(val);

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
        auto limb1 = static_cast<psnip_uint64_t>(val[1]);
        auto limb0 = static_cast<psnip_uint64_t>(val[0]);
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
            auto limbi = static_cast<psnip_uint64_t>(val[i]);
            if (limbi)
            {
                return prefix_len + psnip_builtin_clz64(limbi);
            }
        }
        return prefix_len;
    }
};

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
//             auto limbi = static_cast<psnip_uint64_t>(val[i]);
//             if (limbi)
//             {
//                 return prefix_len + psnip_builtin_clz64(limbi)
//             }
//         }
//         return prefix_len;
//     }
// };
HEDLEY_PRAGMA(GCC diagnostic pop)

template <typename T>
struct is_xor_wrapper : std::false_type {};

template <typename T>
static constexpr bool is_xor_wrapper_v = is_xor_wrapper<T>::value;

template <typename T>
auto data(T & bar)  // NOLINT(runtime/references)
{
    return std::data(bar);
}

template <typename T>
auto data(T * bar)
{
    return bar;
}

template <typename T, typename ...Ts>
HEDLEY_ALWAYS_INLINE
constexpr auto make_tuple(T && t, Ts && ...ts) noexcept
{
    if constexpr(sizeof...(Ts) == 0)
    {
        return std::forward<T>(t);
    }
    else
    {
        return std::make_tuple(std::forward<T>(t), std::forward<Ts>(ts)...);
    }
}

template <typename>
struct is_tuple
  : std::false_type {};

template <typename ...Ts>
struct is_tuple<std::tuple<Ts...>>
  : std::true_type { };

template <typename T>
static constexpr bool is_tuple_v = is_tuple<T>::value;

template <std::size_t I, typename T>
auto & get(T & t)  // NOLINT(runtime/references)
{
    if constexpr(I == 0 && is_tuple_v<T> == false)
    {
        // if 0th value requested, return it --- even if `t` isn't a tuple
        return t;
    }
    else
    {
        // otherwise, just invoke `std::get<I>(t)` and let it succeed or fail
        // as it may
        return std::get<I>(t);
    }
}

template <typename T>
struct is_bit_array : std::false_type {};

template <typename T>
static constexpr bool is_bit_array_v = is_bit_array<T>::value;

template <typename T>
auto size(const T & t)
{
    if constexpr(is_bit_array_v<T> == false)
    {
        return std::size(t);
    }
    else
    {
        return t.data_length();
    }
}

template <typename T>
inline void flip_msb_if_signed_integral(T & x)
{
    if constexpr (is_signed_integral_v<T> == true)
    {
        x ^= static_cast<T>(msb_of_v<T>);
    }
}

template <typename InteriorNodeT,
          std::size_t Depth,
          typename LeafTupleT,
          typename WildcardMaskT>
auto get_common_part_hash(const std::array<InteriorNodeT, Depth> & correction_words,
                          const std::array<psnip_uint8_t, Depth> & correction_advice,
                          const LeafTupleT & leaf_tuple,
                          const WildcardMaskT & wildcard_mask)
{
    using zero_type = unsigned char;
    static constexpr zero_type zero{};

    SHA256 h;
    digest_type digest;

    h.add(&correction_words, sizeof(correction_words));
    h.add(&correction_advice, sizeof(correction_advice));
    std::apply([&h, &wildcard_mask](auto const & ...leaf)
    {
        std::apply([&h, &leaf...](auto ...is_wildcard)
        {
            (h.add(!is_wildcard ? reinterpret_cast<const zero_type*>(&leaf.get()) : &zero, !is_wildcard ? sizeof(leaf) : sizeof(zero)), ...);
        }, wildcard_mask);
    }, leaf_tuple);

    h.getHash(digest.data());
    return digest;
}

template <typename DpfKey>
auto get_common_part_hash(const DpfKey & dpf)
{
    return get_common_part_hash(dpf.correction_words(),
                                dpf.correction_advice(),
                                dpf.leaves(),
                                dpf.wildcard_mask);
}

template <typename OutputT, typename Enable = void>
struct has_operators_plus_minus : public std::false_type { };

template <typename OutputT>
struct has_operators_plus_minus<OutputT,
    std::enable_if_t<std::conjunction_v<std::is_member_pointer<decltype(&OutputT::operator+)>,
                                        std::is_member_pointer<decltype(&OutputT::operator-)>>, void>>
  : public std::true_type { };

template <typename OutputT>
static constexpr bool has_operators_plus_minus_v = has_operators_plus_minus<OutputT>::value;

std::size_t parity(psnip_uint8_t x)    { return psnip_builtin_parity32(static_cast<psnip_uint32_t>(x)); }
std::size_t parity(psnip_uint16_t x)   { return psnip_builtin_parity32(static_cast<psnip_uint32_t>(x)); }
std::size_t parity(psnip_uint32_t x)   { return psnip_builtin_parity32(x); }
std::size_t parity(psnip_uint64_t x)   { return psnip_builtin_parity64(x); }
std::size_t parity(simde_uint128 x)
{
    return (psnip_builtin_parity64(static_cast<psnip_uint64_t>(x))
        + psnip_builtin_parity64(static_cast<psnip_int64_t>(x >> 64))) & 1;
}

std::size_t popcount(psnip_uint8_t x)    { return psnip_builtin_popcount32(static_cast<psnip_uint32_t>(x)); }
std::size_t popcount(psnip_uint16_t x)   { return psnip_builtin_popcount32(static_cast<psnip_uint32_t>(x)); }
std::size_t popcount(psnip_uint32_t x)   { return psnip_builtin_popcount32(x); }
std::size_t popcount(psnip_uint64_t x)   { return psnip_builtin_popcount64(x); }
std::size_t popcount(simde_uint128 x)
{
    return psnip_builtin_popcount64(static_cast<psnip_uint64_t>(x))
        + psnip_builtin_popcount64(static_cast<psnip_int64_t>(x >> 64));
}

std::size_t clz(psnip_uint8_t x)    { return psnip_builtin_clz32(static_cast<psnip_uint32_t>(x)) - 24; }
std::size_t clz(psnip_uint16_t x)   { return psnip_builtin_clz32(static_cast<psnip_uint32_t>(x)) - 16; }
std::size_t clz(psnip_uint32_t x)   { return psnip_builtin_clz32(x); }
std::size_t clz(psnip_uint64_t x)   { return psnip_builtin_clz64(x); }
std::size_t clz(simde_uint128 x)
{
    return (x > UINT64_MAX) ? psnip_builtin_clz64(x >> 64) : 64 + psnip_builtin_clz64(x);
}

std::size_t ctz(psnip_uint8_t x)    { return psnip_builtin_ctz32(x | 0x100); }
std::size_t ctz(psnip_uint16_t x)   { return psnip_builtin_ctz32(x | 0x10000); }
std::size_t ctz(psnip_uint32_t x)   { return psnip_builtin_ctz32(x); }
std::size_t ctz(psnip_uint64_t x)   { return psnip_builtin_ctz64(x); }
std::size_t ctz(simde_uint128 x)
{
    return !(x & UINT64_MAX) ? 64 + psnip_builtin_ctz64(x >> 64) : psnip_builtin_ctz64(x);
}

psnip_uint8_t le(psnip_uint8_t x)   { return x; }
psnip_uint16_t le(psnip_uint16_t x) { return psnip_endian_le16(x); }
psnip_uint32_t le(psnip_uint32_t x) { return psnip_endian_le32(x); }
psnip_uint64_t le(psnip_uint64_t x) { return psnip_endian_le64(x); }
simde_uint128 le(simde_uint128 x)
{
#if PSNIP_ENDIAN_ORDER == PSNIP_ENDIAN_LITTLE
    return x;
#else
    return simde_uint128(psnip_endian_le64(x >> 64)) << 64 | psnip_endian_le64(x);
#endif
}

}  // namespace utils

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_UTILS_HPP__
