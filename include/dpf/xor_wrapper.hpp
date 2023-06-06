/// @file dpf/xor_wrapper.hpp
/// @brief defines the `dpf::xor_wrapper` class and associated helpers
/// @details A `dpf::xor_wrapper` is a struct template that adapts integral
///          types to use bitwise arithmetic; that is, it makes an `N`-bit
///          integer type behave as it it were an element of `GF(2)^N`.
///          Specifically,
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_XOR_WRAPPER_HPP__
#define LIBDPF_INCLUDE_DPF_XOR_WRAPPER_HPP__

#include <cstddef>
#include <type_traits>
#include <functional>
#include <limits>
#include <ostream>
#include <istream>

#include "thirdparty/simde/x86/avx2.h"
#include "thirdparty/psnips/exact-int/exact-int.h"

namespace dpf
{

template <typename T>
struct xor_wrapper
{
  public:
    using value_type = T;
    static constexpr auto bit_xor = std::bit_xor<value_type>{};
    static constexpr auto bit_and = std::bit_and<value_type>{};
    static constexpr auto bit_or = std::bit_or<value_type>{};
    static constexpr auto bit_not = std::bit_not<value_type>{};

    static constexpr std::size_t bits = utils::bitlength_of_v<value_type>;
    using integral_type = utils::integral_type_from_bitlength_t<bits>;

    /// @{

    /// @brief Default c'tor
    constexpr xor_wrapper() = default;

    /// @brief Copy c'tor
    constexpr xor_wrapper(const xor_wrapper &) noexcept = default;

    /// @brief Move c'tor
    constexpr xor_wrapper(xor_wrapper &&) noexcept = default;

    /// @brief Value c'tor
    // cppcheck-suppress noExplicitConstructor
    constexpr xor_wrapper(T v) noexcept : value{v} { }  // NOLINT(runtime/explicit)

    /// @}

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

