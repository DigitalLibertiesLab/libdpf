/// @file dpf/output_buffer.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_OUTPUT_BUFFER_HPP__
#define LIBDPF_INCLUDE_DPF_OUTPUT_BUFFER_HPP__

#include <algorithm>
#include <vector>

#include "dpf/aligned_allocator.hpp"
#include "dpf/leaf_node.hpp"
#include "dpf/utils.hpp"
#include "dpf/bit.hpp"
#include "dpf/bit_array.hpp"
#include "dpf/sequence_recipe.hpp"

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
class output_buffer<dpf::bit> : public dpf::dynamic_bit_array
{
  public:
    explicit output_buffer(size_type size) : dynamic_bit_array(size) { }
    output_buffer(output_buffer &&) = default;
    output_buffer(const output_buffer &) = delete;
};

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT>
auto make_output_buffer_for_interval(const DpfKey &, InputT from, InputT to)
{
    using dpf_type = DpfKey;
    using input_type = InputT;
    using output_type = std::tuple_element_t<I, typename DpfKey::outputs_t>;

    std::size_t from_node = utils::quotient_floor(from, (input_type)dpf_type::outputs_per_leaf),
        to_node = utils::quotient_ceiling((input_type)(to+1), (input_type)dpf_type::outputs_per_leaf);
    std::size_t nodes_in_interval = to_node - from_node;

    return dpf::output_buffer<output_type>(nodes_in_interval*dpf_type::outputs_per_leaf);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename Iterator>
auto make_output_buffer_for_subsequence(const DpfKey &, Iterator begin, Iterator end)
{
    using dpf_type = DpfKey;
    using output_type = std::tuple_element_t<I, typename DpfKey::outputs_t>;
    std::size_t nodes_in_sequence = std::distance(begin, end);

    return output_buffer<output_type>(nodes_in_sequence*dpf_type::outputs_per_leaf);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT>
auto make_output_buffer_for_recipe_subsequence(const DpfKey &, const sequence_recipe<InputT> & recipe)
{
    using dpf_type = DpfKey;
    using output_type = std::tuple_element_t<I, typename DpfKey::outputs_t>;

    std::size_t nodes_in_sequence = recipe.num_leaf_nodes;

    return dpf::output_buffer<output_type>(nodes_in_sequence*dpf_type::outputs_per_leaf);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_OUTPUT_BUFFER_HPP__
