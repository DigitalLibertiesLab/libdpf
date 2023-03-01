/// @file dpf/leaf_node.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief miscellaneous helper functions, structs, preprocessor directives
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_LEAF_NODE_HPP__
#define LIBDPF_INCLUDE_DPF_LEAF_NODE_HPP__

#include <utility>
#include <cstddef>
#include <cstring>
#include <functional>
#include <tuple>

#include "simde/simde/x86/avx2.h"

#include <dpf/bit.hpp>
#include <dpf/xor_wrapper.hpp>
#include <dpf/wildcard.hpp>
#include <dpf/leaf_arithmetic.hpp>
#include <dpf/utils.hpp>
#include <dpf/random.hpp>

namespace dpf
{

/// @brief `value` is `true` if multiple leaves are packed into each leaf node
template <typename output_t, typename node_t>
using is_packable = std::bool_constant<
    std::not_equal_to<>{}(utils::bitlength_of_v<output_t>, utils::bitlength_of_v<node_t>) &&
    std::equal_to<>{}(utils::bitlength_of_v<node_t> % utils::bitlength_of_v<output_t>, 0)>;

template <typename output_t, typename node_t>
static constexpr bool is_packable_v = is_packable<output_t, node_t>::value;

template <typename output_t, typename node_t>
struct outputs_per_leaf
    : public std::integral_constant<std::size_t,
        !is_packable_v<output_t, node_t> ? 1 :
            utils::bitlength_of_v<node_t> / utils::bitlength_of_v<output_t>> { };

template <typename output_t, typename node_t>
static constexpr std::size_t outputs_per_leaf_v
    = outputs_per_leaf<output_t, node_t>::value;

template <typename output_t, typename node_t>
static constexpr std::size_t lg_outputs_per_leaf_v
    = std::log2(outputs_per_leaf<output_t, node_t>::value);

template <typename output_t, typename node_t>
struct block_length_of_leaf : std::integral_constant<std::size_t,
                                !is_packable_v<output_t, node_t> ? 1 :
                                    utils::quotient_ceiling(
                                        utils::bitlength_of_v<output_t>,
                                        utils::bitlength_of_v<node_t>)
                                    >{ };

template <typename output_t, typename node_t>
static constexpr std::size_t block_length_of_leaf_v
    = block_length_of_leaf<output_t, node_t>::value;

template <typename output_t, typename node_t>
constexpr std::size_t offset_within_block(std::size_t x) noexcept
{
    return x % dpf::outputs_per_leaf_v<output_t, node_t>;
}

template <std::size_t I, typename N, std::size_t I_, typename outputs_t>
struct block_offset_of_leaf
{
    static constexpr std::size_t value = dpf::block_length_of_leaf_v<std::tuple_element_t<I_, outputs_t>, N>
        + block_offset_of_leaf<I, N, I_+1, outputs_t>::value;
};

template <std::size_t I, typename N, typename outputs_t>
struct block_offset_of_leaf<I, N, I, outputs_t>
{
    static constexpr std::size_t value = 0;
};

template <std::size_t I, typename N, typename outputs_t>
inline constexpr std::size_t block_offset_of_leaf_v
    = block_offset_of_leaf<I, N, 0, outputs_t>::value;


template <typename node_t,
          typename output_t,
          std::size_t block_len = block_length_of_leaf_v<output_t, node_t>>
struct leaf_node
{
    static_assert(block_len == block_length_of_leaf_v<output_t, node_t>);
    using type = std::array<node_t, block_len>;
};

template <typename node_t,
          typename output_t>
struct leaf_node<node_t, output_t, 1>
{
    static_assert(1 == block_length_of_leaf_v<output_t, node_t>);
    using type = node_t;
};

template <typename node_t,
          typename output_t>
using leaf_node_t = typename leaf_node<node_t, output_t>::type;

template <typename node_t,
          typename output_t,
          typename... output_ts>
struct leaf_tuple
{
    using type = std::tuple<leaf_node_t<node_t, output_t>,
                            leaf_node_t<node_t, output_ts>...>;
};
template <typename node_t,
          typename output_t,
          typename... output_ts>
using leaf_tuple_t = typename leaf_tuple<node_t, output_t, output_ts...>::type;

template <typename node_t, typename output_t>
static output_t extract_leaf(const leaf_node_t<node_t, output_t> & leaf, std::size_t x) noexcept
{
    auto off = offset_within_block<output_t, node_t>(x);

    output_t y;
    if constexpr (std::is_same_v<output_t, dpf::bit>)
    {
        auto yy = simde_mm_set_epi64x(uint64_t(off <= 63) << (off % 64),
                                      uint64_t(off >= 64) << (off % 64));
        y = dpf::to_bit(simde_mm_testz_si128(leaf, yy));
    }
    else
    {
        std::memcpy(&y, reinterpret_cast<const output_t *>(&leaf) + off, sizeof(y));
    }

    return y;
}

template <typename node_t, typename input_t, typename output_t>
auto make_naked_leaf(input_t x, output_t y)
{
    using leaf_t = dpf::leaf_node_t<node_t, output_t>;
    auto off = offset_within_block<output_t, node_t>(x);
    leaf_t Y{0};
    if constexpr (std::is_same_v<output_t, dpf::bit>)
    {
        Y = simde_mm_set_epi64x(uint64_t(off<=63 ? y : dpf::bit::zero) << (off % 64),
                                uint64_t(off>=64 ? y : dpf::bit::zero) << (off % 64));
    }
    else
    {
        std::memcpy(reinterpret_cast<output_t *>(&Y) + off, &y, sizeof(y));
    }

    return Y;
}

template <typename prg_t, std::size_t I, typename seed_t,
    typename outputs_t>
auto make_leaf_mask_inner(const seed_t & seed)
{
    using node_t = typename prg_t::block_t;
    using output_t = std::tuple_element_t<I, outputs_t>;
    using leaf_t = dpf::leaf_node_t<node_t, output_t>;

    // if constexpr(std::tuple_size_v<outputs_t> == 1
    //     && dpf::block_length_of_leaf_v<output_t, node_t> == 1)
    // {
    //     return seed;
    // }
    // else
    {
        auto count = dpf::block_length_of_leaf_v<output_t, node_t>;
        auto pos = dpf::block_offset_of_leaf_v<I, node_t, outputs_t>;
        leaf_t output;
        prg_t::eval(seed, &output, count, pos);

        return output;
    }
}

template <typename prg_t, std::size_t I, typename seed_t,
    typename... output_ts>
auto make_leaf_mask(const seed_t & seed0, const seed_t & seed1, output_ts...)
{
    using node_t = typename prg_t::block_t;
    using outputs_t = std::tuple<output_ts...>;
    using output_t = std::tuple_element_t<I, outputs_t>;

    auto mask0 = make_leaf_mask_inner<prg_t, I, node_t, outputs_t>(seed0);
    auto mask1 = make_leaf_mask_inner<prg_t, I, node_t, outputs_t>(seed1);

    return dpf::subtract<output_t, node_t>(mask1, mask0);
}

template <typename prg_t, std::size_t I, typename input_t, typename seed_t,
    typename... output_ts>
auto make_leaf(input_t x, const seed_t & seed0, const seed_t & seed1, bool sign,
    output_ts...ys)
{
    auto tuple = std::make_tuple(ys...);
    using type = std::tuple_element_t<I, std::tuple<output_ts...>>;
    auto Y = std::get<I>(tuple);

    using node_t = typename prg_t::block_t;
    return sign ? dpf::subtract<type, node_t>(
                    make_naked_leaf<node_t>(x, Y),
                    make_leaf_mask<prg_t, I>(seed0, seed1, ys...))
                : dpf::subtract<type, node_t>(
                    make_leaf_mask<prg_t, I>(seed0, seed1, ys...),
                    make_naked_leaf<node_t>(x, Y));
}

template <typename prg_t, typename input_t, typename seed_t,
    typename... output_ts, std::size_t... Is>
auto make_leaves_impl(input_t x, const seed_t & seed0, const seed_t & seed1,
    bool sign, std::index_sequence<Is...>, output_ts... ys)
{
    return std::make_tuple(make_leaf<prg_t, Is>(x, seed0, seed1, sign, ys...)...);
}

template <typename prg_t, typename input_t, typename seed_t, typename... output_ts,
          typename Indices = std::make_index_sequence<sizeof...(output_ts)>>
auto make_leaves(input_t x, const seed_t & seed0, const seed_t & seed1,
    bool sign, output_ts... ys)
{
    using node_t = typename prg_t::block_t;
    auto tup = make_leaves_impl<prg_t>(x, seed0, seed1, sign, Indices{}, ys...);
    std::pair<decltype(tup), decltype(tup)> ret;

    // post-processing to secret-share any wildcard leaves
    std::apply([&ret](auto &&... args0)
    {
        std::apply([&ret, &args0...](auto &&... args1)
        {
            std::apply([&ret, &args0..., &args1...](auto &&... args2)
            {
                ([](auto & arg0, auto & arg1, auto & arg2)
                {
                    using auto_type = typename std::remove_reference_t<decltype(arg0)>;
                    if constexpr (dpf::is_wildcard_v<auto_type>)
                    {
                        dpf::uniform_fill(arg1);
                        arg2 = dpf::subtract<actual_type_t<auto_type>, node_t>(arg0, arg1);
                    }
                    else
                    {
                        arg1 = arg0;
                        arg2 = arg0;
                    }
                }(args0, args1, args2), ...);
            }, ret.second);
        }, ret.first);
    }, tup);

    return ret;
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_LEAF_NODE_HPP__
