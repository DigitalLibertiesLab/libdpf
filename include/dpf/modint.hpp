/// @file dpf/modint.hpp
/// @brief defines the `dpf::modint` class and associated helpers
/// @details A `dpf::modint` is a thin wrapper around some primitive integral
///          type. The underlying value is reduced modulo `2^Nbits` only when the
///          underlying value is read; arithmetic operations have no overhead
///          relative to native operations on the underlying type.
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_MODINT_HPP__
#define LIBDPF_INCLUDE_DPF_MODINT_HPP__

#include <limits>
#include <string>
#include <memory>

#include "hedley/hedley.h"

#include "dpf/utils.hpp"

namespace dpf
{

/// @brief represents an unsigned integer modulo `2^Nbits` for small values of `Nbits`
template <std::size_t Nbits>
class modint
{
  public:
    /// @brief the primitive integral type used to represent the `modint`
    using integral_type = dpf::utils::nonvoid_integral_type_from_bitlength_t<Nbits>;

    /// @brief construct the `modint`
    /// @{

    /// @brief value constructor
    /// @details Constructs a `modint` whose value is initialized to the
    ///          smallest nonnegative integer that is congruent to `value`
    ///          modulo `2^Nbits`.
    /// @param value the value to initialize with
    HEDLEY_ALWAYS_INLINE
    // cppcheck-suppress noExplicitConstructor
    constexpr modint(integral_type value)  // NOLINT(runtime/explicit)
        : val{value}
    { }

    /// @brief default constructor
    /// @details Constructs a `modint` whose value is initialized to `0`.
    HEDLEY_ALWAYS_INLINE
    constexpr modint() noexcept = default;

    /// @brief copy constructor
    /// @details Constructs the `modint` with a value copied from another
    ///          `modint`.
    HEDLEY_ALWAYS_INLINE
    constexpr modint(const modint &) = default;

    /// @brief move constructor
    /// @details Constructs the `modint` from another `modint` using move
    ///          semantics.
    HEDLEY_ALWAYS_INLINE
    constexpr modint(modint &&) noexcept = default;

    /// @}

    /// @brief assign the `modint`
    /// @{

    /// @brief value assignment
    /// @details Sets the `modint` equal to the smallest nonnegative integer
    ///          that is congruent to `value` modulo `2^Nbits`.
    /// @param value the value to assign with
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator=(integral_type value)
    {
        this->val = value;
        return *this;
    }

    /// @brief copy assignment
    /// @details Assigns the `modint` with a value copied from another
    ///          `modint`.
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator=(const modint &) = default;

    /// @brief move assignment
    /// @details Assigns the `modint` from another `modint` using move
    ///          semantics.
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator=(modint &&) noexcept = default;

    /// @}

    ~modint() = default;

    /// @brief addition operator
    /// @{

    /// @details Performs addition with an `integral_type`.
    /// @param rhs the other addend
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator+(integral_type rhs) const noexcept
    {
        return modint{static_cast<integral_type>(this->val+rhs)};
    }

    /// @details Performs addition with another `modint`.
    /// @param rhs the other addend
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator+(modint rhs) const noexcept
    {
        return this->operator+(rhs.val);
    }

    /// @}

    /// @brief addition-assignment operator
    /// @{

    /// @details Adds an `integral_type` to this `modint`.
    /// @param rhs the other addend
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator+=(integral_type rhs) noexcept
    {
        this->val += rhs;
        return *this;
    }

    /// @details Adds another `modint` to this one.
    /// @param rhs the other addend
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator+=(modint rhs) noexcept
    {
        return this->operator+=(rhs.val);
    }

    /// @}

    /// @brief increment operator
    /// @{

    /// @brief pre-increment operator
    /// @details Increments this `modint` and returns a reference to the
    ///          result.
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator++() noexcept
    {
        return this->operator+=(integral_type{1});
    }

    /// @brief post-increment operator
    /// @details Creates a copy of this `modint`, and then increments this
    ///          `modint` and returns the copy from before the increment.
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator++(int) noexcept
    {
        auto ret = *this;
        this->operator++();
        return ret;
    }

    /// @}

    /// @brief subtraction operator
    /// @{

    /// @details Performs subtraction by an `integral_type`.
    /// @param rhs the subtrahend
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator-(integral_type rhs) const noexcept
    {
        return modint{static_cast<integral_type>(this->val-rhs)};
    }

    /// @details Performs subtraction by another `modint`.
    /// @param rhs the subtrahend
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator-(modint rhs) const noexcept
    {
        return this->operator-(rhs.val);
    }

    /// @}

