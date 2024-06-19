/// @file dpf/bitstring.hpp
/// @brief defines `dpf::bitstring` and associated helpers
/// @details A `dpf::bitstring` represents a fixed-length string of bits that does
///          not semantically stand for a numerical value. It is implemented
///          as a simple subclass of a `dpf::bit_array_base`, but contains
///          helper functions for common tasks like performing lexicographic
///          comparisons or converting to and from regular C-strings. This type
///          is intended for use as an [input type](@ref input_types) for a DPF and, as such,
///          specializes `dpf::utils::bitlength_of`, `dpf::utils::msb_of`, and
///          `dpf::utils::countl_zero_symmetric_difference`. It also defines an
///          efficient `dpf::bitstring::bit_mask` facade to simulate the behavior that the
///          evaluation functions expect of `dpf::utils::msb_of`.
///
///          The `dpf::bitstring` class defines a default constructor that
///          initializes the bitstring with all bits set to `0`,
///          alongside compiler-generated copy and move constructors, and a
///          value constructor that initializes the first `M` bit positions with
///          the bits of the given value, where `M` is the smaller of `Nbits` and
///          `64`. It also defines the user-defined numeric literal
///          `dpf::literals::operator "" _bitstring()` for creating `dpf::bitstring<Nbits>` objects
///          at compile time, where `Nbits` is the length of the numeric literal
///          (i.e., minus the ``"_bitstring"`` suffix). For example, the expression
///          \code{.cpp}
///              auto x = 10101001_bitstring
///          \endcode
///          yields a `dpf::bitstring<8>` with the bits `10101001`, and is
///          therefore equivalent to invoking
///          \code{.cpp}
///              dpf::bitstring<8> x(0b10101001);
///          \endcode
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_BITSTRING_HPP__
#define LIBDPF_INCLUDE_DPF_BITSTRING_HPP__

#include <cstddef>
#include <cmath>
#include <type_traits>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <iterator>
#include <limits>
#include <string>
#include <array>
#include <ostream>
#include <sstream>

#include "portable-snippets/exact-int/exact-int.h"

#include "dpf/bit.hpp"
#include "dpf/bit_array.hpp"
#include "dpf/utils.hpp"
#include "dpf/literals.hpp"

namespace dpf
{

/// @brief a fixed-length string of bits
/// @details The `dpf::bitstring` class template represents a fixed-length
///          string of bits that does not semantically stand for a
///          numerical value. It is implemented as a subclass of
///          `dpf::bit_array_base` and is parametrized on `Nbits`, which is
///          the length of the bitstring.
/// @tparam Nbits the bitlength of the string
template <std::size_t Nbits,
          typename WordT = utils::integral_type_from_bitlength_t<Nbits, 8, 64>>
class bitstring : public bit_array_base<bitstring<Nbits, WordT>, WordT>
{
  private:
    using base = bit_array_base<bitstring<Nbits, WordT>, WordT>;
    using word_pointer = typename base::word_pointer;
    using const_word_pointer = typename base::const_word_pointer;
    using word_type = typename base::word_type;
    using const_pointer = typename base::const_pointer;
    using size_type = typename base::size_type;
    static constexpr auto bits_per_word = base::bits_per_word;
    /// @brief the number of `word_type`s are being used to represent the
    ///        `num_bits_` bits
    static constexpr size_type data_length_
        = utils::quotient_ceiling(Nbits, bits_per_word);
  public:
    /// @brief the primitive integral type used to represent the string,
    ///        note this should be based on Nbits and not WordT
    using integral_type = utils::integral_type_from_bitlength_t<Nbits>;

    /// @name C'tors
    /// @brief Constructs the default allocator. Since the default allocator
    ///        is stateless, the constructors have no visible effect.
    /// @{

    /// @brief Default c'tor
    /// @details Constructs an instance of `dpf::bitstring` with all bits set
    ///          to `0`.
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr bitstring() = default;

    /// @brief Copy c'tor
    /// @details Constructs an instance of `dpf::bitstring` from another
    ///          using copy semantics.
    /// @param other another `dpf::bitstring` to construct with
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr bitstring(const bitstring & other) = default;

    /// @brief Move c'tor
    /// @details Constructs an instance of `dpf::bitstring` from another
    ///          using move semantics.
    /// @param other another `dpf::bitstring` to construct with
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr bitstring(bitstring && other) noexcept = default;

    /// @brief Value c'tor
    /// @details Constructs an instance of `dpf::bitstring` while initializing
    ///          the first (rightmost, least significant) `M` bit positions to
    ///          the corresponding bit values of `val`, where `M` is the
    ///          smaller of `Nbits` and `bits_per_word`.
    /// @param val the number used to initialize the `dpf::bitstring`
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr explicit bitstring(word_type val) noexcept
      : data_{Nbits < bits_per_word ? static_cast<word_type>(val & (static_cast<word_type>(~word_type{0}) >> utils::bitlength_of_v<word_type> - Nbits)) : val} { }

