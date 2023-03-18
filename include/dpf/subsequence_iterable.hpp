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

template <typename DpfKey,
          typename OutputT,
          typename Iterator>
class subsequence_iterable
{
  public:
    using output_type = OutputT;
    static constexpr std::size_t outputs_per_leaf = DpfKey::outputs_per_leaf;
    static constexpr std::size_t mask = (1 << DpfKey::lg_outputs_per_leaf) - 1;
    class const_iterator;  // forward declaration

    subsequence_iterable(const output_type * seq, Iterator begin, Iterator end)
      : seq_{seq}, begin_{begin}, end_{end}, count_{std::distance(begin_, end_)}
    { }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator begin() const noexcept
    {
        return const_iterator(seq_, begin_);
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
        return const_iterator(seq_ + count_ * outputs_per_leaf, end_);
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
        using value_type = output_type;
        using reference = value_type;
        using const_reference = reference;
        using pointer = std::add_pointer_t<reference>;
        using iterator_category = std::random_access_iterator_tag;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using subsequence_iterator_type = Iterator;

        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const output_type * seq, subsequence_iterator_type it) noexcept
          : seq_{seq}, it_{it}
        { }

        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const_iterator &&) noexcept = default;
        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const const_iterator &) noexcept = default;

        HEDLEY_CONST
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr reference operator*() const noexcept
        {
            return seq_[*it_&mask];
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator & operator++() noexcept
        {
            ++it_;
            seq_ += outputs_per_leaf;
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
            seq_ -= outputs_per_leaf;
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
            seq_ += outputs_per_leaf*n;
            return *this;
        }

        const_iterator operator+(std::size_t n) const noexcept
        {
            return const_iterator(seq_ + outputs_per_leaf*n, it_ + n);
        }

        const_iterator & operator-=(std::size_t n) noexcept
        {
            it_ -= n;
            seq_ -= outputs_per_leaf*n;
            return *this;
        }

        const_iterator operator-(std::size_t n) const noexcept
        {
            return const_iterator(seq_ - outputs_per_leaf*n, it_ - n);
        }

        difference_type operator-(const_iterator rhs) const noexcept
        {
            return it_ - rhs.it_;
        }

        reference operator[](std::size_t i) const noexcept
        {
            return seq_[i*outputs_per_leaf + it_[i]&mask];
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bool operator==(const const_iterator & rhs) const noexcept
        {
            return seq_ == rhs.seq_ && it_ == rhs.it_;
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bool operator<(const const_iterator & rhs) const noexcept
        {
            return seq_ < rhs.seq_ && it_ < rhs.it_;
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
        const output_type * seq_;
        subsequence_iterator_type it_;
    };  // class dpf::subsequence_iterable::const_iterator

  private:
    const output_type * seq_;
    const Iterator begin_;
    const Iterator end_;
    const typename std::iterator_traits<Iterator>::difference_type count_;
};  // class dpf::subsequence_iterable

template <typename OutputT>
class recipe_subsequence_iterable
{
  public:
    using output_type = OutputT;
    class const_iterator;  // forward declaration

    recipe_subsequence_iterable(const output_type * seq, const std::vector<std::size_t> & indices)
      : seq_{seq}, indices_{indices}
    { }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator begin() const noexcept
    {
        return const_iterator(seq_, std::begin(indices_));
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
        return const_iterator(seq_, std::end(indices_));
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
        using value_type = output_type;
        using reference = value_type;
        using const_reference = reference;
        using pointer = std::add_pointer_t<reference>;
        using iterator_category = std::random_access_iterator_tag;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using subsequence_iterator_type = typename std::vector<std::size_t>::const_iterator;

        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const output_type * seq, subsequence_iterator_type it) noexcept
          : seq_{seq}, it_{it}
        { }

        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const_iterator &&) noexcept = default;
        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const const_iterator &) noexcept = default;

        HEDLEY_CONST
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr reference operator*() const noexcept
        {
            return seq_[*it_];
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
            return seq_[it_[i]];
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
        const output_type * seq_;
        subsequence_iterator_type it_;
    };  // class dpf::recipe_subsequence_iterable::const_iterator

  private:
    const output_type * seq_;
    const std::vector<std::size_t> & indices_;
};  // class dpf::recipe_subsequence_iterable

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_SUBSEQUENCE_ITERABLE_HPP__
