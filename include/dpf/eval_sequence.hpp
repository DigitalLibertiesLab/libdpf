/// @file dpf/eval_sequence.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_EVAL_SEQUENCE_HPP__
#define LIBDPF_INCLUDE_DPF_EVAL_SEQUENCE_HPP__

#include <functional>
#include <memory>
#include <limits>
#include <tuple>
#include <algorithm>
#include <set>

#include <portable-snippets/builtin/builtin.h>
#include <hedley/hedley.h>

#include "dpf/dpf_key.hpp"
#include "dpf/eval_point.hpp"
#include "dpf/path_memoizer.hpp"
#include "dpf/sequence_memoizer.hpp"
#include "dpf/subsequence_iterable.hpp"
#include "dpf/subinterval_iterable.hpp"

namespace dpf
{

struct return_entire_node_tag_ final {};
// static constexpr auto return_entire_node_tag = return_entire_node_tag_{};

struct return_output_only_tag_ final {};
// static constexpr auto return_output_only_tag = return_output_only_tag_{};

namespace internal
{

template <std::size_t I,
        typename DpfKey,
        typename Iterator,
        typename OutputBuffer>
HEDLEY_ALWAYS_INLINE
DPF_UNROLL_LOOPS
auto eval_sequence_entire_node(const DpfKey & dpf, Iterator begin, Iterator end, OutputBuffer & outbuf)
{
    auto path = make_basic_path_memoizer(dpf);
    using output_type = std::tuple_element_t<I, typename DpfKey::outputs_tuple>;
    using raw_type = std::tuple_element_t<I, typename DpfKey::leaf_tuple>;
    auto rawbuf = reinterpret_cast<raw_type*>(utils::data(outbuf));
    std::size_t i = 0;
    for (auto it = begin; it != end; ++it)
    {
        rawbuf[i++] = internal::eval_point<I>(dpf, *it, path);
    }
    return subsequence_iterable<DpfKey, output_type, Iterator>(utils::data(outbuf), begin, end);
}

template <std::size_t I,
        typename DpfKey,
        typename Iterator,
        typename OutputBuffer>
HEDLEY_ALWAYS_INLINE
DPF_UNROLL_LOOPS
auto eval_sequence_output_only(const DpfKey & dpf, Iterator begin, Iterator end, OutputBuffer & outbuf)
{
    auto path = make_basic_path_memoizer(dpf);
    using output_type = std::tuple_element_t<I, typename DpfKey::outputs_tuple>;
    auto rawbuf = reinterpret_cast<output_type*>(utils::data(outbuf));
    std::size_t i = 0;
    for (auto it = begin; it != end; ++it)
    {
        rawbuf[i++] = *dpf::eval_point<I>(dpf, *it, path);
    }
    return dpf::subinterval_iterable<output_type>(rawbuf, i-1, 0, 0);
}

}  // dpf::internal

template <std::size_t I = 0,
          typename ReturnType = return_entire_node_tag_,
          typename DpfKey,
          typename Iterator,
          typename OutputBuffer>
inline auto eval_sequence(const DpfKey & dpf, Iterator begin, Iterator end, OutputBuffer && outbuf)
{
    static_assert(std::is_same_v<ReturnType, return_entire_node_tag_> ||
                    std::is_same_v<ReturnType, return_output_only_tag_>);
    if constexpr (std::is_same_v<ReturnType, return_entire_node_tag_>)
    {
        return internal::eval_sequence_entire_node<I>(dpf, begin, end, outbuf);
    }
    else
    {
        return internal::eval_sequence_output_only<I>(dpf, begin, end, outbuf);
    }
}

template <std::size_t I = 0,
          typename ReturnType = return_entire_node_tag_,
          typename DpfKey,
          typename Iterator>
auto eval_sequence(const DpfKey & dpf, Iterator begin, Iterator end)
{
    auto outbuf = make_output_buffer_for_subsequence<I>(dpf, begin, end);
    auto iterable = eval_sequence<I>(dpf, begin, end, outbuf);
    return std::make_tuple(std::move(outbuf), std::move(iterable));
}

namespace internal
{

template <typename DpfKey,
          typename InputT,
          typename SequenceMemoizer>
DPF_UNROLL_LOOPS
inline auto eval_sequence_interior(const DpfKey & dpf, const sequence_recipe<InputT> & recipe,
    SequenceMemoizer & memoizer, std::size_t to_level = DpfKey::depth)
{
    using dpf_type = DpfKey;
    using node_type = typename DpfKey::interior_node;

    // level_index represents the current level being built
    // level_index = 0 => root
    // level_index = depth => last layer of interior nodes
    std::size_t level_index = memoizer.assign_dpf(dpf, recipe);
    std::size_t recipe_index = recipe.level_endpoints[level_index-1];
    std::size_t nodes_at_level = memoizer.get_nodes_at_level(level_index-1);

    for (; level_index <= to_level; level_index = memoizer.advance_level(), nodes_at_level = memoizer.get_nodes_at_level(level_index-1))
    {
        const node_type cw[2] = {
            set_lo_bit(dpf.correction_words[level_index-1], dpf.correction_advice[level_index-1]&1),
            set_lo_bit(dpf.correction_words[level_index-1], (dpf.correction_advice[level_index-1]>>1)&1)
        };

        auto prevbuf = memoizer[level_index-1];
        auto currbuf = memoizer[level_index];

        for (std::size_t input_index = 0, output_index = 0; input_index < nodes_at_level; ++input_index, ++recipe_index)
        {
            if (memoizer.traverse_first(recipe_index) == true)
            {
                bool dir = memoizer.get_direction(0);
                currbuf[output_index++] = dpf_type::traverse_interior(prevbuf[input_index], cw[dir], dir);
            }
            if (memoizer.traverse_second(recipe_index) == true)
            {
                bool dir = memoizer.get_direction(1);
                currbuf[output_index++] = dpf_type::traverse_interior(prevbuf[input_index], cw[dir], dir);
            }
        }
    }
}

template <std::size_t I,
          typename DpfKey,
          typename InputT,
          class OutputBuffer,
          typename SequenceMemoizer>
DPF_UNROLL_LOOPS
inline auto eval_sequence_exterior_entire_node(const DpfKey & dpf, const sequence_recipe<InputT> & recipe,
    OutputBuffer & outbuf, SequenceMemoizer & memoizer)
{
    assert_not_wildcard<I>(dpf);

    using dpf_type = DpfKey;

    auto nodes_in_interval = std::max(std::size_t(0), recipe.num_leaf_nodes);

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using raw_type = std::tuple_element_t<I, typename DpfKey::leaf_tuple>;
    auto rawbuf = reinterpret_cast<raw_type*>(utils::data(outbuf));
    auto cw = dpf.template exterior_cw<I>();
    auto buf = memoizer[dpf.depth];
    for (std::size_t j = 0; j < nodes_in_interval; ++j)
    {
        auto leaf = dpf_type::template traverse_exterior<I>(buf[j],
            get_if_lo_bit(cw, buf[j]));
        std::memcpy(&rawbuf[j], &leaf, sizeof(leaf));
    }
HEDLEY_PRAGMA(GCC diagnostic pop)
}

template <std::size_t I,
          typename DpfKey,
          typename InputT,
          class OutputBuffer,
          typename SequenceMemoizer>
DPF_UNROLL_LOOPS
inline auto eval_sequence_exterior_output_only(const DpfKey & dpf, const sequence_recipe<InputT> & recipe,
    OutputBuffer & outbuf, SequenceMemoizer & memoizer)
{
    assert_not_wildcard<I>(dpf);

    using dpf_type = DpfKey;
    using output_type = std::tuple_element_t<I, typename DpfKey::outputs_tuple>;

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    auto rawbuf = reinterpret_cast<output_type*>(utils::data(outbuf));
    auto cw = dpf.template exterior_cw<I>();
    using node_type = typename DpfKey::exterior_node;
    using leaf_node_type = std::tuple_element_t<I, typename DpfKey::leaf_tuple>;
    auto buf = memoizer[dpf.depth];

    leaf_node_type node;
    for (std::size_t i = 0, j = -1, prev = -1,
        curr = recipe.output_indices[i]/dpf_type::outputs_per_leaf;
        i < recipe.output_indices.size();
        prev = curr, curr = recipe.output_indices[++i]/dpf_type::outputs_per_leaf)
    {
        if (prev != curr)
        {
            ++j;
            node = dpf_type::template traverse_exterior<I>(buf[j], get_if_lo_bit(cw, buf[j]));
        }
        rawbuf[i] = extract_leaf<node_type, output_type>(node, recipe.output_indices[i] % dpf_type::outputs_per_leaf);
    }
HEDLEY_PRAGMA(GCC diagnostic pop)
}

}  // namespace internal

template <std::size_t I = 0,
          typename ReturnType = return_entire_node_tag_,
          typename DpfKey,
          typename InputT,
          class OutputBuffer,
          typename SequenceMemoizer>
auto eval_sequence(const DpfKey & dpf, const sequence_recipe<InputT> & recipe,
    OutputBuffer && outbuf, SequenceMemoizer & memoizer)
{
    static_assert(std::is_same_v<ReturnType, return_entire_node_tag_> ||
                  std::is_same_v<ReturnType, return_output_only_tag_>);
    using output_type = std::tuple_element_t<I, typename DpfKey::outputs_tuple>;

    internal::eval_sequence_interior(dpf, recipe, memoizer);

    if constexpr (std::is_same_v<ReturnType, return_entire_node_tag_>)
    {
        internal::eval_sequence_exterior_entire_node<I>(dpf, recipe, outbuf, memoizer);
        return recipe_subsequence_iterable<output_type>(utils::data(outbuf), recipe.output_indices);
    }
    else
    {
        internal::eval_sequence_exterior_output_only<I>(dpf, recipe, outbuf, memoizer);
        return dpf::subinterval_iterable<output_type>(utils::data(outbuf), recipe.output_indices.size(), 0, 0);
    }
}

template <std::size_t I = 0,
          typename ReturnType = return_entire_node_tag_,
          typename DpfKey,
          typename InputT,
          class OutputBuffer>
auto eval_sequence(const DpfKey & dpf, const sequence_recipe<InputT> & recipe,
    OutputBuffer && outbuf)
{
    auto memoizer = make_double_space_sequence_memoizer(dpf, recipe);
    return eval_sequence<I>(dpf, recipe, outbuf, memoizer);
}

template <std::size_t I = 0,
          typename ReturnType = return_entire_node_tag_,
          typename DpfKey,
          typename InputT>
auto eval_sequence(const DpfKey & dpf, const sequence_recipe<InputT> & recipe)
{
    auto memoizer = make_double_space_sequence_memoizer(dpf, recipe);
    auto outbuf = make_output_buffer_for_recipe_subsequence<I>(dpf, recipe);
    auto iterable = eval_sequence<I>(dpf, recipe, outbuf, memoizer);
    return std::make_tuple(std::move(outbuf), std::move(iterable));
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_SEQUENCE_HPP__
