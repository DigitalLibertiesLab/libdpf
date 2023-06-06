/// @file dpf/leaf_node.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_LEAF_NODE_HPP__
#define LIBDPF_INCLUDE_DPF_LEAF_NODE_HPP__

#include <cstddef>
#include <cmath>
#include <cstring>
#include <type_traits>
#include <utility>
#include <memory>
#include <functional>
#include <tuple>
#include <atomic>
#include <array>

#include "thirdparty/simde/x86/avx2.h"

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
    std::less<>{}(utils::bitlength_of_output_v<OutputT, NodeT>, utils::bitlength_of_output_v<NodeT, NodeT>) &&
    std::equal_to<>{}(utils::bitlength_of_output_v<NodeT, NodeT> % utils::bitlength_of_output_v<OutputT, NodeT>, 0)>;

template <typename OutputT,
          typename NodeT>
static constexpr bool is_packable_v = is_packable<OutputT, NodeT>::value;

template <typename OutputT,
          typename NodeT>
struct outputs_per_leaf
    : public std::integral_constant<std::size_t,
        !is_packable_v<OutputT, NodeT> ? 1 :
            utils::bitlength_of_output_v<NodeT, NodeT> / utils::bitlength_of_output_v<OutputT, NodeT>> { };

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
                                        utils::bitlength_of_output_v<OutputT, NodeT>,
                                        utils::bitlength_of_output_v<NodeT, NodeT>)
                                    >{ };

template <typename OutputT,
          typename NodeT>
static constexpr std::size_t block_length_of_leaf_v
    = block_length_of_leaf<OutputT, NodeT>::value;

template <typename OutputT,
          typename NodeT,
          typename InputT>
