/// @file dpf/keyword.hpp
/// @brief defines `dpf::keyword` and associated helpers
/// @details A `dpf::keyword` is an integer representation of a fixed-length
///          string over a given alphabet. The integer representation uses
///          the fewest bits possible for the given string length and alphabet
///          size and uses an encoding that preserves the lexicographic
///          ordering of the underlying strings. This type is inteded to be
///          used as an input type for a DPF and, as such, specializes
///          `dpf::utils::bitlength_of`, `dpf::utils::msb_of`, and
///          `dpf::utils::countl_zero_symmmetric_difference`. When used as a
///          DPF input type, the aforementioned properties of the encoding
///          equate to minimizing the DPF tree depth and maximizing the
///          potential for effective memoization in the context of
///          `eval_point`- and `eval_sequence`-based evaluation. As a discreet
///          (non-numeric) type, `dpf::keyword`s are not optimized for use in
///          `eval_interval`-based evaluation.
///
///          This file also defines the `dpf::alphabets` namespace, which
///          defines various alphabets of interest, including the printable
///          ASCII characters (`dpf::alphabets::printable_ascii`), lowercase
///          Roman letters (`dpf::alphabets::lowercase_alpha`), lowercase
///          hexademical digits (`dpf::alpbabets::lowercase_hex`), among
///          others.
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_KEYWORD_HPP__
#define LIBDPF_INCLUDE_DPF_KEYWORD_HPP__

#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <memory>

#include "hedley/hedley.h"

#include "dpf/utils.hpp"

namespace dpf
{

/// @brief defines common alphabets for convenient use with `dpf::keyword`
namespace alphabets
{
    /// @brief the printable ASCII chars
    static constexpr char printable_ascii[] = " !\"#$%&'()*+,-./0123456789:"\
                                               ";<=>?@ABCDEFGHIJKLMNOPQRSTUV"\
                                               "WXYZ[\\]^_`abcdefghijklmnopq"\
                                               "rstuvwxyz{|}~";
    /// @brief the lowercase Roman alphabet
    static constexpr char lowercase_alpha[] = "abcdefghijklmnopqrstuvwxyz";
    /// @brief the lowercase and uppercase Roman alphabet
    static constexpr char alpha[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKL"\
                                     "MNOPQRSTUVWXYZ";
    /// @brief the lowercase and uppercase Roman alphabet plus digits 0-9
    static constexpr char alphanumeric[] = "abcdefghijklmnopqrstuvwxyzABCDE"\
                                            "FGHIJKLMNOPQRSTUVWXYZ0123456789";
    /// @brief the lowercase Roman alphabet plus digits 0-9
    static constexpr char lowercase_alphanumeric[] = "abcdefghijklmnopqrstu"\
                                                      "vwxyz0123456789";
    /// @brief binary
    static constexpr char binary[] = "01";
    /// @brief hex digits
    static constexpr char hex[] = "0123456789abcdefABCDEF";
    /// @brief hex w/ lowercase letters
    static constexpr char lowercase_hex[] = "0123456789abcdef";
    /// @brief hex w/ uppercase letters
    static constexpr char uppercase_hex[] = "0123456789ABCDEF";
    /// @brief base64 digits
    static constexpr char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijk"\
                                      "lmnopqrstuvwxyz0123456789+/=";
}  // namespace alphabets

template <std::size_t N,
          typename CharT = char,
          const CharT * Alphabet = alphabets::printable_ascii,
          class Traits = std::char_traits<CharT>,
          class Allocator = std::allocator<CharT>>
class basic_fixed_length_string
{
  public:
    using string_view = std::basic_string_view<CharT, Traits>;

    /// @brief the alphabet over which the string is constructed
    static constexpr string_view alphabet = Alphabet;

    /// @brief radix used by the integer representation of the string
    static constexpr std::size_t radix = alphabet.size()+1;

    /// @brief the (maximum) length of a string
    static constexpr std::size_t max_length = N;

    /// @brief the empty string
    static constexpr auto empty_string = basic_fixed_length_string{.};

