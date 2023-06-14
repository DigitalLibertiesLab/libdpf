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
/// @copyright Copyright (c) 2019-2023 Ryan Henry and [others](@ref authors)
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
    constexpr bitstring(const bitstring &) = default;

    /// @brief Move c'tor
    /// @details Constructs an instance of `dpf::bitstring` from another
    ///          using move semantics.
    /// @param other another `dpf::bitstring` to construct with
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr bitstring(bitstring &&) noexcept = default;

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
        CharT one = CharT('1')) : data_{}
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
        CharT one = CharT('1')) : data_{}
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
    /// @details Checks if `lhs` is lexicographically greater than `rhs`.
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
    /// @details Checks if `lhs` is lexicographically greater than or equal to
    ///          `rhs`.
    /// @param lhs left-hand side of the comparison
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
    // alignas(utils::max_align_v)  // memory here cannot be aligned if we
                                    // wish to remain trivially copyable
    std::array<word_type, data_length_> data_;

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
                return prefix_len + psnip_builtin_clz64(limb) - adjust - (64 - bitlength_of_v<word_type>);
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
        for (; i < T::data_length_-1; ++i, val >>= T::bits_per_word)
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

namespace literals
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
    constexpr static auto operator "" _bitstring_8()
    {
        dpf::bitstring<sizeof...(bits), uint8_t> bs{0};
        std::size_t i = 0;
        (bs.set(i++, dpf::to_bit(bits)), ...);
        return bs;
    }
}  // namespace literals

}  // namespace dpf

namespace std
{

/// @brief specializes `std::numeric_limits` for CV-qualified `dpf::bitstring`s
/// @{

/// @details specializes `std::numeric_limits` for `dpf::bitstring<Nbits, WordT>`
// skipcq: CXX-W2017
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
// skipcq: CXX-W2017
template<std::size_t Nbits,
          typename WordT>
class numeric_limits<dpf::bitstring<Nbits, WordT> const>
  : public numeric_limits<dpf::bitstring<Nbits, WordT>> {};

/// @details specializes `std::numeric_limits` for
///          `dpf::bitstring<Nbits, WordT> volatile`
// skipcq: CXX-W2017
template<std::size_t Nbits,
          typename WordT>
class numeric_limits<dpf::bitstring<Nbits, WordT> volatile>
  : public numeric_limits<dpf::bitstring<Nbits, WordT>> {};

/// @details specializes `std::numeric_limits` for
///          `dpf::bitstring<Nbits, WordT> const volatile`
// skipcq: CXX-W2017
template<std::size_t Nbits,
          typename WordT>
class numeric_limits<dpf::bitstring<Nbits, WordT> const volatile>
  : public numeric_limits<dpf::bitstring<Nbits, WordT>> {};

/// @}

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_BITSTRING_HPP__
