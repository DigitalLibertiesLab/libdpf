/// @file dpf/fixedpoint.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_FIXEDPOINT_HPP__
#define LIBDPF_INCLUDE_DPF_FIXEDPOINT_HPP__

#include <cstdint>
#include <limits>
#include <functional>
#include <array>
#include <stdexcept>

#include <cmath>
#include <iostream>

#include <hedley/hedley.h>
#include <portable-snippets/exact-int/exact-int.h>
#include <portable-snippets/builtin/builtin.h>

#include "dpf/utils.hpp"

#define LIBDPF_FIXED_DEFAULT_INTEGRAL_REPRESENTATION psnip_uint64_t

namespace dpf
{

template <unsigned FractionalBits,
          typename IntegralType = LIBDPF_FIXED_DEFAULT_INTEGRAL_REPRESENTATION>
struct fixedpoint
{
    using integral_type = IntegralType;
    static constexpr int fractional_bits = FractionalBits;
    static constexpr int integer_bits = utils::bitlength_of_v<integral_type> - fractional_bits;

    static_assert(std::numeric_limits<integral_type>::is_integer
        || std::is_same_v<integral_type, simde_int128>
        || std::is_same_v<integral_type, simde_uint128>);
    static_assert(fractional_bits <= utils::bitlength_of_v<integral_type>);
    static_assert(integer_bits >= 0);

    /// @name C'tors
    /// @brief Constructs a new fixed-point number.
    /// @{

    /// @brief Default c'tor
    /// @details Default constructs a fixed-point. No initialization takes place, other than zero initialization of static and thread-local objects.
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr fixedpoint() noexcept = default;

    /// @brief Copy c'tor
    /// @details Constructs a fixed-point with the value copied from `other`.
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr fixedpoint(const fixedpoint & other) noexcept = default;

    /// @brief Move c'tor
    /// @details Constructs a fixed-point with the value copied from `other` using move semantics.
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr fixedpoint(fixedpoint && other) noexcept = default;

    /// @brief Value c'tor
    /// @details Initializes the fixed-point with the value determined by `desired`, using the <a href="https://en.cppreference.com/w/cpp/numeric/fenv/FE_round">current rounding mode</a> for the least-significant bit.
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr fixedpoint(double desired) noexcept
      : value{static_cast<integral_type>(std::nearbyint(std::ldexp(desired, fractional_bits)))}
    { }

    /// @}

    /// @name Assignment operators
    /// @brief Assign a new value to a fixed-point number
    /// {@

    /// @brief Copy assignment
    /// @details Assigns the fixed-point with a copy of `other`
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr fixedpoint & operator=(const fixedpoint & other) noexcept = default;

    /// @brief Move assignment
    /// @details Assigns the fixed-point with a copy of `other` using move semantics.
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr fixedpoint & operator=(fixedpoint && other) noexcept = default;

    /// @brief Value assignment
    /// @details Assigns the fixed-point with a value determined by `desired`, using the <a href="https://en.cppreference.com/w/cpp/numeric/fenv/FE_round">current rounding mode</a> for the least-significant bit..
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr fixedpoint & operator=(const double & desired) noexcept
    {
        value = static_cast<integral_type>(std::nearbyint(std::ldexp(desired, fractional_bits)));
    }

    /// @}

    /// @brief Cast to `double`
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    explicit constexpr operator double() const noexcept
    {
        return std::ldexp(static_cast<double>(value), -static_cast<double>(fractional_bits));
    }

    /// @brief 
    /// @param rhs 
    /// @return 
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr bool operator&(integral_type mask) const noexcept
    {
        return static_cast<bool>(this->integral_representation() & mask);
    }

    /// @brief Access underlying integral representation
    /// @details If the represented fixed-point number is `x`, then this
    ///          function returns an `integral_type` whose value is `x*2**fractional_bits`.
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr integral_type integral_representation() const noexcept
    {
        return this->value;
    }

    /// @brief Unary negation operator
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr fixedpoint & operator-() const noexcept
    {
        return fixedpoint(-this->integral_representation());
    }

    /// @brief Binary addition operator
    /// @details Computes the sum of two fixed-point numbers
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr fixedpoint & operator+(fixedpoint rhs) const noexcept
    {
        return fixedpoint(this->integral_representation() + rhs.integral_representation());
    }

