/// @file dpf/fixedpoint.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_FIXEDPOINT_HPP__
#define LIBDPF_INCLUDE_DPF_FIXEDPOINT_HPP__

#include <hedley/hedley.h>
#include <portable-snippets/exact-int/exact-int.h>
#include <portable-snippets/builtin/builtin.h>

#include <cstdint>
#include <limits>
#include <functional>
#include <array>
#include <stdexcept>

#include <cmath>
#include <iostream>

#include "dpf/utils.hpp"

#define LIBDPF_FIXED_DEFAULT_INTEGRAL_REPRESENTATION psnip_uint64_t

namespace dpf
{

template <unsigned FractionalBits,
          typename IntegralType>
auto constexpr make_fixed_from_integral_type(IntegralType value) noexcept;

/// @tparam FractionalBits Number of fractional bits used in the fixed-point
///         representation.
/// @tparam IntegralType The underlying integral type used for the fixed-point
///         representation.
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
    constexpr fixedpoint(double desired) noexcept  // NOLINT (implicit c'tor)
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
        return *this;
    }

    /// @}

    ~fixedpoint() = default;

    /// @brief Cast to `double`
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    explicit constexpr operator double() const noexcept
    {
        return std::ldexp(static_cast<double>(value), -static_cast<double>(fractional_bits));
    }

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
    constexpr fixedpoint operator-() const noexcept
    {
        return fixedpoint(-this->integral_representation());
    }

    /// @brief Binary addition operator
    /// @details Computes the sum of two fixed-point numbers
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    constexpr fixedpoint operator+(fixedpoint rhs) const noexcept
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
    constexpr fixedpoint operator-(fixedpoint rhs) const noexcept
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
        return make_fixed_from_integral_type<FractionalBits + FractionalBits1>(this->integral_representation() * rhs.integral_representation());
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

    template <unsigned F, typename T> friend constexpr auto nextafter(fixedpoint<F, T>) noexcept;
    template <unsigned F, typename T> friend constexpr auto nextbefore(fixedpoint<F, T>) noexcept;
    template <unsigned F0, unsigned F1, typename T> friend constexpr auto precision_cast(fixedpoint<F1, T>) noexcept;
    template <unsigned F, typename T> friend constexpr auto make_fixed_from_integral_type(T) noexcept;
    template <unsigned F, typename T> friend constexpr auto fabs(fixedpoint<F, T>) noexcept;
    template <unsigned F, typename T> friend constexpr auto fmod(fixedpoint<F, T>, double) noexcept;

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
          typename IntegralType>
auto constexpr make_fixed_from_integral_type(IntegralType value) noexcept
{
    return fixedpoint<FractionalBits, IntegralType>(value);
}

template <unsigned FractionalBits,
          typename IntegralType = LIBDPF_FIXED_DEFAULT_INTEGRAL_REPRESENTATION>
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
static constexpr auto make_fixed(double d)
{
    return fixedpoint<FractionalBits, IntegralType>(d);
}

template <typename FixedType>
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
static constexpr auto make_fixed(double d)
{
    return fixedpoint<FixedType::fractional_bits, typename FixedType::integral_type>(d);
}

