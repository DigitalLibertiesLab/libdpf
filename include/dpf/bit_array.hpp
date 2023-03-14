/// @file dpf/bit_array.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_BIT_ARRAY_HPP__
#define LIBDPF_INCLUDE_DPF_BIT_ARRAY_HPP__

#include <cassert>
#include <limits>
#include <utility>
#include <string>
#include <cstring>
#include <memory>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <ostream>

#include "hedley/hedley.h"
#include "portable-snippets/exact-int/exact-int.h"
#include "portable-snippets/builtin/builtin.h"
#include "portable-snippets/endian/endian.h"

#include "dpf/bit.hpp"
#include "dpf/utils.hpp"

namespace dpf
{

/// @brief a class representing a fixed-size sequence of bits
/// @details A `bit_array` represents a (dynamically-allocated) fixed-size
///          sequence of bits. The underlying storage is an array of integers
///          of type `dpf::bit_array::word_type`.
class bit_array_base
{
  public:
    class bit_reference;       // forward reference
    class bit_iterator;        // forward reference
    class const_bit_iterator;  // forward reference

    /// @brief `dpf::bit`
    using value_type = dpf::bit;
    /// @brief proxy class representing a reference to a single bit.
    using reference = bit_array_base::bit_reference;
    /// @brief `bool`
    using const_reference = bool;
    /// @brief a random access iterator to `value_type`.
    /// @note convertible to `bit_array_base::const_bit_iterator`
    using iterator =  bit_array_base::bit_iterator;
    /// @brief a random access iterator to `const value_type`
    using const_iterator =  bit_array_base::const_bit_iterator;
    /// @brief a type that simulates pointer-to-`value_type` behavior,
    ///        identical to `iterator`.
    using pointer = iterator;
    /// @brief a type that simulates pointer-to-`const value_type` behavior,
    ///        identical to `const_iterator`.
    using const_pointer = const_iterator;
    /// @brief a signed integral type, identical to
    ///        `std::iterator_traits<iterator>::difference_type`.
    using difference_type = std::ptrdiff_t;
    static_assert(std::is_integral_v<difference_type>
        && std::is_signed_v<difference_type>);
    /// @brief an unsigned integral type that can represent any non-negative
    ///        value of `difference_type`.
    using size_type = std::size_t;
    static_assert(std::is_integral_v<size_type>
        && std::is_unsigned_v<size_type>);
    /// @brief an unsigned integral type used for the internal representation
    ///        of bits.
    using word_type = uint64_t;
    static_assert(std::is_integral_v<size_type>
        && std::is_unsigned_v<size_type>);
    /// @brief pointer to `word_type`
    using word_pointer = std::add_pointer_t<word_type>;
    /// @brief the number of `dpf::bit`s represented by each `word_type`.
    /// @note guaranteed (via `static_assert`) to be `64`
    static constexpr size_type bits_per_word =
        std::numeric_limits<word_type>::digits;
    static_assert(bits_per_word == 64);

    bit_array_base() = delete;

    /// @brief default copy constructor
    inline constexpr bit_array_base(const bit_array_base &) = default;

    /// @brief default move constructor
    inline constexpr bit_array_base(bit_array_base &&) = default;

    /// @brief default copy assignment
    inline constexpr
    bit_array_base & operator=(const bit_array_base &) = default;

    /// @brief defaulted move assignment
    inline constexpr
    bit_array_base & operator=(bit_array_base &&) noexcept = default;

    /// @brief direct access to the underlying data array
    /// @return a pointer to the start of the data array
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr word_pointer data() const noexcept { return arr_; }

    /// @brief direct access into the underlying data array (w/o bounds
    ///        checking)
    /// @param pos the array element to access
    /// @note Does not perform bounds checking; behaviour is undefined if
    ///       `pos` is out of bounds
    /// @return `data()[pos]`
    HEDLEY_ALWAYS_INLINE
    constexpr word_type & data(size_type pos) const noexcept
    {
        return arr_[pos];
    }

    /// @brief length of the underlying data array
    /// @return the number of elements in the underlying array (excluding a
    ///         non-data sentinel element)
    HEDLEY_ALWAYS_INLINE
    constexpr size_type data_length() const noexcept { return data_length_; }

    /// @brief access specified bit (w/o bounds checking)
    /// @{
    /// @details accesses the bit at position `pos`
    /// @param pos the 0-based position of the bit to return (least
    ///            significant to most significant)
    /// @note Unlike `test` and `at`, does not throw exceptions: the behavior
    ///       is undefined if `pos` is out of bounds
    /// @returns an object of type `dpf::bit_array_base::reference`, which
    ///          allows writing to the requested bit
    /// @complexity `O(1)`
    HEDLEY_NO_THROW
    inline constexpr reference operator[](size_type pos) noexcept
    {
        return reference{&arr_[pos / bits_per_word],
            word_type(1) << (pos % bits_per_word)};
    }

