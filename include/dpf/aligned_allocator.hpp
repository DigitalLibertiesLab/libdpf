/// @file aligned_nodes.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2022 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef  LIBDPF_INCLUDE_DPF_ALIGNED_ALLOCATOR_HPP__
#define  LIBDPF_INCLUDE_DPF_ALIGNED_ALLOCATOR_HPP__

#include <new>
#include <limits>
#include <memory>

#include "hedley/hedley.h"
#include "simde/simde/x86/avx2.h"

namespace dpf
{

namespace detail
{

template <class T, std::size_t Alignment = alignof(T)>
class aligned_allocator : public std::allocator<T>
{
  public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = std::add_pointer_t<value_type>;
    using const_pointer = std::add_const_t<pointer>;
    using reference = value_type &;
    using const_reference = std::add_const_t<reference>;
    static constexpr size_type alignment = Alignment;

    //template <class U, size_type A = alignment> struct rebind { using other = aligned_allocator<U, A>; };

    /// @name C'tors
    /// @brief Constructs the default allocator. Since the default allocator
    ///        is stateless, the constructors have no visible effect.
    /// @{

    /// @brief Default c'tor
    constexpr aligned_allocator() = default;

    /// @brief Copy c'tor
    /// @param other another allocator to construct with
    constexpr aligned_allocator(const aligned_allocator & other)
      : std::allocator<T>(other) {}

    /// @brief Copy c'tor
    /// @param other another allocator to construct with
    template <class U, size_type A>
    constexpr aligned_allocator(const aligned_allocator<U, A> & other)
      : std::allocator<T>(other) {}

    /// @}

    /// @brief D'tor
    ~aligned_allocator() = default;

    /// @brief returns the largest supported allocation size
    /// @details Returns the maximum theoretically possible value of ``num``,
    ///          for which the call ``allocate(num)`` could succeed.
    /// @return The maximum supported allocation size.
    constexpr size_type max_size() const noexcept
    {
        return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }

    /// @brief allocates uninitialized storage
    /// @details Allocates ```num * sizeof(T)``` bytes of uninitialized
    ///          storage by invoking
    ///          ``std::aligned_alloc(std::size_t, std::size_t)``.
    /// @param n the number of instances of ``T`` to allocate storage for
    /// @return Pointer to the first element of an array of ``num`` instaces
    ///         of type ``T`` whose elements have not been constructed yet.
    /// @throws std::bad_array_new_length if ``max_size() < num``
    /// @throws std::bad_alloc if allocation fails.
    [[nodiscard]]
    constexpr pointer allocate(size_type num, const void * /*hint*/ = nullptr)
    {
        if (max_size() < num)
        {
            throw std::bad_array_new_length("alloc size is too large");
        }
        void * ptr = std::aligned_alloc(alignment, num * sizeof(T));
        if (ptr == nullptr)
        {
            throw std::bad_alloc("std::aligned_alloc failed");
        }
        return static_cast<pointer>(ptr);
    }

    /// @brief deallocates storage
    /// @details Deallocates the storage referenced by the pointer ``p``,
    ///          which must be a pointer obtained by an earlier call to
    ///          ``allocate()``.
    /// @param p pointer obtained from allocate()
    constexpr void deallocate(pointer p, size_type /*num*/ = 0)
    {
        free(p);
    }
};

}  // namespace detail

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_ALIGNED_ALLOCATOR_HPP__
