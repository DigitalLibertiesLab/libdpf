/// @file dpf/modint.hpp
/// @brief defines the `dpf::modint` class and associated helpers
/// @details A `dpf::modint` is a thin wrapper around some primitive integral
///          type. The underlying value is reduced modulo `2^Nbits` only when the
///          underlying value is read; arithmetic operations have no overhead
///          relative to native operations on the underlying type.
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_MODINT_HPP__
#define LIBDPF_INCLUDE_DPF_MODINT_HPP__

#include <cstddef>
#include <cmath>
#include <type_traits>
#include <functional>
#include <string>
#include <memory>
#include <limits>
#include <ostream>
#include <istream>

#include "hedley/hedley.h"
#include "portable-snippets/exact-int/exact-int.h"

#include "dpf/utils.hpp"
#include "dpf/literals.hpp"

namespace dpf
{

/// @brief represents an unsigned integer modulo `2^Nbits` for small values of `Nbits`
template <std::size_t Nbits>
class modint
{
  public:
    /// @brief the primitive integral type used to represent the `modint`
    using integral_type = dpf::utils::nonvoid_integral_type_from_bitlength_t<Nbits>;

    static constexpr std::size_t num_bits = Nbits;

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

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint operator%(std::size_t modulus) const noexcept
    {
        return modint{static_cast<integral_type>(this->reduced_value() % modulus)};
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr modint & operator%=(std::size_t modulus) noexcept
    {
        this->val = this->reduced_value() % modulus;
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
        return modint{static_cast<integral_type>(this->val*rhs)};
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
        return modint{static_cast<integral_type>(this->val | rhs)};
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
        return modint{static_cast<integral_type>(this->val^rhs)};
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
        return modint{static_cast<integral_type>(~this->val)};
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
    constexpr explicit operator bool() const noexcept
    {
        return static_cast<bool>(this->reduced_value());
    }

    template <std::size_t NbitsNew>
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr explicit operator modint<NbitsNew>() const noexcept
    {
        return modint<NbitsNew>{static_cast<typename modint<NbitsNew>::integral_type>(this->reduced_value())};
    }

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr integral_type data() const noexcept
    {
        return this->val;
    }

    template <typename CharT,
              typename Traits = std::char_traits<CharT>,
              typename Allocator = std::allocator<CharT>>
    friend std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> & os,
        const modint & i)
    {
        return os << i.reduced_value();
    }

    template <typename CharT,
              typename Traits = std::char_traits<CharT>,
              typename Allocator = std::allocator<CharT>>
    friend std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT, Traits> & is,
        const modint & i)
    {
        return is >> i.val;
    }

    constexpr integral_type reduced_value() const
    {
        return val & modulo_mask;
    }

  private:
    /// @brief bitmask used for performing reductions modulo `2^Nbits`
    static constexpr integral_type modulo_mask = static_cast<integral_type>(~integral_type{0}) >> utils::bitlength_of_v<integral_type> - Nbits;

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

template <std::size_t N>
struct make_unsigned<dpf::modint<N>> { using type = dpf::modint<N>; };

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
        constexpr auto adjust = utils::bitlength_of_v<T> - Nbits;
        auto diff = xor_op(static_cast<T>(lhs), static_cast<T>(rhs));

        if constexpr (std::is_same_v<T, simde_uint128>)
        {
            auto diff_hi = static_cast<psnip_uint64_t>(diff >> 64);
            auto diff_lo = static_cast<psnip_uint64_t>(diff);

            return diff_hi ? utils::clz(diff_hi)-adjust : 64+utils::clz(diff_lo)-adjust;
        }
        else
        {
            return utils::clz(diff)-adjust;
        }
    }
};

template <std::size_t Nbits>
struct mod_pow_2<dpf::modint<Nbits>>
{
    using T = dpf::modint<Nbits>;
    static constexpr auto mod = mod_pow_2<typename T::integral_type>{};
    std::size_t operator()(T val, std::size_t n) const noexcept
    {
        return mod(val.val, n);
    }
};

}  // namespace utils

