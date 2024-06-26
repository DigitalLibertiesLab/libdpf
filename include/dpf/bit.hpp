/// @file dpf/bit.hpp
/// @brief defines `dpf::bit` and associated helpers
/// @details A `dpf::bit` is a binary type whose representation can be packed
///          into one bit. It is implemented as an `enum` with two values:
///          `zero` and `one`. This type is intended for us as an [output type](@ref output_types)
///          for a DPF, in which case leaf nodes will be packed in much
///          the ways as in an `std::bitset` or `std::vector<bool>`.
///
///          In addition to `dpf::bit`, this file defines three overloaded
///          variants of a `dpf::to_bit` function that respectively convert
///          a `bool, a `char`, or (the least significant bit of) an `int` to
///          a `dpf::bit`. Likewise, it defines `dpf::to_string` to convert
///          a `dpf::bit` into an `std::string`. Finally, it overloads stream
///          input and output operators (`<<` and `>>`) for `dpf::bit`.
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_BIT_HPP__
#define LIBDPF_INCLUDE_DPF_BIT_HPP__

#include <cstddef>
#include <type_traits>
#include <limits>
#include <stdexcept>
#include <string>
#include <memory>
#include <ostream>
#include <istream>

#include "hedley/hedley.h"

#include "dpf/utils.hpp"

/// @brief the dpf namespace
namespace dpf
{

/// @brief binary type whose representation can be packed into one bit
enum bit : bool
{
    zero = false,  ///< `0`, `false`, "unset", "off"
    one = true     ///< `1`, `true`, "set", "on"
};

/// @brief converts a value to a `dpf::bit`
/// @{

/// @brief converts (the lsb of) an `int` to a `dpf::bit`
/// @details Convert an `int` to a `dpf::bit`. The resulting `dpf::bit` is
///          equal to `dpf::bit::one` if the *least-significant bit* of
///          `value` is `1` and `dpf::bit::zero` otherwise.
/// @param value the `int` to convert
/// @returns `static_cast<dpf::bit>(value & 1)`
HEDLEY_CONST
HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
static constexpr dpf::bit to_bit(int value) noexcept
{
    return static_cast<dpf::bit>(value & 1);
}

/// @brief converts a `bool` to a `dpf::bit`
/// @details Convert a `bool` to a `dpf::bit`. The resulting `dpf::bit` is
///          equal to `dpf::bit::one` if `value==true` and `dpf::bit::zero`
///          otherwise.
/// @param value the `bool` to convert
/// @returns `static_cast<dpf::bit>(value)`
HEDLEY_CONST
HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
static constexpr dpf::bit to_bit(bool value) noexcept
{
    return static_cast<dpf::bit>(value);
}

/// @brief converts a character to a `dpf::bit`
/// @details Convert a character to a `dpf::bit`. The resulting `dpf::bit` is
///          equal to `dpf::bit::one` if `value==one` and `dpf::bit::zero`
///          otherwise.
/// @param value the character to convert
/// @param zero character used to represent `0` (default: ``CharT('0')``)
/// @param one character used to represent `1` (default: ``CharT('1')``)
/// @returns `static_cast<dpf::bit>(0)` if `value==0` or
///          `static_cast<dpf::bit>(1)` if `value==1`
/// @throws std::domain_error if `value != zero && value != one`
template <typename CharT,
          typename Traits = std::char_traits<CharT>>
HEDLEY_CONST
HEDLEY_ALWAYS_INLINE
static constexpr dpf::bit to_bit(
    CharT value,
    CharT zero = CharT('0'),
    CharT one = CharT('1'))
{
    if (!Traits::eq(value, zero) && !Traits::eq(value, one))
    {
        throw std::domain_error("Unrecognized character");
    }
    return Traits::eq(value, zero) ? dpf::bit::zero : dpf::bit::one;
}

/// @}

/// @brief converts a `dpf::bit` to a `std::basic_string`
/// @details Converts the contents of a `dpf::bit` to a `std::string` for
///          human-friendly printing. Uses `zero` to represent the value
///          `0` and `one` to the value `1`.
/// @param value the `dpf::bit` to convert
/// @param zero character to use to represent `false`/`0` (default: ``CharT('0')``)
/// @param one character to use to represent `true`/`1` (default: ``CharT('1')``)
/// @return `(value == 0) ? zero : one`
template <typename CharT = char,
          typename Traits = std::char_traits<CharT>,
          typename Allocator = std::allocator<CharT>>
static std::basic_string<CharT, Traits, Allocator> to_string(
    dpf::bit value,
    CharT zero = CharT('0'),
    CharT one = CharT('1'))
{
    auto ch = (value == dpf::bit::zero) ? zero : one;
    return std::basic_string<CharT>(1, ch, Allocator{});
}

/// @brief performs stream input and output on `dpf::bit`s
/// @{

/// @brief performs stream output on a `dpf::bit`
/// @details Writes a `dpf::bit` to the character stream `os` as if by first
///          converting it to a `std::basic_string<CharT, Traits>` using
///          `dpf::to_string()`, and then writing it into `os` using `operator<<`
///          (which is a `FormattedOutputFunction` for strings). The
///          characters to use for zero and one are obtained from the
///          currently-imbued locale by calling `os.widen()` with `0` and `1`
///          as the arguments.
/// @param os a character output stream
/// @param value the `dpf::bit` to insert into the output stream
/// @return `os`
template <typename CharT,
          typename Traits>
std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> & os, const dpf::bit & value)
{
    return os << to_string<CharT, Traits>(value, os.widen('0'),
        os.widen('1'));
}

