/// @file dpf/aligned_allocator.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief An allocator that aligns memory allocations to a specified
///        alignment
/// @details The dpf::aligned_allocator class template is used to allocate
///          uninitialized memory with a specified alignment for all libdpf++
///          buffers and memoizers, if no user-specified allocator is
///          provided. It is stateless, so all instances of the allocator are
///          interchangeable. The alignment is specified by the Alignment
///          parameter, which must be a power of two (default:
///          `dpf::utils::max_align`).
///
///          The allocator supports the `allocate()` function for allocating
///          aligned, yet uninitialized memory and the `deallocate()` function
///          for freeing the same. It also includes a convenient
///          `allocate_unique_ptr()` function that returns a `std::unique_ptr`
///          to the output of `allocate()`.
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_ALIGNED_ALLOCATOR_HPP__
#define LIBDPF_INCLUDE_DPF_ALIGNED_ALLOCATOR_HPP__

#include <new>
#include <limits>
#include <memory>

#include "hedley/hedley.h"
#include "simde/simde/x86/avx2.h"

namespace dpf
{

/// @brief an allocator that allocates aligned memory
/// @details The `dpf::aligned_allocator` class template is the default memory
///          allocator used by all `libdpf++` buffers and memoizers, if no
///          user-specified allocator is provided. It allocates uninitialized
///          storage whose alignment is specified by `Alignment` and whose
///          size is an integral multiple of `sizeof(T)`. The allocator is
///          stateless; that is, all instances of the given allocator are
///          interchangeable and can deallocate memory allocated by any other
///          instance of the same allocator type.
/// @tparam T the type to allocate
/// @tparam Alignment specifies the alignment (default: `dpf::utils::max_align`).'
///         The program is ill-formed if `Alignment` is not a power of 2.
template <class T,
          std::size_t Alignment = utils::max_align_v>
class aligned_allocator
{
  private:
    /// @brief a `deleter` functor for use by `std::unique_ptr<T[]>` to free
    ///        memory allocated when the `std::unique_ptr<T[]>` was 
    ///        constructed
    template <typename Pointer>
    struct deleter
    {
        constexpr void operator()(Pointer p) const noexcept { free(p); }
    };
  public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = std::add_pointer_t<value_type>;
    using unique_ptr = std::unique_ptr<value_type[], deleter<pointer>>;
    using const_pointer = std::add_const_t<pointer>;
    using reference = value_type &;
    using const_reference = std::add_const_t<reference>;
    static constexpr size_type alignment = Alignment;

<<<<<<< Updated upstream
=======
    /// @brief class whose member `other` is a typedef of
    ///        `dpf::aligned_allocator` for some type `U` with alignment `A`.
    /// @tparam U the type to rebind to
    /// @tparam A the alignment of the rebound allocator
>>>>>>> Stashed changes
    template <class U, size_type A = alignment> struct rebind
    {
        using other = aligned_allocator<U, A>;
    };

    /// @name Constructors
    /// @brief Constructs the default allocator. Since the default allocator
    ///        is stateless, the constructors have no visible effect.
    /// @{

    /// @brief Default constructor
    /// @details Constructs an instance of `dpf::aligned_allocator`.
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr aligned_allocator() noexcept = default;

    /// @brief Copy constructor
    /// @details Constructs an instance of `dpf::aligned_allocator` from another
    ///          using copy semantics.
    /// @param other another `dpf::aligned_allocator` to construct with
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr aligned_allocator(const aligned_allocator & other) noexcept
        = default;

    /// @brief Move constructor
    /// @details Constructs an instance of `dpf::aligned_allocator` from another
    ///          using move semantics.
    /// @param other another `dpf::aligned_allocator` to construct with
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr aligned_allocator(aligned_allocator && other) noexcept = default;

    /// @}

    /// @brief D'tor
    /// @details Destroys an instance of `dpf::aligned_allocator`.
    ~aligned_allocator() = default;

    /// @brief returns the largest supported allocation size
    /// @details Returns the maximum theoretically possible value of `num`,
    ///          for which the call `allocate(num)` could succeed.
    /// @note This function returns the maximum number of elements that can
    ///       be allocated, not the maximum allocation size in bytes
    /// @return The maximum supported allocation size.
    constexpr size_type max_size() const noexcept
    {
        return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }

    /// @brief allocates aligned, yet uninitialized storage
    /// @details Allocates `num * sizeof(T)` bytes of uninitialized
    ///          storage by invoking
    ///          `std::aligned_alloc(alignment, num * sizeof(T))`.
    /// @param n the number of instances of `T` to allocate storage for
    /// @return Pointer to the first element of an array of `num` instaces
    ///         of type `T` whose elements have not been constructed yet.
    /// @throws std::bad_array_new_length if `max_size() < num`
    /// @throws std::bad_alloc if allocation fails.
    HEDLEY_ALWAYS_INLINE
    [[nodiscard]]
    constexpr
    pointer allocate(size_type num, const void * /*hint*/ = nullptr) const
    {
        if (max_size() < num)
        {
            throw std::bad_array_new_length();
        }

        void * ptr = std::aligned_alloc(alignment, num * sizeof(T));

        if (ptr == nullptr)
        {
            throw std::bad_alloc();
        }

        return assume_aligned(static_cast<pointer>(ptr));
    }

    /// @brief allocates and constructs a `std::unqiue_ptr<T[]>` to aligned, yet
    ///        uninitialized storage
    /// @details Allocates `num * sizeof(T)` bytes of uninitialized
    ///          storage by invoking `allocate(size_type, const void *)` and
    ///          returns a `std::unique_ptr<T[]>` that owns it.
    /// @param num the number of instances of `T` to allocate storage for
    /// @return An `std::unique_ptr<T[]>` owning the pointer to the first
    ///         element of an array of `num` instaces of type `T` whose elements
    ///         have not been constructed yet.
    /// @throws std::bad_array_new_length if `max_size() < num`
    /// @throws std::bad_alloc if allocation fails.
    HEDLEY_ALWAYS_INLINE
    constexpr auto allocate_unique_ptr(size_type num) const
    {
        return unique_ptr{allocate(num)};
    }

    /// @brief deallocates storage
    /// @details Deallocates the storage referenced by the pointer `p`,
    ///          which must be a pointer obtained by an earlier call to
    ///          `allocate()`.
    /// @param p pointer obtained from allocate()
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr void deallocate(pointer p, size_type /*num*/ = 0) const noexcept
    {
        free(p);
    }

    /// @brief informs the compiler that a pointer is aligned
    /// @details Informs the implementation that the object ptr points to is
    ///        aligned to at least `alignment`. The implementation may use
    ///        this information to generate more efficient code, but it might
    ///        only make this assumption if the object is accessed via the
    ///        return value of `assume_aligned`.
    ///
    ///        The behavior is undefined if `ptr` does not point to an object
    ///        of type `T` (ignoring cv-qualification at every level), or if the
    ///        object's alignment is not at least `Alignment`.
    /// @note It is up to the program to ensure that the alignment assumption
    ///       actually holds. A call to `assume_aligned` does not cause the
    ///       compiler to verify or enforce this.
    /// @param ptr the pointer
    /// @return `ptr`
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_NON_NULL(1)
    [[nodiscard]]
    constexpr static auto assume_aligned(pointer ptr) noexcept
    {
        return static_cast<pointer>(
            __builtin_assume_aligned(ptr, alignment));
    }
};

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_ALIGNED_ALLOCATOR_HPP__
