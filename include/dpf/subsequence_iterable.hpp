/// @file dpf/subsequence_iterable.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief defines `dpf::subsequence_iterable` and associated helpers
/// @details
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_SUBSEQUENCE_ITERABLE_HPP__
#define LIBDPF_INCLUDE_DPF_SUBSEQUENCE_ITERABLE_HPP__

#include <vector>

#include "hedley/hedley.h"

namespace dpf
{

template <typename DpfKey,
          typename OutputIterT,
          typename PointsIterT>
class subsequence_iterable
{
  public:
    using output_type = typename std::iterator_traits<OutputIterT>::value_type;
    using input_type = typename DpfKey::input_type;
    using output_iterator = OutputIterT;
    using points_iterator = PointsIterT;
    static constexpr std::size_t outputs_per_leaf = DpfKey::outputs_per_leaf;
    static constexpr std::size_t lg_outputs_per_leaf = DpfKey::lg_outputs_per_leaf;
    static constexpr auto mod = utils::mod_pow_2<input_type>{};
    class const_iterator;  // forward declaration
    using iterator = const_iterator;

    subsequence_iterable(output_iterator out_it, points_iterator begin, points_iterator end)
      : out_it_{out_it}, begin_{begin}, end_{end}, count_{std::distance(begin_, end_)}
    { }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator begin() const noexcept
    {
        return const_iterator(out_it_, begin_);
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
        return const_iterator(out_it_ + count_ * outputs_per_leaf, end_);
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

        using subsequence_iterator_type = points_iterator;

        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(output_iterator out_it, subsequence_iterator_type it) noexcept
          : out_it_{out_it}, it_{it}
        { }

        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const_iterator &&) noexcept = default;
        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator(const const_iterator &) noexcept = default;

        const_iterator & operator=(const_iterator &&) noexcept = default;
        const_iterator & operator=(const const_iterator &) = default;
        ~const_iterator() = default;

        HEDLEY_CONST
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr reference operator*() const noexcept
        {
            return out_it_[mod(*it_, lg_outputs_per_leaf)];
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr const_iterator & operator++() noexcept
        {
            ++it_;
            out_it_ += outputs_per_leaf;
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
            out_it_ -= outputs_per_leaf;
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
            out_it_ += outputs_per_leaf*n;
            return *this;
        }

        const_iterator operator+(std::size_t n) const noexcept
        {
            return const_iterator(out_it_ + outputs_per_leaf*n, it_ + n);
        }

        const_iterator & operator-=(std::size_t n) noexcept
        {
            it_ -= n;
            out_it_ -= outputs_per_leaf*n;
            return *this;
        }

        const_iterator operator-(std::size_t n) const noexcept
        {
            return const_iterator(out_it_ - outputs_per_leaf*n, it_ - n);
        }

        difference_type operator-(const_iterator rhs) const noexcept
        {
            return it_ - rhs.it_;
        }

        reference operator[](std::size_t i) const noexcept
        {
            return out_it_[i*outputs_per_leaf + mod(it_[i], lg_outputs_per_leaf)];
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bool operator==(const const_iterator & rhs) const noexcept
        {
            return out_it_ == rhs.out_it_ && it_ == rhs.it_;
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bool operator<(const const_iterator & rhs) const noexcept
        {
            return out_it_ < rhs.out_it_ && it_ < rhs.it_;
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
        output_iterator out_it_;
        subsequence_iterator_type it_;
    };  // class dpf::subsequence_iterable::const_iterator

  private:
    const output_iterator out_it_;
    const points_iterator begin_;
    const points_iterator end_;
    const typename std::iterator_traits<points_iterator>::difference_type count_;
};  // class dpf::subsequence_iterable

template <typename IteratorT>
class recipe_subsequence_iterable
{
  public:
    using output_type = typename std::iterator_traits<IteratorT>::value_type;
    using output_iterator = IteratorT;
    class const_iterator;  // forward declaration
    using iterator = const_iterator;

    recipe_subsequence_iterable(output_iterator out_it, const std::vector<std::size_t> & indices)
      : out_it_{out_it}, indices_{indices}
    {
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator begin() const noexcept
    {
        return const_iterator(out_it_, std::begin(indices_));
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
        return const_iterator(out_it_, std::end(indices_));
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
        constexpr const_iterator(output_iterator out_it, subsequence_iterator_type it) noexcept
          : out_it_{out_it}, it_{it}
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
            return out_it_[*it_];
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
            return out_it_[it_[i]];
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
        output_iterator out_it_;
        subsequence_iterator_type it_;
    };  // class dpf::recipe_subsequence_iterable::const_iterator

  private:
    const output_iterator out_it_;
    const std::vector<std::size_t> & indices_;
};  // class dpf::recipe_subsequence_iterable

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_SUBSEQUENCE_ITERABLE_HPP__
