/// @file dpf/advice_bit_iterable.hpp
/// @brief
/// @details
/// @author
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_ADVICE_BIT_ITERABLE_HPP__
#define LIBDPF_INCLUDE_DPF_ADVICE_BIT_ITERABLE_HPP__

#include "simde/simde/x86/avx2.h"

namespace dpf
{

namespace detail
{

template <typename Iterator>
struct extract_bit_simde_node
{
    bool operator()(const Iterator it) const
    {
        auto buf = reinterpret_cast<const char *>(&*it);
        return buf[0] & 1;
    }
};

template <typename NodeT, typename Iterator> struct extract_bit;

template <typename Iterator> struct extract_bit<simde__m128i, Iterator> : public extract_bit_simde_node<Iterator> {};
template <typename Iterator> struct extract_bit<simde__m256i, Iterator> : public extract_bit_simde_node<Iterator> {};

}  // namespace detail

template <typename Iterator>
class advice_bit_iterable
{
  public:
    using iterator_type = Iterator;
    using node_type = typename std::iterator_traits<Iterator>::value_type;
    class const_iterator;  // forward declaration

    explicit advice_bit_iterable(const iterator_type it, std::size_t length)
      : it_{it}, length_{length}
    { }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator begin() const noexcept
    {
        return const_iterator(it_);
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
        return const_iterator(it_ + length_);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator cend() const noexcept
    {
        return end();
    }

    class const_iterator
    {
      public:
        using value_type = bool;
        using reference = value_type;
        using const_reference = reference;
        using pointer = std::add_pointer_t<reference>;
        using iterator_category = std::random_access_iterator_tag;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const iterator_type it) noexcept
          : it_{it}
        { }

        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const_iterator &&) noexcept = default;
        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const const_iterator &) noexcept = default;

        const_iterator & operator=(const const_iterator &) = default;
        const_iterator & operator=(const_iterator &&) noexcept = default;
        ~const_iterator() = default;

        HEDLEY_CONST
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr reference operator*() const noexcept
        {
            return bit(it_);
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator & operator++() noexcept
        {
            ++it_;
            return *this;
        }

        HEDLEY_NO_THROW
        const_iterator operator++(int) noexcept
        {
            auto tmp = *this;
            const_iterator::operator++();
            return tmp;
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        const_iterator & operator--() noexcept
        {
            --it_;
            return *this;
        }

        HEDLEY_NO_THROW
        const_iterator operator--(int) noexcept
        {
            auto tmp = *this;
            const_iterator::operator--();
            return tmp;
        }

        const_iterator & operator+=(std::size_t n) noexcept
        {
            it_ += n;
            return *this;
        }

        const_iterator operator+(std::size_t n) const noexcept
        {
            return const_iterator(it_ + n);
        }

        const_iterator & operator-=(std::size_t n) noexcept
        {
            it_ -= n;
            return *this;
        }

        const_iterator operator-(std::size_t n) const noexcept
        {
            return const_iterator(it_ - n);
        }

        difference_type operator-(const_iterator rhs) const noexcept
        {
            return it_ - rhs.it_;
        }

        reference operator[](std::size_t i) const noexcept
        {
            return bit(it_ + i);
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bool operator==(const const_iterator & rhs) const noexcept
        {
            return it_ == rhs.it_;
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bool operator<(const const_iterator & rhs) const noexcept
        {
            return it_ < rhs.it_;
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bool operator!=(const const_iterator & rhs) const noexcept
        {
            return !(*this == rhs);
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bool operator>(const const_iterator & rhs) const noexcept
        {
            return rhs < *this;
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bool operator<=(const const_iterator & rhs) const noexcept
        {
            return !(rhs < *this);
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bool operator>=(const const_iterator & rhs) const noexcept
        {
            return !(*this < rhs);
        }

      private:
        iterator_type it_;
        static constexpr auto bit = detail::extract_bit<node_type, iterator_type>{};
    };  // class dpf::advice_bit_iterable::const_iterator

  private:
    const iterator_type it_;
    const std::size_t length_;

};  // class dpf::advice_bit_iterable

template <typename InputIt>  // typename advice_bit_iterable<Iterator>::const_iterator
auto bit_array_from_advice_bits(InputIt first, InputIt last)
{
    std::size_t bits = last - first;
    auto ret = dynamic_bit_array(bits);

    auto curbit = ret.begin();
    for (; first != last; ++first)
    {
        (*curbit++).assign(*first);
    }

    return ret;
}

}  // namespace dpf

// namespace std
// {

// template <typename Iterator>
// struct iterator_traits<typename dpf::advice_bit_iterable<Iterator>::const_iterator>
// {
//   private:
//     using type = dpf::advice_bit_iterable::const_iterator;
//   public:
//     using iterator_category = type::iterator_category;
//     using difference_type = type::difference_type;
//     using value_type = type::value_type;
//     using reference = type::reference;
//     using const_reference = type::const_reference;
//     using pointer = type::pointer;
// };

// }  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_ADVICE_BIT_ITERABLE_HPP__
