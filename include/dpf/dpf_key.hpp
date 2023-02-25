/// @file dpf/dpf_key.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_DPF_KEY_HPP__
#define LIBDPF_INCLUDE_DPF_DPF_KEY_HPP__

#include <array>
#include <bitset>
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

template <class interior_prg,
          class exterior_prg,
          typename input_t,
          typename output_t,
          typename... output_ts>
struct dpf_key
{
  public:
    using interior_prg_t = interior_prg;
    using interior_node_t = typename interior_prg::block_t;
    using exterior_prg_t = exterior_prg;
    using exterior_node_t = typename exterior_prg::block_t;
    using outputs_t = std::tuple<output_t, output_ts...>;
    using leaf_nodes_t = dpf::leaf_tuple_t<exterior_node_t, output_t, output_ts...>;
    static constexpr std::size_t tree_depth = utils::bitlength_of_v<input_t> - dpf::lg_outputs_per_leaf_v<output_t, exterior_node_t>;
    static constexpr input_t msb_mask = input_t(1) << (utils::bitlength_of_v<input_t>-1);

    static_assert(std::conjunction_v<std::is_trivially_copyable<output_t>,
                                     std::is_trivially_copyable<output_ts>...>,
        "all output types must be trivially copyable");
    static_assert(std::has_unique_object_representations_v<input_t>);

    HEDLEY_ALWAYS_INLINE
    constexpr dpf_key(interior_node_t root_,
                      const std::array<interior_node_t, tree_depth> & interior_cws_,
                      const std::array<uint8_t, tree_depth> & correction_advice_,
                      const leaf_nodes_t & exterior_cw_,
                      std::bitset<sizeof...(output_ts)+1> & wildcards_mask_)
      : wildcard_mask{wildcards_mask_},
        mutable_exterior_cw{exterior_cw_},
        root{root_},
        interior_cws{interior_cws_},
        correction_advice{correction_advice_}
    { }

  private:
    std::bitset<sizeof...(output_ts)+1> wildcard_mask;
    leaf_nodes_t mutable_exterior_cw;

  public:
    const interior_node_t root;
    const std::array<interior_node_t, tree_depth> interior_cws;
    const std::array<uint8_t, tree_depth> correction_advice;

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

    template <std::size_t I>
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    static auto traverse_exterior(const interior_node_t & node,
        const exterior_node_t & cw) noexcept
    {
        return dpf::subtract<std::tuple_element_t<I, outputs_t>, exterior_node_t>(
            make_leaf_mask_inner<exterior_prg_t, I, exterior_node_t, output_t, output_ts...>(unset_lo_2bits(node)),
            dpf::get_if_lo_bit(cw, node));
    }

};  // struct dpf_key

template <typename T>
static T basic_uniform_root_sampler()
{
    T ret = dpf::uniform_fill(ret);
    return ret;
}

template <class interior_prg = dpf::prg::aes128,
          class exterior_prg = interior_prg,
          dpf::root_sampler_t<interior_prg> root_sampler
            = &dpf::basic_uniform_root_sampler,
          typename input_t,
          typename output_t,
          typename... output_ts>
auto make_dpf(input_t x, output_t y, output_ts... ys)
{
    using specialization = dpf_key<interior_prg, exterior_prg, input_t,
                                   output_t, output_ts...>;
    using interior_node_t = typename specialization::interior_node_t;

    constexpr std::size_t depth = specialization::tree_depth;
    input_t mask = specialization::msb_mask;

    const interior_node_t root[2] = {
        dpf::unset_lo_bit(root_sampler()),
        dpf::set_lo_bit(root_sampler())
    };

    std::array<interior_node_t, depth> correction_word;
    std::array<uint8_t, depth> correction_advice;

    interior_node_t parent[2] = { root[0], root[1] };
    bool advice[2];

    for (std::size_t level = 0; level < depth; ++level)
    {
        bool bit = !!(mask & x);
        mask >>= 1;

        advice[0] = dpf::get_lo_bit_and_clear_lo_2bits(parent[0]);
        advice[1] = dpf::get_lo_bit_and_clear_lo_2bits(parent[1]);

        auto child0 = interior_prg::eval01(parent[0]);
        auto child1 = interior_prg::eval01(parent[1]);
        interior_node_t child[2] = {
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

        correction_word[level] = child[!bit];
        correction_advice[level] = uint_fast8_t(t[1] << 1) | t[0];
    }
    auto wildcard_mask = dpf::utils::make_bitset(dpf::is_wildcard_v<output_t>,
        dpf::is_wildcard_v<output_ts>...);

    bool sign0 = dpf::get_lo_bit(parent[0]);
    // bool sign1 = dpf::get_lo_bit(parent[1]);

    auto leaves = dpf::make_leaves<exterior_prg>(x, unset_lo_2bits(parent[0]), unset_lo_2bits(parent[1]),
        sign0, y, ys...);

    return std::make_pair(
        specialization{root[0], correction_word, correction_advice, leaves.first, wildcard_mask},
        specialization{root[1], correction_word, correction_advice, leaves.second, wildcard_mask});
}  // make_dpf

// carries the state for assigning a leaf, which is interactive
struct leaf_assigner
{
};

// carries the state for dorner-shelat method, which is interactive
struct dorner_shelat_dpf_maker
{
};

// carries the state for a linear sketch for 1-hotness
struct linear_sketch
{
};

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_DPF_KEY_HPP__
