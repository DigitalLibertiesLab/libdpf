/// @file dpf/subsequence_iterable.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief defines `dpf::subsequence_iterable` and associated helpers
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_SUBSEQUENCE_ITERABLE_HPP__
#define LIBDPF_INCLUDE_DPF_SUBSEQUENCE_ITERABLE_HPP__

#include <vector>

#include "hedley/hedley.h"

namespace dpf
{

template <typename output_t, typename input_t>
class iterable_subsequence
{
  public:
    class const_iterator;  // forward declaration

    iterable_subsequence(const void * buf, const std::vector<std::size_t> & subseq)
        : sequence{static_cast<const output_t *>(buf)}, subsequence_indices{subseq} { }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator begin() const noexcept
    {
        return const_iterator(sequence, std::begin(subsequence_indices));
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
        return const_iterator(sequence, std::end(subsequence_indices));
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
        using value_type = output_t;
        using reference = value_type;
        using const_reference = reference;
        using pointer = std::add_pointer_t<reference>;
        using iterator_category = std::random_access_iterator_tag;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using subsequence_iterator_type = typename std::vector<std::size_t>::const_iterator;

        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const output_t * seq, subsequence_iterator_type it) noexcept
          : sequence_{seq}, it_(it) {}

        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const_iterator &&) noexcept = default;
        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const const_iterator &) noexcept = default;

        HEDLEY_CONST
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr reference operator*() const noexcept
        {
            return sequence_[*it_];
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

        const_iterator & operator+=(input_t n) noexcept
        {
            it_ += n;
            return *this;
        }

        const_iterator operator+(input_t n) const noexcept
        {
            return const_iterator(it_ + n);
        }

        const_iterator & operator-=(input_t n) noexcept
        {
            it_ -= n;
            return *this;
        }

        const_iterator operator-(input_t n) const noexcept
        {
            return const_iterator(it_ - n);
        }

        difference_type operator-(const_iterator rhs) const noexcept
        {
            return it_ - rhs.it_;
        }

        reference operator[](input_t i) const noexcept
        {
            return sequence_[it_[i]];
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
        const output_t * sequence_;
        subsequence_iterator_type it_;
    };  // class dpf::class subsequence_iterable::const_iterator

  private:
    const output_t * sequence;
    const std::vector<std::size_t> & subsequence_indices;
};  // subsequence_iterable

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_SUBSEQUENCE_ITERABLE_HPP__
