/// @file dpf/dpf_key.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_DPF_KEY_HPP__
#define LIBDPF_INCLUDE_DPF_DPF_KEY_HPP__

#include <array>
#include <bitset>
#include <tuple>
#include <functional>

#include "dpf/prg_aes.hpp"
#include "dpf/wildcard.hpp"
#include "dpf/twiddle.hpp"
#include "dpf/leaf_node.hpp"
#include "dpf/aligned_allocator.hpp"

namespace dpf
{

template <typename InteriorPRG>
using root_sampler_t = std::add_pointer_t<typename InteriorPRG::block_type()>;

template <typename InteriorPRG,
          typename ExteriorPRG,
          typename InputT,
          typename OutputT,
          typename ...OutputTs>
struct dpf_key
{
  public:
    using interior_prg = InteriorPRG;
    using interior_node = typename InteriorPRG::block_type;

    using exterior_prg = ExteriorPRG;
    using exterior_node = typename ExteriorPRG::block_type;

    using input_type = InputT;
    using integral_type = utils::integral_type_from_bitlength_t<utils::bitlength_of_v<input_type>, utils::bitlength_of_v<std::size_t>>;

    using outputs_tuple = std::tuple<OutputT, OutputTs...>;
    template <std::size_t I>
    using output_type = std::tuple_element_t<I, outputs_tuple>;

    using concrete_outputs_tuple
        = std::tuple<concrete_type_t<OutputT>, concrete_type_t<OutputTs>...>;
    template <std::size_t I>
    using concrete_output_type = std::tuple_element_t<I, concrete_outputs_tuple>;

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using leaf_tuple = dpf::leaf_tuple_t<exterior_node, OutputT, OutputTs...>;
    using beaver_tuple = dpf::beaver_tuple_t<exterior_node, OutputT, OutputTs...>;
HEDLEY_PRAGMA(GCC diagnostic pop)

    static constexpr std::size_t outputs_per_leaf = dpf::outputs_per_leaf_v<OutputT, exterior_node>;
    static constexpr std::size_t lg_outputs_per_leaf = dpf::lg_outputs_per_leaf_v<OutputT, exterior_node>;
    static constexpr std::size_t depth
        = utils::bitlength_of_v<input_type> - lg_outputs_per_leaf;
    static constexpr auto msb_mask = utils::msb_of_v<input_type>;

    static_assert(((dpf::utils::bitlength_of_v<OutputT> == dpf::utils::bitlength_of_v<OutputTs>) && ...),
        "all output types must be the same length");
    static_assert(std::conjunction_v<std::is_trivially_copyable<OutputT>,
                                     std::is_trivially_copyable<OutputTs>...>,
        "all output types must be trivially copyable");
    static_assert(std::conjunction_v<std::is_standard_layout<OutputT>,
                                     std::is_standard_layout<OutputTs>...>,
        "all output types must be standard layout");
    // static_assert(std::has_unique_object_representations_v<input_type>);

    HEDLEY_ALWAYS_INLINE
    constexpr dpf_key(interior_node root_,
                      const std::array<interior_node, depth> & correction_words_,
                      const std::array<uint8_t, depth> & correction_advice_,
                      const leaf_tuple & leaves,
                      const std::bitset<sizeof...(OutputTs)+1> & wild_mask_,
                      beaver_tuple && beavers_)
      : mutable_wildcard_mask{wild_mask_},
        mutable_leaf_tuple{leaves},
        mutable_beaver_tuple{std::forward<beaver_tuple>(beavers_)},
        root{root_},
        correction_words{correction_words_},
        correction_advice{correction_advice_},
        common_part_hash{utils::get_common_part_hash(correction_words, correction_advice)}
    { }
    dpf_key(const dpf_key &) = delete;
    dpf_key(dpf_key &&) = default;

  private:
    std::bitset<sizeof...(OutputTs)+1> mutable_wildcard_mask;
    leaf_tuple mutable_leaf_tuple;
    beaver_tuple mutable_beaver_tuple;

  public:
    const interior_node root;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    const std::array<interior_node, depth> correction_words;
HEDLEY_PRAGMA(GCC diagnostic pop)
    const std::array<uint8_t, depth> correction_advice;
    const interior_node common_part_hash;

#ifndef LIBDPF_HAS_ASIO
    template <std::size_t I = 0,
            typename OutputType,
            typename StreamT,
            typename CompletionToken>
    auto async_assign_leaf(StreamT & peer, const OutputType & output,
        CompletionToken && token);
#else
    #include "dpf_key_asio.hpp"
#endif  // LIBDPF_HAS_ASIO

    HEDLEY_ALWAYS_INLINE
    bool is_wildcard(std::size_t i) const
    {
        return mutable_wildcard_mask.test(i);
    }

    std::string wildcard_bitmask() const
    {
        return mutable_wildcard_mask.to_string();
    }

    template <std::size_t I>
    HEDLEY_ALWAYS_INLINE
    const auto & leaf() const
    {
        return std::get<I>(mutable_leaf_tuple);
    }

    HEDLEY_ALWAYS_INLINE
    const auto & leaves() const
    {
        return mutable_leaf_tuple;
    }

    template <std::size_t I = 0>
    HEDLEY_ALWAYS_INLINE
    const auto & beaver() const
    {
        return std::get<I>(mutable_beaver_tuple);
    }