    ~xor_wrapper() = default;

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
    constexpr operator bool() const noexcept
    {
        return static_cast<bool>(value);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr operator T() const noexcept { return value; }

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

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr bool operator<(xor_wrapper rhs) const noexcept
    {
        return value < rhs.value;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr bool operator<=(xor_wrapper rhs) const noexcept
    {
        return value <= rhs.value;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr bool operator>(xor_wrapper rhs) const noexcept
    {
        return value > rhs.value;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr bool operator>=(xor_wrapper rhs) const noexcept
    {
        return value >= rhs.value;
    }

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr value_type data() const noexcept
    {
        return value;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr xor_wrapper & operator<<=(std::size_t amount)
    {
        this->value <<= amount;
        return *this;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr xor_wrapper & operator>>=(std::size_t amount)
    {
        this->value >>= amount;
        return *this;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr xor_wrapper & operator++() noexcept
    {
        ++value;
        return *this;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr xor_wrapper operator++(int) noexcept
    {
        auto ret = *this;
        this->operator++();
        return ret;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr xor_wrapper & operator--() noexcept
    {
        --value;
        return *this;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr xor_wrapper operator--(int) noexcept
    {
        auto ret = *this;
        this->operator--();
        return ret;
    }

  private:
    value_type value;

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator+(const xor_wrapper & lhs,
        const xor_wrapper & rhs) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_xor(lhs.value, rhs.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator-(const xor_wrapper & lhs,
        const xor_wrapper & rhs) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_xor(lhs.value, rhs.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator*(const xor_wrapper & lhs,
        const xor_wrapper & rhs) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_and(lhs.value, rhs.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator&(const xor_wrapper & lhs,
        const xor_wrapper & rhs) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_and(lhs.value, rhs.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator|(const xor_wrapper & lhs,
        const xor_wrapper & rhs) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_or(lhs.value, rhs.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator^(const xor_wrapper & lhs,
        const xor_wrapper & rhs) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_xor(lhs.value, rhs.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator~(const xor_wrapper & val) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_not(val.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator<<(const xor_wrapper & val, std::size_t amount)
    {
        return xor_wrapper(val.value << amount);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator>>(const xor_wrapper & val, std::size_t amount)
    {
        return xor_wrapper(val.value >> amount);
    }

    friend std::ostream & operator<<(std::ostream & os, xor_wrapper<T> val)
    {
        return os << val.value;
    }

    friend std::istream & operator<<(std::istream & is, xor_wrapper<T> & val)
    {
        return is >> val.value;
    }

    friend struct utils::to_integral_type<xor_wrapper>;
    friend struct utils::mod_pow_2<xor_wrapper>;
};

using xint128_t = xor_wrapper<simde_uint128>;  ///< `xor_wrapper<simde_uint128>`
using xint64_t = xor_wrapper<psnip_uint64_t>;  ///< `xor_wrapper<psnip_uint64_t>`
using xint32_t = xor_wrapper<psnip_uint32_t>;  ///< `xor_wrapper<psnip_uint32_t>`
using xint16_t = xor_wrapper<psnip_uint16_t>;  ///< `xor_wrapper<psnip_uint16_t>`
using xint8_t = xor_wrapper<psnip_uint8_t>;    ///< `xor_wrapper<psnip_uint8_t>`
using xchar_t = xor_wrapper<unsigned char>;    ///< `xor_wrapper<unsigned char>`

namespace utils
{

template <typename T>
struct is_xor_wrapper<xor_wrapper<T>> : std::true_type {};

/// @brief specializes `dpf::utils::bitlength_of` for `dpf::xor_wrapper`
template <typename T>
struct bitlength_of<xor_wrapper<T>>
  : public bitlength_of<T>
{ };

template <typename T>
struct msb_of<xor_wrapper<T>>
{
    static constexpr xor_wrapper<T> value = xor_wrapper<T>{T{1} << bitlength_of_v<T> - 1ul};
};

template <typename T>
struct countl_zero_symmetric_difference<xor_wrapper<T>>
  : public countl_zero_symmetric_difference<T>
{ };

template <typename T>
struct to_integral_type<xor_wrapper<T>> : public to_integral_type_base<T>
{
    using parent = to_integral_type_base<T>;
    using typename parent::integral_type;
    static constexpr auto to_integral_type_cast = to_integral_type<T> {};

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr integral_type operator()(const xor_wrapper<T> & input) const noexcept
    {
        return to_integral_type_cast(input.value);
    }
};

template <typename T>
struct mod_pow_2<xor_wrapper<T>>
{
    static constexpr auto mod = mod_pow_2<T>{};
    std::size_t operator()(xor_wrapper<T> val, std::size_t n) const noexcept
    {
        return mod(val.value, n);
    }
};

}  // namespace utils

}  // namespace dpf

namespace std
{

/// @brief specializes `std::numeric_limits` for CV-qualified `dpf::xor_wrapper`s
/// @{

/// @details specializes `std::numeric_limits` for `dpf::xor_wrapper<T>`
template<typename T>
class numeric_limits<dpf::xor_wrapper<T>>
  : public numeric_limits<T> {};

/// @details specializes `std::numeric_limits` for `dpf::xor_wrapper<T> const`
template<typename T>
class numeric_limits<dpf::xor_wrapper<T> const>
  : public numeric_limits<dpf::xor_wrapper<T>> {};

/// @details specializes `std::numeric_limits` for
///          `dpf::xor_wrapper<T> volatile`
template<typename T>
class numeric_limits<dpf::xor_wrapper<T> volatile>
  : public numeric_limits<dpf::xor_wrapper<T>> {};

/// @details specializes `std::numeric_limits` for
///          `dpf::xor_wrapper<T> const volatile`
template<typename T>
class numeric_limits<dpf::xor_wrapper<T> const volatile>
  : public numeric_limits<dpf::xor_wrapper<T>> {};

/// @}

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_XOR_WRAPPER_HPP__
