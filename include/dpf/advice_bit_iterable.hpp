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

template <typename WrappedIteratorType>
class advice_bit_iterable_const_iterator;

template <typename Iterable>
class advice_bit_iterable
{
  public:
    using wrapped_iterator_type = typename Iterable::iterator_type;
    using const_iterator = advice_bit_iterable_const_iterator<wrapped_iterator_type>;

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
    using wrapped_type = WrappedIteratorType;
    using value_type = bool;
    using reference = value_type;
    using const_reference = reference;
    using pointer = std::add_pointer_t<reference>;
    using iterator_category = std::random_access_iterator_tag;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using node_type = typename std::iterator_traits<WrappedIteratorType>::value_type;

    HEDLEY_ALWAYS_INLINE
    constexpr advice_bit_iterable_const_iterator(const wrapped_type it) noexcept
        : it_{it}
    { }

    HEDLEY_ALWAYS_INLINE
    constexpr advice_bit_iterable_const_iterator(advice_bit_iterable_const_iterator &&) noexcept = default;
    HEDLEY_ALWAYS_INLINE
    constexpr advice_bit_iterable_const_iterator(const advice_bit_iterable_const_iterator &) noexcept = default;

    advice_bit_iterable_const_iterator & operator=(const advice_bit_iterable_const_iterator &) = default;
    advice_bit_iterable_const_iterator & operator=(advice_bit_iterable_const_iterator &&) noexcept = default;
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

    difference_type operator-(advice_bit_iterable_const_iterator rhs) const noexcept
    {
        return it_ - rhs.it_;
    }

    reference operator[](std::size_t i) const noexcept
    {
        return bit(it_ + i);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator==(const advice_bit_iterable_const_iterator & rhs) const noexcept
    {
        return it_ == rhs.it_;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator<(const advice_bit_iterable_const_iterator & rhs) const noexcept
    {
        return it_ < rhs.it_;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator!=(const advice_bit_iterable_const_iterator & rhs) const noexcept
    {
        return !(*this == rhs);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator>(const advice_bit_iterable_const_iterator & rhs) const noexcept
    {
        return rhs < *this;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator<=(const advice_bit_iterable_const_iterator & rhs) const noexcept
    {
        return !(rhs < *this);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator>=(const advice_bit_iterable_const_iterator & rhs) const noexcept
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

template <typename Iterator,
          class UnaryFunction>
void for_each_advice_bit(const dpf::advice_bit_iterable<Iterator> it, UnaryFunction f)
{
    for (auto i : it) f(i);
}

template <typename InputIt>  // typename advice_bit_iterable<Iterator>::const_iterator
auto bit_array_from_advice_bits(InputIt first, InputIt last)
{
    std::size_t bits = std::distance(first, last);
    auto ret = dynamic_bit_array(bits);

    auto curbit = ret.begin();
    for (; first != last; ++first)
    {
        (*curbit++).assign(*first);
    }

    return ret;
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