    /// @brief subtraction-assignment operator
    /// @{

    /// @details Subtracts an `integral_type` from this `modint`.
    /// @param rhs the subtrahend
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator-=(integral_type rhs) noexcept
    {
        this->val -= rhs;
        return *this;
    }

    /// @details Subtracts another `modint` from this one.
    /// @param rhs the subtrahend
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator-=(modint rhs) noexcept
    {
        return this->operator-=(rhs.val);
    }

    /// @}

    /// @brief decrement operator
    /// @{

    /// @brief pre-decrement operator
    /// @details Decrements this `modint` and returns a reference to the
    ///          result.
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator--() noexcept
    {
        return this->operator-=(integral_type{1});
    }

    /// @brief post-decrement operator
    /// @details Creates a copy of this `modint`, and then decrements this
    ///          `modint` and returns the copy from before the decrement.
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator--(int) noexcept
    {
        auto ret = *this;
        this->operator--();
        return ret;
    }

    /// @}

    /// @brief bitwise-left-shift operator
    /// @details Returns a `modint` whose value is obtained by shifting this
    ///          one by `shift_amount` bits to the left. The value of `a<<b`
    ///          is therefore a `modint` congruent to `a * 2^b` modulo `2^Nbits`.
    /// @param shift_amount the number of bits to shift by
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator<<(std::size_t shift_amount) const noexcept
    {
        return modint{static_cast<integral_type>(this->val << shift_amount)};
    }

    /// @brief bitwise-left-shift-assignment operator
    /// @details Left-shifts this `modint` by `shift_amount` bits to the left
    ///          and returns a reference to the result. Upon invoking `a<<=b`,
    ///          `a` is congruent to `a * 2^b` modulo `2^Nbits`.
    /// @param shift_amount the number of bits to shift by
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator<<=(std::size_t shift_amount) noexcept
    {
        this->val <<= shift_amount;
        return *this;
    }

    /// @brief bitwise-right-shift operator
    /// @details Returns a `modint` whose value is obtained by shifting this
    ///          one by `shift_amount` bits to the right. The value of `a>>b`
    ///          is therefore a `modint` equal to the integer part of `a/2^b`.
    /// @param shift_amount the number of bits to shift by
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator>>(std::size_t shift_amount) const noexcept
    {
        return modint{static_cast<integral_type>(this->reduced_value() >> shift_amount)};
    }

    /// @brief bitwise-right-shift-assignment operator
    /// @details Right-shifts this `modint` by `shift_amount` bits to the
    ///          right and returns a reference to the result. Upon invoking
    ///          `a>>=b`, `a` is equal to the integer part of `a/2^b`.
    /// @param shift_amount the number of bits to shift by
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator>>=(std::size_t shift_amount) noexcept
    {
        this->val = this->reduced_value() >> shift_amount;
        return *this;
    }

    /// @brief multiplication operator
    /// @{

    /// @brief Multiplies this `modint` with an `integral_type`.
    /// @param rhs the other multiplicand
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator*(integral_type rhs) const noexcept
    {
        return modint{this->val*rhs};
    }

    /// @brief Multiplies another `modint` with this one.
    /// @param rhs the other multiplicand
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator*(modint rhs) const noexcept
    {
        return this->operator*(rhs.val);
    }

    /// @}

    /// @brief multiplication-assignment operator
    /// @{

    /// @details Multiplies an `integral_type` into this `modint`.
    /// @param rhs the other multiplicand
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator*=(integral_type rhs) noexcept
    {
        this->val *= rhs;
        return *this;
    }

    /// @details Multiplies another `moodnt` into this one.
    /// @param rhs the other multiplicand
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator*=(modint rhs) noexcept
    {
        return this->operator*=(rhs.val);
    }

    /// @}

    /// @brief bitwise-logical-AND operator
    /// @{
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator&(integral_type rhs) const noexcept
    {
        return modint{static_cast<integral_type>(this->val & rhs)};
    }

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator&(modint rhs) const noexcept
    {
        return this->operator&(rhs.val);
    }
    /// @}

    /// @brief bitwise-logical-AND-assignment operator
    /// @{
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator&=(integral_type rhs) noexcept
    {
        this->val &= rhs;
        return *this;
    }

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator&=(modint rhs) noexcept
    {
        return this->operator&=(rhs.val);
    }
    /// @}

    /// @brief bitwise logical OR operator
    /// @{
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator|(integral_type rhs) const noexcept
    {
        return modint{this->val | rhs};
    }

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator|(modint rhs) const noexcept
    {
        return this->operator|(rhs.val);
    }
    /// @}

