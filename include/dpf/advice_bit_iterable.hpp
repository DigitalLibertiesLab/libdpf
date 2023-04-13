/// @file dpf/advice_bit_iterable.hpp
/// @brief defines `dpf::advice_bit_iterable` and associated helpers
/// @details A `dpf::advice_bit_iterable` is a convenience class that wraps an
///          existing iterable type to provide a new iterable over advice bits
///          (i.e., over the least-significant bit of each element). The `begin`
///          and `end` member functions of the `dpf::advice_bit_iterable` class
///          each return `LegacyForwardIterator`s compatible with standard
///          library algorithms and range-based loops.
///
///          In addition to `dpf::advice_bit_iterable`, this file defines the
///          following helper functions:
///            - `advice_bits_of`: wraps an iterable type to simplify notation
///              for range-based loops. For example, it lets you write
///               ```cpp
///               for (auto b : advice_bits_of(my_iterable)) foo(b);
///               ```
///               instead of
///               ```cpp
///               advice_bit_iterable advice_bits{my_iterable};
///               for (auto b : advice_bits) foo(b);
///               ```
///            - `for_each_advice_bit`: iterate through and apply a given
///              function to each advice bit
///            - `bit_array_from_advice_bits`: constructs a
///              `dpf::dynamic_bit_array` that holds the advice bits of the
///              underlying iterable.
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_ADVICE_BIT_ITERABLE_HPP__
#define LIBDPF_INCLUDE_DPF_ADVICE_BIT_ITERABLE_HPP__

#include <algorithm>

#include "simde/simde/x86/avx2.h"

#include "dpf/bit_array.hpp"

namespace dpf
{

namespace detail
{

template <typename Iterator>
struct extract_bit_simde_node
{
    bool operator()(Iterator it) const
    {
        auto buf = reinterpret_cast<const char *>(&*it);
        return buf[0] & 1;
    }
};

template <typename NodeT, typename Iterator>
struct extract_bit;

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
template <typename Iterator>
struct extract_bit<simde__m128i, Iterator>
  : public extract_bit_simde_node<Iterator> { };

template <typename Iterator>
struct extract_bit<simde__m256i, Iterator>
  : public extract_bit_simde_node<Iterator> { };
HEDLEY_PRAGMA(GCC diagnostic pop)

}  // namespace detail

template <typename WrappedIteratorType>
class advice_bit_iterable_const_iterator;

template <typename Iterable>
class advice_bit_iterable
{
  public:
    using wrapped_iterator_type = typename Iterable::iterator_type;
    using const_iterator
        = advice_bit_iterable_const_iterator<wrapped_iterator_type>;

