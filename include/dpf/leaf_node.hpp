/// @file dpf/leaf_node.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_LEAF_NODE_HPP__
#define LIBDPF_INCLUDE_DPF_LEAF_NODE_HPP__

#include <utility>
#include <cstddef>
#include <cstring>
#include <functional>
#include <tuple>

#include "simde/simde/x86/avx2.h"

#include "dpf/bit.hpp"
#include "dpf/xor_wrapper.hpp"
#include "dpf/wildcard.hpp"
#include "dpf/leaf_arithmetic.hpp"
#include "dpf/utils.hpp"
#include "dpf/random.hpp"

namespace dpf
{

/// @brief `value` is `true` if multiple leaves are packed into each leaf node
template <typename OutputT,
          typename NodeT>
using is_packable = std::bool_constant<
    std::less<>{}(utils::bitlength_of_v<OutputT>, utils::bitlength_of_v<NodeT>) &&
    std::equal_to<>{}(utils::bitlength_of_v<NodeT> % utils::bitlength_of_v<OutputT>, 0)>;

template <typename OutputT,
          typename NodeT>
static constexpr bool is_packable_v = is_packable<OutputT, NodeT>::value;

template <typename OutputT,
          typename NodeT>
struct outputs_per_leaf
    : public std::integral_constant<std::size_t,
        !is_packable_v<OutputT, NodeT> ? 1 :
            utils::bitlength_of_v<NodeT> / utils::bitlength_of_v<OutputT>> { };

template <typename OutputT,
          typename NodeT>
static constexpr std::size_t outputs_per_leaf_v
    = outputs_per_leaf<OutputT, NodeT>::value;

template <typename OutputT,
          typename NodeT>
static constexpr std::size_t lg_outputs_per_leaf_v
    = std::log2(outputs_per_leaf<OutputT, NodeT>::value);

template <typename OutputT,
          typename NodeT>
struct block_length_of_leaf
    : std::integral_constant<std::size_t, is_packable_v<OutputT, NodeT> ? 1 :
                                    utils::quotient_ceiling(
                                        utils::bitlength_of_v<OutputT>,
                                        utils::bitlength_of_v<NodeT>)
                                    >{ };

template <typename OutputT,
          typename NodeT>
static constexpr std::size_t block_length_of_leaf_v
    = block_length_of_leaf<OutputT, NodeT>::value;

template <typename OutputT,
          typename NodeT>
constexpr std::size_t offset_within_block(std::size_t x) noexcept
{
    return x % dpf::outputs_per_leaf_v<OutputT, NodeT>;
}

template <std::size_t I,
          typename N,
          std::size_t I_,
          typename OutputsT>
struct block_offset_of_leaf
{
    static constexpr std::size_t value = dpf::block_length_of_leaf_v<std::tuple_element_t<I_, OutputsT>, N>
        + block_offset_of_leaf<I, N, I_+1, OutputsT>::value;
};

template <std::size_t I,
          typename N,
          typename OutputsT>
struct block_offset_of_leaf<I, N, I, OutputsT>
{
    static constexpr std::size_t value = 0;
};

template <std::size_t I, typename N, typename OutputsT>
inline constexpr std::size_t block_offset_of_leaf_v
    = block_offset_of_leaf<I, N, 0, OutputsT>::value;

template <typename NodeT,
          typename OutputT,
          std::size_t block_len = block_length_of_leaf_v<OutputT, NodeT>>
struct leaf_node
{
    static_assert(block_len == block_length_of_leaf_v<OutputT, NodeT>);
    using type = std::array<NodeT, block_len>;
};

template <typename NodeT,
          typename OutputT>
struct leaf_node<NodeT, OutputT, 1>
{
    static_assert(1 == block_length_of_leaf_v<OutputT, NodeT>);
    using type = NodeT;
};

template <typename NodeT,
          typename OutputT>
using leaf_node_t = typename leaf_node<NodeT, OutputT>::type;

template <typename NodeT,
          typename OutputT,
          typename ...OutputTs>
struct leaf_tuple
{
    using type = std::tuple<leaf_node_t<NodeT, OutputT>,
                            leaf_node_t<NodeT, OutputTs>...>;
};
template <typename NodeT,
          typename OutputT,
          typename ...OutputTs>
using leaf_tuple_t = typename leaf_tuple<NodeT, OutputT, OutputTs...>::type;

template <typename NodeT,
          typename OutputT>
static OutputT extract_leaf(const leaf_node_t<NodeT, OutputT> & leaf, std::size_t x) noexcept
{
    auto off = offset_within_block<OutputT, NodeT>(x);

    OutputT y;
    if constexpr (std::is_same_v<OutputT, dpf::bit>)
    {
        auto yy = utils::single_bit_mask<NodeT>(off);
        y = dpf::to_bit(simde_mm_testz_si128(leaf, yy));
    }
    else
    {
        std::memcpy(&y, reinterpret_cast<const OutputT *>(&leaf) + off, sizeof(y));
    }

    return y;
}

// Inserts y at correct place (based on x) within a (otherwise 0) NodeT
template <typename NodeT,
          typename InputT,
          typename OutputT>
auto make_naked_leaf(InputT x, OutputT y)
{
    using leaf_type = dpf::leaf_node_t<NodeT, OutputT>;
    auto off = offset_within_block<OutputT, NodeT>(x);
    leaf_type Y{0};
    if constexpr (std::is_same_v<OutputT, dpf::bit>)
    {
        Y = get_if(utils::single_bit_mask<NodeT>(off), y);
    }
    else
    {
        std::memcpy(reinterpret_cast<OutputT *>(&Y) + off, &y, sizeof(y));
    }

    return Y;
}

template <typename ExteriorPRG,
          std::size_t I,
          typename OutputsTuple,
          typename InteriorBlock>
auto make_leaf_mask_inner(const InteriorBlock & seed)
{
    using node_type = typename ExteriorPRG::block_t;
    using output_type = std::tuple_element_t<I, OutputsTuple>;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using leaf_type = dpf::leaf_node_t<node_type, output_type>;

    if constexpr(std::tuple_size_v<OutputsTuple> == 1
        && dpf::block_length_of_leaf_v<output_type, node_type> == 1
        && std::is_same_v<InteriorBlock, node_type>)
    {
        return seed;
    }
    else
    {
        auto count = dpf::block_length_of_leaf_v<output_type, node_type>;
        auto pos = dpf::block_offset_of_leaf_v<I, node_type, OutputsTuple>;
        leaf_type output;
        auto seed_ = utils::to_exterior_node<node_type>(seed);
        ExteriorPRG::eval(seed_, reinterpret_cast<node_type *>(&output), count, pos);

        return output;
    }
HEDLEY_PRAGMA(GCC diagnostic pop)
}

template <typename ExteriorPRG,
          std::size_t I,
          typename OutputsTuple,
          typename InteriorBlock>
auto make_leaf_mask(const InteriorBlock & seed0, const InteriorBlock & seed1)
{
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using node_type = typename ExteriorPRG::block_t;
    using output_type = std::tuple_element_t<I, OutputsTuple>;

    auto mask0 = make_leaf_mask_inner<ExteriorPRG, I, OutputsTuple>(seed0);
    auto mask1 = make_leaf_mask_inner<ExteriorPRG, I, OutputsTuple>(seed1);

    return dpf::subtract<output_type>(mask1, mask0);
HEDLEY_PRAGMA(GCC diagnostic pop)
}

template <typename ExteriorPRG,
          std::size_t I,
          typename InputT,
          typename ExteriorBlock,
          typename ...OutputTs>
auto make_leaf(InputT x, const ExteriorBlock & seed0, const ExteriorBlock & seed1, bool sign,
    OutputTs ...ys)
{
    using output_tuple_type = std::tuple<OutputTs...>;
    output_tuple_type output_tuple = std::make_tuple(ys...);
    using output_type = std::tuple_element_t<I, output_tuple_type>;
    output_type Y = std::get<I>(output_tuple);

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using node_type = typename ExteriorPRG::block_t;
    return sign ? dpf::subtract<output_type>(
                    make_naked_leaf<node_type>(x, Y),
                    make_leaf_mask<ExteriorPRG, I, output_tuple_type>(seed0, seed1))
                : dpf::subtract<output_type>(
                    make_leaf_mask<ExteriorPRG, I, output_tuple_type>(seed0, seed1),
                    make_naked_leaf<node_type>(x, Y));
HEDLEY_PRAGMA(GCC diagnostic pop)
}

template <typename ExteriorPRG,
          typename InputT,
          typename ExteriorBlock,
          typename ...OutputTs,
          std::size_t ...Is>
auto make_leaves_impl(InputT x, const ExteriorBlock & seed0, const ExteriorBlock & seed1,
    bool sign, std::index_sequence<Is...>, OutputTs ...ys)
{
    return std::make_tuple(make_leaf<ExteriorPRG, Is>(x, seed0, seed1, sign, ys...)...);
}

template <typename ExteriorPRG,
          typename InputT,
          typename ExteriorBlock,
          typename ...OutputTs,
          typename Indices = std::make_index_sequence<sizeof...(OutputTs)>>
auto make_leaves(InputT x, const ExteriorBlock & seed0, const ExteriorBlock & seed1,
    bool sign, OutputTs... ys)
{
    using node_type = typename ExteriorPRG::block_t;
    auto tup = make_leaves_impl<ExteriorPRG>(x, seed0, seed1, sign, Indices{}, ys...);

    // post-processing to secret-share any wildcard leaves
    // that is, after the call to `make_leaves_impl`, any values that were
    // should be `wildcards` will currently have a correction_word for `0` in
    // `tup`. Below is a glorified loop that creates two tuples from `tup`
    // (stored in the pair `ret`). For concrete types, it simply copies the
    // corresponding correction_words from `tup`; for the `wildcard`s, it
    // additively shares them.

    std::pair<decltype(tup), decltype(tup)> ret;

    // N.B.: Despite the nesting, the loops below advance in lockstep, making
    // only a single pass over each of the tuples being looped over

    // loop over the original inputs (to interrogate their types)
    std::apply([&ret, &tup](auto && ...types)
    {
        // loop over the elements of `tup`, our "template" for a leaf tuple
        std::apply([&ret, &types...](auto && ...args0)
        {
            // and also over the elements of `ret.first`, the first output
            std::apply([&ret, &types..., &args0...](auto && ...args1)
            {
                // and also `ret.second`, the secound output
                std::apply([&types..., &args0..., &args1...](auto && ...args2)
                {
                    // lambda to decide whether to copy the leaf (for concrete types)
                    // or whether to secret share it (for wildcard types)
                    ([](auto & type, auto & arg0, auto & arg1, auto & arg2)
                    {
                        using auto_type = typename std::remove_reference_t<decltype(type)>;
                        if constexpr (dpf::is_wildcard_v<auto_type>)
                        {
                            // secret share the value
                            dpf::uniform_fill(arg1);
                            arg2 = dpf::subtract<concrete_type_t<auto_type>>(arg0, arg1);
                        }
                        else
                        {
                            // copy concrete value
                            arg1 = arg0;
                            arg2 = arg0;
                        }
                    }(types, args0, args1, args2), ...);
                }, ret.second);
            }, ret.first);
        }, tup);
    }, std::make_tuple(ys...));

    return ret;
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_LEAF_NODE_HPP__