namespace modints
{

// 1--9
using modint1_t = dpf::modint<1>;
using modint2_t = dpf::modint<2>;
using modint3_t = dpf::modint<3>;
using modint4_t = dpf::modint<4>;
using modint5_t = dpf::modint<5>;
using modint6_t = dpf::modint<6>;
using modint7_t = dpf::modint<7>;
using modint8_t = dpf::modint<8>;
using modint9_t = dpf::modint<9>;
// 10--19
using modint10_t = dpf::modint<10>;
using modint11_t = dpf::modint<11>;
using modint12_t = dpf::modint<12>;
using modint13_t = dpf::modint<13>;
using modint14_t = dpf::modint<14>;
using modint15_t = dpf::modint<15>;
using modint16_t = dpf::modint<16>;
using modint17_t = dpf::modint<17>;
using modint18_t = dpf::modint<18>;
using modint19_t = dpf::modint<19>;
// 20--29
using modint20_t = dpf::modint<20>;
using modint21_t = dpf::modint<21>;
using modint22_t = dpf::modint<22>;
using modint23_t = dpf::modint<23>;
using modint24_t = dpf::modint<24>;
using modint25_t = dpf::modint<25>;
using modint26_t = dpf::modint<26>;
using modint27_t = dpf::modint<27>;
using modint28_t = dpf::modint<28>;
using modint29_t = dpf::modint<29>;
// 30--39
using modint30_t = dpf::modint<30>;
using modint31_t = dpf::modint<31>;
using modint32_t = dpf::modint<32>;
using modint33_t = dpf::modint<33>;
using modint34_t = dpf::modint<34>;
using modint35_t = dpf::modint<35>;
using modint36_t = dpf::modint<36>;
using modint37_t = dpf::modint<37>;
using modint38_t = dpf::modint<38>;
using modint39_t = dpf::modint<39>;
// 40--49
using modint40_t = dpf::modint<40>;
using modint41_t = dpf::modint<41>;
using modint42_t = dpf::modint<42>;
using modint43_t = dpf::modint<43>;
using modint44_t = dpf::modint<44>;
using modint45_t = dpf::modint<45>;
using modint46_t = dpf::modint<46>;
using modint47_t = dpf::modint<47>;
using modint48_t = dpf::modint<48>;
using modint49_t = dpf::modint<49>;
// 50--59
using modint50_t = dpf::modint<50>;
using modint51_t = dpf::modint<51>;
using modint52_t = dpf::modint<52>;
using modint53_t = dpf::modint<53>;
using modint54_t = dpf::modint<54>;
using modint55_t = dpf::modint<55>;
using modint56_t = dpf::modint<56>;
using modint57_t = dpf::modint<57>;
using modint58_t = dpf::modint<58>;
using modint59_t = dpf::modint<59>;
// 60--69
using modint60_t = dpf::modint<60>;
using modint61_t = dpf::modint<61>;
using modint62_t = dpf::modint<62>;
using modint63_t = dpf::modint<63>;
using modint64_t = dpf::modint<64>;
using modint65_t = dpf::modint<65>;
using modint66_t = dpf::modint<66>;
using modint67_t = dpf::modint<67>;
using modint68_t = dpf::modint<68>;
using modint69_t = dpf::modint<69>;
// 70--79
using modint70_t = dpf::modint<70>;
using modint71_t = dpf::modint<71>;
using modint72_t = dpf::modint<72>;
using modint73_t = dpf::modint<73>;
using modint74_t = dpf::modint<74>;
using modint75_t = dpf::modint<75>;
using modint76_t = dpf::modint<76>;
using modint77_t = dpf::modint<77>;
using modint78_t = dpf::modint<78>;
using modint79_t = dpf::modint<79>;
// 80--89
using modint80_t = dpf::modint<80>;
using modint81_t = dpf::modint<81>;
using modint82_t = dpf::modint<82>;
using modint83_t = dpf::modint<83>;
using modint84_t = dpf::modint<84>;
using modint85_t = dpf::modint<85>;
using modint86_t = dpf::modint<86>;
using modint87_t = dpf::modint<87>;
using modint88_t = dpf::modint<88>;
using modint89_t = dpf::modint<89>;
using modint90_t = dpf::modint<90>;
// 90--99
using modint91_t = dpf::modint<91>;
using modint92_t = dpf::modint<92>;
using modint93_t = dpf::modint<93>;
using modint94_t = dpf::modint<94>;
using modint95_t = dpf::modint<95>;
using modint96_t = dpf::modint<96>;
using modint97_t = dpf::modint<97>;
using modint98_t = dpf::modint<98>;
using modint99_t = dpf::modint<99>;
// 100--109
using modint100_t = dpf::modint<100>;
using modint101_t = dpf::modint<101>;
using modint102_t = dpf::modint<102>;
using modint103_t = dpf::modint<103>;
using modint104_t = dpf::modint<104>;
using modint105_t = dpf::modint<105>;
using modint106_t = dpf::modint<106>;
using modint107_t = dpf::modint<107>;
using modint108_t = dpf::modint<108>;
using modint109_t = dpf::modint<109>;
// 110--119
using modint110_t = dpf::modint<110>;
using modint111_t = dpf::modint<111>;
using modint112_t = dpf::modint<112>;
using modint113_t = dpf::modint<113>;
using modint114_t = dpf::modint<114>;
using modint115_t = dpf::modint<115>;
using modint116_t = dpf::modint<116>;
using modint117_t = dpf::modint<117>;
using modint118_t = dpf::modint<118>;
using modint119_t = dpf::modint<119>;
// 120--129
using modint120_t = dpf::modint<120>;
using modint121_t = dpf::modint<121>;
using modint122_t = dpf::modint<122>;
using modint123_t = dpf::modint<123>;
using modint124_t = dpf::modint<124>;
using modint125_t = dpf::modint<125>;
using modint126_t = dpf::modint<126>;
using modint127_t = dpf::modint<127>;
using modint128_t = dpf::modint<128>;
using modint129_t = dpf::modint<129>;
// 130--139
using modint130_t = dpf::modint<130>;
using modint131_t = dpf::modint<131>;
using modint132_t = dpf::modint<132>;
using modint133_t = dpf::modint<133>;
using modint134_t = dpf::modint<134>;
using modint135_t = dpf::modint<135>;
using modint136_t = dpf::modint<136>;
using modint137_t = dpf::modint<137>;
using modint138_t = dpf::modint<138>;
using modint139_t = dpf::modint<139>;
// 140--149
using modint140_t = dpf::modint<140>;
using modint141_t = dpf::modint<141>;
using modint142_t = dpf::modint<142>;
using modint143_t = dpf::modint<143>;
using modint144_t = dpf::modint<144>;
using modint145_t = dpf::modint<145>;
using modint146_t = dpf::modint<146>;
using modint147_t = dpf::modint<147>;
using modint148_t = dpf::modint<148>;
using modint149_t = dpf::modint<149>;
// 150--159
using modint150_t = dpf::modint<150>;
using modint151_t = dpf::modint<151>;
using modint152_t = dpf::modint<152>;
using modint153_t = dpf::modint<153>;
using modint154_t = dpf::modint<154>;
using modint155_t = dpf::modint<155>;
using modint156_t = dpf::modint<156>;
using modint157_t = dpf::modint<157>;
using modint158_t = dpf::modint<158>;
using modint159_t = dpf::modint<159>;
// 160--169
using modint160_t = dpf::modint<160>;
using modint161_t = dpf::modint<161>;
using modint162_t = dpf::modint<162>;
using modint163_t = dpf::modint<163>;
using modint164_t = dpf::modint<164>;
using modint165_t = dpf::modint<165>;
using modint166_t = dpf::modint<166>;
using modint167_t = dpf::modint<167>;
using modint168_t = dpf::modint<168>;
using modint169_t = dpf::modint<169>;
// 170--179
using modint170_t = dpf::modint<170>;
using modint171_t = dpf::modint<171>;
using modint172_t = dpf::modint<172>;
using modint173_t = dpf::modint<173>;
using modint174_t = dpf::modint<174>;
using modint175_t = dpf::modint<175>;
using modint176_t = dpf::modint<176>;
using modint177_t = dpf::modint<177>;
using modint178_t = dpf::modint<178>;
using modint179_t = dpf::modint<179>;
// 180--189
using modint180_t = dpf::modint<180>;
using modint181_t = dpf::modint<181>;
using modint182_t = dpf::modint<182>;
using modint183_t = dpf::modint<183>;
using modint184_t = dpf::modint<184>;
using modint185_t = dpf::modint<185>;
using modint186_t = dpf::modint<186>;
using modint187_t = dpf::modint<187>;
using modint188_t = dpf::modint<188>;
using modint189_t = dpf::modint<189>;
// 190--199
using modint190_t = dpf::modint<190>;
using modint191_t = dpf::modint<191>;
using modint192_t = dpf::modint<192>;
using modint193_t = dpf::modint<193>;
using modint194_t = dpf::modint<194>;
using modint195_t = dpf::modint<195>;
using modint196_t = dpf::modint<196>;
using modint197_t = dpf::modint<197>;
using modint198_t = dpf::modint<198>;
using modint199_t = dpf::modint<199>;
// 200--209
using modint200_t = dpf::modint<200>;
using modint201_t = dpf::modint<201>;
using modint202_t = dpf::modint<202>;
using modint203_t = dpf::modint<203>;
using modint204_t = dpf::modint<204>;
using modint205_t = dpf::modint<205>;
using modint206_t = dpf::modint<206>;
using modint207_t = dpf::modint<207>;
using modint208_t = dpf::modint<208>;
using modint209_t = dpf::modint<209>;
// 210--219
using modint210_t = dpf::modint<210>;
using modint211_t = dpf::modint<211>;
using modint212_t = dpf::modint<212>;
using modint213_t = dpf::modint<213>;
using modint214_t = dpf::modint<214>;
using modint215_t = dpf::modint<215>;
using modint216_t = dpf::modint<216>;
using modint217_t = dpf::modint<217>;
using modint218_t = dpf::modint<218>;
using modint219_t = dpf::modint<219>;
// 220--229
using modint220_t = dpf::modint<220>;
using modint221_t = dpf::modint<221>;
using modint222_t = dpf::modint<222>;
using modint223_t = dpf::modint<223>;
using modint224_t = dpf::modint<224>;
using modint225_t = dpf::modint<225>;
using modint226_t = dpf::modint<226>;
using modint227_t = dpf::modint<227>;
using modint228_t = dpf::modint<228>;
using modint229_t = dpf::modint<229>;
// 120--239
using modint230_t = dpf::modint<230>;
using modint231_t = dpf::modint<231>;
using modint232_t = dpf::modint<232>;
using modint233_t = dpf::modint<233>;
using modint234_t = dpf::modint<234>;
using modint235_t = dpf::modint<235>;
using modint236_t = dpf::modint<236>;
using modint237_t = dpf::modint<237>;
using modint238_t = dpf::modint<238>;
using modint239_t = dpf::modint<239>;
// 240--249
using modint240_t = dpf::modint<240>;
using modint241_t = dpf::modint<241>;
using modint242_t = dpf::modint<242>;
using modint243_t = dpf::modint<243>;
using modint244_t = dpf::modint<244>;
using modint245_t = dpf::modint<245>;
using modint246_t = dpf::modint<246>;
using modint247_t = dpf::modint<247>;
using modint248_t = dpf::modint<248>;
using modint249_t = dpf::modint<249>;
// 250--256
using modint250_t = dpf::modint<250>;
using modint251_t = dpf::modint<251>;
using modint252_t = dpf::modint<252>;
using modint253_t = dpf::modint<253>;
using modint254_t = dpf::modint<254>;
using modint255_t = dpf::modint<255>;
using modint256_t = dpf::modint<256>;

namespace literals = dpf::literals::modints;

}  // namespace modints

