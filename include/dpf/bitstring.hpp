/// @file dpf/bitstring.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief defines `dpf::bitstring` and associated helpers
/// @details A `dpf::bitstring` is a wrapper around a `std::bitset`. Used to
///          represent a fixed-length string of bits that does not semantically
///          stand for a numerical value.
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_BITSTRING_HPP__
#define LIBDPF_INCLUDE_DPF_BITSTRING_HPP__

#include <bitset>
#include "dpf/bit.hpp"
#include "dpf/bit_array.hpp"

#include "dpf/utils.hpp"

namespace dpf
{

template <std::size_t N>
class bitstring : public dpf::static_bit_array<N>
{
  private:
    using base = dpf::static_bit_array<N>;
  public:
    struct bit_mask;
    constexpr bitstring() = default;
    constexpr bitstring(const bitstring &) = default;
    constexpr bitstring(bitstring &&) = default;
    constexpr bitstring(uint64_t val) noexcept : base{val} { }

    template <class CharT,
              class Traits,
              class Alloc>
    explicit bitstring(
        const std::basic_string<CharT, Traits, Alloc> & str,
        typename std::basic_string<CharT,Traits,Alloc>::size_type pos = 0,
        typename std::basic_string<CharT,Traits,Alloc>::size_type n
            = std::basic_string<CharT,Traits,Alloc>::npos,
        CharT zero = CharT('0'),
        CharT one = CharT('1'))
      : base(str, pos, n, zero, one) { }

    template <class CharT>
    explicit bitstring(const CharT * str,
        typename std::basic_string<CharT>::size_type n
            = std::basic_string<CharT>::npos,
        CharT zero = CharT('0'),
        CharT one = CharT('1'))
      : base(str, n, zero, one) {}

    bool operator&(const bitstring::bit_mask & rhs) noexcept
    {
        return base::operator[](rhs.which_bit());
    }

    struct bit_mask
    {
        constexpr bit_mask(std::size_t i) : which_bit_{i} { }
        constexpr bit_mask & operator>>=(int shift_by) noexcept
        {
            which_bit_ -= shift_by;
            return *this;
        }
        constexpr operator bool() const noexcept { return 0 <= which_bit_ && which_bit_ < N; }
        constexpr std::size_t which_bit() const { return which_bit_; }

      private:
        std::size_t which_bit_;  // ordinal
    };
};

template <std::size_t N>
constexpr bool operator<(const bitstring<N> & lhs, const bitstring<N> & rhs)
{
    auto lhs_iter = std::rbegin(lhs), rhs_iter = std::rbegin(rhs);
    while (*lhs-- == *rhs--)
    {
        if (lhs_iter == std::rend(lhs)) return false;
    }
    return *lhs < *rhs;
}

template <std::size_t N>
constexpr bool operator<=(const bitstring<N> & lhs, const bitstring<N> & rhs)
{
    return (lhs < rhs) || lhs == rhs;
}

template <std::size_t N>
constexpr bool operator>(const bitstring<N> & lhs, const bitstring<N> & rhs)
{
    return rhs < lhs;
}

template <std::size_t N>
constexpr bool operator>=(const bitstring<N> & lhs, const bitstring<N> & rhs)
{
    return rhs <= lhs;
}

namespace utils
{

template <std::size_t N>
struct bitlength_of<dpf::bitstring<N>>
  : public std::integral_constant<std::size_t, N> { };

template <std::size_t N>
struct msb_of<dpf::bitstring<N>>
{
    constexpr static auto value = typename dpf::bitstring<N>::bit_mask(bitlength_of_v<dpf::bitstring<N>>-1ul);
};

template <std::size_t N>
struct countl_zero_symmmetric_difference<dpf::bitstring<N>>
{
	using T = dpf::bitstring<N>;

	HEDLEY_CONST
	HEDLEY_ALWAYS_INLINE
	constexpr std::size_t operator()(const T & lhs, const T & rhs) const noexcept
	{
		using word_type = typename T::word_type;
		constexpr auto xor_op = std::bit_xor<word_type>{};
		auto adjust = lhs.data_length()*bitlength_of_v<word_type> - N;
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
    (bs.unchecked_set(i--, dpf::to_bit(bits)),...);
    return bs;
}

#endif  // LIBDPF_INCLUDE_DPF_BITSTRING_HPP__