    explicit advice_bit_iterable(const Iterable & iterable)
      : begin_{std::begin(iterable)}, end_{std::end(iterable)}
    { }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator begin() const noexcept
    {
        return const_iterator(begin_);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator cbegin() const noexcept
    {
        return begin();
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator end() const noexcept
    {
        return const_iterator(end_);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator cend() const noexcept
    {
        return end();
    }

  private:
    const wrapped_iterator_type begin_, end_;
};  // class dpf::advice_bit_iterable

template <typename WrappedIteratorType>
class advice_bit_iterable_const_iterator
{
  public:
    using iterator_traits = std::iterator_traits<WrappedIteratorType>;
    using wrapped_type = WrappedIteratorType;
    using value_type = bool;
    using reference = value_type;
    using const_reference = reference;
    using pointer = std::add_pointer_t<reference>;
    using iterator_category = typename iterator_traits::iterator_category;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using node_type = typename iterator_traits::value_type;

    HEDLEY_ALWAYS_INLINE
    constexpr
    explicit advice_bit_iterable_const_iterator(const wrapped_type & it) noexcept
        : it_{it}
    { }

    HEDLEY_ALWAYS_INLINE
    constexpr
    advice_bit_iterable_const_iterator(advice_bit_iterable_const_iterator &&)
        = default;

    HEDLEY_ALWAYS_INLINE
    constexpr
    advice_bit_iterable_const_iterator(
        const advice_bit_iterable_const_iterator &) = default;

    advice_bit_iterable_const_iterator & operator=(
        const advice_bit_iterable_const_iterator &) = default;

    advice_bit_iterable_const_iterator & operator=(
        advice_bit_iterable_const_iterator &&) = default;

    ~advice_bit_iterable_const_iterator() = default;

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr reference operator*() const noexcept
    {
        return bit(it_);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr advice_bit_iterable_const_iterator & operator++() noexcept
    {
        ++it_;
        return *this;
    }

    HEDLEY_NO_THROW
    advice_bit_iterable_const_iterator operator++(int) noexcept
    {
        auto tmp = *this;
        advice_bit_iterable_const_iterator::operator++();
        return tmp;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    advice_bit_iterable_const_iterator & operator--() noexcept
    {
        --it_;
        return *this;
    }

    HEDLEY_NO_THROW
    advice_bit_iterable_const_iterator operator--(int) noexcept
    {
        auto tmp = *this;
        advice_bit_iterable_const_iterator::operator--();
        return tmp;
    }

    advice_bit_iterable_const_iterator & operator+=(std::size_t n) noexcept
    {
        it_ += n;
        return *this;
    }

    advice_bit_iterable_const_iterator operator+(std::size_t n) const noexcept
    {
        return advice_bit_iterable_const_iterator(it_ + n);
    }

    advice_bit_iterable_const_iterator & operator-=(std::size_t n) noexcept
    {
        it_ -= n;
        return *this;
    }

    advice_bit_iterable_const_iterator operator-(std::size_t n) const noexcept
    {
        return advice_bit_iterable_const_iterator(it_ - n);
    }

    difference_type
    operator-(advice_bit_iterable_const_iterator rhs) const noexcept
    {
        return it_ - rhs.it_;
    }

    reference operator[](std::size_t i) const noexcept
    {
        return bit(it_ + i);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool
    operator==(const advice_bit_iterable_const_iterator & rhs) const noexcept
    {
        return it_ == rhs.it_;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool
    operator<(const advice_bit_iterable_const_iterator & rhs) const noexcept
    {
        return it_ < rhs.it_;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool
    operator!=(const advice_bit_iterable_const_iterator & rhs) const noexcept
    {
        return !(*this == rhs);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool
    operator>(const advice_bit_iterable_const_iterator & rhs) const noexcept
    {
        return rhs < *this;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool
    operator<=(const advice_bit_iterable_const_iterator & rhs) const noexcept
    {
        return !(rhs < *this);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool
    operator>=(const advice_bit_iterable_const_iterator & rhs) const noexcept
    {
        return !(*this < rhs);
    }

  private:
    wrapped_type it_;
    static constexpr auto bit = detail::extract_bit<node_type, wrapped_type>{};
};  // class dpf::advice_bit_iterable_const_iterator

template <typename Iterable>
dpf::advice_bit_iterable<Iterable> advice_bits_of(const Iterable & iterable)
{
    return advice_bit_iterable<Iterable>{iterable};
}

template <typename Iterable, class UnaryFunction>
void for_each_advice_bit(const Iterable & iterable, UnaryFunction f)
{
    for (auto i : advice_bits_of(iterable)) f(i);
}

namespace detail
{

template <typename Iterator>
auto bit_array_from_advice_bits_small(Iterator first, Iterator last,
    std::size_t bits)
{
    auto ret = dynamic_bit_array(bits);

    auto curbit = ret.begin();
    for (; first != last; ++first)
    {
        (*curbit++).assign(*first);
    }

    return ret;
}

template <typename Iterator>
auto bit_array_from_advice_bits_simde(Iterator first, Iterator last,
    std::size_t bits)
{
    using simde_type = simde__m256i;
    using simde_ptr = simde_type *;
    static_assert(CHAR_BIT == 8, "CHAR_BIT not equal to 8");

    auto ret = dynamic_bit_array(bits);
    std::size_t bits_per_byte = CHAR_BIT,
                bytes = (bits-1)/bits_per_byte + 1,
                bits_per_word = ret.bits_per_word,
                bits_per_simde = dpf::utils::bitlength_of_v<simde_type>,
                bytes_per_simde = sizeof(simde_type),
                words_per_simde = bits_per_simde / bits_per_word;

    std::size_t curbits = 0, pos = 0;
    std::array<char, 32> in = {0};
    std::array<uint32_t, 8> out;
    while (curbits < bits)
    {
        simde_type simde = {0, 0, 0, 0};

        std::size_t i = 0;
        for (; i < bits_per_byte && curbits < bits; ++i)
        {
            for (std::size_t j = 0; j < 32 && curbits < bits; ++j, ++curbits)
            {
                in[j] = *first++;
            }
            auto tmp = reinterpret_cast<simde_ptr>(std::data(in));
            simde = simde_mm256_or_si256(
                simde_mm256_slli_epi64(simde, 1),
                simde_mm256_loadu_si256(tmp));
        }

        // algorithm expects "first bit" to be MSB in each 8-bit block at next step
        for (std::size_t j = i; j < bits_per_byte; ++j)
        {
            simde = simde_mm256_slli_epi64(simde, 1);
        }

        for (std::size_t j = 0; j < i; ++j)
        {
            out[j] = simde_mm256_movemask_epi8(simde);
            simde = simde_mm256_slli_epi64(simde, 1);
        }

        auto dst = reinterpret_cast<char *>(
            std::addressof(ret.data(pos++ * words_per_simde)));
        auto src = reinterpret_cast<char *>(std::data(out));
        std::memcpy(dst, src, std::min(bytes_per_simde, bytes));
        bytes -= bytes_per_simde;
    }

    return ret;
}

}  // namespace detail

template <std::size_t NbitsCrossover = 1 << 4,
          typename Iterator>
auto
bit_array_from_advice_bits(const advice_bit_iterable<Iterator> & advice_bits)
{
    auto first = std::begin(advice_bits), last = std::end(advice_bits);
    std::size_t bits = std::distance(first, last);

    if (bits < NbitsCrossover)
    {
        return detail::bit_array_from_advice_bits_small(first, last, bits);
    }
    else
    {
        return detail::bit_array_from_advice_bits_simde(first, last, bits);
    }
}

}  // namespace dpf

namespace std
{

template <typename Iterator>
struct iterator_traits<dpf::advice_bit_iterable_const_iterator<Iterator>>
{
  private:
    using type = dpf::advice_bit_iterable_const_iterator<Iterator>;
  public:
    using iterator_category = typename type::iterator_category;
    using difference_type = typename type::difference_type;
    using value_type = typename type::value_type;
    using reference = typename type::reference;
    using const_reference = typename type::const_reference;
    using pointer = typename type::pointer;
};

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_ADVICE_BIT_ITERABLE_HPP__