    /// @brief Constructs a `dpf::bitstring` using the characters in the
    ///        `std::basic_string` `str`. An optional starting position `pos`
    ///        and length `len` can be provided, as well as characters
    ///        denoting alternate values for set (`one`) and unset (`zero`)
    ///        bits.
    /// @param str `string` used to initialize the `dpf::bitstring`
    /// @param pos a starting offset into `str`
    /// @param len number of characters to use from `str`
    /// @param zero character used to represent `0` (default: `CharT('0')`)
    /// @param one character used to represent `1` (default: `CharT('1')`)
    template <typename CharT,
              typename Traits,
              typename Alloc>
    explicit bitstring(
        const std::basic_string<CharT, Traits, Alloc> & str,
        typename std::basic_string<CharT, Traits, Alloc>::size_type pos = 0,
        typename std::basic_string<CharT, Traits, Alloc>::size_type len
            = std::basic_string<CharT, Traits, Alloc>::npos,
        CharT zero = CharT('0'),
        CharT one = CharT('1'))
    {
        if (pos > str.size())
        {
            std::stringstream ss;
            ss << "dpf::bitstring: pos (which is " << pos
               << ") > str.size() (which is " << str.size() << ")";
            throw std::out_of_range(ss.str());
        }
        len = std::min(len, str.size() - pos);
        for (std::size_t i = 0; i < len; ++i)
        {
            this->set(i, dpf::to_bit(str[pos+i]));
        }
    }

    /// @brief Constructs a `dpf::bitstring` using the characters in the
    ///        `CharT *` `str`. An optional starting position `pos` and length
    ///        `len` can be provided, as well as characters denoting alternate
    ///        values for set (`one`) and unset (`zero`) bits.
    /// @param str string used to initialize the `dpf::bitstring`
    /// @param len number of characters to use from `str`
    /// @param zero character used to represent `false`/`0` (default: ``CharT('0')``)
    /// @param one character used to represent `true`/`1` (default: ``CharT('1')``)
    template <typename CharT>
    explicit bitstring(const CharT * str,
        typename std::basic_string<CharT>::size_type len
            = std::basic_string<CharT>::npos,
        CharT zero = CharT('0'),
        CharT one = CharT('1'))
    {
        len = std::min(len, strnlen(str, len));
        for (std::size_t i = 0; i < len; ++i)
        {
            this->set(i, dpf::to_bit(str[i]));
        }
    }

    /// @}

    bitstring & operator=(const bitstring &) = default;
    bitstring & operator=(bitstring &&) noexcept = default;
    ~bitstring() = default;

    // template <std::enable_if_t<std::is_same_v<word_type, integral_type>, bool> = false>
    // bitstring & operator=(integral_type value)
    // {
    //     *data_ = (Nbits < bits_per_word)
    //         ? static_cast<word_type>(value & (static_cast<word_type>(~word_type{0}) >> utils::bitlength_of_v<word_type> - Nbits)) : value;
    //     return *this;
    // }

    /// @brief facade for masking out individual bits of a `dpf::bitstring`
    /// @details A `dpf::bitstring::bit_mask` struct is a facade that simulates
    ///          the behaviour of a 1-bit mask for us in the `dpf::eval_*`
    ///          family of functions. Specifically, it can be used in loops
    ///          such as
    ///          \code{c++}
    ///          auto x = dpf::bitstring<Nbits, WordT> = ...;
    ///          auto mask = dpf::msb_of(x);
    ///          for (auto i = 0; i < Nbits; ++i, mask>>=1)
    ///          {
    ///              bool bit = !!(mask & x);
    ///              // ...
    ///          }
    ///          \endcode
    ///          to iterate iver the individual bits of a `dpf::bitstring`
    ///          efficiently.
    struct bit_mask
    {
        /// @brief constructs a `dpf::bitstring::bit_mask` object that masks
        ///        the bit at the given position
        /// @param which_bit the ordinal position of the bit to mask
        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr explicit bit_mask(std::size_t which_bit) noexcept
          : which_bit_{which_bit} { }

        /// @brief shifts the bit mask to the right by the given number of
        ///        bits
        /// @param shift_by number of bits to shift the mask to the right
        /// @return a reference to the modified `dpf::bitstring::bit_mask`
        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr bit_mask & operator>>=(int shift_by) noexcept
        {
            which_bit_ -= shift_by;
            return *this;
        }

        /// @brief shifts the bit mask to the left by the given number of
        ///        bits
        /// @param shift_by number of bits to shift the mask to the left
        /// @return a reference to the modified `dpf::bitstring::bit_mask`
        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr bit_mask & operator<<=(int shift_by) noexcept
        {
            which_bit_ += shift_by;
            return *this;
        }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr bool operator&(const bitstring & rhs) noexcept
        {
            return rhs[which_bit_];
        }

        /// @brief returns `true` if and only if the bit mask corresponds to a
        ///        valid bit position in a `dpf::bitstring<Nbits, WordT>`
        /// @return `(0 <= which_bit()) && (which_bit() < Nbits)`
        HEDLEY_ALWAYS_INLINE
        HEDLEY_CONST
        HEDLEY_NO_THROW
        constexpr operator bool() const noexcept
        {
            return which_bit_ < Nbits;
        }

