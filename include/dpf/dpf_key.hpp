/// @file dpf/dpf_key.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_DPF_KEY_HPP__
#define LIBDPF_INCLUDE_DPF_DPF_KEY_HPP__

#include <cstddef>
#include <utility>
#include <tuple>
#include <array>
#include <bitset>
#include <atomic>

#include "dpf/prg_aes.hpp"
#include "dpf/wildcard.hpp"
#include "dpf/twiddle.hpp"
#include "dpf/leaf_node.hpp"
#include "dpf/offset_wrapper.hpp"
#include "dpf/leaf_wrapper.hpp"
#include "dpf/emplace.hpp"

namespace dpf
{

template <typename InputT,
          typename OutputT = dpf::bit,
          typename ...OutputTs>
auto make_dpfargs(InputT && x, OutputT && y = dpf::bit::one, OutputTs && ...ys);

template <typename InputT,
          typename OutputT,
          typename ...OutputTs>
struct dpfargs final
{
    using input_type = InputT;
    using output_type = std::tuple<OutputT, OutputTs...>;

    dpfargs() = delete;
    dpfargs(dpfargs &&) = default;
    dpfargs(const dpfargs &) = default;

    input_type x;
    output_type y;
  private:
    dpfargs(input_type x_, output_type y_) { x = x_; y = y_; }

    template <typename I, typename O, typename ...Os>
    friend auto make_dpfargs(I &&, O &&, Os && ...);
    // friend auto make_dpfargs(InputT && x, OutputT && y, OutputTs && ...ys)
};

template <typename InputT,
          typename OutputT,
          typename ...OutputTs>
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
auto make_dpfargs(InputT && x, OutputT && y, OutputTs && ...ys)
{
    return dpfargs<std::decay_t<InputT>,
                   std::decay_t<OutputT>,
                   std::decay_t<OutputTs>...>
            { std::forward<InputT>(x),
              std::make_tuple(std::forward<OutputT>(y),
                              std::forward<OutputTs>(ys)...) };
}

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

    using input_type = dpf::concrete_type_t<InputT>;
    using raw_input_type = InputT;
    using integral_type = utils::integral_type_from_bitlength_t<utils::bitlength_of_v<input_type>, utils::bitlength_of_v<std::size_t>>;

    using outputs_tuple = std::tuple<OutputT, OutputTs...>;
    template <std::size_t I>
    using output_type_t = std::tuple_element_t<I, outputs_tuple>;

    using concrete_outputs_tuple
        = std::tuple<concrete_type_t<OutputT>, concrete_type_t<OutputTs>...>;
    template <std::size_t I>
    using concrete_output_type = std::tuple_element_t<I, concrete_outputs_tuple>;
    using offset_type = offset_wrapper<InputT>;    // N.B.: `InputT`, not `input_type`

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using leaf_tuple = dpf::leaf_tuple_t<exterior_node, OutputT, OutputTs...>;
    using beaver_tuple = dpf::beaver_tuple_t<exterior_node, OutputT, OutputTs...>;
    using leaf_wrapper_tuple = std::tuple<dpf::leaf_wrapper<OutputT, exterior_node>, dpf::leaf_wrapper<OutputTs, exterior_node>...>;
HEDLEY_PRAGMA(GCC diagnostic pop)

    static constexpr std::size_t outputs_per_leaf = dpf::outputs_per_leaf_v<OutputT, exterior_node>;
    static constexpr std::size_t lg_outputs_per_leaf = dpf::lg_outputs_per_leaf_v<OutputT, exterior_node>;
    static constexpr std::size_t depth
        = utils::bitlength_of_v<input_type> - lg_outputs_per_leaf;
    static constexpr auto msb_mask = utils::msb_of_v<input_type>;

    using correction_words_array = std::array<interior_node, depth>;
    using correction_advice_array = std::array<psnip_uint8_t, depth>;

    template <typename Emplaceable>
    HEDLEY_ALWAYS_INLINE
    static void emplace(Emplaceable & output,
                   const interior_node & root, 
                   const correction_words_array & correction_words, 
                   const correction_advice_array & correction_advice,
                   const leaf_tuple & leaves,
                   const beaver_tuple & beavers,
                   const input_type & offset_share)
    {
        utils::dpf_emplacer<dpf_key, Emplaceable>::emplace(output, root, correction_words, correction_advice, leaves, beavers, offset_share);
    }

    template <typename EmplaceableContainer>
    HEDLEY_ALWAYS_INLINE
    static void emplace_back(EmplaceableContainer & output,
                       const interior_node & root, 
                       const correction_words_array & correction_words, 
                       const correction_advice_array & correction_advice,
                       const leaf_tuple & leaves,
                       const beaver_tuple & beavers,
                       const input_type & offset_share)
    {
        utils::dpf_back_emplacer<dpf_key, EmplaceableContainer>::emplace_back(output, root, correction_words, correction_advice, leaves, beavers, offset_share);
    }

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    static_assert(((dpf::utils::bitlength_of_output_v<OutputT, exterior_node>
                    == dpf::utils::bitlength_of_output_v<OutputTs, exterior_node>) && ...),
        "all output types must be the same length");
HEDLEY_PRAGMA(GCC diagnostic pop)
    static_assert(std::conjunction_v<std::is_trivially_copyable<OutputT>,
                                     std::is_trivially_copyable<OutputTs>...>,
        "all output types must be trivially copyable");
    static_assert(std::conjunction_v<std::is_standard_layout<OutputT>,
                                     std::is_standard_layout<OutputTs>...>,
        "all output types must be standard layout");
    // static_assert(std::has_unique_object_representations_v<input_type>);