namespace literals
{

namespace modints
{

// 1--9
constexpr static auto operator "" _u1(unsigned long long int x) { return dpf::modints::modint1_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _u2(unsigned long long int x) { return dpf::modints::modint2_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _u3(unsigned long long int x) { return dpf::modints::modint3_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _u4(unsigned long long int x) { return dpf::modints::modint4_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _u5(unsigned long long int x) { return dpf::modints::modint5_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _u6(unsigned long long int x) { return dpf::modints::modint6_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _u7(unsigned long long int x) { return dpf::modints::modint7_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _u8(unsigned long long int x) { return dpf::modints::modint8_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _u9(unsigned long long int x) { return dpf::modints::modint9_t{static_cast<psnip_uint16_t>(x)}; }
// 10--19
constexpr static auto operator "" _u10(unsigned long long int x) { return dpf::modints::modint10_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _u11(unsigned long long int x) { return dpf::modints::modint11_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _u12(unsigned long long int x) { return dpf::modints::modint12_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _u13(unsigned long long int x) { return dpf::modints::modint13_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _u14(unsigned long long int x) { return dpf::modints::modint14_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _u15(unsigned long long int x) { return dpf::modints::modint15_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _u16(unsigned long long int x) { return dpf::modints::modint16_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _u17(unsigned long long int x) { return dpf::modints::modint17_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u18(unsigned long long int x) { return dpf::modints::modint18_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u19(unsigned long long int x) { return dpf::modints::modint19_t{static_cast<psnip_uint32_t>(x)}; }
// 20--29
constexpr static auto operator "" _u20(unsigned long long int x) { return dpf::modints::modint20_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u21(unsigned long long int x) { return dpf::modints::modint21_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u22(unsigned long long int x) { return dpf::modints::modint22_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u23(unsigned long long int x) { return dpf::modints::modint23_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u24(unsigned long long int x) { return dpf::modints::modint24_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u25(unsigned long long int x) { return dpf::modints::modint25_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u26(unsigned long long int x) { return dpf::modints::modint26_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u27(unsigned long long int x) { return dpf::modints::modint27_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u28(unsigned long long int x) { return dpf::modints::modint28_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u29(unsigned long long int x) { return dpf::modints::modint29_t{static_cast<psnip_uint32_t>(x)}; }
// 30--39
constexpr static auto operator "" _u30(unsigned long long int x) { return dpf::modints::modint30_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u31(unsigned long long int x) { return dpf::modints::modint31_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u32(unsigned long long int x) { return dpf::modints::modint32_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _u33(unsigned long long int x) { return dpf::modints::modint33_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u34(unsigned long long int x) { return dpf::modints::modint34_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u35(unsigned long long int x) { return dpf::modints::modint35_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u36(unsigned long long int x) { return dpf::modints::modint36_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u37(unsigned long long int x) { return dpf::modints::modint37_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u38(unsigned long long int x) { return dpf::modints::modint38_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u39(unsigned long long int x) { return dpf::modints::modint39_t{static_cast<psnip_uint64_t>(x)}; }
// 40--49
constexpr static auto operator "" _u40(unsigned long long int x) { return dpf::modints::modint40_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u41(unsigned long long int x) { return dpf::modints::modint41_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u42(unsigned long long int x) { return dpf::modints::modint42_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u43(unsigned long long int x) { return dpf::modints::modint43_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u44(unsigned long long int x) { return dpf::modints::modint44_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u45(unsigned long long int x) { return dpf::modints::modint45_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u46(unsigned long long int x) { return dpf::modints::modint46_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u47(unsigned long long int x) { return dpf::modints::modint47_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u48(unsigned long long int x) { return dpf::modints::modint48_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u49(unsigned long long int x) { return dpf::modints::modint49_t{static_cast<psnip_uint64_t>(x)}; }
// 50--59
constexpr static auto operator "" _u50(unsigned long long int x) { return dpf::modints::modint50_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u51(unsigned long long int x) { return dpf::modints::modint51_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u52(unsigned long long int x) { return dpf::modints::modint52_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u53(unsigned long long int x) { return dpf::modints::modint53_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u54(unsigned long long int x) { return dpf::modints::modint54_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u55(unsigned long long int x) { return dpf::modints::modint55_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u56(unsigned long long int x) { return dpf::modints::modint56_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u57(unsigned long long int x) { return dpf::modints::modint57_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u58(unsigned long long int x) { return dpf::modints::modint58_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u59(unsigned long long int x) { return dpf::modints::modint59_t{static_cast<psnip_uint64_t>(x)}; }
// 60--69
constexpr static auto operator "" _u60(unsigned long long int x) { return dpf::modints::modint60_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u61(unsigned long long int x) { return dpf::modints::modint61_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u62(unsigned long long int x) { return dpf::modints::modint62_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u63(unsigned long long int x) { return dpf::modints::modint63_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _u64(unsigned long long int x) { return dpf::modints::modint64_t{static_cast<psnip_uint64_t>(x)}; }
template <char ...digits> constexpr static auto operator "" _u65() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint65_t{x}; }
template <char ...digits> constexpr static auto operator "" _u66() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint66_t{x}; }
template <char ...digits> constexpr static auto operator "" _u67() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint67_t{x}; }
template <char ...digits> constexpr static auto operator "" _u68() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint68_t{x}; }
template <char ...digits> constexpr static auto operator "" _u69() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint69_t{x}; }
// 70--79
template <char ...digits> constexpr static auto operator "" _u70() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint70_t{x}; }
template <char ...digits> constexpr static auto operator "" _u71() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint71_t{x}; }
template <char ...digits> constexpr static auto operator "" _u72() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint72_t{x}; }
template <char ...digits> constexpr static auto operator "" _u73() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint73_t{x}; }
template <char ...digits> constexpr static auto operator "" _u74() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint74_t{x}; }
template <char ...digits> constexpr static auto operator "" _u75() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint75_t{x}; }
template <char ...digits> constexpr static auto operator "" _u76() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint76_t{x}; }
template <char ...digits> constexpr static auto operator "" _u77() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint77_t{x}; }
template <char ...digits> constexpr static auto operator "" _u78() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint78_t{x}; }
template <char ...digits> constexpr static auto operator "" _u79() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint79_t{x}; }
// 80--89
template <char ...digits> constexpr static auto operator "" _u80() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint80_t{x}; }
template <char ...digits> constexpr static auto operator "" _u81() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint81_t{x}; }
template <char ...digits> constexpr static auto operator "" _u82() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint82_t{x}; }
template <char ...digits> constexpr static auto operator "" _u83() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint83_t{x}; }
template <char ...digits> constexpr static auto operator "" _u84() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint84_t{x}; }
template <char ...digits> constexpr static auto operator "" _u85() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint85_t{x}; }
template <char ...digits> constexpr static auto operator "" _u86() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint86_t{x}; }
template <char ...digits> constexpr static auto operator "" _u87() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint87_t{x}; }
template <char ...digits> constexpr static auto operator "" _u88() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint88_t{x}; }
template <char ...digits> constexpr static auto operator "" _u89() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint89_t{x}; }
// 90--99
template <char ...digits> constexpr static auto operator "" _u90() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint90_t{x}; }
template <char ...digits> constexpr static auto operator "" _u91() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint91_t{x}; }
template <char ...digits> constexpr static auto operator "" _u92() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint92_t{x}; }
template <char ...digits> constexpr static auto operator "" _u93() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint93_t{x}; }
template <char ...digits> constexpr static auto operator "" _u94() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint94_t{x}; }
template <char ...digits> constexpr static auto operator "" _u95() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint95_t{x}; }
template <char ...digits> constexpr static auto operator "" _u96() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint96_t{x}; }
template <char ...digits> constexpr static auto operator "" _u97() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint97_t{x}; }
template <char ...digits> constexpr static auto operator "" _u98() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint98_t{x}; }
template <char ...digits> constexpr static auto operator "" _u99() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint99_t{x}; }
// 100--109
template <char ...digits> constexpr static auto operator "" _u100() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint100_t{x}; }
template <char ...digits> constexpr static auto operator "" _u101() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint101_t{x}; }
template <char ...digits> constexpr static auto operator "" _u102() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint102_t{x}; }
template <char ...digits> constexpr static auto operator "" _u103() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint103_t{x}; }
template <char ...digits> constexpr static auto operator "" _u104() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint104_t{x}; }
template <char ...digits> constexpr static auto operator "" _u105() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint105_t{x}; }
template <char ...digits> constexpr static auto operator "" _u106() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint106_t{x}; }
template <char ...digits> constexpr static auto operator "" _u107() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint107_t{x}; }
template <char ...digits> constexpr static auto operator "" _u108() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint108_t{x}; }
template <char ...digits> constexpr static auto operator "" _u109() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint109_t{x}; }
// 110--119
template <char ...digits> constexpr static auto operator "" _u110() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint110_t{x}; }
template <char ...digits> constexpr static auto operator "" _u111() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint111_t{x}; }
template <char ...digits> constexpr static auto operator "" _u112() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint112_t{x}; }
template <char ...digits> constexpr static auto operator "" _u113() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint113_t{x}; }
template <char ...digits> constexpr static auto operator "" _u114() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint114_t{x}; }
template <char ...digits> constexpr static auto operator "" _u115() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint115_t{x}; }
template <char ...digits> constexpr static auto operator "" _u116() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint116_t{x}; }
template <char ...digits> constexpr static auto operator "" _u117() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint117_t{x}; }
template <char ...digits> constexpr static auto operator "" _u118() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint118_t{x}; }
template <char ...digits> constexpr static auto operator "" _u119() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint119_t{x}; }
// 120--128
template <char ...digits> constexpr static auto operator "" _u120() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint120_t{x}; }
template <char ...digits> constexpr static auto operator "" _u121() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint121_t{x}; }
template <char ...digits> constexpr static auto operator "" _u122() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint122_t{x}; }
template <char ...digits> constexpr static auto operator "" _u123() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint123_t{x}; }
template <char ...digits> constexpr static auto operator "" _u124() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint124_t{x}; }
template <char ...digits> constexpr static auto operator "" _u125() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint125_t{x}; }
template <char ...digits> constexpr static auto operator "" _u126() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint126_t{x}; }
template <char ...digits> constexpr static auto operator "" _u127() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint127_t{x}; }
template <char ...digits> constexpr static auto operator "" _u128() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint128_t{x}; }
template <char ...digits> constexpr static auto operator "" _u129() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint129_t{x}; }

