/// @file dpf/xor_wrapper.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @version 1.0
/// @date 2022-07-24
///
/// @copyright Copyright (c) 2019-2022 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_XOR_WRAPPER_HPP__
#define LIBDPF_INCLUDE_DPF_XOR_WRAPPER_HPP__

#include <functional>

#include "simde/simde/x86/avx2.h"
#include "portable-snippets/exact-int/exact-int.h"

namespace dpf
{

template <typename T>
struct xor_wrapper
{
  public:
    using value_type = T;
    static constexpr auto bit_xor = std::bit_xor<T>{};
    // static constexpr auto bit_and = std::bit_and<T>{};

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr xor_wrapper() noexcept = default;

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr explicit xor_wrapper(const xor_wrapper &) noexcept = default;

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr explicit xor_wrapper(xor_wrapper &&) noexcept = default;

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr explicit xor_wrapper(T v) noexcept : value{v} { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr xor_wrapper & operator=(const xor_wrapper &) noexcept = default;

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr xor_wrapper & operator=(xor_wrapper &&) noexcept = default;

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr xor_wrapper & operator=(T v) noexcept
    {
        value = v;
        return *this;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr xor_wrapper operator-() const noexcept
    {
        return xor_wrapper{value};
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr explicit operator T() const noexcept { return value; }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr bool operator==(xor_wrapper rhs) const noexcept
    {
        return value == rhs.value;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr bool operator!=(xor_wrapper rhs) const noexcept
    {
        return value != rhs.value;
    }

  private:
    T value;
};

template <typename T>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
constexpr xor_wrapper<T> operator+(const xor_wrapper<T> & lhs,
    const xor_wrapper<T> & rhs) noexcept
{
    return xor_wrapper<T>::bit_xor(lhs.value, rhs.value);
}

template <typename T>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
constexpr xor_wrapper<T> operator-(const xor_wrapper<T> & lhs,
    const xor_wrapper<T> & rhs) noexcept
{
    return xor_wrapper<T>::bit_xor(lhs.value, rhs.value);
}

template <typename T>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
constexpr xor_wrapper<T> operator*(const xor_wrapper<T> & lhs,
    const xor_wrapper<T> & rhs) noexcept
{
    return xor_wrapper<T>::bit_and(lhs.value, rhs.value);
}

using xint128_t = xor_wrapper<simde_uint128>;  ///< `xor_wrapper<simde_uint128>`
using xint64_t = xor_wrapper<psnip_uint64_t>;  ///< `xor_wrapper<psnip_uint64_t>`
using xint32_t = xor_wrapper<psnip_uint32_t>;  ///< `xor_wrapper<psnip_uint32_t>`
using xint16_t = xor_wrapper<psnip_uint16_t>;  ///< `xor_wrapper<psnip_uint16_t>`
using xint8_t = xor_wrapper<psnip_uint8_t>;    ///< `xor_wrapper<psnip_uint8_t>`
using xchar_t = xor_wrapper<unsigned char>;    ///< `xor_wrapper<unsigned char>`

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_XOR_WRAPPER_HPP__
