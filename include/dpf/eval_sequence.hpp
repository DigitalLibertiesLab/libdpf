/// @file dpf/eval_sequence.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

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
#include "dpf/eval_common.hpp"
#include "dpf/eval_point.hpp"
#include "dpf/path_memoizer.hpp"
#include "dpf/sequence_memoizer.hpp"
#include "dpf/subsequence_iterable.hpp"
#include "dpf/subinterval_iterable.hpp"

namespace dpf
{

struct return_type_tag_{};

struct return_entire_node_tag_ final : public return_type_tag_ {};
// static constexpr auto return_entire_node_tag = return_entire_node_tag_{};

struct return_output_only_tag_ final : public return_type_tag_ {};
// static constexpr auto return_output_only_tag = return_output_only_tag_{};

namespace internal
{

template <std::size_t ...Is,
        typename DpfKey,
        typename Iterator,
        typename OutputBuffers,
        std::size_t ...IIs>
HEDLEY_ALWAYS_INLINE
auto eval_sequence_entire_node(const DpfKey & dpf, Iterator begin, Iterator end,
    OutputBuffers && outbufs, std::index_sequence<IIs...>)
{
    auto path = make_basic_path_memoizer(dpf);
    auto rawbuf = utils::make_tuple(
        reinterpret_cast<std::tuple_element_t<Is, typename DpfKey::leaf_tuple>*>(utils::data(utils::get<IIs>(outbufs)))...
    );

    std::size_t i = 0;
    // DPF_UNROLL_LOOP
    auto to_integral_type = dpf::utils::to_integral_type<typename DpfKey::input_type>{};
    for (auto it = begin; it != end; ++it)
    {
        std::tie(utils::get<IIs>(rawbuf)[i]...) = std::make_tuple(
            dpf::eval_point<Is>(dpf, *it, path).node...);
        i++;
    }
    return utils::make_tuple(
        dpf::subsequence_iterable<DpfKey, decltype(std::begin(utils::get<IIs>(outbufs))), Iterator>(std::begin(utils::get<IIs>(outbufs)), begin, end)...
    );
}

template <std::size_t ...Is,
        typename DpfKey,
        typename Iterator,
        typename OutputBuffers,
        std::size_t ...IIs>
HEDLEY_ALWAYS_INLINE
auto eval_sequence_output_only(const DpfKey & dpf, Iterator begin, Iterator end, OutputBuffers && outbufs,
    std::index_sequence<IIs...>)
{
    auto path = make_basic_path_memoizer(dpf);
    auto rawbuf = utils::make_tuple(
        reinterpret_cast<typename DpfKey::concrete_output_type<Is>*>(utils::data(utils::get<IIs>(outbufs)))...
    );

    std::size_t i = 0;
    // DPF_UNROLL_LOOP
    for (auto it = begin; it != end; ++it)
    {
        std::tie(utils::get<IIs>(rawbuf)[i]...) = std::make_tuple(
            *dpf::eval_point<Is>(dpf, *it, path)...);
        i++;
    }
    return utils::make_tuple(
        dpf::subinterval_iterable(utils::get<IIs>(rawbuf), i-1, 0, 0)...
    );
}

}  // dpf::internal

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename Iterator,
          typename OutputBuffers,
          typename ReturnType = return_entire_node_tag_,
          std::enable_if_t<std::is_base_of_v<return_type_tag_, ReturnType>, bool> = true,
          std::enable_if_t<!std::is_base_of_v<return_type_tag_, OutputBuffers>, bool> = true>
inline auto eval_sequence(const DpfKey & dpf, Iterator begin, Iterator end,
    OutputBuffers && outbufs, ReturnType return_type = ReturnType{})
{
    static_assert(std::is_same_v<ReturnType, return_entire_node_tag_> ||
                    std::is_same_v<ReturnType, return_output_only_tag_>);
    if constexpr(std::is_same_v<ReturnType, return_entire_node_tag_>)
    {
        return internal::eval_sequence_entire_node<I, Is...>(dpf, begin, end, outbufs, std::make_index_sequence<1+sizeof...(Is)>{});
    }
    else
    {
        return internal::eval_sequence_output_only<I, Is...>(dpf, begin, end, outbufs, std::make_index_sequence<1+sizeof...(Is)>{});
    }
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename Iterator,
          typename ReturnType = return_entire_node_tag_,
          std::enable_if_t<std::is_base_of_v<return_type_tag_, ReturnType>, bool> = true>
auto eval_sequence(const DpfKey & dpf, Iterator begin, Iterator end,
    ReturnType return_type = ReturnType{})
{
    auto outbufs = utils::make_tuple(
        make_output_buffer_for_subsequence<I>(dpf, begin, end),
        make_output_buffer_for_subsequence<Is>(dpf, begin, end)...);

    // moving `outbufs` is allowed as the `outbufs` are `std::vectors`
    //   the underlying data remains on the heap
    //   and thus the data the iterable refers to is still valid
    auto iterable = eval_sequence<I, Is...>(dpf, begin, end, outbufs, return_type);
    return std::make_pair(std::move(outbufs), std::move(iterable));
}

template <std::size_t I = 0,
          typename DpfKey,
          typename Iterator,
          typename OutputBuffer>
inline auto eval_sequence_breadth_first(const DpfKey & dpf, Iterator begin, Iterator end, OutputBuffer && outbuf)
{
    assert_not_wildcard<I>(dpf);

    using dpf_type = DpfKey;
    using input_type = typename DpfKey::input_type;
    using node_type = typename DpfKey::interior_node;
    using output_type = typename DpfKey::concrete_output_type<I>;

    using allocator = aligned_allocator<typename DpfKey::interior_node>;
    using unique_ptr = typename allocator::unique_ptr;
    allocator alloc = allocator{};

    if (!std::is_sorted(begin, end))
    {
        throw std::runtime_error("list must be sorted");
    }

    auto mask = dpf_type::msb_mask;
    std::size_t nodes_in_sequence = std::distance(begin, end);
    unique_ptr memo{alloc.allocate_unique_ptr(nodes_in_sequence*2)};

    bool curhalf = (dpf_type::depth ^ 1) & 1;
    memo[!curhalf*nodes_in_sequence + 0] = dpf.root;

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
                memo[curhalf*nodes_in_sequence + i++] = dpf_type::traverse_interior(memo[!curhalf*nodes_in_sequence + j++], cw[1], 1);
            }
            else if (it == *upper)  // left only since no element in "block" requires right traversal
            {
                memo[curhalf*nodes_in_sequence + i++] = dpf_type::traverse_interior(memo[!curhalf*nodes_in_sequence + j++], cw[0], 0);
            }
            else                    // both ways since some (non-lower) element within "block" requires right traversal
            {
                auto cur_node = memo[!curhalf*nodes_in_sequence + j++];
                memo[curhalf*nodes_in_sequence + i++] = dpf_type::traverse_interior(cur_node, cw[0], 0);
                memo[curhalf*nodes_in_sequence + i++] = dpf_type::traverse_interior(cur_node, cw[1], 1);
                splits.insert(upper, it);
            }
        }
    }

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using leaf_node_type = std::tuple_element_t<I, typename DpfKey::leaf_tuple>;
    auto rawbuf = reinterpret_cast<leaf_node_type *>(utils::data(outbuf));
    auto cw = dpf.template leaf<I>();
    auto buf = memo.get();

    constexpr auto clz = utils::countl_zero_symmetric_difference<input_type>{};
    auto curr = begin, prev = curr;
    for (std::size_t i = 0, j = 0; i < nodes_in_sequence; ++i)
    {
        j += (clz(*prev, *curr)) < dpf_type::depth;
        auto leaf = dpf_type::template traverse_exterior<I>(buf[j],
            get_if_lo_bit(cw, buf[j]));
        std::memcpy(&rawbuf[i], &leaf, sizeof(leaf));
        prev = curr++;
    }