    /// @brief bitwise-logical-OR-assignment operator
    /// @{
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator|=(integral_type rhs) noexcept
    {
        this->val |= rhs;
        return *this;
    }

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator|=(modint rhs) noexcept
    {
        return this->operator|=(rhs.val);
    }
    /// @}

    /// @brief bitwise logical XOR operator
    /// @{
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator^(integral_type rhs) const noexcept
    {
        return modint{this->val^rhs};
    }

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator^(modint rhs) const noexcept
    {
        return this->operator^(rhs.val);
    }
    /// @}

    /// @brief bitwise-logical-XOR-assignment operator
    /// @{
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator^=(integral_type rhs) noexcept
    {
        this->val ^= rhs;
        return *this;
    }

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator^=(modint rhs) noexcept
    {
        return this->operator^=(rhs.val);
    }
    /// @}

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator~() const noexcept
    {
        return modint{~this->val};
    }

    /// @brief convert this `modint` to the equivalent `integeral_type`
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr explicit operator integral_type() const noexcept
    {
        return this->reduced_value();
    }

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr integral_type data() const noexcept
    {
        return this->val;
    }

    template <typename CharT,
              class Traits = std::char_traits<CharT>,
              class Allocator = std::allocator<CharT>>
    friend std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> & os,
        const modint & i)
    {
        return os << i.reduced_value();
    }

    template <typename CharT,
              class Traits = std::char_traits<CharT>,
              class Allocator = std::allocator<CharT>>
    friend std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT, Traits> & is,
        const modint & i)
    {
        return is >> i.val;
    }

    constexpr operator bool() const noexcept
    {
        return static_cast<bool>(this->reduced_value());
    }

    constexpr integral_type reduced_value() const
    {
        return val & modulo_mask;
    }

  private:
    /// @brief bitmask used for performing reductions modulo `2^Nbits`
    static constexpr integral_type modulo_mask = ~integral_type{0} >> utils::bitlength_of_v<integral_type> - Nbits;

    /// @brief The `integral_type` used to represent this `modint`
    integral_type val;

    friend struct utils::mod_pow_2<modint>;
};

/// @brief Multiplies a `modint<Nbits>` with an `modint::integral_type`.
/// @param lhs the `integral_type` multiplicand
/// @param rhs the `modint` multiplicand
template <std::size_t Nbits>
HEDLEY_CONST
HEDLEY_ALWAYS_INLINE
constexpr bool operator*(typename modint<Nbits>::integral_type lhs,
    modint<Nbits> rhs) noexcept
{
    return modint<Nbits>{lhs
        * static_cast<typename modint<Nbits>::integral_type>(rhs)};
}

/// @brief Compare two `modint<Nbits>`s as if they were regular integers
/// @{

/// @brief less-than operator
template <std::size_t Nbits>
HEDLEY_CONST
HEDLEY_ALWAYS_INLINE
constexpr bool operator<(modint<Nbits> lhs, modint<Nbits> rhs) noexcept
{
    return static_cast<typename modint<Nbits>::integral_type>(lhs)
        < static_cast<typename modint<Nbits>::integral_type>(rhs);
}

/// @brief less-than-or-equal-to operator
template <std::size_t Nbits>
HEDLEY_CONST
HEDLEY_ALWAYS_INLINE
constexpr bool operator<=(modint<Nbits> lhs, modint<Nbits> rhs) noexcept
{
    return static_cast<typename modint<Nbits>::integral_type>(lhs)
        <= static_cast<typename modint<Nbits>::integral_type>(rhs);
}

/// @brief greater-than operator
template <std::size_t Nbits>
HEDLEY_CONST
HEDLEY_ALWAYS_INLINE
constexpr bool operator>(modint<Nbits> lhs, modint<Nbits> rhs) noexcept
{
    return static_cast<typename modint<Nbits>::integral_type>(lhs)
        > static_cast<typename modint<Nbits>::integral_type>(rhs);
}

/// @brief greater-than-or-equal-to operator
template <std::size_t Nbits>
HEDLEY_CONST
HEDLEY_ALWAYS_INLINE
constexpr bool operator>=(modint<Nbits> lhs, modint<Nbits> rhs) noexcept
{
    return static_cast<typename modint<Nbits>::integral_type>(lhs)
        >= static_cast<typename modint<Nbits>::integral_type>(rhs);
}

/// @brief equality operator
template <std::size_t Nbits>
HEDLEY_CONST
HEDLEY_ALWAYS_INLINE
constexpr bool operator==(modint<Nbits> lhs, modint<Nbits> rhs) noexcept
{
    return static_cast<typename modint<Nbits>::integral_type>(lhs)
        == static_cast<typename modint<Nbits>::integral_type>(rhs);
}

