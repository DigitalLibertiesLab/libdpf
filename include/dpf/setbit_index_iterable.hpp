/// @file dpf/setbit_index_iterable.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_SETBIT_INDEX_ITERABLE_HPP__
#define LIBDPF_INCLUDE_DPF_SETBIT_INDEX_ITERABLE_HPP__

#include <cstddef>
#include <type_traits>
#include <iterator>
#include <utility>

#include "hedley/hedley.h"

#include "dpf/bit_array.hpp"
#include "dpf/subinterval_iterable.hpp"

namespace dpf
{

template <typename ChildT,
          typename WordT>
class const_setbit_iterator;  // forward declaration

template <typename ChildT,
          typename WordT>
class setbit_index_iterable
{
  public:
    using iter = subinterval_iterable<bit_iterator<ChildT, WordT>>;
    using size_type = typename bit_array_base<ChildT, WordT>::size_type;
    using word_type = typename bit_array_base<ChildT, WordT>::word_type;
    using word_pointer = typename bit_array_base<ChildT, WordT>::word_pointer;
    using const_iterator = const_setbit_iterator<ChildT, WordT>;
    static constexpr size_type bits_per_word = bit_array_base<ChildT, WordT>::bits_per_word;

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    explicit setbit_index_iterable(const iter & it)
      : it_{it}, begin_{it_.it_.word_ptr_}, end_{begin_+it.buf_size_}, base_index_{calc_base_index(it_)}, length_{calc_length(it_, base_index_)}
    {
        update_bit_array();
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    explicit setbit_index_iterable(iter && it)
      : it_{it}, begin_{it_.it_.word_ptr_}, end_{begin_+it.buf_size_}, base_index_{calc_base_index(it_)}, length_{calc_length(it_, base_index_)}
    {
        update_bit_array();
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator begin() const noexcept
    {
        return const_iterator{begin_, base_index_,
            typename const_iterator::const_iterator_begin_tag{}};
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
        return const_iterator{end_, length_,
            typename const_iterator::const_iterator_end_tag{}};
    }
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator cend() const noexcept
    {
        return end();
    }

  private:
    const iter & it_;
    word_pointer begin_;
    word_pointer end_;
    size_type length_;
    size_type base_index_;

    static constexpr size_type calc_base_index(const iter & it)
    {
        return it.outputs_ == 0 ? it.from_ : utils::quotient_floor(it.from_, it.outputs_) * it.outputs_;
    }

    static constexpr size_type calc_length(const iter & it, size_type base_index)
    {
        return it.outputs_ == 0 ? utils::quotient_ceiling(it.to_, bits_per_word) * bits_per_word : utils::quotient_ceiling(it.to_, it.outputs_) * it.outputs_;
    }

    // outputs_ == 0 implies generated from eval_sequence
    //     so no bits need to be zero'd
    // otherwise generated from eval_interval
    //     depending on node size, word size, and where the from, to points fall within nodes
    //     some words will need to be zero'd completely
    //     while other words just need some bits to be masked out
    constexpr void update_bit_array()
    {
        if (it_.outputs_ == 0)
        {
            return;
        }

        word_type zero = 0;
        word_type mask = (~word_type(0)) << (it_.from_ % bits_per_word);
        word_pointer cur = begin_;
        std::size_t loc = it_.from_ % it_.outputs_;

        while (loc >= bits_per_word)
        {
            *cur = zero;
            ++cur;
            loc -= bits_per_word;
        }
        *cur &= mask;

        mask = (~word_type(0)) >> (bits_per_word - it_.to_ % bits_per_word - 1);
        cur = end_-1;
        loc = it_.to_ % it_.outputs_;

        while (loc < it_.outputs_ - bits_per_word)
        {
            *cur = zero;
            --cur;
            loc += bits_per_word;
        }
        *cur &= mask;
    }
};  // class dpf::setbit_index_iterable

template <typename ChildT,
          typename WordT>
class const_setbit_iterator
{
    using array_type = bit_array_base<ChildT, WordT>;
  public:
    using value_type = typename array_type::size_type;
    using reference = value_type;
    using const_reference = reference;
    using pointer = std::add_pointer_t<reference>;
    using iterator_category = std::bidirectional_iterator_tag;
    using word_type = typename array_type::word_type;
    using word_pointer = typename array_type::const_word_pointer;
    using size_type = typename array_type::size_type;
    using difference_type = std::ptrdiff_t;
    static constexpr auto bits_per_word = array_type::bits_per_word;

    HEDLEY_ALWAYS_INLINE
    constexpr const_setbit_iterator(const_setbit_iterator &&) noexcept = default;
    HEDLEY_ALWAYS_INLINE
    constexpr const_setbit_iterator(const const_setbit_iterator &) noexcept = default;

    ~const_setbit_iterator() noexcept = default;
    const_setbit_iterator & operator=(const_setbit_iterator &&) noexcept = default;
    const_setbit_iterator & operator=(const const_setbit_iterator &) = default;

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr reference operator*() const noexcept
    {
        // count trailing zeros -> offset within current_word_
        return base_index_ + utils::ctz(current_word_);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr const_setbit_iterator & operator++() noexcept
    {
        current_word_ &= current_word_-1;  // clear first (lowest) set bit
        seek_to_next_bit();  // advance `word_ptr` till nonzero word
        return *this;
    }

    HEDLEY_NO_THROW
    const_setbit_iterator operator++(int) noexcept
    {
        auto tmp = *this;
        const_setbit_iterator::operator++();
        return tmp;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_setbit_iterator & operator--() noexcept
    {
        seek_to_prev_bit();
        auto clz = utils::clz(current_word_ ^ *word_ptr_);
        current_word_ |= ~(~word_type(0) >> 1) >> clz;
        return *this;
    }

    HEDLEY_NO_THROW
    const_setbit_iterator operator--(int) noexcept
    {
        auto tmp = *this;
        const_setbit_iterator::operator--();
        return tmp;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator==(const const_setbit_iterator & rhs) const noexcept
    {
        return word_ptr_ == rhs.word_ptr_ &&
                current_word_ == rhs.current_word_;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator<(const const_setbit_iterator & rhs) const noexcept
    {
        return (word_ptr_ < rhs.word_ptr_ ||
            (word_ptr_ == rhs.word_ptr_ &&
                current_word_ > rhs.current_word_));
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator!=(const const_setbit_iterator & rhs) const noexcept
    {
        return !(*this == rhs);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator>(const const_setbit_iterator & rhs) const noexcept
    {
        return rhs < *this;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator<=(const const_setbit_iterator & rhs) const noexcept
    {
        return !(rhs < *this);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator>=(const const_setbit_iterator & rhs) const noexcept
    {
        return !(*this < rhs);
    }

  private:
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr void seek_to_next_bit() noexcept
    {
        while (HEDLEY_PREDICT(current_word_ == 0, false,
            2.0 / bits_per_word))
        {
            current_word_ = utils::le(*(++word_ptr_));
            base_index_ += bits_per_word;
        }
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr void seek_to_prev_bit() noexcept
    {
        word_type lo_bits = current_word_ ^ *word_ptr_;
        while (HEDLEY_PREDICT(lo_bits == 0, false,
            2.0 / bits_per_word))
        {
            lo_bits = utils::le(*(--word_ptr_));
            current_word_ = 0;
            base_index_ -= bits_per_word;
        }
    }

    struct const_iterator_end_tag final {};
    struct const_iterator_begin_tag final {};

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NON_NULL()
    explicit constexpr const_setbit_iterator(word_pointer word_ptr,
        size_type base_index, const_iterator_begin_tag) noexcept
        : word_ptr_{word_ptr},
        current_word_{utils::le(*word_ptr_)},
        base_index_{base_index}
    {
        seek_to_next_bit();
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NON_NULL()
    explicit constexpr const_setbit_iterator(word_pointer word_ptr,
        size_type base_index, const_iterator_end_tag) noexcept
        : word_ptr_{word_ptr},
        current_word_{*word_ptr_},  // utils::le(end) is a nop
        base_index_{base_index}
    { }

    word_pointer word_ptr_;
    word_type current_word_;
    size_type base_index_;

    friend const_setbit_iterator setbit_index_iterable<ChildT, WordT>::begin() const noexcept;
    friend const_setbit_iterator setbit_index_iterable<ChildT, WordT>::end() const noexcept;
};  // class dpf::const_setbit_iterator

template <typename ChildT,
          typename WordT>
HEDLEY_PURE
HEDLEY_ALWAYS_INLINE
dpf::setbit_index_iterable<ChildT, WordT> indices_set_in(const subinterval_iterable<bit_iterator<ChildT, WordT>> & iter) noexcept
{
    return dpf::setbit_index_iterable<ChildT, WordT>{iter};
}

template <typename ChildT,
          typename WordT>
HEDLEY_PURE
HEDLEY_ALWAYS_INLINE
dpf::setbit_index_iterable<ChildT, WordT> indices_set_in(subinterval_iterable<bit_iterator<ChildT, WordT>> && iter) noexcept
{
    return dpf::setbit_index_iterable<ChildT, WordT>{std::forward<subinterval_iterable<bit_iterator<ChildT, WordT>>>(iter)};
}

template <typename ChildT,
          typename WordT,
          class UnaryFunction>
HEDLEY_ALWAYS_INLINE
void for_each_set_index(const subinterval_iterable<bit_iterator<ChildT, WordT>> & arr, UnaryFunction f)
{
    for (auto i : indices_set_in(arr)) f(i);
}

}  // namespace dpf

namespace std
{

template <typename ChildT,
          typename WordT>
struct iterator_traits<dpf::const_setbit_iterator<ChildT, WordT>>
{
  private:
    using type = dpf::const_setbit_iterator<ChildT, WordT>;
  public:
    using iterator_category = typename type::iterator_category;
    using difference_type = typename type::difference_type;
    using value_type = typename type::value_type;
    using reference = typename type::reference;
    using const_reference = typename type::const_reference;
    using pointer = typename type::pointer;
};

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_SETBIT_INDEX_ITERABLE_HPP__