        /// @brief returns the ordinal position of the bit being masked out by
        ///        this `dpf::bitstring::big_mask`
        /// @return 
        HEDLEY_ALWAYS_INLINE
        HEDLEY_CONST
        HEDLEY_NO_THROW
        constexpr std::size_t which_bit() const noexcept
        {
            return which_bit_;
        }

        /// @brief shifts the bit mask to the right by the given number of
        ///        bits
        /// @param shift_by number of bits to shift the mask to the right
        /// @return a reference to the modified `dpf::bitstring::bit_mask`
        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        friend constexpr bit_mask operator>>(const bit_mask & mask, std::size_t shift_by) noexcept
        {
            return bit_mask{mask.which_bit_ - shift_by};
        }

        /// @brief shifts the bit mask to the left by the given number of
        ///        bits
        /// @param shift_by number of bits to shift the mask to the right
        /// @return a reference to the modified `dpf::bitstring::bit_mask`
        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        friend constexpr bit_mask operator<<(const bit_mask & mask, std::size_t shift_by) noexcept
        {
            return bit_mask{mask.which_bits_ + shift_by};
        }

      private:
        std::size_t which_bit_;  // ordinal position of the referenced bit
    };  // struct dpf::bitstring::bit_mask

    /// @brief extract the bit of `dpf::bitstring` that using a
    ///        `dpf::bitstring::bit_mask`
    /// @param rhs the `dpf::bitstring::bit_mask` indicating which bit to
    ///            extract
    /// @return `true` if the referenced bit is set, and `false` otherwise
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    bool operator&(const bitstring::bit_mask & rhs) const
    {
        return this->operator[](rhs.which_bit());
    }

    /// @brief lexicographically compares two `dpf::bitstring`s
    /// @{

    /// @brief Equality
    /// @details Checks if `this` and `rhs` are equal; that is, checks if each
    ///          bit of `this` is equal to the bit at the same position within
    ///          `rhs`.
    /// @param rhs right-hand side of the comparison
    /// @return `true` if the `bitstring`s are equal, `false` otherwise
    /// @complexity `O(Nbits)`
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator==(const bitstring & rhs) const
    {
        return data_ == rhs.data_;
    }

    /// @brief Inequality
    /// @details Checks if `this` and `rhs` are unequal; that is, checks if
    ///          one or more bits of `this` are opposite to the bits in `rhs`
    ///          at the same positions.
    /// @param rhs right-hand side of the comparison
    /// @return `true` if the `bitstring`s are unequal, `false` otherwise
    /// @complexity `O(Nbits)`
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator!=(const bitstring & rhs) const
    {
        return data_ != rhs.data_;
    }

    /// @brief Less than
    /// @details Checks if `this` is lexicographically less than `rhs`.
    /// @param rhs right-hand side of the comparison
    /// @return `true` if `this` comes before `rhs` lexiographically, `false`
    ///          otherwise
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator<(const bitstring & rhs) const
    {
        return std::lexicographical_compare(rbegin(data_), rend(data_),
            rbegin(rhs.data_), rend(rhs.data_), std::less{});
    }

    /// @brief Less than or equal
    /// @details Checks if `this` is lexicographically less than or equal to
    ///          `rhs`.
    /// @param rhs right-hand side of the comparison
    /// @return `true` if `this` comes at or before `rhs` lexiographically,
    ///         `false` otherwise
    /// @complexity `O(Nbits)`
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator<=(const bitstring & rhs) const
    {
        return std::lexicographical_compare(rbegin(data_), rend(data_),
            rbegin(rhs.data_), rend(rhs.data_), std::less_equal{});
    }

    /// @brief Greater than
    /// @details Checks if `this` is lexicographically greater than `rhs`.
    /// @param lhs left-hand side of the comparison
    /// @param rhs right-hand side of the comparison
    /// @return `true` if `lhs` comes at or before `rhs` lexiographically, `false`
    ///          otherwise
    /// @complexity `O(Nbits)`
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator>(const bitstring & rhs) const
    {
        return std::lexicographical_compare(rbegin(data_), rend(data_),
            rbegin(rhs.data_), rend(rhs.data_), std::greater{});
    }

