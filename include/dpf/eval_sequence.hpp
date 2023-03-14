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
// #include "dpf/eval_interval.hpp"
#include "dpf/path_memoizer.hpp"
#include "dpf/sequence_memoizer.hpp"

namespace dpf
{

template <std::size_t I = 0,
          typename DpfKey,
          typename Iterator,
          typename OutputBuffer>
DPF_UNROLL_LOOPS
inline auto eval_sequence(const DpfKey & dpf, Iterator begin, Iterator end, OutputBuffer & outbuf)
{
    auto path = make_basic_path_memoizer(dpf);
    std::size_t i = 0;
    for (auto it = begin; it != end; ++it)
    {
        outbuf[i++] = eval_point<I>(dpf, *it, path);
    }
}

template <std::size_t I = 0,
          typename DpfKey,
          typename Iterator>
auto eval_sequence(const DpfKey & dpf, Iterator begin, Iterator end)
{
    using output_t = std::tuple_element_t<I, typename DpfKey::outputs_t>;
    output_buffer<output_t> outbuf(std::distance(begin, end));
    eval_sequence<I>(dpf, begin, end, outbuf);
    return std::move(outbuf);
}

template <typename DpfKey,
          typename InputT,
          typename SequenceMemoizer>
DPF_UNROLL_LOOPS
inline auto eval_sequence_interior(const DpfKey & dpf, const list_recipe<InputT> & recipe,
    SequenceMemoizer & memoizer, std::size_t to_level = DpfKey::depth)
{
    using node_t = typename DpfKey::interior_node_t;
    bool currhalf = !(dpf.depth & 1);
    std::size_t nodes_at_level = 1;

    for (std::size_t level_index=0, recipe_index=0; level_index < to_level; ++level_index, currhalf = !currhalf)
    {
        const node_t cw[2] = {
            set_lo_bit(dpf.interior_cws[level_index], dpf.correction_advice[level_index]&1),
            set_lo_bit(dpf.interior_cws[level_index], (dpf.correction_advice[level_index]>>1)&1)
        };

        auto [prevbuf, currbuf] = memoizer.get_iterators(currhalf, nodes_at_level);
        if (!level_index) prevbuf[0] = dpf.root;
        std::size_t output_index = 0;
        for (std::size_t input_index = 0; input_index < nodes_at_level; ++input_index, ++recipe_index)
        {
            if (true || currhalf)
            {
                if (memoizer.get_step(currhalf, level_index, recipe_index) > int8_t(-1))
                {
                    currbuf[output_index++] = DpfKey::traverse_interior(prevbuf[input_index], cw[0], 0);
                }
                if (memoizer.get_step(currhalf, level_index, recipe_index) < int8_t(1))
                {
                    currbuf[output_index++] = DpfKey::traverse_interior(prevbuf[input_index], cw[1], 1);
                }
            }
            else
            {
                if (memoizer.get_step(currhalf, level_index, recipe_index) < int8_t(1))
                {
                    currbuf[output_index++] = DpfKey::traverse_interior(prevbuf[input_index], cw[1], 1);
                }
                if (memoizer.get_step(currhalf, level_index, recipe_index) > int8_t(-1))
                {
                    currbuf[output_index++] = DpfKey::traverse_interior(prevbuf[input_index], cw[0], 0);
                }
            }
        }
        nodes_at_level = output_index;
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

    using exterior_node_t = typename DpfKey::exterior_node_t;
    using output_t = std::tuple_element_t<I, typename DpfKey::outputs_t>;

    auto nodes_in_interval = std::max(std::size_t(0), recipe.num_leaf_nodes);

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    auto cw = dpf.template exterior_cw<I>();
    auto rawbuf = reinterpret_cast<decltype(cw)*>(std::data(outbuf));
    for (std::size_t j = 0; j < nodes_in_interval; ++j)
    {
        auto leaf = DpfKey::template traverse_exterior<I>(memoizer.buf[j],
            get_if_lo_bit(cw, memoizer.buf[j]));
        std::memcpy(&rawbuf[j], &leaf, sizeof(leaf));
    }
HEDLEY_PRAGMA(GCC diagnostic pop)
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          class OutputBuffer,
          typename SequenceMemoizer>
auto eval_sequence(const DpfKey & dpf, const list_recipe<InputT> & recipe,
    OutputBuffer & outbuf, SequenceMemoizer & memoizer)
{
    assert_not_wildcard<I>(dpf);

    eval_sequence_interior(dpf, recipe, memoizer);
    eval_sequence_exterior<I>(dpf, recipe, outbuf, memoizer);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          class OutputBuffer>
auto eval_sequence(const DpfKey & dpf, const list_recipe<InputT> & recipe,
    OutputBuffer & outbuf)
{
    auto memoizer = make_double_space_sequence_memoizer(dpf, recipe);
    return eval_sequence(dpf, recipe, outbuf, memoizer);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT>
auto eval_sequence(const DpfKey & dpf, const list_recipe<InputT> & recipe)
{
    using output_t = std::tuple_element_t<I, typename DpfKey::outputs_t>;

    auto memoizer = make_double_space_sequence_memoizer(dpf, recipe);

    output_buffer<output_t> outbuf(recipe.num_leaf_nodes*DpfKey::outputs_per_leaf);

    return eval_sequence(dpf, recipe, outbuf, memoizer);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_SEQUENCE_HPP__