    /// @details accesses the bit at position `pos`
    /// @param pos the 0-based position of the bit to return (least
    ///            significant to most significant)
    /// @note Unlike `test` and `at`, does not throw exceptions: the behavior
    ///       is undefined if `pos` is out of bounds
    /// @returns the value of the requested bit
    /// @complexity `O(1)`
    HEDLEY_NO_THROW
    inline constexpr const_reference operator[](size_type pos) const noexcept
    {
        return reference{&arr_[pos / bits_per_word],
            word_type(1) << (pos % bits_per_word)};
    }
    /// @}

    /// @brief access specified bit with bounds checking
    /// @{
    /// @details accesses the bit at position `pos`
    /// @param pos the 0-based position of the bit to return (least
    ///            significant to most significant)
    /// @throws std::out_of_range if `pos` does not correspond to a valid
    ///         position within the `bit_array_base`
    /// @returns an object of type `dpf::bit_array_base::reference`, which
    ///          allows writing to the requested bit
    /// @complexity `O(1)`
    constexpr reference at(size_type pos)
    {
        utils::constexpr_maybe_throw<std::out_of_range>(
            HEDLEY_UNLIKELY(!(pos < size())),
            "pos is out of range");
        return this->operator[](pos);
    }

    /// @details accesses the bit at position `pos`
    /// @param pos the 0-based position of the bit to return (least
    ///            significant to most significant)
    /// @throws std::out_of_range if `pos` does not correspond to a valid
    ///         position within the `bit_array_base`
    /// @returns the value of the requested bit
    /// @complexity `O(1)`
    constexpr const_reference at(size_type pos) const
    {
        utils::constexpr_maybe_throw<std::out_of_range>(
            HEDLEY_UNLIKELY(!(pos < size())),
            "pos is out of range");
        return this->operator[](pos);
    }
    /// @}

    /// @brief returns an iterator to the first bit
    /// @{
    /// @returns iterator to the first element
    /// @complexity `O(1)`
    constexpr iterator begin() noexcept
    {
        return iterator{arr_};
    }
    /// @returns iterator to the first element
    /// @complexity `O(1)`
    constexpr const_iterator begin() const noexcept
    {
        return const_iterator{arr_};
    }
    /// @returns iterator to the first element
    /// @complexity `O(1)`
    constexpr const_iterator cbegin() const noexcept
    {
        return const_iterator{arr_};
    }
    /// @}

    /// @brief returns an iterator to the end (one past the last bit)
    /// @{
    /// @returns iterator to the element following the last element
    /// @complexity `O(1)`
    constexpr iterator end() noexcept
    {
        return bit_iterator{arr_ + data_length_};
    }
    /// @returns iterator to the element following the last element
    /// @complexity `O(1)`
    constexpr const_iterator end() const noexcept
    {
        return const_iterator{arr_ + data_length_};
    }
    /// @returns iterator to the element following the last element
    /// @complexity `O(1)`
    constexpr const_iterator cend() const noexcept
    {
        return const_iterator{arr_ + data_length_};
    }
    /// @}

    /// @brief checks if the specified bit is set to `true`
    /// @param pos the 0-based position of the bit to return (least
    ///            significant to most significant)
    /// @returns `true` if the requested bit is set, `false` otherwise
    /// @complexity `O(1)`
    bool test(size_type pos) const
    {
        return at(pos);
    }

    /// @brief checks if all, any or none of the bits are set to `true`
    /// @{
    /// @details checks if all bits are set to `true`
    /// @return `true` if all of the bits are set to `true`, otherwise `false`
    /// @complexity `O(size())`
    bool all() const noexcept
    {
        return ~word_type(0) == std::accumulate(arr_, arr_+data_length_,
            word_type(0), std::bit_and<word_type>{});
    }

    /// @details checks if all bits in a range are set to `true`
    /// @param first,last the range of elements under consideration
    /// @tparam Iterator an iterator type
    /// @return `true` if all of the bits in the given range are set to
    ///         `true`, otherwise `false`
    /// @complexity `O(last-first)`
    template <typename Iterator>
    bool all(Iterator first, Iterator last) const noexcept
    {
        word_type init = (*first.word_ptr_ | ~(first.mask_-1)) &
            (*last.word_ptr_ | (last.mask_-1));
        return ~word_type(0) == std::accumulate(first.word_ptr_+1,
            last.word_ptr_, init, std::bit_and<word_type>{});
    }

