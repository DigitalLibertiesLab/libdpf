/// @file dpf/keyword.hpp
/// @brief defines `dpf::keyword` and associated helpers
/// @details A `dpf::keyword` is an integer representation of a fixed-length
///          string over a given alphabet. The integer representation uses
///          the fewest bits possible for the given string length and alphabet
///          size and uses an encoding that preserves the lexicographic
///          ordering of the underlying strings. This type is inteded to be
///          used as an input type for a DPF and, as such, specializes
///          `dpf::utils::bitlength_of`, `dpf::utils::msb_of`, and
///          `dpf::utils::countl_zero_symmetric_difference`. When used as a
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
/// @copyright Copyright (c) 2019-2023 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_KEYWORD_HPP__
#define LIBDPF_INCLUDE_DPF_KEYWORD_HPP__

#include <cstddef>
#include <cmath>
#include <type_traits>
#include <limits>
#include <string>
#include <string_view>
#include <memory>
#include <iterator>
#include <sstream>
#include <istream>
#include <ostream>
#include <stdexcept>

#include "hedley/hedley.h"

#include "dpf/utils.hpp"
#include "dpf/modint.hpp"

namespace dpf
{

/// @brief defines common alphabets for convenient use with `dpf::keyword`
namespace alphabets
{
    /// @brief the printable ASCII chars
    static constexpr char printable_ascii[] = "\0 !\"#$%&'()*+,-./0123456789:"\
                                               ";<=>?@ABCDEFGHIJKLMNOPQRSTUV"\
                                               "WXYZ[\\]^_`abcdefghijklmnopq"\
                                               "rstuvwxyz{|}~";
    /// @brief the lowercase Roman alphabet
    static constexpr char lowercase_alpha[] = "\0abcdefghijklmnopqrstuvwxyz";
    /// @brief the lowercase and uppercase Roman alphabet
    static constexpr char alpha[] = "\0abcdefghijklmnopqrstuvwxyzABCDEFGHIJKL"\
                                     "MNOPQRSTUVWXYZ";
    /// @brief the lowercase and uppercase Roman alphabet plus digits 0-9
    static constexpr char alphanumeric[] = "\0abcdefghijklmnopqrstuvwxyzABCDE"\
                                            "FGHIJKLMNOPQRSTUVWXYZ0123456789";
    /// @brief the lowercase Roman alphabet plus digits 0-9
    static constexpr char lowercase_alphanumeric[] = "\0abcdefghijklmnopqrstu"\
                                                      "vwxyz0123456789";
    /// @brief hashtags
    static constexpr char hashtag[] = "\0abcdefghijklmnopqrstuvwxyz#-";
    /// @brief binary
    static constexpr char binary[] = "01";
    /// @brief octal
    static constexpr char octal[] = "01234567";
    /// @brief decimal
    static constexpr char decimal[] = "0123456789";
    /// @brief hex w/ lowercase letters
    static constexpr char hex[] = "0123456789abcdef";
    /// @brief hex w/ uppercase letters
    static constexpr char uppercase_hex[] = "0123456789ABCDEF";
    /// @brief base64 digits
    static constexpr char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijk"\
                                      "lmnopqrstuvwxyz0123456789+/=";
}  // namespace alphabets

template <std::size_t N,
          typename CharT = char,
          const CharT * Alphabet = alphabets::printable_ascii,
          typename Traits = std::char_traits<CharT>,
          typename Allocator = std::allocator<CharT>>
class basic_fixed_length_string : public dpf::modint<static_cast<std::size_t>(std::ceil(N*std::log2(std::basic_string_view<CharT, Traits>(&Alphabet[1]).size() + 1)))>
{
  public:
    using string_view = std::basic_string_view<CharT, Traits>;

    /// @brief radix used by the integer representation of the string
    static constexpr std::size_t radix = string_view(&Alphabet[1]).size() + 1;

    /// @brief the alphabet over which the string is constructed
    static constexpr string_view alphabet = string_view(Alphabet, radix+1);

