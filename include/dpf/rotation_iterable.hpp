#ifndef LIBDPF_INCLUDE_DPF_ROTATED_VIEW_HPP__
#define LIBDPF_INCLUDE_DPF_ROTATED_VIEW_HPP__

#include <type_traits>
#include <iterator>

namespace dpf
{

template <typename WrappedIterator> struct rotation_iterator;
template <typename WrappedIterator> struct rotation_const_iterator;

template <typename WrappedIterator>
struct rotation_iterable
{
    using wrapped_iterator = WrappedIterator;
    using iterator = rotation_iterator<wrapped_iterator>;
    using const_iterator = rotation_const_iterator<wrapped_iterator>;

    using reference = typename std::iterator_traits<wrapped_iterator>::reference;
    using const_reference = std::add_const_t<reference>;
    using difference_type = typename std::iterator_traits<wrapped_iterator>::difference_type;

    // O(1) if `WrappedIterator` is a random-access iterator
    constexpr rotation_iterable(wrapped_iterator begin, wrapped_iterator end,
        difference_type distance)
      : size_{std::distance(begin, end)},
        distance_{
          [this, &distance]()
            {
                distance %= this->size_;
                if (distance < 0)
                {
                    distance += this->size_;
                }
                return distance;
            }()},
        begin_{std::next(begin, static_cast<difference_type>(distance_))},
        wrap_to_{begin},
        wrap_after_{std::next(end, -difference_type(distance_ > 0))},
        end_after_{std::next(wrap_to_, static_cast<difference_type>(distance_-1))},
        end_{end}
    { }

    // UB if `begin` does not precede `middle` does not precede `end`
    // O(1) if `WrappedIterator` is a random-access iterator
    constexpr rotation_iterable(wrapped_iterator begin, wrapped_iterator end,
        wrapped_iterator middle)
      : size_{std::distance(begin, end)},
        distance_{std::distance(begin, middle)},
        wrap_to_{begin},
        wrap_after_{std::next(end, difference_type(-1))},
        end_after_{std::next(middle, difference_type(-1))}
    { }

    HEDLEY_ALWAYS_INLINE
    constexpr difference_type distance() const
    {
        return distance_;
    }

    // UB if `index<0` or `index>size_`
    // O(1) if `WrappedIterator` is a random-access iterator
    HEDLEY_ALWAYS_INLINE
    constexpr reference operator[](difference_type index)
    {
        index += distance_;
        if (index >= size_)
        {
            index -= size_;
        }
        return *std::next(wrap_to_, static_cast<difference_type>(index));
    }