    /// @brief Greater than or equal
    /// @details Checks if `this` is lexicographically greater than or equal to
    ///          `rhs`.
    /// @param rhs right-hand side of the comparison
    /// @return `true` if `lhs` comes at or before `rhs` lexiographically, `false`
    ///          otherwise
    /// @complexity `O(Nbits)`
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator>=(const bitstring & rhs) const
    {
        return std::lexicographical_compare(rbegin(data_), rend(data_),
            rbegin(rhs.data_), rend(rhs.data_), std::greater_equal{});
    }

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bitstring operator~() const noexcept
    {
        bitstring ret = *this;
        ret.flip();
        return ret;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bitstring & operator++() noexcept
    {
        data_[0] += 1;
        word_type carry = (data_[0] == word_type{0}) ? 1 : 0;
        for (std::size_t i = 1; i < data_.size() && carry == 1; ++i)
        {
            data_[i] += carry;
            carry = (data_[i] == word_type{0}) ? 1 : 0;
        }
        return *this;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bitstring operator++(int) noexcept
    {
        auto ret = *this;
        this->operator++();
        return ret;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bitstring & operator--() noexcept
    {
        data_[0] -= 1;
        word_type borrow = (data_[0] == ~word_type{0}) ? 1 : 0;
        for (std::size_t i = 1; i < data_.size() && borrow == 1; ++i)
        {
            data_[i] -= borrow;
            borrow = (data_[i] == ~word_type{0}) ? 1 : 0;
        }
        return *this;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bitstring operator--(int) noexcept
    {
        auto ret = *this;
        this->operator--();
        return ret;
    }

    /// @}

    /// @brief direct access to the underlying data array
    /// @return a pointer to the start of the data array
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr word_pointer data() noexcept
    {
        return std::data(data_);
    }

    /// @brief direct access to the underlying data array
    /// @return a pointer to the start of the data array
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr const_word_pointer data() const noexcept
    {
        return std::data(data_);
    }

    /// @brief direct access into the underlying data array (w/o bounds
    ///        checking)
    /// @param pos the array element to access
    /// @note Does not perform bounds checking; behaviour is undefined if
    ///       `pos` is out of bounds
    /// @return `data()[pos]`
    HEDLEY_ALWAYS_INLINE
    constexpr word_type data(size_type pos) const noexcept
    {
        return data()[pos];
    }

    /// @brief direct access into the underlying data array (w/o bounds
    ///        checking)
    /// @param pos the array element to access
    /// @note Does not perform bounds checking; behaviour is undefined if
    ///       `pos` is out of bounds
    /// @return `data()[pos]`
    HEDLEY_ALWAYS_INLINE
    constexpr word_type & data(size_type pos) noexcept
    {
        return data()[pos];
    }

    /// @brief length of the underlying data array
    /// @return the number of elements in the underlying array
    HEDLEY_ALWAYS_INLINE
    constexpr size_type data_length() const noexcept
    {
        return data_length_;
    }

    /// @brief returns the number of bits
    /// @returns number of bits that the `bitstring` holds
    /// @complexity `O(1)`
    HEDLEY_PURE
    HEDLEY_ALWAYS_INLINE
    constexpr size_type size() const noexcept
    {
        return Nbits;
    }

  private:
    //alignas(utils::max_align_v)  // memory here cannot be aligned if we wish
                                   // to remain trivially copyable
    std::array<word_type, data_length_> data_{};

    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    friend bitstring operator^(const bitstring & lhs, const bitstring & rhs)
    {
        bitstring ret;
        for (std::size_t i = 0; i < ret.data_length(); ++i)
        {
            ret.data(i) = lhs.data(i) ^ rhs.data(i);
        }
        return ret;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    friend bitstring operator-(const bitstring & lhs, const bitstring & rhs)
    {
        return operator^(lhs, rhs);
    }

    friend struct utils::countl_zero_symmetric_difference<bitstring>;
    friend struct utils::to_integral_type<bitstring>;
    friend struct utils::make_from_integral_value<bitstring>;
    friend struct utils::mod_pow_2<bitstring>;
};  // class dpf::bitstring

template <std::size_t Nbits,
          typename WordT>
inline std::ostream & operator<<(std::ostream & os, bitstring<Nbits, WordT> b)
{
    return os << b.to_string();
}

namespace utils
{

/// @brief specializes `dpf::utils::bitlength_of` for `dpf::bitstring`
template <std::size_t Nbits,
          typename WordT>
struct bitlength_of<dpf::bitstring<Nbits, WordT>>
  : public std::integral_constant<std::size_t, Nbits>
{ };

/// @brief specializes `dpf::utils::msb_of` for `dpf::bitstring`
template <std::size_t Nbits,
          typename WordT>
struct msb_of<dpf::bitstring<Nbits, WordT>>
{
    constexpr static auto value
        = typename dpf::bitstring<Nbits, WordT>::bit_mask(
            bitlength_of_v<dpf::bitstring<Nbits, WordT>>-1ul);
};

/// @brief specializes `dpf::utils::countl_zero_symmetric_difference` for
///        `dpf::bitstring`
template <std::size_t Nbits,
          typename WordT>
struct countl_zero_symmetric_difference<dpf::bitstring<Nbits, WordT>>
{
    using T = dpf::bitstring<Nbits, WordT>;

    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    constexpr
    std::size_t operator()(const T & lhs, const T & rhs) const noexcept
    {
        using word_type = typename T::word_type;
        constexpr auto xor_op = std::bit_xor<word_type>{};
        auto adjust = lhs.data_length()*bitlength_of_v<word_type> - Nbits;
        std::size_t prefix_len = 0;
        for (auto i = lhs.data_length(); i > 0; --i,
            prefix_len += bitlength_of_v<word_type>)
        {
            psnip_uint64_t limb = xor_op(lhs.data(i-1), rhs.data(i-1));
            if (limb)
            {
                return prefix_len + utils::clz(limb) - adjust;
            }
        }
        return prefix_len - adjust;
    }
};

template <std::size_t Nbits,
          typename WordT>
struct to_integral_type<dpf::bitstring<Nbits, WordT>>
    : public to_integral_type_base<dpf::bitstring<Nbits, WordT>>
{
    using parent = to_integral_type_base<dpf::bitstring<Nbits, WordT>>;
    using typename parent::integral_type;

    static constexpr auto bits_per_word = dpf::bitstring<Nbits, WordT>::bits_per_word;
    static constexpr integral_type modulo_mask = static_cast<integral_type>(~integral_type{0}) >> utils::bitlength_of_v<integral_type> - ((Nbits-1) % bits_per_word + 1);

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr integral_type operator()(const dpf::bitstring<Nbits, WordT> & input) const noexcept
    {
        if constexpr(Nbits <= bits_per_word)
        {
            return integral_type(input.data_[0] & modulo_mask);
        }
        else
        {
            integral_type ret = input.data_[(Nbits-1)/bits_per_word] & modulo_mask;

            for (std::size_t i = (Nbits-1)/bits_per_word; i > 0; --i)
            {
                ret <<= bits_per_word;
                ret += input.data_[i-1];
            }
            return ret;
        }
    }

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr integral_type operator()(const typename dpf::bitstring<Nbits, WordT>::bit_mask & input) const noexcept
    {
        return integral_type(1) << input.which_bit();
    }
};

template <std::size_t Nbits,
          typename WordT>
struct make_from_integral_value<dpf::bitstring<Nbits, WordT>>
{
    using T = dpf::bitstring<Nbits, WordT>;
    using T_integral_type = typename T::integral_type;
    using integral_type = std::conditional_t<std::is_void_v<T_integral_type>, simde_uint128, T_integral_type>;
    static constexpr auto mod = utils::mod_pow_2<integral_type>{};
    static constexpr auto bits_per_last_word = Nbits % T::bits_per_word;
    constexpr dpf::bitstring<Nbits, WordT> operator()(integral_type val) const noexcept
    {
        dpf::bitstring<Nbits, WordT> ret;
        std::size_t i = 0;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wshift-count-overflow")
        for (; i < T::data_length_-1; ++i, val >>= T::bits_per_word)
HEDLEY_PRAGMA(GCC diagnostic pop)
        {
            ret.data_[i] = static_cast<typename T::word_type>(val);
        }
        ret.data_[i] = mod(val, bits_per_last_word);
        return ret;
    }
};

template <std::size_t Nbits,
          typename WordT>
struct mod_pow_2<dpf::bitstring<Nbits, WordT>>
{
    using T = dpf::bitstring<Nbits, WordT>;
    static constexpr auto to_int = to_integral_type<T>{};
    static constexpr auto mod = mod_pow_2<typename T::integral_type>{};
    std::size_t operator()(T val, std::size_t n) const noexcept
    {
        return mod(to_int(val), n);
    }
};

}  // namespace utils

namespace bitstrings
{

// 1--9
using bit1_t = dpf::bitstring<1>;
using bit2_t = dpf::bitstring<2>;
using bit3_t = dpf::bitstring<3>;
using bit4_t = dpf::bitstring<4>;
using bit5_t = dpf::bitstring<5>;
using bit6_t = dpf::bitstring<6>;
using bit7_t = dpf::bitstring<7>;
using bit8_t = dpf::bitstring<8>;
using bit9_t = dpf::bitstring<9>;
// 10--19
using bit10_t = dpf::bitstring<10>;
using bit11_t = dpf::bitstring<11>;
using bit12_t = dpf::bitstring<12>;
using bit13_t = dpf::bitstring<13>;
using bit14_t = dpf::bitstring<14>;
using bit15_t = dpf::bitstring<15>;
using bit16_t = dpf::bitstring<16>;
using bit17_t = dpf::bitstring<17>;
using bit18_t = dpf::bitstring<18>;
using bit19_t = dpf::bitstring<19>;
// 20--29
using bit20_t = dpf::bitstring<20>;
using bit21_t = dpf::bitstring<21>;
using bit22_t = dpf::bitstring<22>;
using bit23_t = dpf::bitstring<23>;
using bit24_t = dpf::bitstring<24>;
using bit25_t = dpf::bitstring<25>;
using bit26_t = dpf::bitstring<26>;
using bit27_t = dpf::bitstring<27>;
using bit28_t = dpf::bitstring<28>;
using bit29_t = dpf::bitstring<29>;
// 30-39
using bit30_t = dpf::bitstring<30>;
using bit31_t = dpf::bitstring<31>;
using bit32_t = dpf::bitstring<32>;
using bit33_t = dpf::bitstring<33>;
using bit34_t = dpf::bitstring<34>;
using bit35_t = dpf::bitstring<35>;
using bit36_t = dpf::bitstring<36>;
using bit37_t = dpf::bitstring<37>;
using bit38_t = dpf::bitstring<38>;
using bit39_t = dpf::bitstring<39>;
// 40--49
using bit40_t = dpf::bitstring<40>;
using bit41_t = dpf::bitstring<41>;
using bit42_t = dpf::bitstring<42>;
using bit43_t = dpf::bitstring<43>;
using bit44_t = dpf::bitstring<44>;
using bit45_t = dpf::bitstring<45>;
using bit46_t = dpf::bitstring<46>;
using bit47_t = dpf::bitstring<47>;
using bit48_t = dpf::bitstring<48>;
using bit49_t = dpf::bitstring<49>;
// 50--59
using bit50_t = dpf::bitstring<50>;
using bit51_t = dpf::bitstring<51>;
using bit52_t = dpf::bitstring<52>;
using bit53_t = dpf::bitstring<53>;
using bit54_t = dpf::bitstring<54>;
using bit55_t = dpf::bitstring<55>;
using bit56_t = dpf::bitstring<56>;
using bit57_t = dpf::bitstring<57>;
using bit58_t = dpf::bitstring<58>;
using bit59_t = dpf::bitstring<59>;
// 60--69
using bit60_t = dpf::bitstring<60>;
using bit61_t = dpf::bitstring<61>;
using bit62_t = dpf::bitstring<62>;
using bit63_t = dpf::bitstring<63>;
using bit64_t = dpf::bitstring<64>;
using bit65_t = dpf::bitstring<65>;
using bit66_t = dpf::bitstring<66>;
using bit67_t = dpf::bitstring<67>;
using bit68_t = dpf::bitstring<68>;
using bit69_t = dpf::bitstring<69>;
// 70--79
using bit70_t = dpf::bitstring<70>;
using bit71_t = dpf::bitstring<71>;
using bit72_t = dpf::bitstring<72>;
using bit73_t = dpf::bitstring<73>;
using bit74_t = dpf::bitstring<74>;
using bit75_t = dpf::bitstring<75>;
using bit76_t = dpf::bitstring<76>;
using bit77_t = dpf::bitstring<77>;
using bit78_t = dpf::bitstring<78>;
using bit79_t = dpf::bitstring<79>;
// 80--89
using bit80_t = dpf::bitstring<80>;
using bit81_t = dpf::bitstring<81>;
using bit82_t = dpf::bitstring<82>;
using bit83_t = dpf::bitstring<83>;
using bit84_t = dpf::bitstring<84>;
using bit85_t = dpf::bitstring<85>;
using bit86_t = dpf::bitstring<86>;
using bit87_t = dpf::bitstring<87>;
using bit88_t = dpf::bitstring<88>;
using bit89_t = dpf::bitstring<89>;
// 90--99
using bit90_t = dpf::bitstring<90>;
using bit91_t = dpf::bitstring<91>;
using bit92_t = dpf::bitstring<92>;
using bit93_t = dpf::bitstring<93>;
using bit94_t = dpf::bitstring<94>;
using bit95_t = dpf::bitstring<95>;
using bit96_t = dpf::bitstring<96>;
using bit97_t = dpf::bitstring<97>;
using bit98_t = dpf::bitstring<98>;
using bit99_t = dpf::bitstring<99>;
// 100--109
using bit100_t = dpf::bitstring<100>;
using bit101_t = dpf::bitstring<101>;
using bit102_t = dpf::bitstring<102>;
using bit103_t = dpf::bitstring<103>;
using bit104_t = dpf::bitstring<104>;
using bit105_t = dpf::bitstring<105>;
using bit106_t = dpf::bitstring<106>;
using bit107_t = dpf::bitstring<107>;
using bit108_t = dpf::bitstring<108>;
using bit109_t = dpf::bitstring<109>;
// 110--119
using bit110_t = dpf::bitstring<110>;
using bit111_t = dpf::bitstring<111>;
using bit112_t = dpf::bitstring<112>;
using bit113_t = dpf::bitstring<113>;
using bit114_t = dpf::bitstring<114>;
using bit115_t = dpf::bitstring<115>;
using bit116_t = dpf::bitstring<116>;
using bit117_t = dpf::bitstring<117>;
using bit118_t = dpf::bitstring<118>;
using bit119_t = dpf::bitstring<119>;
// 120--128
using bit120_t = dpf::bitstring<120>;
using bit121_t = dpf::bitstring<121>;
using bit122_t = dpf::bitstring<122>;
using bit123_t = dpf::bitstring<123>;
using bit124_t = dpf::bitstring<124>;
using bit125_t = dpf::bitstring<125>;
using bit126_t = dpf::bitstring<126>;
using bit127_t = dpf::bitstring<127>;
using bit128_t = dpf::bitstring<128>;

namespace literals = dpf::literals::bitstrings;

}  // namespace bitstrings

namespace literals
{

namespace bitstrings
{

/// @brief user-defined numeric literal for creating `dpf::bitstring` objects
/// @details A user-defined literal that provides syntactic sugar for defining
///          compile-time constant `dpf::bitstring` instances. For example,
///          \code{.cpp}auto foo = 1010011101000001011110111010100011101010_bitstring;\endcode
///          defines a `dpf::bitstring<40>` representing the same bits as the
///          literal, in the same order. The length of the resulting `dpf::bitstring`
///          is equal to `sizeof...(bits)`.
/// @tparam bits the bits comprising the bitstring
/// @throws std::domain_error if one or more character in the literal is
///          equal neither to `0` nor to `1`, excluding the `"_bitstring"`
///          suffix itsef.
/// @return the `dpf::bitstring`
template <char ...bits>
constexpr static auto operator "" _bitstring()
{
    dpf::bitstring<sizeof...(bits)> bs{0};
    std::size_t i = 0;
    (bs.set(i++, dpf::to_bit(bits)), ...);
    return bs;
}

template <char ...bits>
constexpr static auto operator "" _bitstring_u8()
{
    dpf::bitstring<sizeof...(bits), psnip_uint8_t> bs{0};
    std::size_t i = 0;
    (bs.set(i++, dpf::to_bit(bits)), ...);
    return bs;
}

template <char ...bits>
constexpr static auto operator "" _bitstring_u16()
{
    dpf::bitstring<sizeof...(bits), psnip_uint16_t> bs{0};
    std::size_t i = 0;
    (bs.set(i++, dpf::to_bit(bits)), ...);
    return bs;
}

template <char ...bits>
constexpr static auto operator "" _bitstring_u32()
{
    dpf::bitstring<sizeof...(bits), psnip_uint32_t> bs{0};
    std::size_t i = 0;
    (bs.set(i++, dpf::to_bit(bits)), ...);
    return bs;
}

template <char ...bits>
constexpr static auto operator "" _bitstring_u64()
{
    dpf::bitstring<sizeof...(bits), psnip_uint64_t> bs{0};
    std::size_t i = 0;
    (bs.set(i++, dpf::to_bit(bits)), ...);
    return bs;
}

template <char ...bits>
constexpr static auto operator "" _bitstring_u128()
{
    dpf::bitstring<sizeof...(bits), simde_uint128> bs{0};
    std::size_t i = 0;
    (bs.set(i++, dpf::to_bit(bits)), ...);
    return bs;
}

// 1--9
template <char ...bits> constexpr static auto operator "" _b1() { dpf::bitstring<1> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b2() { dpf::bitstring<2> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b3() { dpf::bitstring<3> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b4() { dpf::bitstring<4> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b5() { dpf::bitstring<5> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b6() { dpf::bitstring<6> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b7() { dpf::bitstring<7> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b8() { dpf::bitstring<8> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b9() { dpf::bitstring<9> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
// 10--19
template <char ...bits> constexpr static auto operator "" _b10() { dpf::bitstring<10> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b11() { dpf::bitstring<11> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b12() { dpf::bitstring<12> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b13() { dpf::bitstring<13> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b14() { dpf::bitstring<14> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b15() { dpf::bitstring<15> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b16() { dpf::bitstring<16> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b17() { dpf::bitstring<17> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b18() { dpf::bitstring<18> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b19() { dpf::bitstring<19> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
// 20--29
template <char ...bits> constexpr static auto operator "" _b20() { dpf::bitstring<20> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b21() { dpf::bitstring<21> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b22() { dpf::bitstring<22> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b23() { dpf::bitstring<23> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b24() { dpf::bitstring<24> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b25() { dpf::bitstring<25> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b26() { dpf::bitstring<26> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b27() { dpf::bitstring<27> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b28() { dpf::bitstring<28> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b29() { dpf::bitstring<29> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
// 30--39
template <char ...bits> constexpr static auto operator "" _b30() { dpf::bitstring<30> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b31() { dpf::bitstring<31> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b32() { dpf::bitstring<32> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b33() { dpf::bitstring<33> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b34() { dpf::bitstring<34> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b35() { dpf::bitstring<35> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b36() { dpf::bitstring<36> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b37() { dpf::bitstring<37> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b38() { dpf::bitstring<38> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b39() { dpf::bitstring<39> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
// 40--49
template <char ...bits> constexpr static auto operator "" _b40() { dpf::bitstring<40> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b41() { dpf::bitstring<41> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b42() { dpf::bitstring<42> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b43() { dpf::bitstring<43> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b44() { dpf::bitstring<44> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b45() { dpf::bitstring<45> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b46() { dpf::bitstring<46> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b47() { dpf::bitstring<47> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b48() { dpf::bitstring<48> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b49() { dpf::bitstring<49> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
// 50--59
template <char ...bits> constexpr static auto operator "" _b50() { dpf::bitstring<50> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b51() { dpf::bitstring<51> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b52() { dpf::bitstring<52> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b53() { dpf::bitstring<53> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b54() { dpf::bitstring<54> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b55() { dpf::bitstring<55> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b56() { dpf::bitstring<56> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b57() { dpf::bitstring<57> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b58() { dpf::bitstring<58> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b59() { dpf::bitstring<59> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
// 60--69
template <char ...bits> constexpr static auto operator "" _b60() { dpf::bitstring<60> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b61() { dpf::bitstring<61> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b62() { dpf::bitstring<62> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b63() { dpf::bitstring<63> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b64() { dpf::bitstring<64> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b65() { dpf::bitstring<65> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b66() { dpf::bitstring<66> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b67() { dpf::bitstring<67> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b68() { dpf::bitstring<68> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b69() { dpf::bitstring<69> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
// 70--79
template <char ...bits> constexpr static auto operator "" _b70() { dpf::bitstring<70> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b71() { dpf::bitstring<71> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b72() { dpf::bitstring<72> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b73() { dpf::bitstring<73> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b74() { dpf::bitstring<74> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b75() { dpf::bitstring<75> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b76() { dpf::bitstring<76> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b77() { dpf::bitstring<77> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b78() { dpf::bitstring<78> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b79() { dpf::bitstring<79> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
// 80--89
template <char ...bits> constexpr static auto operator "" _b80() { dpf::bitstring<80> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b81() { dpf::bitstring<81> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b82() { dpf::bitstring<82> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b83() { dpf::bitstring<83> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b84() { dpf::bitstring<84> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b85() { dpf::bitstring<85> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b86() { dpf::bitstring<86> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b87() { dpf::bitstring<87> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b88() { dpf::bitstring<88> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b89() { dpf::bitstring<89> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
// 90--99
template <char ...bits> constexpr static auto operator "" _b90() { dpf::bitstring<90> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b91() { dpf::bitstring<91> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b92() { dpf::bitstring<92> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b93() { dpf::bitstring<93> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b94() { dpf::bitstring<94> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b95() { dpf::bitstring<95> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b96() { dpf::bitstring<96> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b97() { dpf::bitstring<97> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b98() { dpf::bitstring<98> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b99() { dpf::bitstring<99> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
// 100--109
template <char ...bits> constexpr static auto operator "" _b100() { dpf::bitstring<100> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b101() { dpf::bitstring<101> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b102() { dpf::bitstring<102> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b103() { dpf::bitstring<103> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b104() { dpf::bitstring<104> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b105() { dpf::bitstring<105> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b106() { dpf::bitstring<106> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b107() { dpf::bitstring<107> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b108() { dpf::bitstring<108> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b109() { dpf::bitstring<109> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
// 110--119
template <char ...bits> constexpr static auto operator "" _b110() { dpf::bitstring<110> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b111() { dpf::bitstring<111> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b112() { dpf::bitstring<112> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b113() { dpf::bitstring<113> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b114() { dpf::bitstring<114> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b115() { dpf::bitstring<115> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b116() { dpf::bitstring<116> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b117() { dpf::bitstring<117> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b118() { dpf::bitstring<118> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b119() { dpf::bitstring<119> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
// 120--128
template <char ...bits> constexpr static auto operator "" _b120() { dpf::bitstring<120> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b121() { dpf::bitstring<121> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b122() { dpf::bitstring<122> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b123() { dpf::bitstring<123> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b124() { dpf::bitstring<124> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b125() { dpf::bitstring<125> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b126() { dpf::bitstring<126> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b127() { dpf::bitstring<127> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }
template <char ...bits> constexpr static auto operator "" _b128() { dpf::bitstring<128> bs{0}; std::size_t i = 0; (bs.set(i++, dpf::to_bit(bits)), ...); return bs; }

}  // namespace bitstrings

}  // namespace literals

}  // namespace dpf

namespace std
{

/// @brief specializes `std::numeric_limits` for CV-qualified `dpf::bitstring`s
/// @{

/// @details specializes `std::numeric_limits` for `dpf::bitstring<Nbits, WordT>`
template<std::size_t Nbits,
          typename WordT>
class numeric_limits<dpf::bitstring<Nbits, WordT>>
{
  public:
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = false;
    static constexpr bool is_integer = false;
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
    static constexpr int digits10 = Nbits * std::log10(2);
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int max_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool traps
        = std::numeric_limits<typename dpf::bitstring<Nbits, WordT>::integral_type>::traps;
    static constexpr bool tinyness_before = false;

    static constexpr dpf::bitstring<Nbits, WordT> min() noexcept { return dpf::bitstring<Nbits, WordT>{}; }
    static constexpr dpf::bitstring<Nbits, WordT> lowest() noexcept { return dpf::bitstring<Nbits, WordT>{}; }
    static constexpr dpf::bitstring<Nbits, WordT> max() noexcept { return ~dpf::bitstring<Nbits, WordT>{}; }
    static constexpr dpf::bitstring<Nbits, WordT> epsilon() noexcept { return 0; }
    static constexpr dpf::bitstring<Nbits, WordT> round_error() noexcept { return 0; }
    static constexpr dpf::bitstring<Nbits, WordT> infinity() noexcept { return 0; }
    static constexpr dpf::bitstring<Nbits, WordT> quiet_NaN() noexcept { return 0; }
    static constexpr dpf::bitstring<Nbits, WordT> signaling_NaN() noexcept { return 0; }
    static constexpr dpf::bitstring<Nbits, WordT> denorm_min() noexcept { return 0; }
};

/// @details specializes `std::numeric_limits` for `dpf::bitstring<Nbits, WordT> const`
template<std::size_t Nbits,
          typename WordT>
class numeric_limits<dpf::bitstring<Nbits, WordT> const>
  : public numeric_limits<dpf::bitstring<Nbits, WordT>> {};

/// @details specializes `std::numeric_limits` for
///          `dpf::bitstring<Nbits, WordT> volatile`
template<std::size_t Nbits,
          typename WordT>
class numeric_limits<dpf::bitstring<Nbits, WordT> volatile>
  : public numeric_limits<dpf::bitstring<Nbits, WordT>> {};

/// @details specializes `std::numeric_limits` for
///          `dpf::bitstring<Nbits, WordT> const volatile`
template<std::size_t Nbits,
          typename WordT>
class numeric_limits<dpf::bitstring<Nbits, WordT> const volatile>
  : public numeric_limits<dpf::bitstring<Nbits, WordT>> {};

/// @}

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_BITSTRING_HPP__
