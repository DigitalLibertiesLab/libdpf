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
    using interior_prg_t = InteriorPRG;
    using interior_node_t = typename InteriorPRG::block_t;
    using exterior_prg_t = ExteriorPRG;
    using exterior_node_t = typename ExteriorPRG::block_t;
    using input_type = InputT;
    using integral_type = utils::integral_type_from_bitlength_t<utils::bitlength_of_v<input_type>>;
    using outputs_t = std::tuple<OutputT, OutputTs...>;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using leaf_nodes_t = dpf::leaf_tuple_t<exterior_node_t,
        OutputT, OutputTs...>;
HEDLEY_PRAGMA(GCC diagnostic pop)
    static constexpr std::size_t depth = utils::bitlength_of_v<input_type>
        - dpf::lg_outputs_per_leaf_v<OutputT, exterior_node_t>;
    static constexpr input_type msb_mask = utils::msb_of_v<input_type>;
    static constexpr std::size_t outputs_per_leaf = dpf::outputs_per_leaf_v<OutputT, exterior_node_t>;
    static constexpr std::size_t lg_outputs_per_leaf = dpf::lg_outputs_per_leaf_v<OutputT, exterior_node_t>;

    static_assert(std::conjunction_v<std::is_trivially_copyable<OutputT>,
                                     std::is_trivially_copyable<OutputTs>...>,
        "all output types must be trivially copyable");
    static_assert(std::has_unique_object_representations_v<input_type>);

    HEDLEY_ALWAYS_INLINE
    constexpr dpf_key(interior_node_t root_,
                      const std::array<interior_node_t, depth> & interior_cws_,
                      const std::array<uint8_t, depth> & correction_advice_,
                      const leaf_nodes_t & exterior_cw_,
                      const std::bitset<sizeof...(OutputTs)+1> & wild_mask_)
      : wildcard_mask{wild_mask_},
        mutable_exterior_cw{exterior_cw_},
        root{root_},
        interior_cws{interior_cws_},
        correction_advice{correction_advice_}
    { }

  private:
    std::bitset<sizeof...(OutputTs)+1> wildcard_mask;
    leaf_nodes_t mutable_exterior_cw;

    template <std::size_t I,
              typename LeafT = std::tuple_element_t<I, leaf_nodes_t>>
    auto __make_concrete(const LeafT & cw)
    {
        std::get<I>(mutable_exterior_cw) = cw;
        wildcard_mask.reset(I);
    }

  public:
    const interior_node_t root;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    const std::array<interior_node_t, depth> interior_cws;
HEDLEY_PRAGMA(GCC diagnostic pop)
    const std::array<uint8_t, depth> correction_advice;

    HEDLEY_ALWAYS_INLINE
    bool is_wildcard(std::size_t i) const
    {
        return wildcard_mask.test(i);
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
    static auto traverse_interior(const interior_node_t & node,
        const interior_node_t & cw, bool dir) noexcept
    {
        return dpf::xor_if_lo_bit(
            interior_prg_t::eval(unset_lo_2bits(node), dir), cw, node);
    }

    template <std::size_t I,
              typename LeafT = std::tuple_element_t<I, leaf_nodes_t>>
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    static auto traverse_exterior(const interior_node_t & node,
        const LeafT & cw) noexcept
    {
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
        using output_t = std::tuple_element_t<I, outputs_t>;
        return dpf::subtract<output_t, exterior_node_t>(
            make_leaf_mask_inner<exterior_prg_t, I, outputs_t>(unset_lo_2bits(node)),
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
    using specialization = dpf_key<InteriorPRG, ExteriorPRG, InputT,
                                   OutputT, OutputTs...>;
    using interior_node_t = typename specialization::interior_node_t;

    constexpr auto depth = specialization::depth;
    InputT mask = specialization::msb_mask;

    const interior_node_t root[2] = {
        dpf::unset_lo_bit(RootSampler()),
        dpf::set_lo_bit(RootSampler())
    };

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    std::array<interior_node_t, depth> correction_word;
HEDLEY_PRAGMA(GCC diagnostic pop)
    std::array<uint8_t, depth> correction_advice;

    interior_node_t parent[2] = { root[0], root[1] };
    bool advice[2];

    for (std::size_t level = 0; level < depth; ++level, mask >>= 1)
    {
        bool bit = !!(mask & x);

        advice[0] = dpf::get_lo_bit_and_clear_lo_2bits(parent[0]);
        advice[1] = dpf::get_lo_bit_and_clear_lo_2bits(parent[1]);

        auto child0 = InteriorPRG::eval01(parent[0]);
        auto child1 = InteriorPRG::eval01(parent[1]);
        interior_node_t child[2] = {
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

        correction_word[level] = child[!bit];
        correction_advice[level] = uint_fast8_t(t[1] << 1) | t[0];
    }
    auto wildcard_mask = dpf::utils::make_bitset(dpf::is_wildcard_v<OutputT>,
        dpf::is_wildcard_v<OutputTs>...);

    bool sign0 = dpf::get_lo_bit(parent[0]);
    // bool sign1 = dpf::get_lo_bit(parent[1]);

    auto leaves = dpf::make_leaves<ExteriorPRG>(x, unset_lo_2bits(parent[0]),
        unset_lo_2bits(parent[1]), sign0, y, ys...);

    return std::make_pair(
        specialization{root[0], correction_word, correction_advice,
            leaves.first, wildcard_mask},
        specialization{root[1], correction_word, correction_advice,
            leaves.second, wildcard_mask});
}  // make_dpf

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_DPF_KEY_HPP__