    /// @details checks if any bits are set to `true`
    /// @return `true` if any of the bits are set to `true`, otherwise `false`
    /// @complexity `O(size())`
    bool any() const noexcept
    {
        return word_type(0) != std::accumulate(arr_, arr_+data_length_,
            word_type(0), std::bit_or<word_type>{});
    }

    /// @details checks if any bits in a range are set to `true`
    /// @param first,last the range of elements under consideration
    /// @tparam Iterator an iterator type
    /// @return `true` if any of the bits in the given range are set to
    ///         `true`, otherwise `false`
    /// @complexity `O(last-first)`
    template <typename Iterator>
    bool any(Iterator first, Iterator last) const noexcept
    {
        word_type init = (*first.word_ptr_ & ~(first.mask_-1)) |
            (*last.word_ptr_ & (last.mask_-1));
        return word_type(0) != std::accumulate(first.word_ptr_+1,
            last.word_ptr_, init, std::bit_or<word_type>{});
    }

    /// @details checks if none of the bits are set to `true`
    /// @return `true` if none of the bits are set to `true`, otherwise `false`
    /// @complexity `O(size())`
    bool none() const noexcept
    {
        return !any();
    }

    /// @details checks if none bits in a range are set to `true`
    /// @param first,last the range of elements under consideration
    /// @tparam Iterator an iterator type
    /// @return `true` if none of the bits in the given range are set to
    ///         `true`, otherwise `false`
    /// @complexity `O(last-first)`
    template <typename Iterator>
    bool none(Iterator first, Iterator last) const noexcept
    {
        return !any(first, last);
    }
    /// @}

    /// @brief returns the number of bits set to `true`
    /// @{
    /// @details counts the number of bits that are set to `true`
    /// @returns the number of bits set to `true`
    /// @complexity `O(size())`
    size_type count() const noexcept
    {
        return std::accumulate(arr_, arr_+data_length_, size_type(0),
            [](word_type lhs, word_type rhs)
            {
                return lhs + psnip_builtin_popcount64(rhs);
            });
    }
    /// @details counts the number of bits in a range that are set to `true`
    /// @param first,last the range of elements under consideration
    /// @tparam Iterator an iterator type
    /// @return the number of bits in the given range that are set to `true`
    /// @complexity `O(last-first)`
    template <typename Iterator>
    size_type count(Iterator begin, Iterator end) const noexcept
    {
        return std::accumulate(begin.word_ptr_, end.word_ptr_,
            size_type(psnip_builtin_popcount64(*end.word_ptr_ & (end.mask_-1))
              - psnip_builtin_popcount64(*begin.word_ptr_ & (begin.mask_-1))),
            [](word_type lhs, word_type rhs)
            {
                return lhs + psnip_builtin_popcount64(rhs);
            });
    }
    /// @}

    /// @brief returns the parity of all stored bits
    /// @{
    /// @details counts the parity of all stored bits
    /// @returns the parity of all stored bits
    /// @complexity `O(size())`
    size_type parity() const noexcept
    {
        auto x = std::accumulate(arr_, arr_+data_length_, word_type(0),
            std::bit_xor<word_type>{});
        return psnip_builtin_parity64(x);
    }

    /// @details counts the parity of bits in a range
    /// @param first,last the range of elements under consideration
    /// @tparam Iterator an iterator type
    /// @return the parity of all bits in the given range
    /// @complexity `O(last-first)`
    template <typename Iterator>
    size_type parity(Iterator begin, Iterator end) const noexcept
    {
        auto x = std::accumulate(begin.word_ptr_, end.word_ptr_,
            word_type((*begin.word_ptr_ & (begin.mask_-1))
                ^ (*end.word_ptr_ & (end.mask_-1))),
            std::bit_xor<word_type>{});
        return psnip_builtin_parity64(x);
    }
    /// @}

    /// @brief returns the number of bits
    HEDLEY_PURE
    HEDLEY_ALWAYS_INLINE
    /// @returns number of bits that the `bit_array_base` holds
    /// @complexity `O(1)`
    constexpr size_type size() const noexcept
    {
        return num_bits_;
    }

    /// @brief sets bits to `true` or given value
    /// @{
    /// @brief sets all bits to `true`
    /// @complexity `O(size())`
    void set() noexcept
    {
        std::fill(arr_, arr_+data_length_, ~word_type(0));
    }
    /// @brief sets the bit at position `pos` to the value `value`
    /// @param pos the 0-based position of the bit to set (least significant
    ///            to most significant)
    /// @param value the value to set the bit to
    /// @throws std::out_of_range if `pos` does not correspond to a valid
    ///         position within the `bit_array_base`
    /// @complexity `O(1)`
    constexpr void set(size_type pos, bool value = true)
    {
        at(pos) = value;
    }

