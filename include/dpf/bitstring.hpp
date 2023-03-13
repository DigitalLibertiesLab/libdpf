/// @file dpf/bitstring.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief defines `dpf::bitstring` and associated helpers
/// @details A `dpf::bitstring` is a wrapper around a `dpf::static_bit_array`.
///          It is used to represent a fixed-length string of bits that does
///          not semantically stand for a numerical value. It is implemented
///          as a thin wrapper around a `dpf::static_bit_array`, but contains
///          helper functions for common tasks like performing lexicographic
///          comparisons or converting to and from regular C-strings. It is
///          intended for use as an input type for a DPF and, as such,
///          specializes `dpf::utils::bitlength_of`, `dpf::utils::msb_of`, and
///          `dpf::utils::countl_zero_symmmetric_difference`. It defines an
///          efficient `bit_mask` facade to simulate the behavior that the
///          evaluation functions expect of `dpf::utils::msb_of`.
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_BITSTRING_HPP__
#define LIBDPF_INCLUDE_DPF_BITSTRING_HPP__

#include <string>

#include "dpf/bit.hpp"
#include "dpf/bit_array.hpp"
#include "dpf/utils.hpp"

namespace dpf
{

/// @brief a fixed-length string of bits
/// @details The `dpf::bitstring` class template represents a fixe-length
///          string of bits that does not semantically stand for a
///          numerical value. It is implemented as a thin wrapper around a
///          `dpf::static_bit_array`.
/// @tparam Nbits the bitlength of the string
template <std::size_t Nbits>
class bitstring : public dpf::static_bit_array<Nbits>
{
  private:
    using base = dpf::static_bit_array<Nbits>;

  public:
    /// @name C'tors
    /// @brief Constructs the default allocator. Since the default allocator
    ///        is stateless, the constructors have no visible effect.
    /// @{

    /// @brief Default c'tor
    /// @details Constructs an instance of `dpf::bitstring` with all bits set
    ///          to `0`.
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr bitstring() noexcept = default;

    /// @brief Copy c'tor
    /// @details Constructs an instance of `dpf::bitstring` from another
    ///          using copy semantics.
    /// @param other another `dpf::bitstring` to construct with
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr bitstring(const bitstring & other) noexcept = default;

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
    ///          smaller of `Nbits` and `64`.
    /// @param val the number used to initialize the `dpf::bitstring`
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr explicit bitstring(uint64_t val) noexcept : base{val} { }

    /// @brief Constructs a `dpf::bitstring` using the characters in the
    ///        `std::basic_string` `str`. An optional starting position `pos`
    ///        and length `len` can be provided, as well as characters
    ///        denoting alternate values for set (`one`) and unset (`zero`)
    ///        bits.
    /// @param str `string` used to initialize the `dpf::bitstring`
    /// @param pos a starting offset into `str`
    /// @param len number of characters to use from `str`
    /// @param zero character used to represent `0` (default: `CharT{'0'}`)
    /// @param one character used to represent `1` (default: `CharT{'1'}`)
    template <class CharT,
              class Traits,
              class Alloc>
    explicit bitstring(
        const std::basic_string<CharT, Traits, Alloc> & str,
        typename std::basic_string<CharT, Traits, Alloc>::size_type pos = 0,
        typename std::basic_string<CharT, Traits, Alloc>::size_type len
            = std::basic_string<CharT, Traits, Alloc>::npos,
        CharT zero = CharT('0'),
        CharT one = CharT('1'))
      : base(str, pos, len, zero, one) { }

    /// @brief Constructs a `dpf::bitstring` using the characters in the
    ///        `CharT *` `str`. An optional starting position `pos` and length
    ///        `len` can be provided, as well as characters denoting alternate
    ///        values for set (`one`) and unset (`zero`) bits.
    /// @param str string used to initialize the `dpf::bitstring`
    /// @param len number of characters to use from `str`
    /// @param zero character used to represent `0` (default: `CharT{'0'}`)
    /// @param one character used to represent `1` (default: `CharT{'1'}`)
    template <class CharT>
    explicit bitstring(const CharT * str,
        typename std::basic_string<CharT>::size_type len
            = std::basic_string<CharT>::npos,
        CharT zero = CharT('0'),
        CharT one = CharT('1'))
      : base(str, len, zero, one) { }

    /// @}