    HEDLEY_ALWAYS_INLINE
    constexpr dpf_key(interior_node root,
                      const correction_words_array & correction_words,
                      const correction_advice_array & correction_advice,
                      const leaf_tuple & leaves,
                      const beaver_tuple & beavers,
                      input_type offset_share)
      : root_{root},
        correction_words_{correction_words},
        correction_advice_{correction_advice},
        mutable_wildcard_mask_{dpf::utils::make_bitset(dpf::is_wildcard_v<OutputT>,
            dpf::is_wildcard_v<OutputTs>...)},
        leaf_nodes(get_wrappers(leaves, beavers)),
        common_part_hash_{utils::get_common_part_hash(correction_words_, correction_advice_, leaf_nodes, wildcard_mask)},
        offset_x{offset_share}
    { }
    dpf_key(const dpf_key &) = delete;
    dpf_key(dpf_key &&) = default;

    const interior_node & root() const { return root_; }
    const correction_words_array & correction_words() const { return correction_words_; }
    const correction_advice_array & correction_advice() const { return correction_advice_; }
    const digest_type & common_part_hash() const { return common_part_hash_; }

    std::string wildcard_bitmask() const
    {
        return mutable_wildcard_mask_.to_string();
    }

    HEDLEY_ALWAYS_INLINE
    const interior_node & correction_word(std::size_t level) const
    {
        return correction_words_[level];
    }

    HEDLEY_ALWAYS_INLINE
    psnip_uint8_t correction_advice(std::size_t level) const
    {
        return correction_advice_[level];
    }

    HEDLEY_ALWAYS_INLINE
    auto correction_word(std::size_t level, bool direction) const
    {
        return set_lo_bit(correction_word(level),
            (correction_advice_[level] >> direction) & 1);
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
        const LeafT & correction_word) noexcept
    {
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
        using output_type = std::tuple_element_t<I, concrete_outputs_tuple>;
        return dpf::subtract_leaf<output_type>(
            make_leaf_mask_inner<exterior_prg, I, concrete_outputs_tuple>(unset_lo_2bits(node)),
            dpf::get_if_lo_bit(correction_word, node));
HEDLEY_PRAGMA(GCC diagnostic pop)
    }

    template <std::size_t I = 0>
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    auto traverse_exterior(const interior_node & node) const noexcept
    {
        return traverse_exterior(node, std::get<I>(leaf_nodes).get());
    }

    leaf_wrapper_tuple leaf_nodes;
    offset_type offset_x;
    static constexpr std::array<bool, sizeof...(OutputTs)+1> wildcard_mask{dpf::is_wildcard_v<OutputT>,
        dpf::is_wildcard_v<OutputTs>...};

  private:
    static auto get_wrappers(const leaf_tuple & leaves,
                             const beaver_tuple & beavers)
    {
        outputs_tuple tmp{};
        return std::apply([&beavers, &tmp](auto & ...leaf)
        {
            return std::apply([&leaf..., &tmp](auto & ...beaver)
            {
                return std::apply([&leaf..., &beaver...](auto & ...foo)
                {
                    return std::make_tuple(
                        dpf::leaf_wrapper<std::decay_t<decltype(foo)>, exterior_node>(leaf, beaver)...
                    );
                }, tmp);
            }, beavers);
        }, leaves);
    }

    interior_node root_;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    correction_words_array correction_words_;
HEDLEY_PRAGMA(GCC diagnostic pop)
    correction_advice_array correction_advice_;
    std::bitset<sizeof...(OutputTs)+1> mutable_wildcard_mask_;
    digest_type common_part_hash_;
};  // struct dpf_key

template <typename PRG>
struct pseudorandom_root_sampler
{
    using root_type = typename PRG::block_type;

    pseudorandom_root_sampler(
        root_type && seed = dpf::uniform_sample<root_type>())
      : seed_{seed}, counter_{0} { }

    root_type operator()(psnip_uint32_t i) const
    {
        return PRG::eval(seed_, i);
    }

    root_type operator()()
    {
        return this->operator()(counter_.fetch_add(1));
    }

    const root_type & seed() const { return seed_; }
    psnip_uint32_t count() const { return counter_; }

  private:
    root_type seed_;
    std::atomic_uint32_t counter_;
};

