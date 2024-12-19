// /// @file dpf/rotated_iterable.hpp
// /// @author Ryan Henry <ryan.henry@ucalgary.ca>
// /// @brief defines `dpf::rotated_iterable` and associated helpers
// /// @details
// /// @copyright Copyright (c) 2019-2023 Ryan Henry and others
// /// @license Released under a GNU General Public v2.0 (GPLv2) license;
// ///          see [LICENSE.md](@ref GPLv2) for details.

// #ifndef LIBDPF_INCLUDE_DPF_ROTATED_VIEW_HPP__
// #define LIBDPF_INCLUDE_DPF_ROTATED_VIEW_HPP__

// namespace dpf
// {

// template <typename ContainerT>
// struct rotated_iterable_iterator;        // forward declaration

// template <typename ContainerT>
// struct rotated_iterable_const_iterator;  // forward declaration

// template <typename ContainerT>
// struct rotated_iterable
// {
//     using container_type = ContainerT;

//     using value_type = typename container_type::value_type;
//     using size_type = typename container_type::size_type;
//     using difference_type = typename container_type::difference_type;
//     using reference = typename container_type::reference;
//     using const_reference = typename container_type::const_reference;
//     using pointer = typename container_type::pointer;
//     using const_pointer = typename container_type::const_pointer;

//     using iterator = rotated_iterable_iterator<container_type>;
//     using const_iterator = rotated_iterable_const_iterator<container_type>;
//     using wrapped_iterator = typename ContainerT::iterator;

//     rotated_iterable(const ContainerT & container, difference_type distance)
//       : container_{container},
//         distance_{distance >= 0 ? distance % container_.size() : (distance % container_.size()) + container_.size()},
//         wrap_to{std::begin(container)},
//         wrap_after{std::next(std::end(container), -1)},
//         end_after{std::next(wrap_to, distance-1)}
//     {
//         distance_ %= container_.size();
//         if (distance_ < 0)
//         {
//             distance_ += container_.size();
//         }
//     }

//     HEDLEY_ALWAYS_INLINE
//     reference operator[](size_type index)
//     {
//         index += distance_;
//         if (index > container_.size())
//         {
//             index -= container_.size();
//         }
//         return container_[index];
//     }

//     HEDLEY_ALWAYS_INLINE
//     const_reference operator[](size_type index) const
//     {
//         index += distance_;
//         if (index > container_.size())
//         {
//             index -= container_.size();
//         }
//         return container_[index];
//     }

//     HEDLEY_NO_THROW
//     HEDLEY_ALWAYS_INLINE
//     iterator begin() noexcept
//     {
//         return iterator{*this, std::next(end_after, 1)};
//     }

//     HEDLEY_NO_THROW
//     HEDLEY_ALWAYS_INLINE
//     const_iterator begin() const noexcept
//     {
//         return const_iterator{*this, std::next(end_after, 1)};
//     }

//     HEDLEY_NO_THROW
//     HEDLEY_ALWAYS_INLINE
//     const_iterator cbegin() const noexcept
//     {
//         return begin();
//     }

//     HEDLEY_NO_THROW
//     HEDLEY_ALWAYS_INLINE
//     iterator end() noexcept
//     {
//         return iterator{*this, std::next(wrap_after, 1)};
//     }

//     HEDLEY_NO_THROW
//     HEDLEY_ALWAYS_INLINE
//     const_iterator end() const noexcept
//     {
//         return const_iterator{*this, std::next(wrap_after, 1)};
//     }

//     HEDLEY_NO_THROW
//     HEDLEY_ALWAYS_INLINE
//     const_iterator cend() const noexcept
//     {
//         return end();
//     }

//     auto distance() const
//     {
//         return distance_;
//     }

//   private:
//     container_type & container_;
//     difference_type distance_;

//     wrapped_iterator wrap_to;
//     wrapped_iterator wrap_after;
//     wrapped_iterator end_after;
// };  // rotated_iterable

// template <typename ContainerT>
// struct rotated_iterator
// {
//     using wrapped_iterable_type = rotated_iterable<ContainerT>;
//     using wrapped_iterator = typename wrapped_iterable_type::iterator;
//     using size_type = typename wrapped_iterable_type::size_type;
//     using reference = typename wrapped_iterable_type::reference;

//     rotated_iterable<ContainerT> & v;
//     wrapped_iterator it;
//     rotated_iterator & operator++()
//     {
//         if (it == v.wrap_after)
//         {
//             it = v.wrap_to;
//         }
//         else if (it == v.end_after)
//         {
//             it = std::next(v.wrap_after, 1);
//         }
//         else
//         {
//             ++it;
//         }
        
//         return *this;
//     }
//     rotated_iterator & operator--()
//     {
//         --it;
//         if (it == v.wrap_after)
//         {
//             it = v.end_after;
//         }
//         else if (it == v.end_after)
//         {
//             it = std::next(v.wrap_to, -1);
//         }

//         return *this;
//     }
//     reference operator*() const { return *it; }
//     bool operator!=(const rotated_iterator other) const
//         { return &v != &other.v || it != other.it;  }
// };

// template <typename ContainerT>
// struct rotated_const_iterator
// {
//     using wrapped_iterable_type = rotated_iterable<ContainerT>;
//     using wrapped_iterator = typename wrapped_iterable_type::iterator;
//     using size_type = typename wrapped_iterable_type::size_type;
//     using const_reference = typename wrapped_iterable_type::const_reference;

//     const rotated_iterable<ContainerT> & v;
//     wrapped_iterator it;
//     rotated_const_iterator & operator++()
//     {
//         if (it == v.wrap_after)
//         {
//             it = v.wrap_to;
//         }
//         else if (it == v.end_after)
//         {
//             it = std::next(v.wrap_after, 1);
//         }
//         else
//         {
//             ++it;
//         }
        
//         return *this;
//     }
//     rotated_const_iterator & operator--()
//     {
//         --it;
//         if (it == v.wrap_after)
//         {
//             it = v.end_after;
//         }
//         else if (it == v.end_after)
//         {
//             it = std::next(v.wrap_to, -1);
//         }

//         return *this;
//     }
//     const_reference operator*() const { return *it; }
//     bool operator!=(const rotated_const_iterator other) const
//         { return &v != &other.v || it != other.it;  }
// };

// template <typename ContainerT>
// auto rotated_by(const ContainerT & container,
//     typename ContainerT::size_type rotate_by)
// {
//     return rotated_iterable{container, rotate_by};
// }

// template <typename ContainerT,
//           typename UnaryFunction>
// auto for_each_rotated_by(const ContainerT & container,
//     typename ContainerT::size_type rotate_by, UnaryFunction && f)
// {
//     for (auto i = rotate_by; i < container.size(); ++i) f(container[i]);
//     for (auto i = 0;         i < rotate_by;        ++i) f(container[i]);
// }

// }  // namespace dpf

// #endif  // LIBDPF_INCLUDE_DPF_ROTATED_VIEW_HPP__
