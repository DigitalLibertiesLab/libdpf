/// @file dpf/subinterval_iterable.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief defines `dpf::subinterval_iterable` and associated helpers
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_SUBINTERVAL_ITERABLE_HPP__
#define LIBDPF_INCLUDE_DPF_SUBINTERVAL_ITERABLE_HPP__

template <class Container>
class subinterval_iterable
{
  public:
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;
    using size_type = typename Container::size_type;

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    explicit subinterval_iterable(Container * c, std::size_t preclip, std::size_t postclip)
      : cont_{c}, preclip_{preclip}, postclip_{postclip} { }

    HEDLEY_ALWAYS_INLINE
    iterator begin() noexcept
    {
        return std::begin(*cont_)+preclip_;
    }
    HEDLEY_ALWAYS_INLINE
    const_iterator begin() const noexcept
    {
        return std::begin(*cont_)+preclip_;
    }
    HEDLEY_ALWAYS_INLINE
    const_iterator cbegin() const noexcept
    {
        return std::cbegin(*cont_)+preclip_;
    }
    HEDLEY_ALWAYS_INLINE
    iterator end() noexcept
    {
        return std::end(*cont_)-postclip_;
    }
    HEDLEY_ALWAYS_INLINE
    const_iterator end() const noexcept
    {
        return std::end(*cont_)-postclip_;
    }
    HEDLEY_ALWAYS_INLINE
    const_iterator cend() const noexcept
    {
        return std::cend(*cont_)-postclip_;
    }

  private:
    Container * cont_;
    std::size_t preclip_, postclip_;
};  // class dpf::subinterval_iterable

#endif  // LIBDPF_INCLUDE_DPF_SUBINTERVAL_ITERABLE_HPP__
