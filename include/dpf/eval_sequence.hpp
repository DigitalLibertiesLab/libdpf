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
auto eval_sequence_entire_node(const DpfKey & dpf, Iterator begin, Iterator end, OutputBuffer & outbuf)
{
    auto path = make_basic_path_memoizer(dpf);
    using output_type = std::tuple_element_t<I, typename DpfKey::concrete_outputs_tuple>;
    using leaf_node_type = std::tuple_element_t<I, typename DpfKey::leaf_tuple>;
    auto rawbuf = reinterpret_cast<leaf_node_type *>(utils::data(outbuf));
    std::size_t i = 0;
    DPF_UNROLL_LOOP
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
auto eval_sequence_output_only(const DpfKey & dpf, Iterator begin, Iterator end, OutputBuffer & outbuf)
{
    auto path = make_basic_path_memoizer(dpf);
    using output_type = std::tuple_element_t<I, typename DpfKey::concrete_outputs_tuple>;
    auto rawbuf = reinterpret_cast<output_type*>(utils::data(outbuf));
    std::size_t i = 0;
    DPF_UNROLL_LOOP
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

template <std::size_t I = 0,
          typename DpfKey,
          typename Iterator,
          typename OutputBuffer>
inline auto eval_sequence_with_recipe(const DpfKey & dpf, Iterator begin, Iterator end, OutputBuffer && outbuf)
{
    using dpf_type = DpfKey;
    using input_type = typename DpfKey::input_type;
    using node_type = typename DpfKey::interior_node;
    using output_type = std::tuple_element_t<I, typename DpfKey::concrete_outputs_tuple>;

    if (!std::is_sorted(begin, end))
    {
        throw std::runtime_error("list must be sorted");
    }

    auto mask = dpf_type::msb_mask;
    std::size_t nodes_in_sequence = std::distance(begin, end);
    std::vector<std::vector<node_type>> memo(2, std::vector<node_type>(nodes_in_sequence));

    bool curhalf = (dpf_type::depth ^ 1) & 1;
    memo[!curhalf][0] = dpf.root;
    std::size_t nodes_in_interval;

    std::list<Iterator> splits{begin, end};
    for (std::size_t level_index = 1; level_index <= dpf_type::depth; ++level_index, mask>>=1, curhalf=!curhalf)
    {
        std::size_t i = 0, j = 0;
        const node_type cw[2] = {
            set_lo_bit(dpf.correction_words[level_index-1], dpf.correction_advice[level_index-1]&1),
            set_lo_bit(dpf.correction_words[level_index-1], (dpf.correction_advice[level_index-1]>>1)&1)
        };
        // `lower` and `upper` are always adjacent elements of `splits` with `lower` < `upper`
        // [lower, upper) = "block"
        for (auto upper = std::begin(splits), lower = upper++; upper != std::end(splits); lower = upper++)
        {
            // `upper_bound()` returns iterator to first element where the relevant bit (based on `mask`) is set
            auto it = std::upper_bound(*lower, *upper, mask,
                [](auto a, auto b){ return a&b; });
            if (it == *lower)       // right only since first element in "block" requires right traversal
            {
                memo[curhalf][i++] = dpf_type::traverse_interior(memo[!curhalf][j++], cw[1], 1);
            }
            else if (it == *upper)  // left only since no element in "block" requires right traversal
            {
                memo[curhalf][i++] = dpf_type::traverse_interior(memo[!curhalf][j++], cw[0], 0);
            }
            else                    // both ways since some (non-lower) element within "block" requires right traversal
            {
                auto cur_node = memo[!curhalf][j++];
                memo[curhalf][i++] = dpf_type::traverse_interior(cur_node, cw[0], 0);
                memo[curhalf][i++] = dpf_type::traverse_interior(cur_node, cw[1], 1);
                splits.insert(upper, it);
            }
        }
        nodes_in_interval = i;
    }

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using leaf_node_type = std::tuple_element_t<I, typename DpfKey::leaf_tuple>;
    auto rawbuf = reinterpret_cast<leaf_node_type *>(utils::data(outbuf));
    auto cw = dpf.template leaf<I>();
    auto buf = memo[0];

    auto curr = begin, prev = curr;
    for (std::size_t i = 0, j = 0; i < nodes_in_sequence; ++i)
    {
        j += *prev/dpf_type::outputs_per_leaf < *curr/dpf_type::outputs_per_leaf;
        auto leaf = dpf_type::template traverse_exterior<I>(buf[j],
            get_if_lo_bit(cw, buf[j]));
        std::memcpy(&rawbuf[i], &leaf, sizeof(leaf));
        prev = curr++;
    }
HEDLEY_PRAGMA(GCC diagnostic pop)

   return subsequence_iterable<DpfKey, output_type, Iterator>(utils::data(outbuf), begin, end);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename Iterator>
auto eval_sequence_with_recipe(const DpfKey & dpf, Iterator begin, Iterator end)
{
    auto outbuf = make_output_buffer_for_subsequence<I>(dpf, begin, end);
    auto iterable = eval_sequence_with_recipe<I>(dpf, begin, end, outbuf);
    return std::make_tuple(std::move(outbuf), std::move(iterable));
}

namespace internal
{

template <typename DpfKey,
          typename InputT,
          typename SequenceMemoizer>
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

        DPF_UNROLL_LOOP
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
inline auto eval_sequence_exterior_entire_node(const DpfKey & dpf, const sequence_recipe<InputT> & recipe,
    OutputBuffer & outbuf, SequenceMemoizer & memoizer)
{
    assert_not_wildcard<I>(dpf);

    using dpf_type = DpfKey;

    auto nodes_in_interval = std::max(std::size_t(0), recipe.num_leaf_nodes);

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using leaf_node_type = std::tuple_element_t<I, typename DpfKey::leaf_tuple>;
    auto rawbuf = reinterpret_cast<leaf_node_type *>(utils::data(outbuf));
    auto cw = dpf.template leaf<I>();
    auto buf = memoizer[dpf.depth];
    DPF_UNROLL_LOOP
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
inline auto eval_sequence_exterior_output_only(const DpfKey & dpf, const sequence_recipe<InputT> & recipe,
    OutputBuffer & outbuf, SequenceMemoizer & memoizer)
{
    assert_not_wildcard<I>(dpf);

    using dpf_type = DpfKey;
    using output_type = std::tuple_element_t<I, typename DpfKey::concrete_outputs_tuple>;

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    auto rawbuf = reinterpret_cast<output_type*>(utils::data(outbuf));
    auto cw = dpf.template leaf<I>();
    using node_type = typename DpfKey::exterior_node;
    using leaf_node_type = std::tuple_element_t<I, typename DpfKey::leaf_tuple>;
    auto buf = memoizer[dpf.depth];

    leaf_node_type node;
    DPF_UNROLL_LOOP
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
    using output_type = std::tuple_element_t<I, typename DpfKey::concrete_outputs_tuple>;

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
