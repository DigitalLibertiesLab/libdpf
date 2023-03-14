/// @file dpf/bit.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief defines the `dpf::bit` class and associated helpers
/// @details A `dpf::bit` is a binary type whose representation can be packed
///          into one bit. This type is inteded to be used as an output value
///          of a DPF, in which case leaf nodes will be packed in much the
///          ways as in an `std::bitset` or `std::vector<bool>`.
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see <a href="LICENSE">`LICENSE`</a> for details.

#ifndef LIBDPF_INCLUDE_DPF_BIT_HPP__
#define LIBDPF_INCLUDE_DPF_BIT_HPP__

#include <string>
#include <limits>
#include <memory>

#include "hedley/hedley.h"

#include "dpf/utils.hpp"

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
/// @param zero character used to represent `0` (default: ``CharT{'0'}``)
/// @param one character used to represent `1` (default: ``CharT{'1'}``)
/// @returns `static_cast<dpf::bit>(0)` if `value==0` or
///          `static_cast<dpf::bit>(1)` if `value==1`
/// @throws `std::domain_error` if `value != zero && value != one`
template <typename CharT,
          class Traits = std::char_traits<CharT>>
HEDLEY_CONST
HEDLEY_ALWAYS_INLINE
static constexpr dpf::bit to_bit(
    CharT value,
    CharT zero = CharT{'0'},
    CharT one = CharT{'1'})
{
    if (!Traits::eq(value, zero) && !Traits::eq(value, one))
    {
        throw std::domain_error("Unrecognized character");
    }
    return (value == zero) ? dpf::bit::zero : dpf::bit::one;
}

/// @}

/// @brief converts a `dpf::bit` to a `std::basic_string`
/// @details Converts the contents of a `dpf::bit` to a `std::string` for
///          human-friendly printing. Uses `zero` to represent the value
///          `0` and `one` to the value `1`.
/// @param value the `dpf::bit` to convert
/// @param zero character to use to represent `false`/`0` (default: ``CharT{'0'}``)
/// @param one character to use to represent `true`/`1` (default: ``CharT{'1'}``)
/// @return `(value == 0) ? zero : one`
template <class CharT = char,
          class Traits = std::char_traits<CharT>,
          class Allocator = std::allocator<CharT>>
static constexpr std::basic_string<CharT, Traits, Allocator> to_string(
    dpf::bit value,
    CharT zero = CharT{'0'},
    CharT one = CharT{'1'}) noexcept
{
    auto ch = (value == false) ? zero : one;
    return std::basic_string(&ch, 1, Allocator{});
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
template <class CharT,
          class Traits>
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
/// @throws `std::domain_error` if `value != zero && value != one`
template <class CharT,
          class Traits>
std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT, Traits> & is, dpf::bit & value)
{
    value = to_bit<CharT>(is.get(), is.widen('0'), is.widen('1'));
    return is;
}

/// @}

namespace utils
{

/// @brief specializes `dpf::bitlength_of` for `dpf::bit`
template <>
struct bitlength_of<dpf::bit>
  : public std::integral_constant<std::size_t, 1> { };

}  // namespace utils

}  // namespace dpf

namespace std
{

/// @brief specializes `std::numeric_limits` for CV-qualified `dpf::bit`s
/// @{

/// @details specializes `std::numeric_limits` for `dpf::bit`
template<> class numeric_limits<dpf::bit>
    : public numeric_limits<bool> { };
/// @details specializes `std::numeric_limits` for `dpf::bit const`
template<> class numeric_limits<dpf::bit const>
    : public numeric_limits<dpf::bit> {};
/// @details specializes `std::numeric_limits` for `dpf::bit volatile`
template<> class numeric_limits<dpf::bit volatile>
    : public numeric_limits<dpf::bit> {};
/// @details specializes `std::numeric_limits` for `dpf::bit const volatile`
template<> class numeric_limits<dpf::bit const volatile>
    : public numeric_limits<dpf::bit> {};

/// @}

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_BIT_HPP__
