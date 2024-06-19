/// @file grotto/prefix_parity.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @details
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_PREFIX_PARITY_HPP__
#define LIBDPF_INCLUDE_GROTTO_PREFIX_PARITY_HPP__

#include "dpf/dpf_key.hpp"
#include "dpf/bit.hpp"
#include "dpf/bitstring.hpp"
#include "dpf/twiddle.hpp"
#include "dpf/leaf_node.hpp"
#include "grotto/offset_iterable.hpp"
#include "dpf/path_memoizer.hpp"
#include "dpf/utils.hpp"

namespace grotto
{

template <typename NodeT,
          typename InputT>
auto parity_of_substring_prefix(NodeT & node, InputT x)
{
    static constexpr auto bits_per_limb = dpf::utils::bitlength_of_v<decltype(node[0])>;
    std::size_t off = dpf::offset_within_block<dpf::bit, NodeT>(x);
    auto div = std::lldiv(off, bits_per_limb);
    auto parity = node[div.quot] & ((1ul << div.rem) - 1ul);
    for (std::size_t i = 0; i < div.quot; ++i) parity ^= node[i];
    return psnip_builtin_parity64(parity);
}

template <bool use_early_terminate_optimization = true,
          typename InputT,
          typename DpfKey,
          std::size_t NumParts,
          std::enable_if_t<std::is_same_v<InputT, typename DpfKey::input_type>, bool> = false>
static auto prefix_parities(const DpfKey & dpf, std::array<InputT, NumParts> endpoints)
{
    static constexpr std::size_t num_parts = NumParts;
    static constexpr std::size_t depth = DpfKey::depth;
    using input_type = typename DpfKey::input_type;
    static constexpr std::size_t input_bits = dpf::utils::bitlength_of_v<input_type>;
    using interior_node = typename DpfKey::interior_node;
    using exterior_node = typename DpfKey::exterior_node;

    exterior_node leaf;
    auto path = make_basic_path_memoizer(dpf);
    std::array<uint_fast8_t, depth+1> direction = { 0 }; // always "traverse left" to get to the root
    std::array<uint_fast8_t, depth+1> parity    = { 0 };

    std::array<bool, num_parts> prefix_parities;

    // TODO is this handling early-terminate properly? full leaves come to mind.
    std::size_t new_first = for_each_offset(std::begin(endpoints), std::end(endpoints), dpf.offset_x(0),
        [&](std::size_t which_part, input_type current_endpoint)
        {
            // this will set path[0] = dpf.root and return next_level=1, since level 0 is now "done"
            // indeed, we did all the other prefix-parity parts manually in the initializers above
            auto next_level = path.assign_x(dpf, current_endpoint),
                to_level = dpf.depth, //input_bits-std::max(dpf.lg_outputs_per_leaf, std::size_t(psnip_builtin_ctz64(current_endpoint))),
                level_index = next_level - 1; //TODO to_level should be determined based on number of trailing zeros!

            DPF_UNROLL_LOOP
            for (auto mask = dpf.msb_mask >> level_index;
                 level_index < to_level; ++level_index, mask>>=1)
            {
                bool bit = !!(mask & current_endpoint);
                direction[level_index+1] = bit;
                path[level_index+1] = DpfKey::traverse_interior(path[level_index], dpf.correction_word(level_index, direction[level_index+1]), direction[level_index+1]);
                parity[level_index+1] = parity[level_index] ^ ((direction[level_index] ^ direction[level_index+1]) & dpf::get_lo_bit(path[level_index]));
            }
            if (HEDLEY_UNLIKELY(to_level != depth))
            {
                prefix_parities[which_part] = parity[level_index+1];
            }
            else
            {
                if (next_level <= depth)
                {
                    leaf = dpf.template traverse_exterior<0>(path[depth]);
                }

                prefix_parities[which_part] = parity[depth]
                    ^ (direction[depth] & dpf::get_lo_bit(path[depth])
                    ^ parity_of_substring_prefix(leaf, current_endpoint));
            }
        });

    return std::make_tuple(prefix_parities, new_first);
}

template <typename DpfKey,
          std::size_t NumParts>
static auto all_segment_parities_from_prefix_parities(const DpfKey & dpf,
    const std::array<bool, NumParts> & prefix_parities, std::size_t new_first)
{
    static constexpr std::size_t num_parts = NumParts;
    std::array<bool, num_parts> segment_parities;

    // currently setup so that for endpoints = {A, B, C}, segment_parities[0] = [A, B) while segment_parities[2] = [C, A)
    // to change to having the wrapping segment first, xor with previous prefix_parity and not subsequent one
    //     also need to change final xor to simply using new_first
    // segment_parities[0] = prefix_parities[0] ^ prefix_parities[num_parts-1];
    for (std::size_t i = 0; i < num_parts; ++i)
    {
        segment_parities[i] = prefix_parities[i] ^ prefix_parities[(i+1)%num_parts];
    }
    segment_parities[(new_first+num_parts-1)%num_parts] ^= dpf::get_lo_bit(dpf.root());

    return segment_parities;
}

template <typename DpfKey,
          std::size_t NumSegments,
          std::size_t NumParts>
static auto specific_segment_parities_from_prefix_parities(const DpfKey & dpf,
    const std::array<std::size_t, NumSegments> & segment_indices,
    const std::array<bool, NumParts> & prefix_parities, std::size_t new_first)
{
    assert(std::is_sorted(std::begin(segment_indices), std::end(segment_indices)));

    static constexpr std::size_t num_segments = NumSegments;
    static constexpr std::size_t num_parts = NumParts;
    std::array<bool, num_segments> segment_parities;

    for (std::size_t i = 0; i < num_segments; ++i)
    {
        segment_parities[i] = prefix_parities[segment_indices[i]] ^ prefix_parities[segment_indices[(i+1)%num_segments]];
    }
    auto begin = std::begin(segment_indices), end = std::end(segment_indices);
    segment_parities[(std::distance(begin, std::lower_bound(begin, end, new_first))+num_segments-1)%num_segments] ^= dpf::get_lo_bit(dpf.root());

    return segment_parities;
}

template <bool use_early_terminate_optimization = true,
          typename InputT,
          typename DpfKey,
          std::size_t NumParts,
          std::enable_if_t<std::is_same_v<InputT, typename DpfKey::input_type>, bool> = false>
static auto segment_parities(const DpfKey & dpf, const std::array<InputT, NumParts> & endpoints)
{
    // TODO: add special cases to check if `NumParts` is:
    //     0 -> return empty array
    //     1 -> return dpf::get_lo_bit(dpf.root())
    auto [prefix_parities, new_first] = grotto::prefix_parities<use_early_terminate_optimization>(dpf, endpoints);
    return all_segment_parities_from_prefix_parities(dpf, prefix_parities, new_first);
}

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_PREFIX_PARITY_HPP__