constexpr std::size_t offset_within_block(InputT x) noexcept
{
    constexpr auto mod = utils::mod_pow_2<InputT>{};
    return mod(x, dpf::lg_outputs_per_leaf_v<OutputT, NodeT>);
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

template <bool isWildcard,
          typename NodeT,
          typename OutputT,
          std::size_t outputs_per_leaf = outputs_per_leaf_v<OutputT, NodeT>>
struct beaver final { };

template <typename NodeT,
          typename OutputT>
struct beaver<true, NodeT, OutputT, 1> final
{
    static_assert(1 == outputs_per_leaf_v<OutputT, NodeT>);
    beaver() : is_locked{new std::atomic_flag{ATOMIC_FLAG_INIT}} { }
    beaver(beaver &&) = default;
    std::unique_ptr<std::atomic_flag> is_locked;
};

template <typename NodeT,
          typename OutputT,
          std::size_t outputs_per_leaf>
struct beaver<true, NodeT, OutputT, outputs_per_leaf> final
{
    static_assert(outputs_per_leaf == outputs_per_leaf_v<OutputT, NodeT>);
    beaver() : is_locked{new std::atomic_flag{ATOMIC_FLAG_INIT}} { }

    std::unique_ptr<std::atomic_flag> is_locked;
    OutputT output_blind;
    NodeT vector_blind;
    NodeT blinded_vector;
};

template <typename NodeT,
          typename OutputT,
          typename ...OutputTs>
struct beaver_tuple
{
    using type = std::tuple<beaver<is_wildcard_v<OutputT>, NodeT, concrete_type_t<OutputT>>,
                            beaver<is_wildcard_v<OutputTs>, NodeT, concrete_type_t<OutputTs>>...>;
};
template <typename NodeT,
          typename OutputT,
          typename ...OutputTs>
using beaver_tuple_t = typename beaver_tuple<NodeT, OutputT, OutputTs...>::type;

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
    using node_type = typename ExteriorPRG::block_type;
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
    using node_type = typename ExteriorPRG::block_type;
    using output_type = concrete_type_t<std::tuple_element_t<I, OutputsTuple>>;

    auto mask0 = make_leaf_mask_inner<ExteriorPRG, I, OutputsTuple>(seed0);
    auto mask1 = make_leaf_mask_inner<ExteriorPRG, I, OutputsTuple>(seed1);

    return dpf::subtract_leaf<output_type>(mask1, mask0);
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
    using node_type = typename ExteriorPRG::block_type;
    return sign ? dpf::subtract_leaf<concrete_type_t<output_type>>(
                    make_naked_leaf<node_type>(x, Y),
                    make_leaf_mask<ExteriorPRG, I, output_tuple_type>(seed0, seed1))
                : dpf::subtract_leaf<concrete_type_t<output_type>>(
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
          typename OutputT,
          typename ...OutputTs,
          typename Indices = std::make_index_sequence<1+sizeof...(OutputTs)>>
auto make_leaves(InputT x, const ExteriorBlock & seed0, const ExteriorBlock & seed1,
    bool sign, OutputT y, OutputTs ...ys)
{
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using node_type = typename ExteriorPRG::block_type;
    using leaf_type = dpf::leaf_tuple_t<node_type, OutputT, OutputTs...>;
    using beaver_type = dpf::beaver_tuple_t<node_type, OutputT, OutputTs...>;
HEDLEY_PRAGMA(GCC diagnostic pop)

    leaf_type leaves = make_leaves_impl<ExteriorPRG>(x, seed0, seed1, sign, Indices{}, y, ys...);
    beaver_type beavers{};

    // post-processing to secret-share any wildcard leaves
    // that is, after the call to `make_leaves_impl`, any values that were
    // should be `wildcards` will currently have a correction_word for `0` in
    // `leaves`. Below is a glorified loop that creates two tuples from `leaves`
    // (stored in the pair `return_tuple`). For concrete output_types, it simply copies the
    // corresponding correction_words from `leaves`; for the `wildcard`s, it
    // additively shares them.

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    std::pair<
        std::pair<leaf_type, beaver_type>,
        std::pair<leaf_type, beaver_type> > return_tuple;

    // N.B.: Despite the nesting, the loops below advance in lockstep, making
    // only a single pass over each of the tuples being looped over

    // loop over the original inputs (to interrogate their output_types)
    std::apply([x, &sign, &return_tuple, &leaves](auto && ...output_types)
    {
        // loop over the elements of `leaves`, our "template" for a leaf tuple
        std::apply([x, &sign, &return_tuple, &output_types...](auto && ...args0)
        {
            // and also over the elements of `return_tuple.first.first`, the first leaf tuple
            std::apply([x, &sign, &return_tuple, &output_types..., &args0...](auto && ...leaves0)
            {
                // and also `return_tuple.second.first`, the secound leaf tuple
                std::apply([x, &sign, &return_tuple, &output_types..., &args0..., &leaves0...](auto && ...leaves1)
                {
                    // plus `return_tuple.first.second`, the first beaver tuple
                    std::apply([x, &sign, &return_tuple, &output_types..., &args0..., &leaves0..., &leaves1...](auto && ...beavers0)
                    {
                        // and `return_tuple.second.second`, the secound beaver tuple
                        std::apply([x, &sign, &output_types..., &args0..., &leaves0..., &leaves1..., &beavers0...](auto && ...beavers1)
                        {
                            // lambda to decide whether to copy the leaf (for concrete output_types)
                            // or whether to secret share it (for wildcard output_types)
                            ([](auto & x, auto & type, auto & leaf, auto & leaf0, auto & leaf1, auto & beaver0, auto & beaver1, bool sign)
                            {
                                using output_type = typename std::remove_reference_t<decltype(type)>;
                                if constexpr(dpf::is_wildcard_v<output_type>)
                                {
                                    using concrete_type = dpf::concrete_type_t<output_type>;
                                    // secret share the value
                                    dpf::uniform_fill(leaf0);
                                    leaf1 = dpf::subtract_leaf<concrete_type>(leaf, leaf0);
                                    // also initialize the beavers
                                    if constexpr(dpf::outputs_per_leaf_v<concrete_type, node_type> > 1)
                                    {
                                        dpf::leaf_node_t<node_type, concrete_type> vector;
                                        if constexpr(utils::is_xor_wrapper_v<decltype(x)> == true)
                                        {
                                            vector = make_naked_leaf<node_type>(x, concrete_type(~0));
                                        }
                                        else
                                        {
                                            vector = make_naked_leaf<node_type>(x, concrete_type(2*sign-1));
                                        }

                                        uniform_fill(beaver0.output_blind);
                                        uniform_fill(beaver0.vector_blind);

                                        uniform_fill(beaver1.output_blind);
                                        uniform_fill(beaver1.vector_blind);

                                        beaver0.blinded_vector = dpf::add_leaf<output_type>(vector, beaver1.vector_blind);
                                        beaver1.blinded_vector = dpf::add_leaf<output_type>(vector, beaver0.vector_blind);

                                        leaf0 = dpf::add_leaf<output_type>(leaf0,
                                            dpf::multiply_leaf(beaver0.vector_blind, beaver1.output_blind));
                                        leaf1 = dpf::add_leaf<output_type>(leaf1,
                                            dpf::multiply_leaf(beaver1.vector_blind, beaver0.output_blind));
                                    }
                                }
                                else
                                {
                                    // copy concrete value; beaver is a trivial type
                                    leaf0 = leaf;
                                    leaf1 = leaf;
                                }
                            }(x, output_types, args0, leaves0, leaves1, beavers0, beavers1, sign), ...);
                        }, return_tuple.second.second);
                    }, return_tuple.first.second);
                }, return_tuple.second.first);
            }, return_tuple.first.first);
        }, leaves);
    }, std::make_tuple(y, ys...));

HEDLEY_PRAGMA(GCC diagnostic pop)

    return return_tuple;
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_LEAF_NODE_HPP__
