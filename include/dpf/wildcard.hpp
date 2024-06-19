/// @file dpf/wildcard.hpp
/// @brief defines the `dpf::wildcard_value` template and associated helpers
/// @details A `dpf::wildcard` is a struct template with a single parameter
///          `T`, which must be a trivially copyable type (as indicated by
///          `std::is_trivially_copyable<T>`). It is used as a placeholder
///          for an instance of type `T`, which can be assigned later. Its
///          intended to wrap an [output type](@ref output_types) of a DPF.
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_WILDCARD_HPP__
#define LIBDPF_INCLUDE_DPF_WILDCARD_HPP__

#include <type_traits>
#include <limits>
#include <array>
#include <optional>

#include "dpf/bit.hpp"
#include "dpf/bitstring.hpp"
#include "dpf/random.hpp"
#include "dpf/xor_wrapper.hpp"

#include "simde/simde/x86/avx2.h"
// #include "simde/simde/x86/avx512.h"

namespace dpf
{

// namespace
// {

/// @brief represents a placeholder value of a given type, with a concrete value to be assigned later
/// @tparam T the underlying type
template <typename T>
struct wildcard_value
{
    static_assert(std::is_trivially_copyable_v<T>,
        "T must be a trivially copyable type");
    static_assert(std::numeric_limits<T>::is_iec559 ||
        !(std::is_same_v<T, float> || std::is_same_v<T, double>),
        "floating point types only supported for iec559");
    inline constexpr wildcard_value() noexcept : val{std::nullopt} { }
    inline constexpr wildcard_value(T && t) noexcept : val{t} { }

    HEDLEY_ALWAYS_INLINE
    constexpr auto operator()(T && t) const
    {
        return wildcard_value(std::forward<T>(t));
    }

    HEDLEY_ALWAYS_INLINE
    auto operator()() const
    {
        auto t = val.value_or(dpf::uniform_sample<T>());
        return std::tuple_cat(std::make_tuple(t), dpf::additively_share(t));
    }

  private:
    std::optional<T> val;
};

// }  // namespace

template <typename T> static constexpr wildcard_value<T> wildcard{};

/// @brief Checks whether `T` is a wildcard type.
/// @details A trait class that provides the member constant `value` which is
///          equal to `true`, if `T` is a specialization of the `wildcard_value`
///          template and `false` otherwise.
/// @see dpf::is_wildcard_v
template <typename T> struct is_wildcard : std::false_type { };
template <typename T> struct is_wildcard<wildcard_value<T>> : std::true_type { };

/// @brief Checks whether `T` is a wildcard type.
template <typename T> constexpr bool is_wildcard_v = is_wildcard<T>::value;

template <typename T> struct concrete_type { using type = T; };
template <typename T> struct concrete_type<wildcard_value<T>>
    : public concrete_type<T> { };
template <typename T> using concrete_type_t = typename concrete_type<T>::type;

template <typename T> struct concrete_value
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    constexpr std::optional<T> operator()(T y) const noexcept
    {
        return y;
    }
};
template <typename T> struct concrete_value<wildcard_value<T>>
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    constexpr std::optional<T> operator()(wildcard_value<T>) const noexcept
    {
        return std::nullopt;
    }
};
template <typename T> constexpr auto concrete_value_v = concrete_value<T>{};

namespace wildcards
{
    using bit_t = wildcard_value<dpf::bit>;
    static constexpr auto bit = wildcard<dpf::bit>;

    using signed_char_t = wildcard_value<signed char>;
    static constexpr auto signed_char = wildcard<signed char>;
    using uchar_t = wildcard_value<unsigned char>;
    static constexpr auto uchar = wildcard<unsigned char>;
    using xchar_t = wildcard_value<dpf::xor_wrapper<unsigned char>>;
    static constexpr auto xchar = wildcard<dpf::xor_wrapper<unsigned char>>;

    using int8_t = wildcard_value<psnip_int8_t>;
    static constexpr auto int8 = wildcard<psnip_int8_t>;
    using uint8_t = wildcard_value<psnip_uint8_t>;
    static constexpr auto uint8 = wildcard<psnip_uint8_t>;
    using xint8_t = wildcard_value<dpf::xints::xint8_t>;
    static constexpr auto xint8 = wildcard<dpf::xints::xint8_t>;

    using int16_t = wildcard_value<psnip_int16_t>;
    static constexpr auto int16 = wildcard<psnip_int16_t>;
    using uint16_t = wildcard_value<psnip_uint16_t>;
    static constexpr auto uint16 = wildcard<psnip_uint16_t>;
    using xint16_t = wildcard_value<dpf::xints::xint16_t>;
    static constexpr auto xint16 = wildcard<dpf::xints::xint16_t>;

    using int32_t = wildcard_value<psnip_int32_t>;
    static constexpr auto int32 = wildcard<psnip_int32_t>;
    using uint32_t = wildcard_value<psnip_uint32_t>;
    static constexpr auto uint32 = wildcard<psnip_uint32_t>;
    using xint32_t = wildcard_value<dpf::xints::xint32_t>;
    static constexpr auto xint32 = wildcard<dpf::xints::xint32_t>;

