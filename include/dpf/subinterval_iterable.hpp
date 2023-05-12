/// @file dpf/subinterval_iterable.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief defines `dpf::subinterval_iterable` and associated helpers
/// @copyright Copyright (c) 2019-2023 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_SUBINTERVAL_ITERABLE_HPP__
#define LIBDPF_INCLUDE_DPF_SUBINTERVAL_ITERABLE_HPP__

namespace dpf
{

template <typename IteratorT>
class subinterval_iterable
{
  public:
    using iterator = IteratorT;
    using size_type = std::size_t;

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    explicit subinterval_iterable(iterator it, size_type length, size_type preclip, size_type postclip)
      : it_{it}, length_{length}, preclip_{preclip}, postclip_{postclip}
    { }

    HEDLEY_ALWAYS_INLINE
    iterator begin() const noexcept
    {
        return it_ + preclip_;
    }
    HEDLEY_ALWAYS_INLINE
    iterator cbegin() const noexcept
    {
        return it_ + preclip_;
    }
    HEDLEY_ALWAYS_INLINE
    iterator end() const noexcept
    {
        return it_ + (length_ - postclip_ + 1);
    }
    HEDLEY_ALWAYS_INLINE
    iterator cend() const noexcept
    {
        return it_ + (length_ - postclip_ + 1);
    }

  private:
    iterator it_;
    size_type length_;
    size_type preclip_;
    size_type postclip_;
};  // class dpf::subinterval_iterable

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_SUBINTERVAL_ITERABLE_HPP__
