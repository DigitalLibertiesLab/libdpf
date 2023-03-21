/// @file dpf/wildcard.hpp
/// @brief defines the `dpf::wildcard_value` template and associated helpers
/// @details A `dpf::wildcard` is a struct template with a single parameter
///          `T`, which must be a trivially copyable type (as indicated by
///          `std::is_trivially_copyable<T>`). It is used as a placeholder
///          for an instance of type `T`, which can be assigned later. Its
///          intended to wrap an [output type](@ref output_types) of a DPF.
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_WILDCARD_HPP__
#define LIBDPF_INCLUDE_DPF_WILDCARD_HPP__

#include <cstdint>
#include <limits>
#include <optional>

#include "dpf/bit.hpp"
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
    inline constexpr wildcard_value() noexcept = default;
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
    static constexpr auto bit = wildcard<dpf::bit>;
    static constexpr auto boolean = wildcard<bool>;

    static constexpr auto signed_char = wildcard<signed char>;
    static constexpr auto uchar = wildcard<unsigned char>;
    static constexpr auto xchar = wildcard<dpf::xor_wrapper<unsigned char>>;

    static constexpr auto int8 = wildcard<psnip_int8_t>;
    static constexpr auto uint8 = wildcard<psnip_uint8_t>;
    static constexpr auto xint8 = wildcard<dpf::xor_wrapper<psnip_uint8_t>>;

    static constexpr auto int16 = wildcard<psnip_int16_t>;
    static constexpr auto uint16 = wildcard<psnip_uint16_t>;
    static constexpr auto xint16 = wildcard<dpf::xor_wrapper<psnip_uint16_t>>;

    static constexpr auto int32 = wildcard<psnip_int32_t>;
    static constexpr auto uint32 = wildcard<psnip_uint32_t>;
    static constexpr auto xint32 = wildcard<dpf::xor_wrapper<psnip_uint32_t>>;

    static constexpr auto int64 = wildcard<psnip_int64_t>;
    static constexpr auto uint64 = wildcard<psnip_uint64_t>;
    static constexpr auto xint64 = wildcard<dpf::xor_wrapper<psnip_uint64_t>>;

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    static constexpr auto int128 = wildcard<simde_int128>;
    static constexpr auto uint128 = wildcard<simde_uint128>;
    // static constexpr auto xint128 = wildcard<dpf::xor_wrapper<simde_uint128>>;

    static constexpr auto m128 = wildcard<simde__m128>;
    static constexpr auto m128i = wildcard<simde__m128i>;
    static constexpr auto m128d = wildcard<simde__m128d>;

    static constexpr auto m256 = wildcard<simde__m256>;
    static constexpr auto m256i = wildcard<simde__m256i>;
    static constexpr auto m256d = wildcard<simde__m256d>;

    // static constexpr auto m512 = wildcard<simde__m512>;
    // static constexpr auto m512i = wildcard<simde__m512i>;
    // static constexpr auto m512d = wildcard<simde__m512d>;
HEDLEY_PRAGMA(GCC diagnostic pop)

    static constexpr auto ieee_float = wildcard<float>;
    static constexpr auto ieee_double = wildcard<double>;

}  // namespace wildcards

namespace utils
{

/// @brief specializes `dpf::utils::bitlength_of` for `dpf::wildcard_value`
template <typename T>
struct bitlength_of<wildcard_value<T>>
  : public bitlength_of<T>
{ };

}  // namespace utils

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_WILDCARD_HPP__