    /// @brief the (maximum) length of a string
    static constexpr std::size_t max_length = N;

    /// @brief the number of bits needed to uniquely represent any string
    ///        of length at most `max_length` over `alphabet`
    static constexpr std::size_t bits
        = std::ceil(max_length*std::log2(radix));

    static_assert(!alphabet.empty(), "alphabet must be non-empty");
    static_assert(N != 0, "maximum string length must be positive");

  private:
    using parent = dpf::modint<bits>;

  public:
    /// @brief the primitive integral type used to represent the string
    using integral_type = dpf::utils::nonvoid_integral_type_from_bitlength_t<bits>;

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
    // cppcheck-suppress noExplicitConstructor
    basic_fixed_length_string(string_view str)  // NOLINT(runtime/explicit)
    noexcept
      : parent::modint(encode_(str)) { }

    /// @brief value constructor
    /// @details Constructs a `basic_fixed_length_string` whose value is
    ///          initialized to the integer representation of `str`.
    /// @param str the string to initialize with
    constexpr
    // cppcheck-suppress noExplicitConstructor
    basic_fixed_length_string(const CharT * str)  // NOLINT(runtime/explicit)
    noexcept
      : parent::modint(encode_(str)) { }

    /// @}

    /// @brief assign the `basic_fixed_length_string`
    /// @{

    /// @brief value assignment
    /// @details Sets the value of this `basic_fixed_length_string` to
    ///          the integer representation of `str`.
    /// @param str the string to assign with
    constexpr basic_fixed_length_string & operator=(string_view str) noexcept
    {
        parent::operator=(encode_(str));
        return *this;
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
    operator=(basic_fixed_length_string &&) noexcept = default;

    /// @}

    ~basic_fixed_length_string() = default;

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr basic_fixed_length_string operator~() const noexcept
    {
        return basic_fixed_length_string{parent::operator~()};
    }

    /// @brief recreates the string representation of this
    ///        `basic_fixed_length_string`
    /// @complexity `O(N)` where `N` is the maximum string length
    constexpr
    operator std::basic_string<CharT, Traits, Allocator>() const noexcept
    {
        std::basic_stringstream<CharT, Traits, Allocator> ss{};
        auto tmp = parent::reduced_value();
        while (tmp != 0)                           // O(N)
        {
            ss << alphabet[(tmp % radix)-1];
            tmp /= radix;
        }
        auto s = ss.str();                         // O(N)
        std::reverse(std::begin(s), std::end(s));  // O(N)
        return s;
    }

  private:
    constexpr
    // cppcheck-suppress noExplicitConstructor
    basic_fixed_length_string(integral_type val)  // NOLINT(runtime/explicit)
    noexcept
      : parent::modint(val) { }

    constexpr basic_fixed_length_string(parent val) noexcept
      : parent::modint(val) { }

    /// @brief converts a string of length at-most `max_length` over
    ///        `alphabet` into an integer
    /// @throws `std::length_error` if `str` exceeds `max_length`
    /// @throws `std::domain_error` if `str` contains a char not in `alphabet`
    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    static constexpr integral_type encode_(string_view str)
    {
        constexpr auto npos = string_view::npos;
        using std::string_literals::operator""s;

        utils::constexpr_maybe_throw<std::length_error>(
            str.size() > max_length,
            "str.size() cannot exceed max_length");

        integral_type val{0};
        for (CharT c : str)
        {
            auto next_digit = alphabet.find(c);
            utils::constexpr_maybe_throw<std::domain_error>(
                next_digit == npos,
                "str contains a disallowed char: "s + c);
            val = val * radix + next_digit;
        }
        return val;
    }

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
        return os << k.reduced_value();
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
        integral_type tmp;
        is >> tmp;
        k = basic_fixed_length_string(tmp);
        return is;
    }

    /// @}