    /// @brief sets the bit at position `pos` to the value `value`
    /// @param pos the 0-based position of the bit to set (least significant
    ///            to most significant)
    /// @param value the value to set the bit to
    /// @throws std::out_of_range if `pos` does not correspond to a valid
    ///         position within the `bit_array_base`
    /// @complexity `O(1)`
    constexpr void unchecked_set(size_type pos, bool value = true)
    {
        this->operator[](pos) = value;
    }
    /// @}

    /// @brief sets bits to `false`
    /// @{
    /// @brief sets all bits to `false'
    /// @complexity `O(size())`
    void unset() noexcept
    {
        std::fill(arr_, arr_+data_length_, word_type(0));
    }
    /// @brief sets the bit at position `pos` to `false`
    /// @param pos the 0-based position of the bit to unset (least
    ///            significant to most significant)
    /// @throws std::out_of_range if `pos` does not correspond to a valid
    ///         position within the `bit_array_base`
    /// @complexity `O(1)`
    constexpr void unset(size_type pos)
    {
        at(pos) = dpf::bit::zero;
    }
    /// @}

    constexpr void unchecked_unset(size_type pos)
    {
        this->operator[](pos) = dpf::bit::zero;
    }

    /// @brief toggles the values of bits
    /// @{
    /// @brief flips all bits (like `operator~`, but in-place)
    /// @complexity `O(size())`
    void flip()
    {
        std::transform(arr_, arr_+data_length_, arr_,
            std::bit_not<word_type>{});
    }
    /// @brief flips the bit at the position `pos`
    /// @param pos the 0-based position of the bit to flip (least
    ///            significant to most significant)
    /// @throws std::out_of_range if `pos` does not correspond to a valid
    ///         position within the `bit_array_base`
    /// @complexity `O(1)`
    constexpr void flip(size_type pos)
    {
        at(pos).flip();
    }
    /// @}

    /// @brief returns a string representation of the data
    /// @details converts the contents of the `bit_array_base` to a string. Uses
    ///          `zero` to represent bits with value `false` and `one` to
    ///          represent bits with value `true`. The resulting string
    ///          contains `size()` characters with the first character
    ///          corresponding to the last `(size()-1th)` bit and the last
    ///          character corresponding tot he first `(0th)` bit.
    /// @param zero character to use to represent `false`/`0` (default: ``CharT{'0'}``)
    /// @param one character to use to represent `true`/`1` (default: ``CharT{'1'}``)
    /// @returns the converted string
    /// @throws May throw `std::bad_alloc` from the `std::string` constructor.
    /// @complexity `O(size())`
    template <class CharT = char,
              class Traits = std::char_traits<CharT>,
              class Allocator = std::allocator<CharT>>
    std::basic_string<CharT, Traits, Allocator> to_string(
        CharT zero = CharT{'0'},
        CharT one = CharT{'1'}) const
    {
        std::basic_stringstream<CharT, Traits, Allocator> ss{};
        for (const_reference b : *this) ss << (b ? one : zero);
        return ss.str();
    }

    /// @brief proxy class representing a reference to a bit
    /// @details This class is used as a proxy object to allow users to
    ///          interact with individual bits of a ``bit_array_base`, since
    ///          standard C++ types have insufficient precision to specify
    ///          individual bits.
    /// @note The primary use of ``bit_array_base::bit_reference`` is to
    ///       provide an l-value that can be returned from
    ///       `bit_array_base::operator[]`.
    class bit_reference
    {
    public:
        using word_type = bit_array_base::word_type;
        using word_pointer = bit_array_base::word_pointer;

