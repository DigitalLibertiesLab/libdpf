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

#define DPF_UNROLL_LOOP_N(N) HEDLEY_PRAGMA(GCC unroll N)
#define DPF_UNROLL_LOOP DPF_UNROLL_LOOP_N(16)
#define DPF_ALWAYS_VECTORIZE (#pragma GCC ivdep)

namespace dpf
{

namespace utils
{

/// @brief Ugly hack to implement `constexpr`-friendly conditional `throw`
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
    return simde_mm_slli_epi64(simde_mm_set_epi64x(uint64_t(i >= 64), uint64_t(i <= 63)), i % 64);
}

template <>
HEDLEY_ALWAYS_INLINE
HEDLEY_PURE
HEDLEY_NO_THROW
simde__m256i single_bit_mask<simde__m256i>(std::size_t i)
{
    return simde_mm256_slli_epi64(simde_mm256_set_epi64x(uint64_t(i >= 192),
                                     uint64_t(i >= 128 && i <= 191),
                                     uint64_t(i >= 64 && i <= 127),
                                     uint64_t(i <= 63)), i % 64);
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

/// @brief the primitive integral type used to represent non integral types
template <std::size_t Nbits,
          std::size_t MinBits = Nbits,
          std::size_t MaxBits = std::max(Nbits, MinBits)>
struct integral_type_from_bitlength
{
    static_assert(MinBits <= MaxBits);
    static constexpr auto effective_Nbits = std::min(std::max(Nbits, MinBits), MaxBits);
    static constexpr auto less_equal = std::less_equal<void>{};
    using type = std::conditional_t<less_equal(effective_Nbits, 128),
        std::conditional_t<less_equal(effective_Nbits, 64),
            std::conditional_t<less_equal(effective_Nbits, 32),
                std::conditional_t<less_equal(effective_Nbits, 16),
                    std::conditional_t<less_equal(effective_Nbits, 8), psnip_uint8_t,
                    psnip_uint16_t>,
                psnip_uint32_t>,
            psnip_uint64_t>,
        simde_uint128>,
    void>;
};

template <std::size_t Nbits,
          std::size_t MinBits = Nbits,
          std::size_t MaxBits = std::max(Nbits, MinBits)>
using integral_type_from_bitlength_t = typename integral_type_from_bitlength<Nbits, MinBits>::type;

/// @brief the primitive integral type used to represent non integral types
template <std::size_t Nbits,
          std::size_t MinBits = Nbits,
          std::size_t MaxBits  = std::max(std::size_t(128), MinBits)>
struct nonvoid_integral_type_from_bitlength : public integral_type_from_bitlength<Nbits, MinBits>
{
    static_assert(Nbits && Nbits <= MaxBits, "representation must fit in 128 bits");
};

template <std::size_t Nbits,
          std::size_t MinBits = Nbits,
          std::size_t MaxBits = std::max(std::size_t(128), MinBits)>
using nonvoid_integral_type_from_bitlength_t = typename nonvoid_integral_type_from_bitlength<Nbits, MinBits>::type;

template <typename T>
struct to_integral_type_base
{
    static constexpr std::size_t bits = bitlength_of_v<T>;
    // Select integer type larger than or equal to size of std::size_t
    using integral_type = integral_type_from_bitlength_t<bits, bitlength_of_v<std::size_t>>;
    static_assert(!std::is_void_v<integral_type>, "cannot convert to void type");
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
    using integral_type = std::conditional_t<std::is_void_v<T_integral_type>, simde_uint128, T_integral_type>;
    constexpr T operator()(integral_type val) const noexcept
    {
        return T{val};
    }
};

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
static constexpr std::size_t get_nodes_in_interval_impl(IntegralT from_node, IntegralT to_node)
{
    return static_cast<std::size_t>(to_node - from_node);
}

template <typename DpfKey,
          typename InputT = typename DpfKey::input_t,
          typename IntegralT = typename DpfKey::integral_type>
static constexpr std::size_t get_nodes_in_interval(InputT from, InputT to)
{
    return get_nodes_in_interval_impl(get_from_node<DpfKey, InputT, IntegralT>(from),
        get_to_node<DpfKey, InputT, IntegralT>(to));
}

template <typename T>
struct mod_pow_2
{
    std::size_t operator()(T val, std::size_t n) const noexcept
    {
        return static_cast<std::size_t>(val % (1ul << n));
    }
};

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

template <typename T>
struct is_xor_wrapper : std::false_type {};

template <typename T>
static constexpr bool is_xor_wrapper_v = is_xor_wrapper<T>::value;

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

template <typename ...Ts>
HEDLEY_ALWAYS_INLINE
constexpr auto remove_tuple_if_trivial(std::tuple<Ts...> && ts) noexcept
{
    if constexpr(sizeof...(Ts) == 1) { return std::move(std::get<0>(ts)); }
    else { return std::forward<std::tuple<Ts...>>(ts); }
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
auto & get(T & t)
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


}  // namespace utils

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_UTILS_HPP__
