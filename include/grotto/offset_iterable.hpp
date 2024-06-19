/// @file grotto/offset_iterable.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief defines `dpf::rotated_iterable` and associated helpers
/// @details
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_OFFSET_VIEW_HPP__
#define LIBDPF_INCLUDE_GROTTO_OFFSET_VIEW_HPP__

#include <iterator>
#include <algorithm>

#include "dpf/rotation_iterable.hpp"

namespace grotto
{

template <typename WrappedIterator> struct offset_iterator;

template <typename WrappedIterator> struct offset_const_iterator;

template <typename WrappedIterator>
struct offset_iterable
{
    using wrapped_iterator = WrappedIterator;
    using iterator = offset_iterator<WrappedIterator>;
    using const_iterator = offset_const_iterator<WrappedIterator>;

    using value_type = typename std::iterator_traits<wrapped_iterator>::value_type;
    using size_type = typename std::iterator_traits<wrapped_iterator>::size_type;
    using difference_type = typename std::iterator_traits<wrapped_iterator>::difference_type;
    using reference = std::add_const_t<typename std::iterator_traits<wrapped_iterator>::reference>;
    using const_reference = std::add_const_t<typename std::iterator_traits<wrapped_iterator>::reference>;
    using pointer = typename std::iterator_traits<wrapped_iterator>::pointer;

    HEDLEY_ALWAYS_INLINE
    constexpr offset_iterable(wrapped_iterator begin, wrapped_iterator end,
        size_type offset)
      : rotated_iterable_{begin, end, std::distance(begin,
                std::upper_bound(begin, end, offset, std::less_equal<value_type>{}))},
        offset_{offset}
    {
        assert(std::is_sorted(begin, end));
    }

    HEDLEY_ALWAYS_INLINE
    constexpr value_type operator[](size_type index) const
    {
        return rotated_iterable_[index] - offset_;
    }

    HEDLEY_ALWAYS_INLINE
    constexpr auto rotation() const
    {
        return rotated_iterable_;
    }

    HEDLEY_ALWAYS_INLINE
    constexpr auto offset() const
    {
        return offset_;
    }

    iterator begin()
    {
        return offset_iterator{std::begin(rotated_iterable_), offset_};
    }
    const_iterator begin() const
    {
        return offset_const_iterator{std::begin(rotated_iterable_), offset_};
    }
    constexpr const_iterator cbegin() const
    {
        return begin();
    }

    iterator end()
    {
        return offset_iterator{std::end(rotated_iterable_), offset_};
    }

    const_iterator end() const
    {
        return offset_const_iterator{std::end(rotated_iterable_), offset_};
    }

    constexpr const_iterator cend() const
    {
        return end();
    }

  private:
    dpf::rotation_iterable<wrapped_iterator> rotated_iterable_;
    size_type offset_;
};

template <typename WrappedIterator>
struct offset_iterator_base
{
  public:
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr offset_iterator_base & operator++() noexcept
    {
        return (++it) - offset_;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr offset_iterator_base operator++(int) noexcept
    {
        auto tmp = *this;
        offset_iterator_base::operator++();
        return tmp;
    }


    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr offset_iterator_base & operator--() noexcept
    {
        return (--it) - offset_;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr offset_iterator_base operator--(int) noexcept
    {
        auto tmp = *this;
        offset_iterator_base::operator--();
        return tmp;
    }

    constexpr auto operator*() const = 0;

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator==(const offset_iterator_base & rhs) const noexcept
    {
        return it == rhs.it
            && offset_ == rhs.offset_;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator!=(const offset_iterator_base & rhs) const noexcept
    {
        return !(*this == rhs);
    }

  private:
    using wrapped_iterator = WrappedIterator;
    using size_type = typename std::iterator_traits<wrapped_iterator>::size_type;
    
    using const_reference = typename std::iterator_traits<wrapped_iterator>::const_reference;

    wrapped_iterator it;
    size_type offset_;
};

template <typename WrappedIterator>
struct offset_iterator : public offset_iterator_base<WrappedIterator>
{
  private:
    using base = offset_iterator_base<WrappedIterator>;
    using reference = typename base::reference;
  public:
    HEDLEY_ALWAYS_INLINE
    constexpr auto operator*() const
    {
        reference{*base::it};
    };
};

template <typename WrappedIterator>
struct offset_const_iterator : public offset_iterator_base<WrappedIterator>
{
  private:
    using base = offset_iterator_base<WrappedIterator>;
    using const_reference = typename base::const_reference;
  public:
    HEDLEY_ALWAYS_INLINE
    constexpr auto operator*() const
    {
        const_reference{*base::it};
    };
};

template <typename ContainerT>
auto offset_by(ContainerT container, typename ContainerT::size_type offset)
{
    return offset_iterable{std::begin(container), std::end(container), offset};
}

// assumes the range is sorted
template <typename IteratorT,
          typename InputT,
          typename Function,
          std::enable_if_t<std::is_same_v<InputT, typename std::iterator_traits<IteratorT>::value_type>, bool> = false>
constexpr auto for_each_offset(IteratorT begin, IteratorT end, InputT offset, Function && f)
{
    assert(std::is_sorted(begin, end));

    InputT val = -offset;
    dpf::utils::flip_msb_if_signed_integral(val);

    auto size = std::distance(begin, end);
    // find the new "start" of the sorted sequence after offsetting by offset
    std::size_t new_first = std::distance(begin,
        std::upper_bound(begin, end, val, std::less_equal<void>{}));

    // iterate in sorted order. `new_first` is the index of the
    // lexicographically smallest value in the `[begin,end)` after
    // shifting each element by `-offset`
    // in the input such that `*next(begin, new_first)
    auto it = std::next(begin, new_first);
    for (std::size_t i = new_first; i < size; ++i, ++it) f(i, *it - val);
    it = begin;
    for (std::size_t i = 0; i < new_first; ++i, ++it) f(i, *it - val);
    return new_first;
}

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_OFFSET_VIEW_HPP__
