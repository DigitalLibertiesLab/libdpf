/// @file dpf/output_buffer.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_OUTPUT_BUFFER_HPP__
#define LIBDPF_INCLUDE_DPF_OUTPUT_BUFFER_HPP__

#include <algorithm>
#include <vector>

#include "dpf/aligned_allocator.hpp"
#include "dpf/leaf_node.hpp"
#include "dpf/utils.hpp"
#include "dpf/bit.hpp"
#include "dpf/bit_array.hpp"

namespace dpf
{

template <typename T>
class output_buffer final
  : private std::vector<T, dpf::aligned_allocator<T>>
{
  private:
    using vector = std::vector<T, dpf::aligned_allocator<T>>;
  public:
    using value_type = typename vector::value_type;
    using iterator = typename vector::iterator;
    using const_iterator = typename vector::const_iterator;
    using size_type = typename vector::size_type;
    output_buffer() = default;
    explicit output_buffer(size_type size) : vector(size) { }
    output_buffer(output_buffer &&) = default;
    output_buffer(const output_buffer &) = delete;

    // "selectively public" inheritance
    using vector::at;
    using vector::operator[];
    using vector::data;
    using vector::begin;
    using vector::cbegin;
    using vector::end;
    using vector::cend;
    using vector::size;
};

template <>
using output_buffer<dpf::bit> = dpf::dynamic_bit_array;
// class output_buffer<dpf::bit> : public dpf::dynamic_bit_array
// {
//   public:
//     explicit output_buffer(size_type size) : dynamic_bit_array(size) { }
//     output_buffer(output_buffer &&) = default;
//     output_buffer(const output_buffer &) = delete;
// };

template <class Container>
class clipped_iterable
{
  public:
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;
    using size_type = typename Container::size_type;

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    explicit clipped_iterable(Container * c, std::size_t preclip, std::size_t postclip)
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
};  // class dpf::clipped_iterable

template <typename DpfKey,
          typename InputT>
auto make_output_buffer_for_interval(const DpfKey &, InputT from, InputT to)
{
    using output_t = std::tuple_element_t<0, typename DpfKey::outputs_t>;

    std::size_t from_node = utils::quotient_floor(from, (InputT)DpfKey::outputs_per_leaf), to_node = utils::quotient_ceiling(to, (InputT)DpfKey::outputs_per_leaf);
    std::size_t nodes_in_interval = to_node - from_node;

    return dpf::output_buffer<output_t>(nodes_in_interval*DpfKey::outputs_per_leaf);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_OUTPUT_BUFFER_HPP__
