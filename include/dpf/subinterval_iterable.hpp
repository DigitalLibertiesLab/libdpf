/// @file dpf/subinterval_iterable.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief defines `dpf::subinterval_iterable` and associated helpers
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_SUBINTERVAL_ITERABLE_HPP__
#define LIBDPF_INCLUDE_DPF_SUBINTERVAL_ITERABLE_HPP__

namespace dpf
{

template <typename OutputT>
class subinterval_iterable
{
  public:
    using output_type = OutputT;
    using iterator = output_type *;
    using const_iterator = const output_type *;
    using size_type = std::size_t;

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    explicit subinterval_iterable(const output_type * cont, std::size_t length, std::size_t preclip, std::size_t postclip)
      : cont_{cont}, length_{length}, preclip_{preclip}, postclip_{postclip}
    { }

    HEDLEY_ALWAYS_INLINE
    const_iterator begin() const noexcept
    {
        return cont_ + preclip_;
    }
    HEDLEY_ALWAYS_INLINE
    const_iterator cbegin() const noexcept
    {
        return cont_ + preclip_;
    }
    HEDLEY_ALWAYS_INLINE
    const_iterator end() const noexcept
    {
        return cont_ + length_ - postclip_ + 1;
    }
    HEDLEY_ALWAYS_INLINE
    const_iterator cend() const noexcept
    {
        return cont_ + length_ - postclip_ + 1;
    }

  private:
    output_type * cont_;
    std::size_t length_;
    std::size_t preclip_;
    std::size_t postclip_;
};  // class dpf::subinterval_iterable

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_SUBINTERVAL_ITERABLE_HPP__
