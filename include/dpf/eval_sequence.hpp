/// @file dpf/memoization.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief miscellaneous helper functions, structs, preprocessor directives
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_SEQUENCE_HPP__
#define LIBDPF_INCLUDE_DPF_SEQUENCE_HPP__

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
#include "dpf/eval_interval.hpp"

namespace dpf
{

template <std::size_t I = 0,
          typename DpfKey,
          typename Iterator,
          typename OutputBuffer>
DPF_UNROLL_LOOPS
inline auto eval_sequence(const DpfKey & dpf, Iterator begin, Iterator end, OutputBuffer & outbuf)
{
    auto path = dpf::make_path_memoizer(dpf);
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
    dpf::output_buffer<output_t> outbuf(std::distance(begin, end));
    eval_sequence<I>(dpf, begin, end, outbuf);
    return std::move(outbuf);
}

template <typename InputT>
struct list_recipe
{
    list_recipe(const std::vector<int8_t> & steps_,
                const std::vector<std::size_t> subsequence_indexes_,
                std::size_t leaf_index_,
                std::vector<std::size_t> level_endpoints_)
      : recipe_steps{steps_},
        output_indices{subsequence_indexes_},
        num_leaf_nodes{leaf_index_},
        level_endpoints{level_endpoints_}
    { }

    const std::vector<int8_t> recipe_steps;
    const std::vector<std::size_t> output_indices;
    const std::size_t num_leaf_nodes;
    const std::vector<std::size_t> level_endpoints;
};

template <typename DpfKey,
          typename InputT,
          typename SequenceMemoizer>
DPF_UNROLL_LOOPS
inline auto eval_sequence_interior(const DpfKey & dpf, const list_recipe<InputT> & recipe,
    SequenceMemoizer & memoizer, std::size_t to_level = DpfKey::tree_depth)
{
    using node_t = typename DpfKey::interior_node_t;
    bool currhalf = !(dpf.tree_depth & 1);
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
    // eval_interval_exterior<I>(dpf, std::size_t(0), recipe.num_leaf_nodes, outbuf, memoizer.buf);
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
            dpf::get_if_lo_bit(cw, memoizer.buf[j]));
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

template <typename RandomAccessIterator>
auto make_recipe(std::size_t outputs_per_leaf, RandomAccessIterator begin, RandomAccessIterator end)
{
    using input_t = std::remove_reference_t<decltype(*begin)>;
    struct IteratorComp
    {
        bool operator()(const RandomAccessIterator & lhs,
                        const RandomAccessIterator & rhs) const
        { 
            return *lhs < *rhs;
        }
    };

    if (!std::is_sorted(begin, end))
    {
        throw std::runtime_error("list must be sorted");
    }

    auto depth = dpf::utils::bitlength_of_v<input_t> - std::log2(outputs_per_leaf);
    auto mask = input_t(1) << (dpf::utils::bitlength_of_v<input_t>-1);

    std::set<RandomAccessIterator, IteratorComp> splits;
    splits.insert(begin);

    std::vector<std::size_t> level_endpoints;
    level_endpoints.push_back(0);
    std::vector<int8_t> recipe_steps;
    for (std::size_t level_index = 0; level_index < depth; ++level_index, mask>>=1)
    {
        for (auto upper = std::begin(splits), lower = upper++; ; lower = upper++)
        {
            bool at_end = (upper == std::end(splits));
            auto upper_ = at_end ? end : *upper;
            auto it = std::upper_bound(*lower, upper_, mask,
                [](auto a, auto b){ return a&b; });
            if (it == *lower) recipe_steps.push_back(-1);       // right only
            else if (it == upper_) recipe_steps.push_back(+1);  // left only
            else
            {
                recipe_steps.push_back(0);                      // both ways
                splits.insert(it);
            }
            if (at_end) break;
        }
        level_endpoints.push_back(recipe_steps.size());
    }

    std::vector<std::size_t> output_indices;
    // output_indices.push_back(*begin % outputs_per_leaf);
    std::size_t leaf_index = 0;//*begin/outputs_per_leaf < *(begin+1)/outs_per_leaf;
    for (auto curr = begin, prev = curr; curr != end; prev = curr++)
    {
        leaf_index += *prev/outputs_per_leaf < *curr/outputs_per_leaf;
        output_indices.push_back(leaf_index * outputs_per_leaf + (*curr % outputs_per_leaf));
    }

    return list_recipe<input_t>{recipe_steps, output_indices, leaf_index+1, level_endpoints};
}

template <typename InputT,
          typename NodeT,
          typename Allocator = detail::aligned_allocator<NodeT>>
struct inplace_reversing_sequence_memoizer
{
  public:
    using unique_ptr = typename Allocator::unique_ptr;
    using forward_iter = NodeT *;
    using reverse_iter = std::reverse_iterator<forward_iter>;

