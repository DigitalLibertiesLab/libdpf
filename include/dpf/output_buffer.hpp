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
class output_buffer<dpf::bit> : public dpf::dynamic_bit_array {};

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
using output_buffer128 = output_buffer<simde__m128i>;
using output_buffer256 = output_buffer<simde__m256i>;
HEDLEY_PRAGMA(GCC diagnostic pop)

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_BUFFER_HPP__