    /// @brief Binary addition assignment operator
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr fixedpoint & operator+=(fixedpoint rhs) noexcept
    {
        this->value += rhs.integral_representation();
        return *this;
    }

    /// @brief Binary subtraction operator
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr fixedpoint & operator-(fixedpoint rhs) const noexcept
    {
        return fixedpoint(this->integral_representation() - rhs.integral_representation());
    }

    /// @brief Binary addition assignment operator
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr fixedpoint & operator-=(fixedpoint rhs) noexcept
    {
        this->value -= rhs.integral_representation();
        return *this;
    }

    /// @brief Binary multiplication operator
    template <unsigned FractionalBits1>
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr auto operator*(fixedpoint<FractionalBits1, IntegralType> rhs) noexcept
    {
        using product_t = fixedpoint<FractionalBits + FractionalBits1, IntegralType>;
        return product_t(this->integral_representation() * rhs.integral_representation());
    }

    /// @name Equality
    /// @brief Strict equality operator
    /// @{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr bool operator==(fixedpoint rhs) const noexcept
    {
        return (this->integral_representation() == rhs.integral_representation());
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr bool operator==(double rhs) const noexcept
    {
        return is_in_range(rhs) && (this == fixedpoint(rhs));
    }
    /// @}

    /// @name Inequality
    /// @brief Strict inequality operator
    /// @{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr bool operator!=(fixedpoint rhs) const noexcept
    {
        return (this->integral_representation() != rhs.integral_representation());
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr bool operator!=(double rhs) const noexcept
    {
        return is_in_range(rhs) && (this != fixedpoint(rhs));
    }
    /// @}

    /// @name Less than
    /// @brief Binary less-than operator
    /// @{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr bool operator<(fixedpoint rhs) const noexcept
    {
        return (this->integral_representation() < rhs.integral_representation());
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr bool operator<(double rhs) const noexcept
    {
        return is_in_range(rhs) && (this < fixedpoint(rhs));
    }
    /// @}

    /// @name Less than or equal
    /// @brief Binary less-than-or-equal operator
    /// @{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr bool operator<=(fixedpoint rhs) const noexcept
    {
        return (this->integral_representation() <= rhs.integral_representation());
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr bool operator<=(double rhs) const noexcept
    {
        return is_in_range(rhs) && (*this <= fixedpoint(rhs));
    }
    /// @}

    /// @name Greater than
    /// @brief Binary greater-than operator
    /// @{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr bool operator>(fixedpoint rhs) const noexcept
    {
        return (this->integral_representation() > rhs.integral_representation());
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr bool operator>(double rhs) const noexcept
    {
        return is_in_range(rhs) && (*this > fixedpoint(rhs));
    }
    /// @}

    /// @name Greater than or equal
    /// @brief Binary greater-than-or-equal operator
    /// @{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr bool operator>=(fixedpoint rhs) const noexcept
    {
        return (this->integral_representation() >= rhs.integral_representation());
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr bool operator>=(double rhs) const noexcept
    {
        return is_in_range(rhs) && (*this >= fixedpoint(rhs));
    }
    /// @}

  private:
    /// @brief Determine if a floating-point is within range
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    static constexpr bool is_in_range(double d) noexcept
    {
        return HEDLEY_LIKELY(
            static_cast<double>(std::numeric_limits<fixedpoint>::lowest()) <= d
              && d <= static_cast<double>(std::numeric_limits<fixedpoint>::max()));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr explicit fixedpoint(integral_type val) noexcept
      : value{val}
    { }

    template <unsigned F, typename T> friend constexpr auto nextafter(fixedpoint<F, T> f) noexcept;
    template <unsigned F, typename T> friend constexpr auto nextbefore(fixedpoint<F, T> f) noexcept;
    template <unsigned F0, unsigned F1, typename T> friend constexpr auto precision_cast(fixedpoint<F1, T> f) noexcept;


    integral_type value;
};

template <class CharT,
          class Traits,
          unsigned FractionalBits,
          typename IntegralType>
std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> & os, 
    const fixedpoint<FractionalBits, IntegralType> & f) noexcept
{
    return os << static_cast<double>(f);
}

template <class CharT,
          class Traits,
          unsigned FractionalBits,
          typename IntegralType>
std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> & is,
    fixedpoint<FractionalBits, IntegralType> & f)
{
    double d;
    is >> d;
    f = d;
    return is;
}

template <unsigned FractionalBits,
          typename IntegralType = LIBDPF_FIXED_DEFAULT_INTEGRAL_REPRESENTATION>
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
static auto make_fixed(double d)
{
    return fixedpoint<FractionalBits, IntegralType>(d);
}

template <unsigned FractionalBits,
          typename IntegralType = LIBDPF_FIXED_DEFAULT_INTEGRAL_REPRESENTATION>
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
static auto make_fixed_safe(double d)
{
    using fixed_t = fixedpoint<FractionalBits, IntegralType>;

    if (HEDLEY_UNLIKELY(d < std::numeric_limits<fixed_t>::lowest()))
    {
        throw std::range_error("value is too small (underflows integral representation)");
    }

    if (HEDLEY_UNLIKELY(std::numeric_limits<fixed_t>::max() < d))
    {
        throw std::range_error("value is too large (overflows integral representation)");
    }

    return make_fixed<FractionalBits, IntegralType>(d);
}

template <unsigned ToFractionalBits,
          unsigned FromFractionalBits,
          typename IntegralType>
constexpr auto precision_cast(fixedpoint<FromFractionalBits, IntegralType> f) noexcept
{
    auto value = f.integral_representation();
    if constexpr (ToFractionalBits > FromFractionalBits)
    {
        return fixedpoint<ToFractionalBits, IntegralType>(value << (ToFractionalBits - FromFractionalBits));
    }
    return fixedpoint<ToFractionalBits, IntegralType>(value >> (FromFractionalBits - ToFractionalBits));
}

template <unsigned FractionalBits,
          typename IntegralType>
static constexpr auto precision_of(fixedpoint<FractionalBits, IntegralType> f) noexcept
{
    return FractionalBits;
}

template <unsigned FractionalBits,
          typename IntegralType>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
HEDLEY_CONST
constexpr auto nextafter(fixedpoint<FractionalBits, IntegralType> f) noexcept
{
    return fixedpoint<FractionalBits, IntegralType>(f.integral_representation()+1);
}

template <unsigned FractionalBits,
          typename IntegralType>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
HEDLEY_CONST
constexpr auto nextbefore(fixedpoint<FractionalBits, IntegralType> f) noexcept
{
    return fixedpoint<FractionalBits, IntegralType>(f.integral_representation()-1);
}

enum fixed_cast_policy
{
    use_default,
    use_left_arg,
    use_right_arg,
    use_min_arg,
    use_max_arg,
    use_arg_sum  //< for multiplies only
};

template <typename BinaryOperator, fixed_cast_policy Mode = use_max_arg>
struct binary_operator_precast_wrapper
{
    static_assert(Mode == use_default   ||
                  Mode == use_left_arg  ||
                  Mode == use_right_arg ||
                  Mode == use_min_arg   ||
                  Mode == use_max_arg);

    template <typename IntegralType,
              unsigned FractionalBitsLHS,
              unsigned FractionalBitsRHS>
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr auto operator()(
        fixedpoint<FractionalBitsLHS, IntegralType> lhs,
        fixedpoint<FractionalBitsRHS, IntegralType> rhs) const
    {
        constexpr bool lt = (FractionalBitsLHS < FractionalBitsRHS);
        constexpr bool eq = (FractionalBitsLHS == FractionalBitsRHS);
        constexpr BinaryOperator op{};

        if constexpr (eq) return op(lhs, rhs);      // no cast necessary
        else if constexpr ((Mode == use_left_arg)   // always casting to lhs
            || ((Mode == use_min_arg) &&  lt)       // lhs happens to be min
            || ((Mode == use_max_arg) && !lt)       // lhs happens to be max
            || ((Mode == use_default) && !lt))      // default == use_max
        {
            return op(lhs, precision_cast<FractionalBitsLHS>(rhs));
        }
        else if constexpr ((Mode == use_right_arg)  // always casting to rhs
            || ((Mode == use_min_arg) && !lt)       // rhs happens to be min
            || ((Mode == use_max_arg) &&  lt)       // rhs happens to be max
            || ((Mode == use_default) &&  lt))      // default == use_max
        {
            return op(precision_cast<FractionalBitsRHS>(lhs), rhs);
        }
        else
        {
            __builtin_unreachable();
        }
    }
};

template <fixed_cast_policy Mode = use_arg_sum>
struct multiplies
{
    static_assert(Mode == use_default   ||
                  Mode == use_left_arg  ||
                  Mode == use_right_arg ||
                  Mode == use_min_arg   ||
                  Mode == use_max_arg   ||
                  Mode == use_arg_sum);

    template <unsigned FractionalBitsOut = unsigned(-1),
              typename IntegralType,
              unsigned FractionalBitsLHS,
              unsigned FractionalBitsRHS>
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr auto operator()(
        fixedpoint<FractionalBitsLHS, IntegralType> lhs,
        fixedpoint<FractionalBitsRHS, IntegralType> rhs) const
    {
        constexpr unsigned reduce_by = []()
        {
            constexpr bool lt = (FractionalBitsLHS < FractionalBitsRHS);
            constexpr bool eq = (FractionalBitsLHS == FractionalBitsRHS);
            if constexpr (FractionalBitsOut != unsigned(-1))
            {
                return FractionalBitsRHS+FractionalBitsLHS-FractionalBitsOut;
            }
            if constexpr (eq) return (FractionalBitsLHS+FractionalBitsLHS)/2;
            if constexpr ((Mode == use_left_arg)    // always casting to lhs
                || ((Mode == use_min_arg) &&  lt)   // lhs happens to be min
                || ((Mode == use_max_arg) && !lt))  // lhs happens to be max
            {
                return FractionalBitsRHS;  // = (LHS+RHS)-LHS
            };
            if constexpr ((Mode == use_right_arg)   // always casting to rhs
                || ((Mode == use_min_arg) && !lt)   // rhs happens to be min
                || ((Mode == use_max_arg) &&  lt))  // rhs happens to be max
            {
                return FractionalBitsLHS;  // = (LHS+RHS)-RHS
            }
            else if constexpr ((Mode == use_arg_sum) || (Mode == use_default))
            {
                return 0;  // = (LHS+RHS)-(LHS+RHS)
            }
        };

        if constexpr (std::numeric_limits<IntegralType>::digits > 32)
        {
            if constexpr (reduce_by < 0)
                return IntegralType((simde_uint128(lhs.integral_representation())*rhs.integral_representation()) << reduce_by);
            else
                return IntegralType((simde_uint128(lhs.integral_representation())*rhs.integral_representation()) >> reduce_by);
        }
        else
        {
            if constexpr (reduce_by < 0)
                return IntegralType((uint64_t(lhs.integral_representation())*rhs.integral_representation()) << reduce_by);
            else
                return IntegralType((uint64_t(lhs.integral_representation())*rhs.integral_representation()) >> reduce_by);
        }
    }
};

template <unsigned FractionalBits,
          typename IntegralType>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
HEDLEY_CONST
constexpr bool operator==(double lhs, fixedpoint<FractionalBits, IntegralType> rhs) noexcept
{
    return (rhs == lhs);
}

template <unsigned FractionalBits,
          typename IntegralType>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
HEDLEY_CONST
constexpr bool operator<(double lhs, fixedpoint<FractionalBits, IntegralType> rhs) noexcept
{
    return (rhs > lhs);
}

template <unsigned FractionalBits,
          typename IntegralType>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
HEDLEY_CONST
constexpr bool operator<=(double lhs, fixedpoint<FractionalBits, IntegralType> rhs) noexcept
{
    return (rhs >= lhs);
}

template <unsigned FractionalBits,
          typename IntegralType>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
HEDLEY_CONST
constexpr bool operator>(double lhs, fixedpoint<FractionalBits, IntegralType> rhs) noexcept
{
    return (rhs < lhs);
}

template <unsigned FractionalBits,
          typename IntegralType>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
HEDLEY_CONST
constexpr bool operator>=(double lhs, fixedpoint<FractionalBits, IntegralType> rhs) noexcept
{
    return (rhs <= lhs);
}

template <typename FixedPointType,
          std::size_t Degree>
struct fixedpoint_polynomial : public std::array<FixedPointType, Degree>
{
    using coefficient_t = FixedPointType;
    static constexpr std::size_t degree = Degree;
    auto operator()(coefficient_t x)
    {
        constexpr auto product_of = multiplies<use_arg_sum>{};
        constexpr auto sum_of = binary_operator_precast_wrapper<std::plus<coefficient_t>, use_max_arg>{};
        auto coeff = this->rbegin();
        coefficient_t y{*coeff};
        while (++coeff != this->rend())
        {
            y = sum_of(product_of(y, x), *coeff);
        }
        return y;
    }
};

template <typename FixedPointType,
          std::size_t Degree>
static constexpr auto evaluate(const fixedpoint_polynomial<FixedPointType, Degree> & poly, FixedPointType x)
{
    return poly.evaluate(x);
}

namespace utils
{

template <unsigned FractionalBits,
          typename IntegralType>
struct bitlength_of<dpf::fixedpoint<FractionalBits, IntegralType>>
  : public bitlength_of<IntegralType> { };

template <unsigned FractionalBits,
          typename IntegralType>
struct msb_of<dpf::fixedpoint<FractionalBits, IntegralType>>
  : public msb_of<IntegralType> { };

template <unsigned FractionalBits,
          typename IntegralType>
struct countl_zero_symmmetric_difference<dpf::fixedpoint<FractionalBits, IntegralType>>
{
	using T = dpf::fixedpoint<FractionalBits, IntegralType>;
    static constexpr auto clz = dpf::utils::countl_zero_symmmetric_difference<typename T::integral_type>{};

	HEDLEY_CONST
	HEDLEY_ALWAYS_INLINE
	constexpr std::size_t operator()(const T & lhs, const T & rhs) const noexcept
	{
        constexpr auto adjust = utils::bitlength_of_v<typename T::integral_type>-T::bits;
		return clz(static_cast<typename T::integral_type>(lhs), static_cast<typename T::integral_type>(rhs))-adjust;
	}
};

}  // namespace utils

}  // namespace dpf

using dpf::precision_cast;

namespace std
{

template <unsigned FractionalBits,
          typename IntegralType>
class numeric_limits<dpf::fixedpoint<FractionalBits, IntegralType>>
  : private std::numeric_limits<IntegralType>
{
  public:
    using T = dpf::fixedpoint<FractionalBits, IntegralType>;
    using integral_limits = std::numeric_limits<IntegralType>;

    using integral_limits::is_specialized;
    using integral_limits::is_signed;
    static constexpr bool is_integer = !FractionalBits;
    using integral_limits::is_exact;
    using integral_limits::has_infinity;
    using integral_limits::has_quiet_NaN;
    using integral_limits::has_signaling_NaN;
    using integral_limits::has_denorm;
    using integral_limits::has_denorm_loss;
    using integral_limits::round_style;
    using integral_limits::is_iec559;
    using integral_limits::is_bounded;
    using integral_limits::is_modulo;
    using integral_limits::digits;
    using integral_limits::digits10;
    using integral_limits::max_digits10;
    using integral_limits::radix;
    // static constexpr int min_exponent = -FractionalBits;
    using integral_limits::min_exponent;
    // static constexpr int min_exponent10 = -FractionalBits * 0.3;
    using integral_limits::min_exponent10;
    // static constexpr int min_exponent = digits - FractionalBits;
    using integral_limits::max_exponent;
    // static constexpr int min_exponent = (digits - FractionalBits)*0.3;
    using integral_limits::max_exponent10;
    using integral_limits::traps;
    using integral_limits::tinyness_before;

    static constexpr T min() noexcept { return std::ldexp(IntegralType(1), -FractionalBits); }
    static constexpr T lowest() noexcept { return std::ldexp(integral_limits::lowest(), -FractionalBits); }
    static constexpr T max() noexcept { return std::ldexp(integral_limits::max(), -FractionalBits); }
    static constexpr T epsilon() noexcept { return is_integer ? T(0.0) : T(1.0) + min(); }
    static constexpr T round_error() noexcept { return T(is_integer ? 0.0 : 0.5); }
    static constexpr T infinity() noexcept { return T(0); }
    static constexpr T quiet_NaN() noexcept { return T(0); }
    static constexpr T signalling_NaN() noexcept { return T(0); }
    static constexpr T denorm_min() noexcept { return T(0); }
};

template <unsigned F, typename T>
struct numeric_limits<dpf::fixedpoint<F, T> const>
  : public numeric_limits<dpf::fixedpoint<F, T>> {};

template <unsigned F, typename T>
struct numeric_limits<dpf::fixedpoint<F, T> volatile>
  : public numeric_limits<dpf::fixedpoint<F, T>> {};

template <unsigned F, typename T>
struct numeric_limits<dpf::fixedpoint<F, T> const volatile>
  : public numeric_limits<dpf::fixedpoint<F, T>> {};

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_FIXEDPOINT_HPP__
