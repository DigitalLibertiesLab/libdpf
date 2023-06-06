/// @file dpf/zip_iterable.hpp
/// @brief defines the `dpf::zip_itrable` class and associated helpers
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_ZIP_ITERABLE_HPP__
#define LIBDPF_INCLUDE_DPF_ZIP_ITERABLE_HPP__

#include <tuple>
#include <iterator>

namespace dpf
{

template <typename ...Iterables> struct zip_iterator;

template <typename ...Iterables>
struct zip_iterable
{
    using value_type = std::tuple<typename Iterables::iterator...>;
    using iterator = zip_iterator<Iterables...>;

    zip_iterable(value_type && begin, value_type && end)
      : begin_{begin}, end_{end} { }

    auto begin()
    {
        return iterator{begin_};
    }
    auto begin() const
    {
        return iterator{begin_};
    }
    auto cbegin() const
    {
        return begin();
    }
    auto end()
    {
        return iterator{end_};
    }
    auto end() const
    {
        return iterator{end_};
    }
    auto cend() const
    {
        return end();
    }

    value_type begin_, end_;
};

template <typename ...Iterables>
struct zip_iterator
{
    using value_type = std::tuple<typename Iterables::iterator...>;

    auto operator*()
    {
        return std::apply([](auto && ...its){ return std::make_tuple(*its...); }, wrapped_iterators);
    }
    zip_iterator & operator++()
    {
        std::apply([](auto && ...its){ (++its, ...); }, wrapped_iterators);
        return *this;
    }
    zip_iterator operator++(int)
    {
        zip_iterator old = *this;
        std::apply([](auto && ...its){ (++its, ...); }, wrapped_iterators);
        return old;
    }
    zip_iterator & operator--()
    {
        std::apply([](auto && ...its){ (--its, ...); }, wrapped_iterators);
        return *this;
    }
    zip_iterator operator--(int)
    {
        zip_iterator old = *this;
        std::apply([](auto && ...its){ (--its, ...); }, wrapped_iterators);
        return old;
    }
    bool operator==(const zip_iterator & rhs) const
    {
        return wrapped_iterators == rhs.wrapped_iterators;
    }
    bool operator!=(const zip_iterator & rhs) const
    {
        return wrapped_iterators != rhs.wrapped_iterators;
    }
    value_type wrapped_iterators;
};

template <typename ...Iterables>
HEDLEY_PURE
HEDLEY_ALWAYS_INLINE
zip_iterable<Iterables...> tuple_as_zip(const std::tuple<Iterables...> & tup) noexcept
{
    return zip_iterable<Iterables...>(
        std::apply([](auto && ...elements){ return std::make_tuple(std::begin(elements)...); }, tup),
        std::apply([](auto && ...elements){ return std::make_tuple(std::end(elements)...); }, tup));
}

template <typename TupleT,
          typename UnaryFunction>
HEDLEY_ALWAYS_INLINE
void for_each_in_zip(TupleT && tuple, UnaryFunction f)
{
    for (auto i : tuple_as_zip(tuple)) f(i);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_ZIP_ITERABLE_HPP__