    /// @brief the number of bits needed to uniquely represent any string
    ///        of length at most `max_length` over `alphabet`
    static constexpr std::size_t bits
        = std::ceil(max_length*std::log2(radix));

    static_assert(!alphabet.empty(), "alphabet must be non-empty");
    static_assert(N != 0, "maximum string length must be positive");

    /// @brief the primitive integral type used to represent the string
    static_assert(bits && bits <= 128, "representation must fit in 128 bits");
    using integral_type = dpf::utils::integral_type_from_bitlength_t<bits>;

    /// @brief construct the `basic_fixed_length_string`
    /// @{

    /// @brief default constructor
    /// @details Constructs the `basic_fixed_length_string` with a value
    ///          corresponding to the empty string.
    constexpr basic_fixed_length_string() noexcept = default;

    /// @brief copy constructor
    /// @details Constructs the `basic_fixed_length_string` with a value
    ///          copied from another `basic_fixed_length_string`.
    constexpr
    basic_fixed_length_string(const basic_fixed_length_string &)
    noexcept = default;

    /// @brief move constructor
    /// @details Constructs the `basic_fixed_length_string` from another
    ///          `basic_fixed_length_string` using move semantics.
    constexpr
    basic_fixed_length_string(basic_fixed_length_string &&)
    noexcept = default;

    /// @brief value constructor
    /// @details Constructs a `basic_fixed_length_string` whose value is
    ///          initialized to the integer representation of `str`.
    /// @param str the string to initialize with
    constexpr
    basic_fixed_length_string(string_view str)  // NOLINT
    noexcept
      : val{encode_(str)} { }

    /// @brief value constructor
    /// @details Constructs a `basic_fixed_length_string` whose value is
    ///          initialized to the integer representation of `str`.
    /// @param str the string to initialize with
    constexpr
    basic_fixed_length_string(const CharT * str)  // NOLINT
    noexcept
      : val{encode_(str)} { }

    /// @}

    /// @brief assign the `basic_fixed_length_string`
    /// @{

    /// @brief value assignment
    /// @details Sets the value of this `basic_fixed_length_string` to
    ///          the integer representation of `str`.
    /// @param str the string to assign with
    constexpr auto & operator=(string_view str) noexcept
    {
        val = encode_(str);
    }

    /// @brief copy assignment
    /// @details Assigns the `basic_fixed_length_string` with a value
    ///          copied from another `basic_fixed_length_string`.
    constexpr basic_fixed_length_string &
    operator=(const basic_fixed_length_string &) = default;

    /// @brief move assignment
    /// @details Assigns the `basic_fixed_length_string` from another
    ///          `basic_fixed_length_string` using move semantics.
    constexpr basic_fixed_length_string &
    operator=(basic_fixed_length_string &&) = default;

    /// @}

    /// @brief recreates the string representation of this
    ///        `basic_fixed_length_string`
    /// @complexity `O(N)` where `N` is the maximum string length
    constexpr
    operator std::basic_string<CharT, Traits, Allocator>() const noexcept
    {
        std::basic_stringstream<CharT, Traits, Allocator> ss{};
        auto tmp = val;
        while (tmp != 0)                           // O(N)
        {
            ss << alphabet[(tmp % radix)-1];
            tmp /= radix;
        }
        auto s = ss.str();                         // O(N)
        std::reverse(std::begin(s), std::end(s));  // O(N)
        return s;
    }

    /// @brief retrieve the integer representation of this
    ///        `basic_fixed_length_string`
    /// @return the integer representation of this string
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr operator integral_type() const noexcept
    {
        return val;
    }

  private:
    /// @brief converts a string of length at-most `max_length` over
    ///        `alphabet` into an integer
    /// @throws `std::length_error` if `str` exceeds `max_length`
    /// @throws `std::domain_error` if `str` contains a char not in `alphabet`
    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    static constexpr integral_type encode_(string_view str)
    {
        constexpr auto npos = string_view::npos;

        utils::constexpr_maybe_throw<std::length_error>(
            str.size() > max_length,
            "str.size() cannot exceed max_length");

        integral_type val{0};
        for (CharT c : str)
        {
            auto next_digit = alphabet.find(c);
            utils::constexpr_maybe_throw<std::domain_error>(
                next_digit == npos,
                "str contains a disallowed char");
            val = val * radix + next_digit + 1;
        }
        return val;
    }