/// @brief Creates a fixed-point number from a double with bounds checking.
/// @throws std::range_error If the input double is outside the representable
///         range of the fixed-point number.
template <unsigned FractionalBits,
          typename IntegralType = LIBDPF_FIXED_DEFAULT_INTEGRAL_REPRESENTATION>
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
static auto make_fixed_safe(double d)
{
    using fixed_type = fixedpoint<FractionalBits, IntegralType>;

    if (HEDLEY_UNLIKELY(d < std::numeric_limits<fixed_type>::lowest()))
    {
        throw std::range_error("value is too small (underflows integral representation)");
    }

    if (HEDLEY_UNLIKELY(std::numeric_limits<fixed_type>::max() < d))
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
static constexpr auto precision_of(fixedpoint<FractionalBits, IntegralType>) noexcept
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

template <unsigned FractionalBits,
          typename IntegralType>
constexpr auto fabs(fixedpoint<FractionalBits, IntegralType> v) noexcept
{
    return fixedpoint<FractionalBits, IntegralType>(std::abs(v.integral_representation()));
}

template <unsigned FractionalBits,
          typename IntegralType>
constexpr auto fmod(fixedpoint<FractionalBits, IntegralType> v, double modulus) noexcept
{
    auto mod = make_fixed<FractionalBits, IntegralType>(modulus);
    return fixedpoint<FractionalBits, IntegralType>(v.integral_representation() % mod.integral_representation());
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

template <typename BinaryOperator,
          fixed_cast_policy Mode = use_max_arg>
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
    using coefficient_type = FixedPointType;
    static constexpr std::size_t degree = Degree;
    auto operator()(coefficient_type x)
    {
        constexpr auto product_of = multiplies<use_arg_sum>{};
        constexpr auto sum_of = binary_operator_precast_wrapper<std::plus<coefficient_type>, use_max_arg>{};
        auto coeff = this->rbegin();
        coefficient_type y{*coeff};
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

}  // namespace dpf::utils

namespace literals
{

constexpr auto operator "" _fixed0(long double val)
{
    return dpf::make_fixed<0>(val);
}

constexpr auto operator "" _fixed1(long double val)
{
    return dpf::make_fixed<1>(val);
}

constexpr auto operator "" _fixed2(long double val)
{
    return dpf::make_fixed<2>(val);
}

constexpr auto operator "" _fixed3(long double val)
{
    return dpf::make_fixed<3>(val);
}

constexpr auto operator "" _fixed4(long double val)
{
    return dpf::make_fixed<4>(val);
}

constexpr auto operator "" _fixed5(long double val)
{
    return dpf::make_fixed<5>(val);
}

constexpr auto operator "" _fixed6(long double val)
{
    return dpf::make_fixed<6>(val);
}

constexpr auto operator "" _fixed7(long double val)
{
    return dpf::make_fixed<7>(val);
}

constexpr auto operator "" _fixed8(long double val)
{
    return dpf::make_fixed<8>(val);
}

constexpr auto operator "" _fixed9(long double val)
{
    return dpf::make_fixed<9>(val);
}

constexpr auto operator "" _fixed10(long double val)
{
    return dpf::make_fixed<10>(val);
}

constexpr auto operator "" _fixed11(long double val)
{
    return dpf::make_fixed<11>(val);
}

constexpr auto operator "" _fixed12(long double val)
{
    return dpf::make_fixed<12>(val);
}

constexpr auto operator "" _fixed13(long double val)
{
    return dpf::make_fixed<13>(val);
}

constexpr auto operator "" _fixed14(long double val)
{
    return dpf::make_fixed<14>(val);
}

constexpr auto operator "" _fixed15(long double val)
{
    return dpf::make_fixed<15>(val);
}

constexpr auto operator "" _fixed16(long double val)
{
    return dpf::make_fixed<16>(val);
}

constexpr auto operator "" _fixed17(long double val)
{
    return dpf::make_fixed<17>(val);
}

constexpr auto operator "" _fixed18(long double val)
{
    return dpf::make_fixed<18>(val);
}

constexpr auto operator "" _fixed19(long double val)
{
    return dpf::make_fixed<19>(val);
}

constexpr auto operator "" _fixed20(long double val)
{
    return dpf::make_fixed<20>(val);
}

constexpr auto operator "" _fixed21(long double val)
{
    return dpf::make_fixed<21>(val);
}

constexpr auto operator "" _fixed22(long double val)
{
    return dpf::make_fixed<22>(val);
}

constexpr auto operator "" _fixed23(long double val)
{
    return dpf::make_fixed<23>(val);
}

constexpr auto operator "" _fixed24(long double val)
{
    return dpf::make_fixed<24>(val);
}

constexpr auto operator "" _fixed25(long double val)
{
    return dpf::make_fixed<25>(val);
}

constexpr auto operator "" _fixed26(long double val)
{
    return dpf::make_fixed<26>(val);
}

constexpr auto operator "" _fixed27(long double val)
{
    return dpf::make_fixed<27>(val);
}

constexpr auto operator "" _fixed28(long double val)
{
    return dpf::make_fixed<28>(val);
}

constexpr auto operator "" _fixed29(long double val)
{
    return dpf::make_fixed<29>(val);
}

constexpr auto operator "" _fixed30(long double val)
{
    return dpf::make_fixed<30>(val);
}

constexpr auto operator "" _fixed31(long double val)
{
    return dpf::make_fixed<31>(val);
}

constexpr auto operator "" _fixed32(long double val)
{
    return dpf::make_fixed<32>(val);
}

constexpr auto operator "" _fixed33(long double val)
{
    return dpf::make_fixed<33>(val);
}
constexpr auto operator "" _fixed34(long double val)
{
    return dpf::make_fixed<34>(val);
}
constexpr auto operator "" _fixed35(long double val)
{
    return dpf::make_fixed<35>(val);
}
constexpr auto operator "" _fixed36(long double val)
{
    return dpf::make_fixed<36>(val);
}
constexpr auto operator "" _fixed37(long double val)
{
    return dpf::make_fixed<37>(val);
}
constexpr auto operator "" _fixed38(long double val)
{
    return dpf::make_fixed<38>(val);
}
constexpr auto operator "" _fixed39(long double val)
{
    return dpf::make_fixed<39>(val);
}
constexpr auto operator "" _fixed40(long double val)
{
    return dpf::make_fixed<40>(val);
}
constexpr auto operator "" _fixed41(long double val)
{
    return dpf::make_fixed<41>(val);
}
constexpr auto operator "" _fixed42(long double val)
{
    return dpf::make_fixed<42>(val);
}
constexpr auto operator "" _fixed43(long double val)
{
    return dpf::make_fixed<43>(val);
}
constexpr auto operator "" _fixed44(long double val)
{
    return dpf::make_fixed<44>(val);
}
constexpr auto operator "" _fixed45(long double val)
{
    return dpf::make_fixed<45>(val);
}
constexpr auto operator "" _fixed46(long double val)
{
    return dpf::make_fixed<46>(val);
}
constexpr auto operator "" _fixed47(long double val)
{
    return dpf::make_fixed<47>(val);
}
constexpr auto operator "" _fixed48(long double val)
{
    return dpf::make_fixed<48>(val);
}
constexpr auto operator "" _fixed49(long double val)
{
    return dpf::make_fixed<49>(val);
}
constexpr auto operator "" _fixed50(long double val)
{
    return dpf::make_fixed<50>(val);
}
constexpr auto operator "" _fixed51(long double val)
{
    return dpf::make_fixed<51>(val);
}
constexpr auto operator "" _fixed52(long double val)
{
    return dpf::make_fixed<52>(val);
}
constexpr auto operator "" _fixed53(long double val)
{
    return dpf::make_fixed<53>(val);
}
constexpr auto operator "" _fixed54(long double val)
{
    return dpf::make_fixed<54>(val);
}
constexpr auto operator "" _fixed55(long double val)
{
    return dpf::make_fixed<55>(val);
}
constexpr auto operator "" _fixed56(long double val)
{
    return dpf::make_fixed<56>(val);
}
constexpr auto operator "" _fixed57(long double val)
{
    return dpf::make_fixed<57>(val);
}
constexpr auto operator "" _fixed58(long double val)
{
    return dpf::make_fixed<58>(val);
}
constexpr auto operator "" _fixed59(long double val)
{
    return dpf::make_fixed<59>(val);
}
constexpr auto operator "" _fixed60(long double val)
{
    return dpf::make_fixed<60>(val);
}
constexpr auto operator "" _fixed61(long double val)
{
    return dpf::make_fixed<61>(val);
}
constexpr auto operator "" _fixed62(long double val)
{
    return dpf::make_fixed<62>(val);
}
constexpr auto operator "" _fixed63(long double val)
{
    return dpf::make_fixed<63>(val);
}
constexpr auto operator "" _fixed64(long double val)
{
    return dpf::make_fixed<64>(val);
}

}  // namespace dpf::literals

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