    friend struct utils::countl_zero_symmetric_difference<basic_fixed_length_string>;
    friend struct utils::msb_of<basic_fixed_length_string>;
    friend struct utils::mod_pow_2<basic_fixed_length_string>;
    friend struct utils::make_from_integral_value<basic_fixed_length_string>;
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
          typename Traits = std::char_traits<CharT>,
          typename Allocator = std::allocator<CharT>>
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
          typename Traits,
          typename Alloc>
struct bitlength_of<
    dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>>
  : public std::integral_constant<std::size_t,
    dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>::bits> { };

/// @brief specializes `dpf::msb_of` for `dpf::basic_fixed_length_string`
template <std::size_t N,
          typename CharT,
          const CharT * Alpha,
          typename Traits,
          typename Alloc>
struct msb_of<dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>>
{
    using T = dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>;
    using U = typename T::integral_type;
    static constexpr T value = U{1} << bitlength_of_v<T> - 1ul;
};

/// @brief specializes `dpf::countl_zero_symmetric_difference` for
///        `dpf::basic_fixed_length_string`
template <std::size_t N,
          typename CharT,
          const CharT * Alpha,
          typename Traits,
          typename Alloc>
struct countl_zero_symmetric_difference<dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>>
  : countl_zero_symmetric_difference<typename dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>::parent>
{ };

template <std::size_t N,
          typename CharT,
          const CharT * Alpha,
          typename Traits,
          typename Alloc>
struct mod_pow_2<dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>>
  : mod_pow_2<typename dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>::parent>
{ };

template <std::size_t N,
          typename CharT,
          const CharT * Alpha,
          typename Traits,
          typename Alloc>
struct make_from_integral_value<dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>>
{
    using T = dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>;
    using integral_type = integral_type_from_bitlength_t<bitlength_of_v<T>>;
    constexpr T operator()(integral_type val) const noexcept
    {
        return T{val};
    }
};

}  // namespace utils

}  // namespace dpf

namespace std
{

/// @brief specializes `std::numeric_limits` for CV-qualified `dpf::keyword`s
/// @{

/// @details specializes `std::numeric_limits` for `dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>`
template <std::size_t N,
          typename CharT,
          const CharT * Alpha,
          typename Traits,
          typename Alloc>
class numeric_limits<dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>>
{
  public:
    using keyword_type = dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>;
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
    static constexpr int digits = keyword_type::bits;
    static constexpr int digits10 = keyword_type::bits * std::log10(2);  //< correct if `keyword_type::bits<129`
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int max_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool traps
        = std::numeric_limits<typename keyword_type::integral_type>::traps;
    static constexpr bool tinyness_before = false;

    static constexpr keyword_type min() noexcept { return keyword_type{""}; }
    static constexpr keyword_type lowest() noexcept { return keyword_type{""}; }
    static constexpr keyword_type max() noexcept { return ~keyword_type{""}; }
    static constexpr keyword_type epsilon() noexcept { return 0; }
    static constexpr keyword_type round_error() noexcept { return 0; }
    static constexpr keyword_type infinity() noexcept { return 0; }
    static constexpr keyword_type quiet_NaN() noexcept { return 0; }
    static constexpr keyword_type signaling_NaN() noexcept { return 0; }
    static constexpr keyword_type denorm_min() noexcept { return 0; }
};

/// @details specializes `std::numeric_limits` for `dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc> const`
template <std::size_t N,
          typename CharT,
          const CharT * Alpha,
          typename Traits,
          typename Alloc>
class numeric_limits<dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc> const>
  : public numeric_limits<dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>> {};

/// @details specializes `std::numeric_limits` for
///          `dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc> volatile`
template <std::size_t N,
          typename CharT,
          const CharT * Alpha,
          typename Traits,
          typename Alloc>
class numeric_limits<dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc> volatile>
  : public numeric_limits<dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>> {};

/// @details specializes `std::numeric_limits` for
///          `dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc> const volatile`
template <std::size_t N,
          typename CharT,
          const CharT * Alpha,
          typename Traits,
          typename Alloc>
class numeric_limits<dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc> const volatile>
  : public numeric_limits<dpf::basic_fixed_length_string<N, CharT, Alpha, Traits, Alloc>> {};

/// @}

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_KEYWORD_HPP__