    /// @brief facade for masking out individual bits of a `dpf::bitstring`
    /// @details A `dpf::bitstring::bit_mask` struct is a facade that simulates
    ///          the behaviour of a 1-bit mask for us in the `dpf::eval_*`
    ///          family of functions. Specifically, it can be used in loops
    ///          such as
    ///          ```auto x = dpf::bitstring<Nbits> = ...;
    ///          auto mask = dpf::msb_of(x);
    ///          for (auto i = 0; i < Nbits; ++i, mask>>=1)
    ///          {
    ///              bool bit = !!(mask & x);
    ///              // ...
    ///           }```
    ///          to iterate iver the individual bits of a `dpf::bitstring`
    ///          efficiently.
    struct bit_mask
    {
        /// @brief constructs a `dpf::bitstring::bit_mask` object that masks
        ///        the bit at the given position
        /// @param which_bit the ordinal position of the bit to mask
        HEDLEY_ALWAYS_INLINE
        constexpr explicit bit_mask(std::size_t which_bit) noexcept
          : which_bit_{which_bit} { }

        /// @brief shifts the bit mask to the right by the given number of
        ///        bits
        /// @param shift_by number of bits to shift the mask to the right
        /// @return a reference to the modified `dpf::bitstring::bit_mask`
        HEDLEY_ALWAYS_INLINE
        constexpr bit_mask & operator>>=(int shift_by) noexcept
        {
            which_bit_ -= shift_by;
            return *this;
        }

        /// @brief returns `true` if and only if the bit mask corresponds to a
        ///        valid bit position in a `dpf::bitstring<Nbits>`
        /// @return `(0 <= which_bit()) && (which_bit() < Nbits)`
        HEDLEY_ALWAYS_INLINE
        HEDLEY_CONST
        HEDLEY_NO_THROW
        constexpr operator bool() const noexcept
        {
            return (0 <= which_bit_) && (which_bit_ < Nbits);
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

      private:
        std::size_t which_bit_;  // ordinal position of the referenced bit
    };

    
    /// @brief extract the bit of `dpf::bitstring` that using a
    ///        `dpf::bitstring::bit_mask`
    /// @param rhs the `dpf::bitstring::bit_mask` indicating which bit to
    ///            extract
    /// @return `true` if the referenced bit is set, and `false` otherwise
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    bool operator&(const bitstring::bit_mask & rhs) const
    {
        return base::operator[](rhs.which_bit());
    }
};

template <std::size_t Nbits>
constexpr bool
operator<(const bitstring<Nbits> & lhs, const bitstring<Nbits> & rhs)
{
    auto lhs_iter = std::rbegin(lhs), rhs_iter = std::rbegin(rhs);
    while (*lhs-- == *rhs--)
    {
        if (lhs_iter == std::rend(lhs)) return false;
    }
    return *lhs < *rhs;
}

template <std::size_t Nbits>
constexpr bool
operator<=(const bitstring<Nbits> & lhs, const bitstring<Nbits> & rhs)
{
    return (lhs < rhs) || lhs == rhs;
}

template <std::size_t Nbits>
constexpr bool
operator>(const bitstring<Nbits> & lhs, const bitstring<Nbits> & rhs)
{
    return rhs < lhs;
}

template <std::size_t Nbits>
constexpr bool
operator>=(const bitstring<Nbits> & lhs, const bitstring<Nbits> & rhs)
{
    return rhs <= lhs;
}

namespace utils
{

template <std::size_t Nbits>
struct bitlength_of<dpf::bitstring<Nbits>>
  : public std::integral_constant<std::size_t, Nbits> { };

template <std::size_t Nbits>
struct msb_of<dpf::bitstring<Nbits>>
{
    constexpr static auto value
        = typename dpf::bitstring<Nbits>::bit_mask(
            bitlength_of_v<dpf::bitstring<Nbits>>-1ul);
};

template <std::size_t Nbits>
struct countl_zero_symmmetric_difference<dpf::bitstring<Nbits>>
{
    using T = dpf::bitstring<Nbits>;

    HEDLEY_CONST
    HEDLEY_ALWAYS_INLINE
    constexpr
    std::size_t operator()(const T & lhs, const T & rhs) const noexcept
    {
        using word_type = typename T::word_type;
        constexpr auto xor_op = std::bit_xor<word_type>{};
        auto adjust = lhs.data_length()*bitlength_of_v<word_type> - Nbits;
        std::size_t prefix_len = 0;
        for (auto i = lhs.data_length()-1; i >= 0; --i,
            prefix_len += bitlength_of_v<word_type>)
        {
            word_type limb = xor_op(lhs.data(i), rhs.data(i));
            if (limb)
            {
                return prefix_len + psnip_builtin_clz64(limb) - adjust;
            }
        }
        return prefix_len - adjust;
    }
};

}  // namespace utils


}  // namespace dpf

template <char... bits>
constexpr static auto operator "" _bits()
{
    dpf::bitstring<sizeof...(bits)> bs;
    std::size_t i = bs.size()-1;
    (bs.unchecked_set(i--, dpf::to_bit(bits)), ...);
    return bs;
}

#endif  // LIBDPF_INCLUDE_DPF_BITSTRING_HPP__
