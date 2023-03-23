/// @file dpf/dpf_key.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

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

template <typename I>
using root_sampler_t = std::add_pointer_t<typename I::block_t()>;

template <class InteriorPRG,
          class ExteriorPRG,
          typename InputT,
          typename OutputT,
          typename... OutputTs>
struct dpf_key
{
  public:
    using interior_prg_type = InteriorPRG;
    using interior_node = typename InteriorPRG::block_t;
    using exterior_prg_type = ExteriorPRG;
    using exterior_node = typename ExteriorPRG::block_t;
    using input_type = InputT;
    using outputs_tuple = std::tuple<OutputT, OutputTs...>;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using leaf_tuple = dpf::leaf_tuple_t<exterior_node,
        OutputT, OutputTs...>;
HEDLEY_PRAGMA(GCC diagnostic pop)
    static constexpr std::size_t depth = utils::bitlength_of_v<input_type>
        - dpf::lg_outputs_per_leaf_v<OutputT, exterior_node>;
    static constexpr input_type msb_mask = utils::msb_of_v<input_type>;
    static constexpr std::size_t outputs_per_leaf = dpf::outputs_per_leaf_v<OutputT, exterior_node>;
    static constexpr std::size_t lg_outputs_per_leaf = dpf::lg_outputs_per_leaf_v<OutputT, exterior_node>;

    static_assert(std::conjunction_v<std::is_trivially_copyable<OutputT>,
                                     std::is_trivially_copyable<OutputTs>...>,
        "all output types must be trivially copyable");
    static_assert(std::has_unique_object_representations_v<input_type>);

    HEDLEY_ALWAYS_INLINE
    constexpr dpf_key(interior_node root_,
                      const std::array<interior_node, depth> & interior_cws_,
                      const std::array<uint8_t, depth> & correction_advice_,
                      const leaf_tuple & exterior_cw_,
                      const std::bitset<sizeof...(OutputTs)+1> & wild_mask_)
      : wildcard_mask{wild_mask_},
        mutable_exterior_cw{exterior_cw_},
        root{root_},
        correction_words{interior_cws_},
        correction_advice{correction_advice_}
    { }

    std::bitset<sizeof...(OutputTs)+1> wildcard_mask;
    leaf_tuple mutable_exterior_cw;
  private:

  public:
    const interior_node root;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    const std::array<interior_node, depth> correction_words;
HEDLEY_PRAGMA(GCC diagnostic pop)
    const std::array<uint8_t, depth> correction_advice;

    HEDLEY_ALWAYS_INLINE
    bool is_wildcard(std::size_t i) const
    {
        return wildcard_mask.test(i);
    }

    std::string wildcard_bitmask() const
    {
        return wildcard_mask.to_string();
    }

    template <std::size_t I>
    HEDLEY_ALWAYS_INLINE
    const auto & exterior_cw() const
    {
        return std::get<I>(mutable_exterior_cw);
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    static auto traverse_interior(const interior_node & node,
        const interior_node & cw, bool dir) noexcept
    {
        return dpf::xor_if_lo_bit(
            interior_prg_type::eval(unset_lo_2bits(node), dir), cw, node);
    }

    template <std::size_t I>
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    static auto traverse_exterior(const interior_node & node,
        const exterior_node & cw) noexcept
    {
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
        using output_t = std::tuple_element_t<I, outputs_tuple>;
        return dpf::subtract<output_t, exterior_node>(
            make_leaf_mask_inner<exterior_prg_type, I, exterior_node,
                outputs_tuple>(unset_lo_2bits(node)),
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

template <class InteriorPRG = dpf::prg::aes128,
          class ExteriorPRG = InteriorPRG,
          dpf::root_sampler_t<InteriorPRG> RootSampler
              = &dpf::basic_uniform_root_sampler,
          typename InputT,
          typename OutputT,
          typename... OutputTs>
auto make_dpf(InputT x, OutputT y, OutputTs... ys)
{
    using dpf_type = dpf_key<InteriorPRG, ExteriorPRG, InputT,
                                   OutputT, OutputTs...>;
    using interior_node = typename dpf_type::interior_node;

    constexpr auto depth = dpf_type::depth;
    InputT mask = dpf_type::msb_mask;

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
            dpf::get_lo_bit(child[0]) ^ !bit,
            dpf::get_lo_bit(child[1]) ^ bit
        };
        auto cw = dpf::set_lo_bit(child[!bit], t[bit]);
        parent[0] = dpf::xor_if(child0[bit], cw, advice[0]);
        parent[1] = dpf::xor_if(child1[bit], cw, advice[1]);

        correction_words[level] = child[!bit];
        correction_advice[level] = uint_fast8_t(t[1] << 1) | t[0];
    }
    auto wildcard_mask = dpf::utils::make_bitset(dpf::is_wildcard_v<OutputT>,
        dpf::is_wildcard_v<OutputTs>...);

    bool sign0 = dpf::get_lo_bit(parent[0]);
    // bool sign1 = dpf::get_lo_bit(parent[1]);

    auto leaves = dpf::make_leaves<ExteriorPRG>(x, unset_lo_2bits(parent[0]),
        unset_lo_2bits(parent[1]), sign0, y, ys...);

    return std::make_pair(
        dpf_type{root[0], correction_words, correction_advice,
            leaves.first, wildcard_mask},
        dpf_type{root[1], correction_words, correction_advice,
            leaves.second, wildcard_mask});
}  // make_dpf

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_DPF_KEY_HPP__
