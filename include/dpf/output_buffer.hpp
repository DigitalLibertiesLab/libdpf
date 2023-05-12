/// @file dpf/output_buffer.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

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

template <typename T,
          std::size_t Alignment = utils::max_align_v>
class output_buffer final
  : private std::vector<T, dpf::aligned_allocator<T, Alignment>>
{
  private:
    using vector = std::vector<T, dpf::aligned_allocator<T, Alignment>>;
  public:
    using value_type = typename vector::value_type;
    using iterator = typename vector::iterator;
    using const_iterator = typename vector::const_iterator;
    using size_type = typename vector::size_type;
    output_buffer() = default;
    explicit output_buffer(size_type size) : vector(size) { }
    output_buffer(output_buffer &&) noexcept = default;
    output_buffer(const output_buffer &) = delete;
    output_buffer & operator=(output_buffer &&) noexcept = default;
    output_buffer & operator=(const output_buffer &) = default;
    ~output_buffer() = default;

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
    output_buffer(output_buffer &&) noexcept = default;
    output_buffer(const output_buffer &) = delete;
    output_buffer & operator=(output_buffer &&) noexcept = default;
    output_buffer & operator=(const output_buffer &) = delete;
    ~output_buffer() = default;
};

template <typename DpfKey,
          std::size_t I = 0,
          typename InputT>
auto make_output_buffer_for_interval(InputT from, InputT to)
{
    using dpf_type = DpfKey;
    using output_type = typename DpfKey::concrete_output_type<I>;

    std::size_t nodes_in_interval = utils::get_nodes_in_interval<dpf_type>(from, to);
    return dpf::output_buffer<output_type>(nodes_in_interval*dpf_type::outputs_per_leaf);
}

template <typename DpfKey,
          std::size_t I0,
          std::size_t I1,
          std::size_t ...Is,
          typename InputT>
auto make_output_buffer_for_interval(InputT from, InputT to)
{
    return std::make_tuple(
        make_output_buffer_for_interval<DpfKey, I0>(from, to),
        make_output_buffer_for_interval<DpfKey, I1>(from, to),
        make_output_buffer_for_interval<DpfKey, Is>(from, to)...
    );
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT>
inline auto make_output_buffer_for_interval(const DpfKey &, InputT from, InputT to)
{
    return make_output_buffer_for_interval<DpfKey, I>(from, to);
}

template <std::size_t I0,
          std::size_t I1,
          std::size_t ...Is,
          typename DpfKey,
          typename InputT>
inline auto make_output_buffer_for_interval(const DpfKey &, InputT from, InputT to)
{
    return make_output_buffer_for_interval<DpfKey, I0, I1, Is...>(from, to);
}

template <typename DpfKey,
          std::size_t I = 0>
auto make_output_buffer_for_full()
{
    using dpf_type = DpfKey;
    using input_type = typename DpfKey::input_type;

    return make_output_buffer_for_interval<DpfKey, I>(
        std::numeric_limits<input_type>::min(),
        std::numeric_limits<input_type>::max());
}

template <typename DpfKey,
          std::size_t I0,
          std::size_t I1,
          std::size_t ...Is>
auto make_output_buffer_for_full()
{
    return std::make_tuple(
        make_output_buffer_for_full<DpfKey, I0>(),
        make_output_buffer_for_full<DpfKey, I1>(),
        make_output_buffer_for_full<DpfKey, Is>()...
    );
}

template <std::size_t I = 0,
          typename DpfKey>
inline auto make_output_buffer_for_full(const DpfKey &)
{
    return make_output_buffer_for_full<DpfKey, I>();
}

template <std::size_t I0,
          std::size_t I1,
          std::size_t ...Is,
          typename DpfKey>
inline auto make_output_buffer_for_full(const DpfKey &)
{
    return make_output_buffer_for_full<DpfKey, I0, I1, Is...>();
}

template <typename DpfKey,
          std::size_t I = 0,
          typename Iterator>
auto make_output_buffer_for_subsequence(Iterator begin, Iterator end)
{
    using dpf_type = DpfKey;
    using output_type = typename DpfKey::concrete_output_type<I>;
    std::size_t nodes_in_sequence = std::distance(begin, end);

    return output_buffer<output_type>(nodes_in_sequence*dpf_type::outputs_per_leaf);
}

template <typename DpfKey,
          std::size_t I0,
          std::size_t I1,
          std::size_t ...Is,
          typename Iterator>
auto make_output_buffer_for_subsequence(Iterator begin, Iterator end)
{
    return std::make_tuple(
        make_output_buffer_for_subsequence<DpfKey, I0>(begin, end),
        make_output_buffer_for_subsequence<DpfKey, I1>(begin, end),
        make_output_buffer_for_subsequence<DpfKey, Is>(begin, end)...);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename Iterator>
inline auto make_output_buffer_for_subsequence(const DpfKey &, Iterator begin, Iterator end)
{
    return make_output_buffer_for_subsequence<DpfKey, I>(begin, end);
}

template <std::size_t I0,
          std::size_t I1,
          std::size_t ...Is,
          typename DpfKey,
          typename Iterator>
inline auto make_output_buffer_for_subsequence(const DpfKey &, Iterator begin, Iterator end)
{
    return make_output_buffer_for_subsequence<DpfKey, I0, I1, Is...>(begin, end);
}

template <typename DpfKey,
          std::size_t I = 0>
auto make_output_buffer_for_recipe_subsequence(const sequence_recipe & recipe)
{
    using dpf_type = DpfKey;
    using output_type = typename DpfKey::concrete_output_type<I>;

    std::size_t nodes_in_sequence = recipe.num_leaf_nodes();

    return dpf::output_buffer<output_type>(nodes_in_sequence*dpf_type::outputs_per_leaf);
}

template <typename DpfKey,
          std::size_t I0,
          std::size_t I1,
          std::size_t ...Is>
inline auto make_output_buffer_for_recipe_subsequence(const sequence_recipe & recipe)
{
    return std::make_tuple(
        make_output_buffer_for_recipe_subsequence<DpfKey, I0>(recipe),
        make_output_buffer_for_recipe_subsequence<DpfKey, I1>(recipe),
        make_output_buffer_for_recipe_subsequence<DpfKey, Is>(recipe)...);
}

template <std::size_t I = 0,
          typename DpfKey>
inline auto make_output_buffer_for_recipe_subsequence(const DpfKey &, const sequence_recipe & recipe)
{
    return make_output_buffer_for_recipe_subsequence<DpfKey, I>(recipe);
}

template <std::size_t I0,
          std::size_t I1,
          std::size_t ...Is,
          typename DpfKey>
inline auto make_output_buffer_for_recipe_subsequence(const DpfKey &, const sequence_recipe & recipe)
{
    return make_output_buffer_for_recipe_subsequence<DpfKey, I0, I1, Is...>(recipe);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_OUTPUT_BUFFER_HPP__