    /// @brief the `integral_type` used to store the integer representation of
    ///        this string
    integral_type val;

    /// @brief performs stream input and output on
    ///        `dpf::basic_fixed_length_string`s
    /// @{

    /// @brief performs stream output on a `dpf::basic_fixed_length_string`
    /// @details Writes a `dpf::basic_fixed_length_string` to the character
    ///          stream `os` by writing the `integral_type` that holds its its
    ///          integer value into `os` using `operator<<` (which is a
    ///          `FormattedOutputFunction` for strings).
    /// @param os a character output stream
    /// @param k the `basic_fixed_length_string` to extract from the input
    ///          stream
    /// @return `is`
    friend std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> & os,
        const basic_fixed_length_string & k)
    {
        return os << k.val;
    }

    /// @brief performs stream input on a `dpf::basic_fixed_length_string`
    /// @details Extracts one `integral_type` from `is` and stores it as the
    ///          integer representation of `k`.
    /// @param is a character input stream
    /// @param k the `basic_fixed_length_string` to extract from the input
    ///          stream
    /// @return `is`
    friend std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT, Traits> & is,
        basic_fixed_length_string & k)
    {
        return is >> k.val;
    }

    /// @}
};  // class dpf::basic_fixed_length_string

/// @brief instantiation of the `dpf::basic_fixed_length_string` class that
///        uses `char` (i.e., bytes) as its **character type**, with its
///        default `char_traits` and `allocator` types (see
///        `dpf::basic_fixed_length_string` for more info on the template).
template <std::size_t N,
          const char * Alphabet = alphabets::lowercase_alpha>
using keyword = basic_fixed_length_string<N, char, Alphabet>;

/// @brief convert a `dpf::basic_fixed_length_string` to a `std::basic_string`
/// @details Uses a `static_cast` to convert `str` to recreate the string
///          representation of a `basic_fixed_length_string`
/// @complexity `O(N)` where `N` is the maximum string length
template <std::size_t N,
          typename CharT,
          const CharT * Alphabet,
          class Traits = std::char_traits<CharT>,
          class Allocator = std::allocator<CharT>>
static constexpr std::basic_string<CharT, Traits, Allocator>
to_string(basic_fixed_length_string<N, CharT, Alphabet, Traits, Allocator>
    str) noexcept
{
    return static_cast<std::basic_string<CharT, Traits, Allocator>>(str);
}

namespace utils
{

/// @brief specializes `dpf::bitlength_of` for `dpf::basic_fixed_length_string`
template <std::size_t N,
          typename CharT,
          const CharT * Alpha,
          class Traits,
          class Alloc>
struct bitlength_of<
    dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>>
  : public std::integral_constant<std::size_t,
    dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>::bits> { };

/// @brief specializes `dpf::msb_of` for `dpf::basic_fixed_length_string`
template <std::size_t N,
          typename CharT,
          const CharT * Alpha,
          class Traits,
          class Alloc>
struct msb_of<dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>>
{
    using T = dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>;
    using U = typename T::integral_type;
    static constexpr U value
        = U{1} << bitlength_of_v<T> - 1ul;
};

/// @brief specializes `dpf::countl_zero_symmmetric_difference` for
///        `dpf::basic_fixed_length_string`
template <std::size_t N,
          typename CharT,
          const CharT * Alpha,
          class Traits,
          class Alloc>
struct countl_zero_symmmetric_difference<dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>>
{
    using T = dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>;
    static constexpr auto clz = dpf::utils::countl_zero_symmmetric_difference<typename T::integral_type>{};

    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    constexpr std::size_t operator()(const T & lhs, const T & rhs) const noexcept
    {
        constexpr auto adjust = utils::bitlength_of_v<typename T::integral_type>-T::bits;
        return clz(static_cast<typename T::integral_type>(lhs),
                   static_cast<typename T::integral_type>(rhs)) - adjust;
    }
};

}  // namespace utils

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_KEYWORD_HPP__