// 120--139
template <char ...digits> constexpr static auto operator "" _u130() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint130_t{x}; }
template <char ...digits> constexpr static auto operator "" _u131() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint131_t{x}; }
template <char ...digits> constexpr static auto operator "" _u132() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint132_t{x}; }
template <char ...digits> constexpr static auto operator "" _u133() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint133_t{x}; }
template <char ...digits> constexpr static auto operator "" _u134() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint134_t{x}; }
template <char ...digits> constexpr static auto operator "" _u135() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint135_t{x}; }
template <char ...digits> constexpr static auto operator "" _u136() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint136_t{x}; }
template <char ...digits> constexpr static auto operator "" _u137() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint137_t{x}; }
template <char ...digits> constexpr static auto operator "" _u138() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint138_t{x}; }
template <char ...digits> constexpr static auto operator "" _u139() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint139_t{x}; }
// 140--149
template <char ...digits> constexpr static auto operator "" _u140() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint140_t{x}; }
template <char ...digits> constexpr static auto operator "" _u141() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint141_t{x}; }
template <char ...digits> constexpr static auto operator "" _u142() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint142_t{x}; }
template <char ...digits> constexpr static auto operator "" _u143() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint143_t{x}; }
template <char ...digits> constexpr static auto operator "" _u144() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint144_t{x}; }
template <char ...digits> constexpr static auto operator "" _u145() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint145_t{x}; }
template <char ...digits> constexpr static auto operator "" _u146() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint146_t{x}; }
template <char ...digits> constexpr static auto operator "" _u147() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint147_t{x}; }
template <char ...digits> constexpr static auto operator "" _u148() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint148_t{x}; }
template <char ...digits> constexpr static auto operator "" _u149() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint149_t{x}; }
// 150--159
template <char ...digits> constexpr static auto operator "" _u150() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint150_t{x}; }
template <char ...digits> constexpr static auto operator "" _u151() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint151_t{x}; }
template <char ...digits> constexpr static auto operator "" _u152() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint152_t{x}; }
template <char ...digits> constexpr static auto operator "" _u153() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint153_t{x}; }
template <char ...digits> constexpr static auto operator "" _u154() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint154_t{x}; }
template <char ...digits> constexpr static auto operator "" _u155() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint155_t{x}; }
template <char ...digits> constexpr static auto operator "" _u156() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint156_t{x}; }
template <char ...digits> constexpr static auto operator "" _u157() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint157_t{x}; }
template <char ...digits> constexpr static auto operator "" _u158() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint158_t{x}; }
template <char ...digits> constexpr static auto operator "" _u159() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint159_t{x}; }
// 160--169
template <char ...digits> constexpr static auto operator "" _u160() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint160_t{x}; }
template <char ...digits> constexpr static auto operator "" _u161() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint161_t{x}; }
template <char ...digits> constexpr static auto operator "" _u162() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint162_t{x}; }
template <char ...digits> constexpr static auto operator "" _u163() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint163_t{x}; }
template <char ...digits> constexpr static auto operator "" _u164() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint164_t{x}; }
template <char ...digits> constexpr static auto operator "" _u165() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint165_t{x}; }
template <char ...digits> constexpr static auto operator "" _u166() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint166_t{x}; }
template <char ...digits> constexpr static auto operator "" _u167() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint167_t{x}; }
template <char ...digits> constexpr static auto operator "" _u168() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint168_t{x}; }
template <char ...digits> constexpr static auto operator "" _u169() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint169_t{x}; }
// 170-179
template <char ...digits> constexpr static auto operator "" _u170() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint170_t{x}; }
template <char ...digits> constexpr static auto operator "" _u171() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint171_t{x}; }
template <char ...digits> constexpr static auto operator "" _u172() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint172_t{x}; }
template <char ...digits> constexpr static auto operator "" _u173() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint173_t{x}; }
template <char ...digits> constexpr static auto operator "" _u174() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint174_t{x}; }
template <char ...digits> constexpr static auto operator "" _u175() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint175_t{x}; }
template <char ...digits> constexpr static auto operator "" _u176() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint176_t{x}; }
template <char ...digits> constexpr static auto operator "" _u177() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint177_t{x}; }
template <char ...digits> constexpr static auto operator "" _u178() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint178_t{x}; }
template <char ...digits> constexpr static auto operator "" _u179() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint179_t{x}; }
// 180--189
template <char ...digits> constexpr static auto operator "" _u180() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint180_t{x}; }
template <char ...digits> constexpr static auto operator "" _u181() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint181_t{x}; }
template <char ...digits> constexpr static auto operator "" _u182() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint182_t{x}; }
template <char ...digits> constexpr static auto operator "" _u183() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint183_t{x}; }
template <char ...digits> constexpr static auto operator "" _u184() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint184_t{x}; }
template <char ...digits> constexpr static auto operator "" _u185() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint185_t{x}; }
template <char ...digits> constexpr static auto operator "" _u186() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint186_t{x}; }
template <char ...digits> constexpr static auto operator "" _u187() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint187_t{x}; }
template <char ...digits> constexpr static auto operator "" _u188() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint188_t{x}; }
template <char ...digits> constexpr static auto operator "" _u189() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint189_t{x}; }
// 190--199
template <char ...digits> constexpr static auto operator "" _u190() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint190_t{x}; }
template <char ...digits> constexpr static auto operator "" _u191() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint191_t{x}; }
template <char ...digits> constexpr static auto operator "" _u192() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint192_t{x}; }
template <char ...digits> constexpr static auto operator "" _u193() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint193_t{x}; }
template <char ...digits> constexpr static auto operator "" _u194() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint194_t{x}; }
template <char ...digits> constexpr static auto operator "" _u195() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint195_t{x}; }
template <char ...digits> constexpr static auto operator "" _u196() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint196_t{x}; }
template <char ...digits> constexpr static auto operator "" _u197() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint197_t{x}; }
template <char ...digits> constexpr static auto operator "" _u198() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint198_t{x}; }
template <char ...digits> constexpr static auto operator "" _u199() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint199_t{x}; }
// 200--209
template <char ...digits> constexpr static auto operator "" _u200() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint200_t{x}; }
template <char ...digits> constexpr static auto operator "" _u201() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint201_t{x}; }
template <char ...digits> constexpr static auto operator "" _u202() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint202_t{x}; }
template <char ...digits> constexpr static auto operator "" _u203() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint203_t{x}; }
template <char ...digits> constexpr static auto operator "" _u204() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint204_t{x}; }
template <char ...digits> constexpr static auto operator "" _u205() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint205_t{x}; }
template <char ...digits> constexpr static auto operator "" _u206() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint206_t{x}; }
template <char ...digits> constexpr static auto operator "" _u207() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint207_t{x}; }
template <char ...digits> constexpr static auto operator "" _u208() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint208_t{x}; }
template <char ...digits> constexpr static auto operator "" _u209() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint209_t{x}; }
// 210--219
template <char ...digits> constexpr static auto operator "" _u210() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint210_t{x}; }
template <char ...digits> constexpr static auto operator "" _u211() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint211_t{x}; }
template <char ...digits> constexpr static auto operator "" _u212() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint212_t{x}; }
template <char ...digits> constexpr static auto operator "" _u213() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint213_t{x}; }
template <char ...digits> constexpr static auto operator "" _u214() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint214_t{x}; }
template <char ...digits> constexpr static auto operator "" _u215() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint215_t{x}; }
template <char ...digits> constexpr static auto operator "" _u216() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint216_t{x}; }
template <char ...digits> constexpr static auto operator "" _u217() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint217_t{x}; }
template <char ...digits> constexpr static auto operator "" _u218() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint218_t{x}; }
template <char ...digits> constexpr static auto operator "" _u219() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint219_t{x}; }
// 220--229
template <char ...digits> constexpr static auto operator "" _u220() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint220_t{x}; }
template <char ...digits> constexpr static auto operator "" _u221() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint221_t{x}; }
template <char ...digits> constexpr static auto operator "" _u222() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint222_t{x}; }
template <char ...digits> constexpr static auto operator "" _u223() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint223_t{x}; }
template <char ...digits> constexpr static auto operator "" _u224() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint224_t{x}; }
template <char ...digits> constexpr static auto operator "" _u225() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint225_t{x}; }
template <char ...digits> constexpr static auto operator "" _u226() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint226_t{x}; }
template <char ...digits> constexpr static auto operator "" _u227() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint227_t{x}; }
template <char ...digits> constexpr static auto operator "" _u228() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint228_t{x}; }
template <char ...digits> constexpr static auto operator "" _u229() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint229_t{x}; }
// 230--239
template <char ...digits> constexpr static auto operator "" _u230() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint230_t{x}; }
template <char ...digits> constexpr static auto operator "" _u231() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint231_t{x}; }
template <char ...digits> constexpr static auto operator "" _u232() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint232_t{x}; }
template <char ...digits> constexpr static auto operator "" _u233() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint233_t{x}; }
template <char ...digits> constexpr static auto operator "" _u234() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint234_t{x}; }
template <char ...digits> constexpr static auto operator "" _u235() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint235_t{x}; }
template <char ...digits> constexpr static auto operator "" _u236() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint236_t{x}; }
template <char ...digits> constexpr static auto operator "" _u237() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint237_t{x}; }
template <char ...digits> constexpr static auto operator "" _u238() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint238_t{x}; }
template <char ...digits> constexpr static auto operator "" _u239() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint239_t{x}; }
// 240--249
template <char ...digits> constexpr static auto operator "" _u240() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint240_t{x}; }
template <char ...digits> constexpr static auto operator "" _u241() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint241_t{x}; }
template <char ...digits> constexpr static auto operator "" _u242() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint242_t{x}; }
template <char ...digits> constexpr static auto operator "" _u243() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint243_t{x}; }
template <char ...digits> constexpr static auto operator "" _u244() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint244_t{x}; }
template <char ...digits> constexpr static auto operator "" _u245() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint245_t{x}; }
template <char ...digits> constexpr static auto operator "" _u246() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint246_t{x}; }
template <char ...digits> constexpr static auto operator "" _u247() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint247_t{x}; }
template <char ...digits> constexpr static auto operator "" _u248() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint248_t{x}; }
template <char ...digits> constexpr static auto operator "" _u249() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint249_t{x}; }
// 250--256
template <char ...digits> constexpr static auto operator "" _u250() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint250_t{x}; }
template <char ...digits> constexpr static auto operator "" _u251() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint251_t{x}; }
template <char ...digits> constexpr static auto operator "" _u252() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint252_t{x}; }
template <char ...digits> constexpr static auto operator "" _u253() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint253_t{x}; }
template <char ...digits> constexpr static auto operator "" _u254() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint254_t{x}; }
template <char ...digits> constexpr static auto operator "" _u255() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint255_t{x}; }
template <char ...digits> constexpr static auto operator "" _u256() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::modints::modint256_t{x}; }

}  // namespace modints

}  // namespace literals

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
    static constexpr dpf::modint<Nbits> max() noexcept { return ~dpf::modint<Nbits>{0}; }
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