HEDLEY_PRAGMA(GCC diagnostic pop)

   return subsequence_iterable<DpfKey, decltype(std::begin(outbuf)), Iterator>(std::begin(outbuf), begin, end);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename Iterator>
auto eval_sequence_breadth_first(const DpfKey & dpf, Iterator begin, Iterator end)
{
    auto outbuf = make_output_buffer_for_subsequence<I>(dpf, begin, end);

    // moving `outbuf` is allowed as `outbuf` is a `std::vectors`
    //   the underlying data remains on the heap
    //   and thus the data the iterable refers to is still valid
    auto iterable = eval_sequence_breadth_first<I>(dpf, begin, end, outbuf);
    return std::make_pair(std::move(outbuf), std::move(iterable));
}

namespace internal
{

template <typename DpfKey,
          typename SequenceMemoizer>
inline auto eval_sequence_interior(const DpfKey & dpf, const sequence_recipe & recipe,
    SequenceMemoizer && memoizer, std::size_t to_level = DpfKey::depth)
{
    using dpf_type = DpfKey;
    using node_type = typename DpfKey::interior_node;

    // level_index represents the current level being built
    // level_index = 0 => root
    // level_index = depth => last layer of interior nodes
    std::size_t level_index = memoizer.assign_dpf(dpf, recipe);
    std::size_t recipe_index = recipe.level_endpoints()[level_index-1];
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
          typename OutputBuffer,
          typename SequenceMemoizer>
inline auto eval_sequence_exterior_entire_node(const DpfKey & dpf, const sequence_recipe & recipe,
    OutputBuffer && outbuf, SequenceMemoizer && memoizer)
{
    assert_not_wildcard<I>(dpf);

    using dpf_type = DpfKey;

    auto nodes_in_interval = recipe.num_leaf_nodes();

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
          typename OutputBuffer,
          typename SequenceMemoizer>
inline auto eval_sequence_exterior_output_only(const DpfKey & dpf, const sequence_recipe & recipe,
    OutputBuffer && outbuf, SequenceMemoizer && memoizer)
{
    assert_not_wildcard<I>(dpf);

    using dpf_type = DpfKey;
    using output_type = typename DpfKey::concrete_output_type<I>;

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    auto cw = dpf.template leaf<I>();
    using node_type = typename DpfKey::exterior_node;
    using leaf_node_type = std::tuple_element_t<I, typename DpfKey::leaf_tuple>;
    auto buf = memoizer[dpf.depth];

    leaf_node_type node;
    // DPF_UNROLL_LOOP
    for (std::size_t i = 0, j = -1, prev = -1, curr;
        i < recipe.output_indices().size();
        prev = curr, ++i)
    {
        curr = recipe.output_indices()[i]/dpf_type::outputs_per_leaf;
        if (prev != curr)
        {
            ++j;
            node = dpf_type::template traverse_exterior<I>(buf[j], get_if_lo_bit(cw, buf[j]));
        }
        outbuf[i] = extract_leaf<node_type, output_type>(node, recipe.output_indices()[i] % dpf_type::outputs_per_leaf);
    }
HEDLEY_PRAGMA(GCC diagnostic pop)
}

template <std::size_t ...Is,
          typename DpfKey,
          typename OutputBuffers,
          typename SequenceMemoizer,
          typename ReturnType = return_entire_node_tag_,
          std::enable_if_t<std::is_base_of_v<return_type_tag_, ReturnType>, bool> = true,
          std::size_t ...IIs>
auto eval_sequence(const DpfKey & dpf, const sequence_recipe & recipe,
    OutputBuffers && outbufs, SequenceMemoizer && memoizer, ReturnType return_type, std::index_sequence<IIs...>)
{
    internal::eval_sequence_interior(dpf, recipe, memoizer);

    if constexpr (std::is_same_v<ReturnType, return_entire_node_tag_>)
    {
        (internal::eval_sequence_exterior_entire_node<Is>(dpf, recipe, utils::get<IIs>(outbufs), memoizer), ...);
        return utils::make_tuple(
            recipe_subsequence_iterable(std::begin(utils::get<IIs>(outbufs)), recipe.output_indices())...);
    }
    else
    {
        (internal::eval_sequence_exterior_output_only<Is>(dpf, recipe, utils::get<IIs>(outbufs), memoizer), ...);
        return utils::make_tuple(
            subinterval_iterable(std::begin(utils::get<IIs>(outbufs)), recipe.output_indices().size()-1, 0, 0)...
        );
    }
}

}  // namespace internal

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename OutputBuffers,
          typename SequenceMemoizer,
          typename ReturnType = return_entire_node_tag_,
          std::enable_if_t<!std::is_base_of_v<return_type_tag_, SequenceMemoizer>, bool> = true,
          std::enable_if_t<std::is_base_of_v<return_type_tag_, ReturnType>, bool> = true>
HEDLEY_ALWAYS_INLINE
auto eval_sequence(const DpfKey & dpf, const sequence_recipe & recipe,
    OutputBuffers & outbufs, SequenceMemoizer && memoizer,
    ReturnType return_type = ReturnType{})
{
    assert_not_wildcard<I, Is...>(dpf);

    return internal::eval_sequence<I, Is...>(dpf, recipe, outbufs, memoizer, return_type, std::make_index_sequence<1+sizeof...(Is)>());
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename OutputBuffers,
          typename ReturnType = return_entire_node_tag_,
          std::enable_if_t<!std::is_base_of_v<sequence_memoizer_tag_,
              std::remove_reference_t<OutputBuffers>>, bool> = true,
          std::enable_if_t<std::is_base_of_v<return_type_tag_, ReturnType>, bool> = true>
HEDLEY_ALWAYS_INLINE
auto eval_sequence(const DpfKey & dpf, const sequence_recipe & recipe,
    OutputBuffers & outbufs, ReturnType return_type = ReturnType{})
{
    return eval_sequence<I, Is...>(dpf, recipe, outbufs,
        dpf::make_double_space_sequence_memoizer<DpfKey>(recipe), return_type);
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename SequenceMemoizer,
          typename ReturnType = return_entire_node_tag_,
          std::enable_if_t<std::is_base_of_v<sequence_memoizer_tag_,
              std::remove_reference_t<SequenceMemoizer>>, bool> = true,
          std::enable_if_t<std::is_base_of_v<return_type_tag_, ReturnType>, bool> = true>
HEDLEY_ALWAYS_INLINE
auto eval_sequence(const DpfKey & dpf, const sequence_recipe & recipe,
    SequenceMemoizer && memoizer, ReturnType return_type = ReturnType{})
{
    auto outbufs = utils::make_tuple(
        make_output_buffer_for_recipe_subsequence<I>(dpf, recipe),
        make_output_buffer_for_recipe_subsequence<Is>(dpf, recipe)...);

    // moving `outbufs` is allowed as the `outbufs` are `std::vectors`
    //   the underlying data remains on the heap
    //   and thus the data the iterable refers to is still valid
    auto iterable = eval_sequence<I, Is...>(dpf, recipe, outbufs, memoizer, return_type);
    return std::make_pair(std::move(outbufs), std::move(iterable));
}

template <std::size_t I = 0,
          std::size_t ...Is,
          typename DpfKey,
          typename ReturnType = return_entire_node_tag_,
          std::enable_if_t<std::is_base_of_v<return_type_tag_, ReturnType>, bool> = true>
HEDLEY_ALWAYS_INLINE
auto eval_sequence(const DpfKey & dpf, const sequence_recipe & recipe,
     ReturnType return_type = ReturnType{})
{
    return eval_sequence<I, Is...>(dpf, recipe,
       dpf::make_double_space_sequence_memoizer<DpfKey>(recipe), return_type);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_SEQUENCE_HPP__