        /// @brief (deleted) default c'tor
        inline bit_reference() = delete;
        /// @brief copy c'tor
        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr bit_reference(const bit_reference &) noexcept = default;
        /// @brief move c'tor
        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr bit_reference(bit_reference &&) noexcept = default;

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bit_reference & operator=(bool b) noexcept
        {
            this->assign(b);
            return *this;
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bit_reference & operator=(const bit_reference & b) noexcept
        {
            this->assign(static_cast<bool>(b));
            return *this;
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bit_reference & operator=(bit_reference && b) noexcept
        {
            this->assign(static_cast<bool>(b));
            return *this;
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr operator bool() const noexcept
        {
            assert(word_ptr_ != nullptr);
            assert(psnip_builtin_popcount64(mask_) == 1);
            return !(!(*word_ptr_ & psnip_endian_le64(mask_)));
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr operator int() const noexcept
        {
            return int{static_cast<bool>(*this)};
        }

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr operator dpf::bit() const noexcept
        {
            return dpf::bit{static_cast<bool>(*this)};
        }

        /// @brief performs binary AND, OR, XOR and NOT
        /// @{
        /// @details sets `*this` to the result of binary AND on `*this` and `b`
        /// @param b the other bit
        /// @returns `*this`
        /// @complexity `O(1)`
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bit_reference & operator&=(bool b) noexcept
        {
            if (HEDLEY_UNPREDICTABLE(!b)) *word_ptr_ &= ~this->mask_;
            return *this;
        }

        /// @details sets `*this` to the result of binary OR on `*this` and `b`
        /// @param b the other bit
        /// @returns `*this`
        /// @complexity `O(1)`
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bit_reference & operator|=(bool b) noexcept
        {
            if (HEDLEY_UNPREDICTABLE(b)) *word_ptr_ |= this->mask_;
            return *this;
        }

        /// @details sets `*this` to the result of binary XOR on `*this` and `b`
        /// @param b the other bit
        /// @returns `*this`
        /// @complexity `O(1)`
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bit_reference & operator^=(bool b) noexcept
        {
            if (HEDLEY_UNPREDICTABLE(b)) *word_ptr_ ^= this->mask_;
            return *this;
        }

        /// @details returns a temporary copy of `*this` with its value
        ///          flipped (binary NOT)
        /// @complexity `O(1)`
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr dpf::bit operator~() const noexcept
        {
            return dpf::bit{!static_cast<bool>(*this)};
        }
        /// @}

        /// @brief sets to the referenced bit to 1
        /// @returns `*this`
        /// @complexity `O(1)`
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bit_reference& set() noexcept
        {
            assert(word_ptr_ != nullptr);
            assert(psnip_builtin_popcount64(mask_) == 1);
            *word_ptr_ |= psnip_endian_le64(mask_);
            return *this;
        }

        /// @brief unsets the referenced bit to 0
        /// @returns `*this`
        /// @complexity `O(1)`
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bit_reference& unset() noexcept
        {
            assert(word_ptr_ != nullptr);
            assert(psnip_builtin_popcount64(mask_) == 1);
            *word_ptr_ &= psnip_endian_le64(~mask_);
            return *this;
        }

        /// @brief assigns `b ? 1 : 0` to the referenced bit
        /// @returns `*this`
        /// @complexity `O(1)`
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bit_reference& assign(bool b) noexcept
        {
            assert(word_ptr_ != nullptr);
            assert(psnip_builtin_popcount64(mask_) == 1);
            *word_ptr_ = b ? (*word_ptr_ | psnip_endian_le64(mask_))
                           : (*word_ptr_ & psnip_endian_le64(~mask_));
            return *this;
        }

        /// @brief flips the referenced bit
        /// @returns `*this`
        /// @complexity `O(1)`
        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr bit_reference& flip() noexcept
        {
            assert(word_ptr_ != nullptr);
            assert(psnip_builtin_popcount64(mask_) == 1);
            *word_ptr_ ^= psnip_endian_le64(mask_);
            return *this;
        }
    private:
        word_pointer word_ptr_;  //< pointer to word holding the bit.
        word_type mask_;  //< bitmask for referenced bit within `*word_ptr`.

        /// @brief private c'tor.
        /// @param word_ptr pointer to the word holding the actual bit.
        /// @param mask bitmask indicating referenced bit of `*word_ptr`.
        /// @note `word_ptr` must not be `nullptr`.
        /// @note `mask` must be `1ull << b` for some `0 <= b < 64`.
        /// @note accessible only to `dpf::bit_array_base` and its iterators.
        HEDLEY_NO_THROW
        HEDLEY_NON_NULL()
        HEDLEY_ALWAYS_INLINE
        constexpr bit_reference(word_pointer word_ptr, word_type mask) noexcept
        : word_ptr_{word_ptr},
          mask_{mask}
        {
            assert(word_ptr_ != nullptr);
            assert(psnip_builtin_popcount64(mask_) == 1);
        }

        friend class bit_array_base;            //< access to c'tor
        friend struct bit_iterator_base;   //< access to c'tor
        friend struct bit_iterator;        //< access to c'tor
        friend struct const_bit_iterator;  //< access to c'tor
    };  // class bit_array_base::bit_reference

    /// @brief a base class provided to simplify the definition of
    ///        `bit_iterator` and `const_bit_iterator`
    struct bit_iterator_base
    {
      public:
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;
        using word_type = bit_array_base::word_type;
        using word_pointer = bit_array_base::word_pointer;
        static constexpr auto bits_per_word = bit_array_base::bits_per_word;

        inline constexpr bool operator==(const bit_iterator_base & rhs) const
        {
            return (word_ptr_ == rhs.word_ptr_) && (mask_ == rhs.mask_);
        }

        inline constexpr bool operator<(const bit_iterator_base & rhs) const
        {
            return word_ptr_ < rhs.word_ptr_ ||
                (word_ptr_ == rhs.word_ptr_ && mask_ < rhs.mask_);
        }

        inline constexpr bool operator!=(const bit_iterator_base & rhs) const
        {
            return !(*this == rhs);
        }

        inline constexpr bool operator>(const bit_iterator_base & rhs) const
        {
            return rhs < *this;
        }

        inline constexpr bool operator<=(const bit_iterator_base & rhs) const
        {
            return !(rhs < *this);
        }

        inline constexpr bool operator>=(const bit_iterator_base & rhs) const
        {
            return !(*this < rhs);
        }

      protected:
        /// @brief bitmask for the least-significant bit of a word
        static constexpr word_type lsb = word_type(1);

        /// @brief bitmask for the most-significant bit of a word
        static constexpr word_type msb = ~(~word_type(0) >> 1);

        /// @brief pointer to the word containing the current iteration bit
        word_pointer word_ptr_;
        /// @brief mask into `*word_ptr_` to obtain current iteration bit
        word_type mask_;

        /// @brief constructs a `bit_iterator_base`
        /// @{
        /// @brief constructs a `bit_iterator_base`
        /// @param word_ptr pointer to the word containing the first iteration
        ///                 bit
        /// @note `word_ptr` must be dereferencable (not `nullptr`)
        HEDLEY_NO_THROW
        HEDLEY_NON_NULL()
        inline constexpr explicit bit_iterator_base(word_pointer word_ptr)
        : word_ptr_{word_ptr},
          mask_{lsb}
        {
            assert(word_ptr_ != nullptr);
        }

        /// @brief constructs a `bit_iterator_base`
        /// @param word_ptr pointer to the word containing the first iteration
        ///                 bit
        /// @param mask bitmask to obtain `*word_ptr` first iteration bit from
        ///             `word_ptr`
        /// @note `word_ptr` must be dereferencable (not `nullptr`)
        /// @note `mask` must have exactly one bit set
        HEDLEY_NO_THROW
        HEDLEY_NON_NULL()
        inline constexpr bit_iterator_base(word_pointer word_ptr,
            word_type mask)
          : word_ptr_{word_ptr},
            mask_{mask}
        {
            assert(word_ptr_ != nullptr);
            assert(psnip_builtin_popcount64(mask_) == 1);
        }
        /// @}

        /// @brief increments the iterator by one bit
        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        void constexpr increment() noexcept
        {
            if (HEDLEY_UNLIKELY(!(mask_ <<= 1)))
            {
                mask_ = lsb;
                ++word_ptr_;
            }
        }

        /// @brief decrements the iterator by one bit
        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        void constexpr decrement() noexcept
        {
            if (HEDLEY_UNLIKELY(!(mask_ >>= 1)))
            {
                mask_ = msb;
                --word_ptr_;
            }
        }

        /// @brief increments the iterator by specified number of bits
        /// @param amt the number of bits to increment by
        HEDLEY_NO_THROW
        inline constexpr void increment_by(difference_type amt) noexcept
        {
            if (HEDLEY_UNLIKELY(amt == 0)) return;

            difference_type offset = amt + psnip_builtin_ctz64(mask_);
            word_ptr_ += offset / bits_per_word;
            offset %= bits_per_word;
            if (offset < 0)
            {
                offset += bits_per_word;
                --word_ptr_;
            }
            mask_ = lsb << offset;
        }

        /// @brief decrements the iterator by specified number of bits
        /// @param amt the number of bits to decrement by
        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        void constexpr decrement_by(difference_type amt) noexcept
        {
            increment_by(-amt);
        }

        friend difference_type operator-(const bit_iterator_base &,
            const bit_iterator_base &);
    };  // class bit_array_base::bit_iterator_base

    class bit_iterator : public bit_iterator_base
    {
      public:
        using value_type = bit_array_base::value_type;
        using reference = bit_array_base::reference;
        using const_reference = bit_array_base::const_reference;
        using pointer = std::add_pointer_t<reference>;
        using iterator = bit_array_base::bit_iterator;

        /// @brief (deleted) default c'tor
        inline bit_iterator() = delete;

        inline constexpr
        bit_iterator(const bit_iterator &) noexcept = default;

        inline constexpr bit_iterator(bit_iterator &&) noexcept = default;

        HEDLEY_NO_THROW
        HEDLEY_NON_NULL()
        inline constexpr explicit bit_iterator(word_pointer word_ptr) noexcept
          : bit_iterator_base{word_ptr} { }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr
        bit_iterator & operator=(const bit_iterator &) noexcept = default;

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr
        bit_iterator & operator=(bit_iterator &&) noexcept = default;

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr reference operator*() const
        {
            return reference{word_ptr_, mask_};
        }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr iterator & operator++() noexcept
        {
            bit_iterator_base::increment();
            return *this;
        }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr iterator operator++(int) noexcept
        {
            iterator tmp = *this;
            bit_iterator_base::increment();
            return tmp;
        }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr iterator & operator--() noexcept
        {
            bit_iterator_base::decrement();
            return *this;
        }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr iterator operator--(int) noexcept
        {
            iterator tmp = *this;
            bit_iterator_base::decrement();
            return tmp;
        }

        inline constexpr iterator & operator+=(difference_type amt)
        {
            bit_iterator_base::increment_by(amt);
            return *this;
        }

        inline constexpr iterator & operator-=(difference_type amt)
        {
            bit_iterator_base::decrement_by(amt);
            return *this;
        }

        inline constexpr iterator operator+(difference_type amt) const
        {
            iterator tmp = *this;
            return tmp += amt;
        }

        inline constexpr iterator operator-(difference_type amt) const
        {
            iterator tmp = *this;
            return tmp -= amt;
        }

        inline constexpr reference operator[](difference_type i)
        {
            return *(*this + i);
        }

        inline constexpr const_reference operator[](difference_type i) const
        {
            return *(*this + i);
        }

        friend class bit_array_base;
        friend class bit_array_base::const_bit_iterator;
    };  // class bit_array_base::bit_iterator

    struct const_bit_iterator : public bit_iterator_base
    {
      public:
        using value_type = bit_array_base::value_type;
        using reference = bit_array_base::const_reference;
        using const_reference = bit_array_base::const_reference;
        using pointer = std::add_pointer_t<reference>;
        using iterator = bit_array_base::const_bit_iterator;
        using const_iterator = bit_array_base::const_bit_iterator;

        /// @brief (deleted) default c'tor
        inline const_bit_iterator() = delete;
        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr const_bit_iterator(const const_bit_iterator &) noexcept
            = default;
        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr const_bit_iterator(const_bit_iterator &&) noexcept
            = default;

        HEDLEY_NO_THROW
        HEDLEY_NON_NULL()
        inline constexpr explicit const_bit_iterator(word_pointer word_ptr)
          : bit_iterator_base{word_ptr} { }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr
        const_bit_iterator & operator=(const const_bit_iterator &) noexcept
            = default;

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr
        const_bit_iterator & operator=(const_bit_iterator &&) noexcept
            = default;

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr
        explicit const_bit_iterator(const bit_iterator & to_copy) noexcept
          : bit_iterator_base{to_copy.word_ptr_, to_copy.mask_} {}

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr
        explicit const_bit_iterator(bit_iterator && to_copy) noexcept
          : bit_iterator_base{to_copy.word_ptr_, to_copy.mask_} {}

        HEDLEY_NO_THROW
        HEDLEY_ALWAYS_INLINE
        constexpr const_reference operator*() const
        {
            return bit_reference{word_ptr_, mask_};
        }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr const_iterator & operator++() noexcept
        {
            bit_iterator_base::increment();
            return *this;
        }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr const_iterator operator++(int) noexcept
        {
            const_iterator tmp = *this;
            bit_iterator_base::increment();
            return tmp;
        }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr const_iterator & operator--() noexcept
        {
            bit_iterator_base::decrement();
            return *this;
        }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        constexpr const_iterator operator--(int) noexcept
        {
            const_iterator tmp = *this;
            bit_iterator_base::decrement();
            return tmp;
        }

        inline constexpr
        const_iterator & operator+=(difference_type amt) noexcept
        {
            bit_iterator_base::increment_by(amt);
            return *this;
        }

        inline constexpr
        const_iterator & operator-=(difference_type amt) noexcept
        {
            bit_iterator_base::decrement_by(amt);
            return *this;
        }

        inline constexpr
        const_iterator operator+(difference_type amt) const noexcept
        {
            const_iterator tmp = *this;
            return tmp += amt;
        }

        inline constexpr
        const_iterator operator-(difference_type amt) const noexcept
        {
            const_iterator tmp = *this;
            return tmp -= amt;
        }

        inline constexpr
        const_reference operator[](difference_type i) const noexcept
        {
            return *(*this + i);
        }

        friend class bit_array_base;
    };  // class bit_array_base::const_bit_iterator

  protected:
    /// @brief constructs a `bit_array_base` that holds `num_bits` bits
    inline constexpr bit_array_base(size_type num_bits, word_pointer arr)
      : num_bits_{num_bits},
        data_length_{utils::quotient_ceiling(num_bits, bits_per_word)},
        arr_{arr}
    {
        // arr_[data_length_] = sentinel;
    }

  protected:
    /// @brief an all-`1`s sentinel word marking the end of the data
    /// @note `sentinel` exists to assist `setbit_index_iterator` in deciding
    ///       if it has hit the end of the data array
    static constexpr word_type sentinel = ~word_type(0);

    /// @brief the number of bits represented by the `bit_array_base`
    size_type num_bits_;
    /// @brief the number of `word_type`s are being used to represent the
    ///        `num_bits_` bits
    size_type data_length_;

    /// @brief the array
    word_pointer arr_;
};  // class bit_array_base

inline bit_array_base::bit_iterator_base::difference_type operator-(
    const bit_array_base::bit_iterator_base & lhs,
    const bit_array_base::bit_iterator_base & rhs)
{
    constexpr auto bits_per_word
        = bit_array_base::bit_iterator_base::bits_per_word;
    return bits_per_word*(lhs.word_ptr_ - rhs.word_ptr_)
        + (psnip_builtin_ctz64(lhs.mask_) - psnip_builtin_ctz64(rhs.mask_));
}

template <std::size_t Nbits>
class alignas(utils::max_align_v) static_bit_array final : public bit_array_base
{
    static constexpr std::size_t length_in_words
        = utils::quotient_ceiling(Nbits + bits_per_word, bits_per_word);
  public:
    constexpr static_bit_array(static_bit_array &&) = default;
    constexpr static_bit_array(const static_bit_array &) = default;
    /// @brief constructs a `static_bit_array` that holds `num_bits` bits
    inline constexpr static_bit_array()
      : bit_array_base{Nbits, &arr[0]} { arr_[data_length_] = sentinel; }
    inline constexpr static_bit_array(std::size_t val)
      : bit_array_base{Nbits, &arr[0]}
    {
        arr_[0] = val;
        arr_[data_length_] = sentinel;
    }

  private:
    std::array<word_type, length_in_words> arr;
};

class dynamic_bit_array final : public bit_array_base
{
  public:
    constexpr dynamic_bit_array(dynamic_bit_array && other)
        : bit_array_base(other)
    {
        other.arr_ = nullptr;
    }
    constexpr dynamic_bit_array(const dynamic_bit_array &) = default;
    /// @brief constructs a `dynamic_bit_array` that holds `num_bits` bits
    /// @throws `std::bad_alloc` if allocating storage fails
    inline explicit dynamic_bit_array(std::size_t nbits)
      : bit_array_base{nbits, static_cast<word_pointer>(
            std::aligned_alloc(utils::max_align_v, sizeof(word_type) *
                utils::quotient_ceiling(nbits+bits_per_word, bits_per_word)))
            }
    {
        if (HEDLEY_UNLIKELY(arr_ == nullptr)) throw std::bad_alloc{};
        arr_[data_length_] = sentinel;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    ~dynamic_bit_array() noexcept
    {
        if (arr_ != nullptr) free(arr_);
    }
};

/// @brief
inline constexpr void swap(bit_array_base::bit_reference lhs,
    bit_array_base::bit_reference rhs) noexcept
{
    bool tmp = lhs;
    lhs = rhs;
    rhs = tmp;
}

std::ostream & operator<<(std::ostream & os, bit_array_base::bit_reference bit)
{
    return os << dpf::to_string(bit);
}

}  // namespace dpf

namespace std
{

template <>
struct iterator_traits<dpf::bit_array_base::iterator>
{
  private:
    using type = dpf::bit_array_base::iterator;
  public:
    using iterator_category = type::iterator_category;
    using difference_type = type::difference_type;
    using value_type = type::value_type;
    using reference = type::reference;
    using const_reference = type::const_reference;
    using pointer = type::pointer;
};

template <>
struct iterator_traits<dpf::bit_array_base::const_iterator>
{
  private:
    using type = dpf::bit_array_base::const_iterator;
  public:
    using iterator_category = type::iterator_category;
    using difference_type = type::difference_type;
    using value_type = type::value_type;
    using reference = type::reference;
    using const_reference = type::const_reference;
    using pointer = type::pointer;
};

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_BIT_ARRAY_HPP__
