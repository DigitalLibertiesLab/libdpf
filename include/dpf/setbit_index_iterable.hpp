/// @file dpf/setbit_index_iterable.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_SETBIT_INDEX_ITERABLE_HPP__
#define LIBDPF_INCLUDE_DPF_SETBIT_INDEX_ITERABLE_HPP__

#include <utility>

#include "hedley/hedley.h"

#include "dpf/bit_array.hpp"

namespace dpf
{

class setbit_index_iterable
{
  public:
    class const_iterator;  // forward declaration
    using size_type = bit_array_base::size_type;

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    explicit setbit_index_iterable(const bit_array_base & b, size_type base = 0)
      : arr_{b}, base_index_{base}
    { }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    explicit setbit_index_iterable(bit_array_base && b, size_type base = 0)
      : arr_{b}, base_index_{base}
    { }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator begin() const noexcept
    {
        return const_iterator{arr_.data(), base_index_,
            const_iterator::const_iterator_begin_tag{}};
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
        return const_iterator{arr_.data() + arr_.data_length(), arr_.size(),
            const_iterator::const_iterator_end_tag{}};
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
        using value_type = typename bit_array_base::size_type;
        using reference = value_type;
        using const_reference = reference;
        using pointer = std::add_pointer_t<reference>;
        using iterator_category = std::bidirectional_iterator_tag;
        using word_type = bit_array_base::word_type;
        using word_pointer = bit_array_base::word_pointer;
        using size_type = bit_array_base::size_type;
        using difference_type = std::ptrdiff_t;
        static constexpr auto bits_per_word = bit_array_base::bits_per_word;

        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const_iterator &&) noexcept = default;
        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const const_iterator &) noexcept = default;

        ~const_iterator() noexcept = default;
        const_iterator & operator=(const_iterator &&) noexcept = default;
        const_iterator & operator=(const const_iterator &) = default;

        HEDLEY_CONST
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr reference operator*() const noexcept
        {
            // count trailing zeros -> offset within current_word_
            return base_index_ + psnip_builtin_ctz64(current_word_);
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator & operator++() noexcept
        {
            current_word_ &= current_word_-1;  // clear first (lowest) set bit
            seek_to_next_bit();  // advance `word_ptr` till nonzero word
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
            seek_to_prev_bit();
            auto clz = psnip_builtin_clz64(current_word_ ^ *word_ptr_);
            current_word_ |= ~(~word_type(0) >> 1) >> clz;
            return *this;
        }

        HEDLEY_NO_THROW
        const_iterator operator--(int) noexcept
        {
            auto tmp = *this;
            const_iterator::operator--();
            return tmp;
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bool operator==(const const_iterator & rhs) const noexcept
        {
            return word_ptr_ == rhs.word_ptr_ &&
                   current_word_ == rhs.current_word_;
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bool operator<(const const_iterator & rhs) const noexcept
        {
            return (word_ptr_ < rhs.word_ptr_ ||
                (word_ptr_ == rhs.word_ptr_ &&
                    current_word_ > rhs.current_word_));
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
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr void seek_to_next_bit() noexcept
        {
            while (HEDLEY_PREDICT(current_word_ == 0, false,
                2.0 / bits_per_word))
            {
                current_word_ = psnip_endian_le64(*(++word_ptr_));
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
                lo_bits = psnip_endian_le64(*(--word_ptr_));
                current_word_ = 0;
                base_index_ -= bits_per_word;
            }
        }

        struct const_iterator_end_tag final {};
        struct const_iterator_begin_tag final {};

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NON_NULL()
        explicit constexpr const_iterator(word_pointer word_ptr,
            size_type base_index, const_iterator_begin_tag) noexcept
          : word_ptr_{word_ptr},
            current_word_{psnip_endian_le64(*word_ptr_)},
            base_index_{base_index}
        {
            seek_to_next_bit();
        }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NON_NULL()
        explicit constexpr const_iterator(word_pointer word_ptr,
            size_type base_index, const_iterator_end_tag) noexcept
          : word_ptr_{word_ptr},
            current_word_{*word_ptr_},  // psnip_endian_le64(end) is a nop
            base_index_{base_index}
        { }

        word_pointer word_ptr_;
        word_type current_word_;
        size_type base_index_;

        friend const_iterator setbit_index_iterable::begin() const noexcept;
        friend const_iterator setbit_index_iterable::end() const noexcept;
    };  // class dpf::setbit_index_iterable::const_iterator

  private:
    const bit_array_base & arr_;
    const size_type base_index_;
};  // class dpf::setbit_index_iterable

HEDLEY_PURE
HEDLEY_ALWAYS_INLINE
dpf::setbit_index_iterable indices_set_in(const bit_array_base & b,
    std::size_t count_from = 0) noexcept
{
    return dpf::setbit_index_iterable{b, count_from};
}

HEDLEY_PURE
HEDLEY_ALWAYS_INLINE
dpf::setbit_index_iterable indices_set_in(bit_array_base && b,
    std::size_t count_from = 0) noexcept
{
    return dpf::setbit_index_iterable{std::forward<bit_array_base>(b), count_from};
}

template <class UnaryFunction>
HEDLEY_ALWAYS_INLINE
void for_each_set_index(const bit_array_base & arr, UnaryFunction f)
{
    for (auto i : indices_set_in(arr)) f(i);
}

}  // namespace dpf

namespace std
{

template <>
struct iterator_traits<dpf::setbit_index_iterable::const_iterator>
{
  private:
    using type = dpf::setbit_index_iterable::const_iterator;
  public:
    using iterator_category = type::iterator_category;
    using difference_type = type::difference_type;
    using value_type = type::value_type;
    using reference = type::reference;
    using const_reference = type::const_reference;
    using pointer = type::pointer;
};

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_SETBIT_INDEX_ITERABLE_HPP__