/// @brief performs stream input on a `dpf::bit`
/// @details Extracts one character from `is` and attempts to convert it to
///          a `dpf::bit` using `dpf::to_bit()`. If successful, the result is
///          stored in `value`. The characters to use for zero and one are
///          obtained from the currently-imbued locale by calling `is.widen()`
///          with `0` and `1` as the arguments.
/// @param is a character input stream
/// @param value the `dpf::bit` to extract from the input stream
/// @return `is`
template <typename CharT,
          typename Traits>
std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> & is, dpf::bit & value)
{
    try
    {
        value = to_bit<CharT>(is.get(), is.widen('0'), is.widen('1'));
    }
    catch(const std::exception & e)
    {
        is.setstate(std::ios::failbit);
    }
    return is;
}

/// @}

dpf::bit operator+(dpf::bit lhs, dpf::bit rhs) noexcept
{
    return static_cast<dpf::bit>(lhs ^ rhs);
}

namespace utils
{

/// @brief specializes `dpf::utils::bitlength_of` for `dpf::bit`
template <>
struct bitlength_of<dpf::bit>
  : public std::integral_constant<std::size_t, 1> { };

template <typename NodeT>
struct bitlength_of_output<dpf::bit, NodeT>
  : public std::integral_constant<std::size_t, 1> { };

template <>
struct make_from_integral_value<dpf::bit>
{
    constexpr dpf::bit operator()(bool val) const noexcept
    {
        return val ? dpf::bit::one : dpf::bit::zero;
    }
};

}  // namespace utils

namespace literals
{

namespace bit
{

constexpr static auto operator "" _bit(unsigned long long int x) { return dpf::to_bit(x); }

}  // namespace bit

}  // namespace literals

}  // namespace dpf

namespace std
{

/// @{

/// @brief specializes `std::numeric_limits` for `dpf::bit`
template<> class numeric_limits<dpf::bit>
    : public numeric_limits<bool> { };

/// @brief specializes `std::numeric_limits` for `dpf::bit const`
template<> class numeric_limits<dpf::bit const>
    : public numeric_limits<dpf::bit> {};

/// @brief specializes `std::numeric_limits` for `dpf::bit volatile`
template<> class numeric_limits<dpf::bit volatile>
    : public numeric_limits<dpf::bit> {};

/// @brief specializes `std::numeric_limits` for `dpf::bit const volatile`
template<> class numeric_limits<dpf::bit const volatile>
    : public numeric_limits<dpf::bit> {};

/// @}

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_BIT_HPP__