    using int64_t = wildcard_value<psnip_int64_t>;
    static constexpr auto int64 = wildcard<psnip_int64_t>;
    using uint64_t = wildcard_value<psnip_uint64_t>;
    static constexpr auto uint64 = wildcard<psnip_uint64_t>;
    using xint64_t = wildcard_value<dpf::xints::xint64_t>;
    static constexpr auto xint64 = wildcard<dpf::xints::xint64_t>;

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using __int128_t = wildcard_value<simde_int128>;  //< gcc builtin `__int128` type
    static constexpr auto _int128 = wildcard<simde_int128>;
    using __uint128_t = wildcard_value<simde_uint128>;  //< gcc builtin `unsigned __int128` type
    static constexpr auto _uint128 = wildcard<simde_uint128>;
    using uint128_t = wildcard_value<::uint128_t>;  //< `uint128_t` type
    static constexpr auto uint128 = wildcard<::uint128_t>;
    using xint128_t = wildcard_value<dpf::xints::xint128_t>;
    static constexpr auto xint128 = wildcard<dpf::xints::xint128_t>;

    using uint256_t = wildcard_value<::uint256_t>;  //< `uint256_t` type
    static constexpr auto uint256 = wildcard<::uint256_t>;
    using xint256_t = wildcard_value<dpf::xints::xint256_t>;
    static constexpr auto xint256 = wildcard<dpf::xints::xint256_t>;

    template <std::size_t Nbits> using bitstring_t = wildcard_value<dpf::bitstring<Nbits>>;
    template <std::size_t Nbits> static constexpr auto bitstring = wildcard<dpf::bitstring<Nbits>>;

    template <std::size_t Nbits> using xint_t = wildcard_value<dpf::xint<Nbits>>;
    template <std::size_t Nbits> static constexpr auto xint = wildcard<dpf::xint<Nbits>>;
    
    template <std::size_t Nbits> using modint_t = wildcard_value<dpf::modint<Nbits>>;
    template <std::size_t Nbits> static constexpr auto modint = wildcard<dpf::modint<Nbits>>;

    using m128_t = wildcard_value<simde__m128>;
    static constexpr auto m128 = wildcard<simde__m128>;
    using m128i_t = wildcard_value<simde__m128i>;
    static constexpr auto m128i = wildcard<simde__m128i>;
    using m128d_t = wildcard_value<simde__m128d>;
    static constexpr auto m128d = wildcard<simde__m128d>;

    using m256_t = wildcard_value<simde__m256>;
    static constexpr auto m256 = wildcard<simde__m256>;
    using m256i_t = wildcard_value<simde__m256i>;
    static constexpr auto m256i = wildcard<simde__m256i>;
    using m256d_t = wildcard_value<simde__m256d>;
    static constexpr auto m256d = wildcard<simde__m256d>;

    // using m512_t = wildcard_value<simde__m512>;
    // static constexpr auto m512 = wildcard<simde__m512>;
    // using m512i_t = wildcard_value<simde__m512i>;
    // static constexpr auto m512i = wildcard<simde__m512i>;
    // using m512d_t = wildcard_value<simde__m512d>;
    // static constexpr auto m512d = wildcard<simde__m512d>;
HEDLEY_PRAGMA(GCC diagnostic pop)

    using ieee_float_t = wildcard_value<float>;
    static constexpr auto ieee_float = wildcard<float>;
    using ieee_double_t = wildcard_value<double>;
    static constexpr auto ieee_double = wildcard<double>;

}  // namespace wildcards

template <std::size_t I,
          typename DpfKey>
void assert_wildcard_output(const DpfKey & dpf)
{
    if (HEDLEY_UNLIKELY(std::get<I>(dpf.leaves).is_ready()))
    {
        throw std::runtime_error("output not an unassigned wildcard");
    }
}

template <typename DpfKey>
void assert_wildcard_input(const DpfKey & dpf)
{
    if (HEDLEY_UNLIKELY(dpf.offset_x.is_ready()))
    {
        throw std::runtime_error("input is not an unassigned wildcard");
    }
}

template <std::size_t ...Is,
          typename DpfKey>
HEDLEY_ALWAYS_INLINE
void assert_not_wildcard_output(const DpfKey & dpf)
{
    if (HEDLEY_UNLIKELY(!std::get<Is>(dpf.leaf_nodes).is_ready() || ...))
    {
        throw std::runtime_error("one or more outputs is an unassigned wildcard");
    }
}

template <typename DpfKey>
HEDLEY_ALWAYS_INLINE
void assert_not_wildcard_input(const DpfKey & dpf)
{
    if (HEDLEY_UNLIKELY(!dpf.offset_x.is_ready()))
    {
        throw std::runtime_error("input is unassigned wildcard");
    }
}

namespace utils
{

/// @brief specializes `dpf::utils::bitlength_of` for `dpf::wildcard_value`
template <typename T>
struct bitlength_of<wildcard_value<T>>
  : public bitlength_of<T>
{ };

/// @brief specializes `dpf::utils::bitlength_of_output` for `dpf::wildcard_value`
template <typename T,
          typename NodeT>
struct bitlength_of_output<wildcard_value<T>, NodeT>
  : public bitlength_of_output<T, NodeT>
{ };

template <typename T>
struct make_default<wildcard_value<T>>
{
    static constexpr wildcard_value<T> value = wildcard_value<T>();
};

}  // namespace dpf::utils

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_WILDCARD_HPP__