    // UB if `index<0` or `index>size_`
    // O(1) if `WrappedIterator` is a random-access iterator
    HEDLEY_ALWAYS_INLINE
    constexpr const_reference operator[](difference_type index) const
    {
        index += distance_;
        if (index >= size_)
        {
            index -= size_;
        }
        return *std::next(wrap_to_, static_cast<difference_type>(index));
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr iterator begin() noexcept
    {
        return iterator(*this, begin_);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr const_iterator begin() const noexcept
    {
        return const_iterator(*this, begin_);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr const_iterator cbegin() const noexcept
    {
        return begin();
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr iterator end() noexcept
    {
        return iterator(*this, end_);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr const_iterator end() const noexcept
    {
        return const_iterator(*this, end_);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr const_iterator cend() const noexcept
    {
        return end();
    }

  private:
    difference_type size_;
    difference_type distance_;
    wrapped_iterator begin_;
    wrapped_iterator wrap_to_;
    wrapped_iterator wrap_after_;
    wrapped_iterator end_after_;
    wrapped_iterator end_;

    template <typename Iter>
    friend struct rotation_iterator_base;
};  // rotation_iterable

template <typename WrappedIterator>
struct rotation_iterator_base
{
  public:
    using wrapped_iterator = WrappedIterator;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = typename std::iterator_traits<WrappedIterator>::difference_type;
    using value_type = typename std::iterator_traits<WrappedIterator>::value_type;
    using reference = typename std::iterator_traits<WrappedIterator>::reference;
    using const_reference = std::add_const_t<reference>;
    using pointer = typename std::iterator_traits<WrappedIterator>::pointer;

    rotation_iterator_base(rotation_iterable<wrapped_iterator> & iterable_,
        wrapped_iterator iterator)
      : iterable{iterable_}, it{iterator} { }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr rotation_iterator_base & operator++() noexcept
    {
        if (it == iterable.wrap_after_)
        {
            it = iterable.wrap_to_;
        }
        else if (it == iterable.end_after_)
        {
            it = iterable.end_;
        }
        else
        {
            ++it;
        }

        return *this;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr rotation_iterator_base operator++(int) noexcept
    {
        auto tmp = *this;
        this->operator++();
        return tmp;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr rotation_iterator_base & operator--() noexcept
    {
        --it;
        if (it == iterable.wrap_after_)
        {
            it = iterable.end_after_;
        }
        else if (it == iterable.end_after_)
        {
            it = std::next(iterable.wrap_to_, -1);
        }

        return *this;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr rotation_iterator_base operator--(int) noexcept
    {
        auto tmp = *this;
        this->operator--();
        return tmp;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator==(const rotation_iterator_base & rhs) const noexcept
    {
        return iterable.begin_ == rhs.iterable.begin_
                && iterable.wrap_to_ == rhs.iterable.wrap_to_
                && iterable.wrap_after_ == rhs.iterable.wrap_after_
                && iterable.end_after_ == rhs.iterable.end_after_
                && iterable.end_ == rhs.iterable.end_
                && it == rhs.it;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator!=(const rotation_iterator_base & rhs) const noexcept
    {
        return !(*this == rhs);
    }

  protected:
    rotation_iterable<WrappedIterator> & iterable;
    wrapped_iterator it;
};  // rotation_iterator_base

template <typename WrappedIterator>
struct rotation_iterator final
  : public rotation_iterator_base<WrappedIterator>
{
  private:
    using base = rotation_iterator_base<WrappedIterator>;
  public:
    using wrapped_iterator = WrappedIterator;
    using reference = typename base::reference;

    rotation_iterator(rotation_iterable<WrappedIterator> & iterable,
        wrapped_iterator iterator)
      : base{iterable, iterator} {}

    HEDLEY_ALWAYS_INLINE
    constexpr reference operator*() const
    {
        return *base::it;
    }
};  // rotation_iterator

template <typename WrappedIterator>
struct rotation_const_iterator final
  : public rotation_iterator_base<WrappedIterator>
{
  private:
    using base = rotation_iterator_base<WrappedIterator>;
  public:
    using wrapped_iterator = WrappedIterator;
    using const_reference = typename base::const_reference;

    rotation_const_iterator(rotation_iterable<WrappedIterator> & iterable,
        wrapped_iterator iterator)
      : base{iterable, iterator} {}

    HEDLEY_ALWAYS_INLINE
    constexpr const_reference operator*() const
    {
        return *base::it;
    }
};  // rotation_const_iterator

template <typename ContainerT>
auto rotated_by(const ContainerT & container,
    typename ContainerT::size_type rotate_by)
{
    return rotation_iterable{std::begin(container), std::end(container), rotate_by};
}

template <typename IteratorT,
          typename UnaryFunction>
constexpr void for_each_rotated_by(IteratorT begin, IteratorT end,
    typename std::iterator_traits<IteratorT>::difference_type rotate_by,
    UnaryFunction && f)
{
    auto size = std::distance(begin, end);
    auto it = std::next(begin, rotate_by);
    for (std::size_t i = rotate_by; i < size; ++i, ++it) f(i, *it);
    it = begin;
    for (std::size_t i = 0; i < rotate_by; ++i, ++it) f(i, *it);
}

}  // namespace dpf

namespace std
{

template <typename WrappedIterator>
struct iterator_traits<dpf::rotation_iterator<WrappedIterator>>
{
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = typename std::iterator_traits<WrappedIterator>::difference_type;
    using value_type = typename std::iterator_traits<WrappedIterator>::value_type;
    using reference = typename std::iterator_traits<WrappedIterator>::reference;
    using const_reference = typename std::add_const_t<reference>;
    using pointer = typename std::iterator_traits<WrappedIterator>::pointer;
};

template <typename WrappedIterator>
struct iterator_traits<dpf::rotation_const_iterator<WrappedIterator>>
{
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = typename std::iterator_traits<WrappedIterator>::difference_type;
    using value_type = typename std::iterator_traits<WrappedIterator>::value_type;
    using reference = typename std::iterator_traits<WrappedIterator>::reference;
    using const_reference = typename std::add_const_t<reference>;
    using pointer = typename std::iterator_traits<WrappedIterator>::pointer;
};

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_ROTATED_VIEW_HPP__