    explicit inplace_reversing_sequence_memoizer(const list_recipe<InputT> & r, Allocator alloc = Allocator{})
      : recipe{r},
        buf{alloc.make_unique(recipe.num_leaf_nodes)} { }

    struct pointer_facade
    {
        HEDLEY_ALWAYS_INLINE
        pointer_facade(bool flip, forward_iter it, reverse_iter rit)
          : flip_{flip}, it_{it}, rit_{rit} { }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        HEDLEY_PURE
        auto & operator[](std::size_t i)
        {
            return flip_ ? rit_[i] : it_[i];
        }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        HEDLEY_PURE
        const auto & operator[](std::size_t i) const
        {
            return flip_ ? rit_[i] : it_[i];
        }

      private:
        const bool flip_;
        forward_iter it_;
        reverse_iter rit_;
    };

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    auto get_iterators(bool b, InputT x) const noexcept
    {
        return std::make_pair(
          pointer_facade(b, &buf[recipe.num_leaf_nodes-x],
              std::make_reverse_iterator(&buf[x])),
          pointer_facade(b, &buf[0],
              std::make_reverse_iterator(&buf[recipe.num_leaf_nodes])));
    }

    auto get_step(bool b, std::size_t level, std::size_t step)
    {
        step = b ? step : recipe.level_endpoints[level+1] - step - 1 + recipe.level_endpoints[level];
        return recipe.recipe_steps(step);
    }

    const list_recipe<InputT> & recipe;
    const unique_ptr buf;
};

template <typename InputT,
          typename NodeT,
          typename Allocator = detail::aligned_allocator<NodeT>>
struct double_space_sequence_memoizer
{
  public:
    using unique_ptr = typename Allocator::unique_ptr;

    explicit double_space_sequence_memoizer(const list_recipe<InputT> & r, Allocator alloc = Allocator{})
      : recipe{r},
        buf{alloc.allocate_unique_ptr(2*recipe.num_leaf_nodes * sizeof(NodeT))} { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    auto get_iterators(bool b, InputT) const noexcept
    {
        return std::make_pair(
          Allocator::assume_aligned(&buf[recipe.num_leaf_nodes*(!b)]),
          Allocator::assume_aligned(&buf[recipe.num_leaf_nodes*b]));
    }

    auto get_step(bool, std::size_t, std::size_t step)
    {
        return recipe.recipe_steps[step];
    }

    const list_recipe<InputT> & recipe;
    const unique_ptr buf;
};

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
    using exterior_node_t = typename DpfKey::exterior_node_t;
    using output_t = std::tuple_element_t<I, typename DpfKey::outputs_t>;

    auto memoizer = make_double_space_sequence_memoizer(dpf, recipe);

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    static constexpr auto outputs_per_leaf = outputs_per_leaf_v<output_t, exterior_node_t>;
HEDLEY_PRAGMA(GCC diagnostic pop)
    dpf::output_buffer<output_t> outbuf(recipe.num_leaf_nodes*outputs_per_leaf);

    return eval_sequence(dpf, recipe, outbuf, memoizer);
}

template <typename DpfKey,
          typename InputT>
auto make_inplace_reversing_sequence_memoizer(const DpfKey &, const list_recipe<InputT> & recipe)
{
    using node_t = typename DpfKey::interior_node_t;
    using allocator_t = detail::aligned_allocator<node_t, utils::max_align_v>;
    return inplace_reversing_sequence_memoizer<InputT, node_t, allocator_t>(recipe);
}

template <typename DpfKey,
          typename InputT>
auto make_double_space_sequence_memoizer(const DpfKey &, const list_recipe<InputT> & recipe)
{
    using node_t = typename DpfKey::interior_node_t;
    using allocator_t = detail::aligned_allocator<node_t, utils::max_align_v>;
    return double_space_sequence_memoizer<InputT, node_t, allocator_t>(recipe);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_SEQUENCE_HPP__
