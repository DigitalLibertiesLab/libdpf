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

namespace dpf
{

template <std::size_t I = 0,
          typename DpfKey,
          typename Iterator,
          typename OutputBuffer>
DPF_UNROLL_LOOPS
inline auto eval_sequence(const DpfKey & dpf, Iterator begin, Iterator end, OutputBuffer & outbuf)
{
    using output_type = std::tuple_element_t<I, typename DpfKey::outputs_t>;

    auto path = make_basic_path_memoizer(dpf);
    std::size_t i = 0;
    using raw_type = std::tuple_element_t<I, typename DpfKey::leaf_nodes_t>;
    auto rawbuf = reinterpret_cast<raw_type*>(std::data(outbuf));
    for (auto it = begin; it != end; ++it)
    {
        rawbuf[i++] = internal::eval_point<I>(dpf, *it, path);
    }

    return subsequence_iterable<DpfKey, output_type, Iterator>(std::data(outbuf), begin, end);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename Iterator>
auto eval_sequence(const DpfKey & dpf, Iterator begin, Iterator end)
{
    auto outbuf = make_output_buffer_for_subsequence<I>(dpf, begin, end);
    auto subsequence_iterable = eval_sequence<I>(dpf, begin, end, outbuf);
    return std::make_tuple(std::move(outbuf), std::move(subsequence_iterable));
}

namespace internal
{

template <typename DpfKey,
          typename InputT,
          typename SequenceMemoizer>
DPF_UNROLL_LOOPS
inline auto eval_sequence_interior(const DpfKey & dpf, const list_recipe<InputT> & recipe,
    SequenceMemoizer & memoizer, std::size_t to_level = DpfKey::depth)
{
    using dpf_type = DpfKey;
    using node_type = typename DpfKey::interior_node_t;

    // level_index represents the current level being built
    // level_index = 0 => root
    // level_index = depth => last layer of interior nodes
    std::size_t level_index = memoizer.assign_dpf(dpf, recipe);
    std::size_t recipe_index = recipe.level_endpoints[level_index-1];
    std::size_t nodes_at_level = memoizer.get_nodes_at_level(level_index-1);

    for (; level_index <= to_level; level_index = memoizer.advance_level())
    {
        const node_type cw[2] = {
            set_lo_bit(dpf.interior_cws[level_index-1], dpf.correction_advice[level_index-1]&1),
            set_lo_bit(dpf.interior_cws[level_index-1], (dpf.correction_advice[level_index-1]>>1)&1)
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
        nodes_at_level = memoizer.get_nodes_at_level(level_index);
    }
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          class OutputBuffer,
          typename SequenceMemoizer>
DPF_UNROLL_LOOPS
inline auto eval_sequence_exterior(const DpfKey & dpf, const list_recipe<InputT> & recipe,
    OutputBuffer & outbuf, SequenceMemoizer & memoizer)
{
    assert_not_wildcard<I>(dpf);

    using dpf_type = DpfKey;

    auto nodes_in_interval = std::max(std::size_t(0), recipe.num_leaf_nodes);

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using raw_type = std::tuple_element_t<I, typename DpfKey::leaf_nodes_t>;
    auto rawbuf = reinterpret_cast<raw_type*>(std::data(outbuf));
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

}  // namespace internal

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          class OutputBuffer,
          typename SequenceMemoizer>
auto eval_sequence(const DpfKey & dpf, const list_recipe<InputT> & recipe,
    OutputBuffer & outbuf, SequenceMemoizer & memoizer)
{
    using output_type = std::tuple_element_t<I, typename DpfKey::outputs_t>;

    internal::eval_sequence_interior(dpf, recipe, memoizer);
    internal::eval_sequence_exterior<I>(dpf, recipe, outbuf, memoizer);

    return recipe_subsequence_iterable<output_type>(std::data(outbuf), recipe.output_indices);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          class OutputBuffer>
auto eval_sequence(const DpfKey & dpf, const list_recipe<InputT> & recipe,
    OutputBuffer & outbuf)
{
    auto memoizer = make_double_space_sequence_memoizer(dpf, recipe);
    return eval_sequence<I>(dpf, recipe, outbuf, memoizer);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT>
auto eval_sequence(const DpfKey & dpf, const list_recipe<InputT> & recipe)
{
    auto memoizer = make_double_space_sequence_memoizer(dpf, recipe);
    auto outbuf = make_output_buffer_for_recipe_subsequence<I>(dpf, recipe);
    auto subsequence_iterable = eval_sequence<I>(dpf, recipe, outbuf, memoizer);
    return std::make_tuple(std::move(outbuf), std::move(subsequence_iterable));
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_SEQUENCE_HPP__