/// @brief inequality operator
template <std::size_t Nbits>
HEDLEY_CONST
HEDLEY_ALWAYS_INLINE
constexpr bool operator!=(modint<Nbits> lhs, modint<Nbits> rhs) noexcept
{
    return static_cast<typename modint<Nbits>::integral_type>(lhs)
        != static_cast<typename modint<Nbits>::integral_type>(rhs);
}

/// @}

namespace utils
{

template <std::size_t Nbits>
struct bitlength_of<dpf::modint<Nbits>>
  : public std::integral_constant<std::size_t, Nbits> { };

template <std::size_t Nbits>
struct msb_of<dpf::modint<Nbits>>
{
    static constexpr dpf::modint<Nbits> value
        = dpf::modint<Nbits>{1} << bitlength_of_v<dpf::modint<Nbits>> - 1ul;
};

template <std::size_t Nbits>
struct countl_zero_symmetric_difference<dpf::modint<Nbits>>
{
    using T = dpf::modint<Nbits>;
    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    constexpr std::size_t operator()(T lhs, T rhs) const noexcept
    {
        using T = typename dpf::modint<Nbits>::integral_type;
        constexpr auto xor_op = std::bit_xor<T>{};
        constexpr auto adjust = bitlength_of_v<T> - Nbits;
        auto diff = xor_op(static_cast<T>(lhs), static_cast<T>(rhs));

        if constexpr (std::is_same_v<T, simde_uint128>)
        {
            uint64_t diff_hi = static_cast<uint64_t>(diff >> 64);
            uint64_t diff_lo = static_cast<uint64_t>(diff);

            return diff_hi ? psnip_builtin_clz64(diff_hi)-adjust : 64+psnip_builtin_clz64(diff_lo)-adjust;
        }
        else
        {
            return psnip_builtin_clz64(static_cast<uint64_t>(diff))-adjust;
        }
    }
};

template <std::size_t Nbits>
struct mod_pow_2<dpf::modint<Nbits>>
{
    using T = dpf::modint<Nbits>;
    std::size_t operator()(T val, std::size_t n) const noexcept
    {
        return static_cast<std::size_t>(val.val % (1ul << n));
    }
};

}  // namespace utils

}  // namespace dpf

namespace std
{

/// @brief specializes `std::numeric_limits` for CV-qualified `dpf::modint`s
/// @{

/// @details specializes `std::numeric_limits` for `dpf::modint<Nbits>`
template<std::size_t Nbits>
class numeric_limits<dpf::modint<Nbits>>
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
    static constexpr int digits = Nbits;
    static constexpr int digits10 = Nbits * std::log10(2);  //< correct if `Nbits<129`
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int max_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool traps
        = std::numeric_limits<typename dpf::modint<Nbits>::integral_type>::traps;
    static constexpr bool tinyness_before = false;

    static constexpr dpf::modint<Nbits> min() noexcept { return dpf::modint<Nbits>{0}; }
    static constexpr dpf::modint<Nbits> lowest() noexcept { return dpf::modint<Nbits>{0}; }
    static constexpr dpf::modint<Nbits> max() noexcept { return dpf::modint<Nbits>{-1}; }
    static constexpr dpf::modint<Nbits> epsilon() noexcept { return 0; }
    static constexpr dpf::modint<Nbits> round_error() noexcept { return 0; }
    static constexpr dpf::modint<Nbits> infinity() noexcept { return 0; }
    static constexpr dpf::modint<Nbits> quiet_NaN() noexcept { return 0; }
    static constexpr dpf::modint<Nbits> signaling_NaN() noexcept { return 0; }
    static constexpr dpf::modint<Nbits> denorm_min() noexcept { return 0; }
};

/// @details specializes `std::numeric_limits` for `dpf::modint<Nbits> const`
template<std::size_t Nbits>
class numeric_limits<dpf::modint<Nbits> const>
  : public numeric_limits<dpf::modint<Nbits>> {};

/// @details specializes `std::numeric_limits` for
///          `dpf::modint<Nbits> volatile`
template<std::size_t Nbits>
class numeric_limits<dpf::modint<Nbits> volatile>
  : public numeric_limits<dpf::modint<Nbits>> {};

/// @details specializes `std::numeric_limits` for
///          `dpf::modint<Nbits> const volatile`
template<std::size_t Nbits>
class numeric_limits<dpf::modint<Nbits> const volatile>
  : public numeric_limits<dpf::modint<Nbits>> {};

/// @}

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_MODINT_HPP__
