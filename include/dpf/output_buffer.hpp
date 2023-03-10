/// @file dpf/buffer.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_BUFFER_HPP__
#define LIBDPF_INCLUDE_DPF_BUFFER_HPP__

#include <vector>

#include "dpf/aligned_allocator.hpp"
#include "dpf/leaf_node.hpp"
#include "dpf/utils.hpp"
#include "dpf/bit.hpp"
#include "dpf/bit_array.hpp"

namespace dpf
{

template <typename T>
class output_buffer
  : private std::vector<T, dpf::detail::aligned_allocator<T, utils::max_align_v>>
{
  private:
    using vector = std::vector<T, dpf::detail::aligned_allocator<T, utils::max_align_v>>;
  public:
    using value_type = typename vector::value_type;
    using iterator = typename vector::iterator;
    using const_iterator = typename vector::const_iterator;
    using size_type = typename vector::size_type;
    output_buffer() = default;
    output_buffer(size_type size) : vector(size) { };
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
class output_buffer<dpf::bit> : public dpf::dynamic_bit_array
{
  public:
    output_buffer(size_type size) : dynamic_bit_array(size) { };
    output_buffer(output_buffer &&) = default;
    output_buffer(const output_buffer &) = delete;
};

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
using output_buffer128 = output_buffer<simde__m128i>;
using output_buffer256 = output_buffer<simde__m256i>;
HEDLEY_PRAGMA(GCC diagnostic pop)

template <class Container>
class clipped_iterable
{
  public:
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;
    using size_type = typename Container::size_type;

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    explicit clipped_iterable(Container & c, std::size_t preclip, std::size_t postclip)
      : cont_{c}, preclip_{preclip}, postclip_{postclip} { }

    HEDLEY_ALWAYS_INLINE
    iterator begin() noexcept
    {
        return std::begin(cont_)+preclip_;
    }
    HEDLEY_ALWAYS_INLINE
    const_iterator begin() const noexcept
    {
        return std::begin(cont_)+preclip_;
    }
    HEDLEY_ALWAYS_INLINE
    const_iterator cbegin() const noexcept
    {
        return std::cbegin(cont_)+preclip_;
    }
    HEDLEY_ALWAYS_INLINE
    iterator end() noexcept
    {
        return std::end(cont_)-postclip_;
    }
    HEDLEY_ALWAYS_INLINE
    const_iterator end() const noexcept
    {
        return std::end(cont_)-postclip_;
    }
    HEDLEY_ALWAYS_INLINE
    const_iterator cend() const noexcept
    {
        return std::cend(cont_)-postclip_;
    }

  private:
    Container & cont_;
    std::size_t preclip_, postclip_;
};  // class dpf::clipped_iterable

template <typename DpfKey,
          typename InputT>
auto make_output_buffer_for_interval(const DpfKey &, InputT from, InputT to)
{
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using exterior_node_t = typename DpfKey::exterior_node_t;
    using output_t = std::tuple_element_t<0, typename DpfKey::outputs_t>;

    static constexpr auto outputs_per_leaf = outputs_per_leaf_v<output_t, exterior_node_t>;
    std::size_t from_node = utils::quotient_floor(from, (InputT)outputs_per_leaf), to_node = utils::quotient_ceiling(to, (InputT)outputs_per_leaf);
    std::size_t nodes_in_interval = std::max(std::size_t(0), std::size_t(to_node - from_node));
HEDLEY_PRAGMA(GCC diagnostic pop)

    return dpf::output_buffer<output_t>(nodes_in_interval*outputs_per_leaf);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_BUFFER_HPP__