namespace utils
{

template <typename InteriorPRG,
          typename ExteriorPRG,
          typename InputT,
          typename OutputT,
          typename ...OutputTs>
struct dpf_type
{
    using type = dpf_key<InteriorPRG, ExteriorPRG,
        std::decay_t<InputT>,
        std::decay_t<OutputT>,
        std::decay_t<OutputTs>...>;
};

template <typename InteriorPRG,
          typename ExteriorPRG,
          typename InputT,
          typename OutputT,
          typename ...OutputTs>
using dpf_type_t = typename dpf_type<InteriorPRG, ExteriorPRG, InputT, OutputT, OutputTs...>::type;

}  // namespace utils

namespace detail
{

template <typename InteriorPRG,
          typename ExteriorPRG,
          typename InputT,
          typename OutputT,
          typename ...OutputTs>
auto make_dpf_impl(dpfargs<InputT, OutputT, OutputTs...> args, root_sampler_t<InteriorPRG> && root_sampler = dpf::uniform_sample<typename InteriorPRG::block_type>)
{
    using dpf_type = utils::dpf_type_t<InteriorPRG, ExteriorPRG, InputT,
                                       OutputT, OutputTs...>;
    using interior_node = typename dpf_type::interior_node;
    using input_type = typename dpf_type::input_type;
    using correction_words_array = typename dpf_type::correction_words_array;
    using correction_advice_array = typename dpf_type::correction_advice_array;

    constexpr auto depth = dpf_type::depth;
    auto mask = dpf_type::msb_mask;

    input_type x, x0{}, x1{};
    if constexpr (dpf::is_wildcard_v<InputT>)
    {
        std::tie(x, x0, x1) = args.x();
    }
    else
    {
        x = args.x;
    }

    utils::flip_msb_if_signed_integral(x);

    const interior_node root[2] = {
        dpf::unset_lo_bit(root_sampler()),
        dpf::set_lo_bit(root_sampler())
    };

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    correction_words_array correction_words;
HEDLEY_PRAGMA(GCC diagnostic pop)
    correction_advice_array correction_advice;

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
        correction_advice[level] = static_cast<psnip_uint8_t>(t[1] << 1) | t[0];
    }

    bool sign0 = dpf::get_lo_bit(parent[0]);
    // bool sign1 = dpf::get_lo_bit(parent[1]);

    auto [pair0, pair1] = std::apply([&x, &parent, &sign0](auto && ...ys)
        {
            return dpf::make_leaves<ExteriorPRG>(x,
                                                 dpf::unset_lo_2bits(parent[0]),
                                                 dpf::unset_lo_2bits(parent[1]),
                                                 sign0, ys...); }, args.y);
    auto && [leaves0, beavers0] = pair0;
    auto && [leaves1, beavers1] = pair1;

    return std::make_tuple(correction_words, correction_advice,
        std::make_tuple(root[0], leaves0, beavers0, x0),
        std::make_tuple(root[1], leaves1, beavers1, x1));
}  // make_dpf_impl

}  // namespace detail

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          typename InputT,
          typename OutputT = dpf::bit,
          typename ...OutputTs>
auto make_dpf(dpfargs<InputT, OutputT, OutputTs...> args, root_sampler_t<InteriorPRG> && root_sampler = dpf::uniform_sample<typename InteriorPRG::block_type>)
{
    using dpf_type = utils::dpf_type_t<InteriorPRG, ExteriorPRG, InputT,
                                       OutputT, OutputTs...>;

    auto [correction_words, correction_advice,
          tuple0, tuple1] = detail::make_dpf_impl<InteriorPRG, ExteriorPRG>(args,
            std::forward<root_sampler_t<InteriorPRG>>(root_sampler));
    auto & [root0, leaves0, beavers0, offset0] = tuple0;
    auto & [root1, leaves1, beavers1, offset1] = tuple1;

    return std::make_pair(
        dpf_type{root0, correction_words, correction_advice,
            leaves0, beavers0, offset0},
        dpf_type{root1, correction_words, correction_advice,
            leaves1, beavers1, offset1});
}  // make_dpf

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          typename InputT,
          typename ...OutputTs>
auto make_dpf(InputT && x, OutputTs && ...ys)
{
    return make_dpf<InteriorPRG, ExteriorPRG>(dpf::make_dpfargs(x, ys...));
}

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          typename InputT,
          typename OutputT = dpf::bit,
          typename ...OutputTs>
auto deduce_dpf_type(InputT x, OutputT y = dpf::bit::one, OutputTs ...ys)
{
    return utils::dpf_type<InteriorPRG, ExteriorPRG, InputT, OutputT, OutputTs...>{};
}

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          typename InputT,
          typename OutputT = dpf::bit,
          typename ...OutputTs>
auto deduce_dpf_type(dpf::dpfargs<InputT, OutputT, OutputTs...> args)
{
    return utils::dpf_type<InteriorPRG, ExteriorPRG, InputT, OutputT, OutputTs...>{};
}

#define DEDUCE_DPF_TYPE_T(...) typename decltype(dpf::deduce_dpf_type(__VA_ARGS__))::type;

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_DPF_KEY_HPP__