    HEDLEY_ALWAYS_INLINE
    const auto & beavers() const
    {
        return mutable_beaver_tuple;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    static auto traverse_interior(const interior_node & node,
        const interior_node & cw, bool dir) noexcept
    {
        return dpf::xor_if_lo_bit(
            interior_prg::eval(unset_lo_2bits(node), dir), cw, node);
    }

    template <std::size_t I = 0,
              typename LeafT>
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    static auto traverse_exterior(const interior_node & node,
        const LeafT & cw) noexcept
    {
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
        using output_type = std::tuple_element_t<I, outputs_tuple>;
        return dpf::subtract_leaf<output_type>(
            make_leaf_mask_inner<exterior_prg, I, outputs_tuple>(unset_lo_2bits(node)),
            dpf::get_if_lo_bit(cw, node));
HEDLEY_PRAGMA(GCC diagnostic pop)
    }
};  // struct dpf_key

template <typename T>
static T basic_uniform_root_sampler()
{
    T ret = dpf::uniform_fill(ret);
    return ret;
}

namespace utils
{

template <typename InteriorPRG,
          typename ExteriorPRG,
          typename InputT,
          typename OutputT,
          typename ...OutputTs>
struct dpf_type
{
    using type = dpf_key<InteriorPRG, ExteriorPRG, InputT,
                                   OutputT, OutputTs...>;
};

template <typename InteriorPRG,
          typename ExteriorPRG,
          typename InputT,
          typename OutputT,
          typename ...OutputTs>
using dpf_type_t = typename dpf_type<InteriorPRG, ExteriorPRG, InputT, OutputT, OutputTs...>::type;

}  // namespace utils

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          dpf::root_sampler_t<InteriorPRG> RootSampler
              = &dpf::basic_uniform_root_sampler,
          typename InputT,
          typename OutputT = dpf::bit,
          typename ...OutputTs>
auto make_dpf(InputT x, OutputT y = dpf::bit::one, OutputTs ...ys)
{
    using dpf_type = utils::dpf_type_t<InteriorPRG, ExteriorPRG, InputT,
                                       OutputT, OutputTs...>;
    using interior_node = typename dpf_type::interior_node;

    constexpr auto depth = dpf_type::depth;
    auto mask = dpf_type::msb_mask;

    const interior_node root[2] = {
        dpf::unset_lo_bit(RootSampler()),
        dpf::set_lo_bit(RootSampler())
    };

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    std::array<interior_node, depth> correction_words;
HEDLEY_PRAGMA(GCC diagnostic pop)
    std::array<uint8_t, depth> correction_advice;

    interior_node parent[2] = { root[0], root[1] };
    bool advice[2];

    for (std::size_t level = 0; level < depth; ++level, mask >>= 1)
    {
        bool bit = !!(mask & x);

        advice[0] = dpf::get_lo_bit_and_clear_lo_2bits(parent[0]);
        advice[1] = dpf::get_lo_bit_and_clear_lo_2bits(parent[1]);

        auto child0 = InteriorPRG::eval01(parent[0]);
        auto child1 = InteriorPRG::eval01(parent[1]);
        interior_node child[2] = {
            child0[0] ^ child1[0],
            child0[1] ^ child1[1]
        };

        bool t[2] = {
            static_cast<bool>(dpf::get_lo_bit(child[0]) ^ !bit),
            static_cast<bool>(dpf::get_lo_bit(child[1]) ^ bit)
        };
        auto cw = dpf::set_lo_bit(child[!bit], t[bit]);
        parent[0] = dpf::xor_if(child0[bit], cw, advice[0]);
        parent[1] = dpf::xor_if(child1[bit], cw, advice[1]);

        correction_words[level] = child[!bit];
        correction_advice[level] = uint_fast8_t(t[1] << 1) | t[0];
    }
    auto mutable_wildcard_mask = dpf::utils::make_bitset(dpf::is_wildcard_v<OutputT>,
        dpf::is_wildcard_v<OutputTs>...);

    bool sign0 = dpf::get_lo_bit(parent[0]);
    // bool sign1 = dpf::get_lo_bit(parent[1]);

    auto [pair0, pair1] = dpf::make_leaves<ExteriorPRG>(x, unset_lo_2bits(parent[0]),
        unset_lo_2bits(parent[1]), sign0, y, ys...);
    auto & [leaves0, beavers0] = pair0;
    auto & [leaves1, beavers1] = pair1;

    return std::make_pair(
        dpf_type{root[0], correction_words, correction_advice,
            leaves0, mutable_wildcard_mask, std::move(beavers0)},
        dpf_type{root[1], correction_words, correction_advice,
            leaves1, mutable_wildcard_mask, std::move(beavers1)});
}  // make_dpf

// template <typename PeerT,
//           typename CompletionToken,
//           typename InteriorPRG = dpf::prg::aes128,
//           typename ExteriorPRG = InteriorPRG,
//           dpf::root_sampler_t<InteriorPRG> RootSampler
//               = &dpf::basic_uniform_root_sampler,
//           typename InputT,
//           typename OutputT,
//           typename ...OutputTs>
// auto make_dpf_send(PeerT & peer0, PeerT & peer1, CompletionToken && token, InputT x, OutputT y = dpf::bit::one, OutputTs ...ys)
// {
//     auto [dpf0, dpf1] = dpf::make_dpf(x, y, ys...);
//     return std::make_tuple(
//         std::move(ds.first),
//         std::move(ds.second),
//         ds.first.async_send_dpf(peer0, token),
//         ds.second.async_send_dpf(peer1, token));
// }

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          typename InputT,
          typename OutputT = dpf::bit,
          typename ...OutputTs>
auto deduce_dpf_type(InputT x, OutputT y = dpf::bit::one, OutputTs ...ys)
{
    return utils::dpf_type<InteriorPRG, ExteriorPRG, InputT, OutputT, OutputTs...>{};
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_DPF_KEY_HPP__
