/// @file dpf/memoization.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief miscellaneous helper functions, structs, preprocessor directives
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_EVAL_INTERVAL_HPP__
#define LIBDPF_INCLUDE_DPF_EVAL_INTERVAL_HPP__

#include <functional>
#include <memory>
#include <limits>
#include <tuple>
#include <algorithm>

#include <portable-snippets/builtin/builtin.h>
#include <hedley/hedley.h>

#include "dpf/dpf_key.hpp"

namespace dpf
{

template <class interior_prg,
          typename node_t = typename interior_prg::block_t>
HEDLEY_NO_THROW
HEDLEY_ALWAYS_INLINE
HEDLEY_CONST
auto traverse(const node_t & node, const node_t & cw, bool direction)
{
    return dpf::xor_if_lo_bit(
        interior_prg::eval(unset_lo_2bits(node), direction), cw, node);
}

template <std::size_t I = 0,
          typename dpf_t,
          class memoizer_t,
          class outbuf_t,
          typename input_t>
auto eval_interval(const dpf_t & dpf, input_t from, input_t to,
    memoizer_t & memoizer, outbuf_t & outbuf)
{
    if (dpf.is_wildcard(I))
    {
        throw std::runtime_error("cannot evaluate wildcards");
    }
    using interior_prg = typename dpf_t::interior_prg_t;
    using exterior_node_t = typename dpf_t::exterior_node_t;
    using outputs_t = typename dpf_t::outputs_t;
    using output_t = std::tuple_element_t<I, outputs_t>;
    using exterior_prg = typename dpf_t::exterior_prg_t;
    static constexpr auto outputs_per_leaf = outputs_per_leaf_v<output_t, exterior_node_t>;

    input_t mask = dpf.msb_mask;

    std::size_t from_node = utils::quotient_floor(from, (input_t)outputs_per_leaf), to_node = utils::quotient_ceiling(to, (input_t)outputs_per_leaf);
    auto nodes_in_interval = std::max(std::size_t(0), to_node - from_node);

    bool currhalf = (dpf.tree_depth & 1);
    memoizer[!currhalf][0] = dpf.root;

    std::size_t nodes_at_level = 1;
    for (std::size_t level_index = 0; level_index < dpf.tree_depth-1; ++level_index, currhalf=!currhalf, mask>>=1)
    {
        const __m128i cw[2] = {
            set_lo_bit(dpf.interior_cws[level_index], dpf.correction_advice[level_index]&1),
            set_lo_bit(dpf.interior_cws[level_index], (dpf.correction_advice[level_index]>>1)&1)
        };

        std::size_t i = !!(mask & from), j = i;
        if (i == 1)
        {
            memoizer[currhalf][0] = traverse<interior_prg>(memoizer[!currhalf][0], cw[1], 1);
        }
        for (; j < nodes_at_level-1; ++j)
        {
            memoizer[currhalf][i++] = traverse<interior_prg>(memoizer[!currhalf][j], cw[0], 0);
            memoizer[currhalf][i++] = traverse<interior_prg>(memoizer[!currhalf][j], cw[1], 1);
        }
        nodes_at_level = std::ceil(std::ldexp(nodes_in_interval, level_index-dpf.tree_depth+2));
        memoizer[currhalf][i++] = traverse<interior_prg>(memoizer[!currhalf][j], cw[0], 0);
        if (i < nodes_at_level)
        {
            memoizer[currhalf][i++] = traverse<interior_prg>(memoizer[!currhalf][j], cw[1], 1);
        }
    }

    auto cw = dpf.template exterior_cw<I>();
    auto rawbuf = reinterpret_cast<exterior_node_t *>(std::data(outbuf));
    for (std::size_t j = 0; j < nodes_in_interval; ++j)
    {
        auto leaf = dpf::subtract<output_t, exterior_node_t>(
            make_leaf_mask_inner<exterior_prg, I, exterior_node_t, outputs_t>(unset_lo_2bits(memoizer[0][j])),
            dpf::get_if_lo_bit(cw, memoizer[0][j]));
        std::memcpy(&rawbuf[j], &leaf, sizeof(leaf));
    }

    auto begin = std::begin(outbuf) + (from % outputs_per_leaf);
    auto end = std::end(outbuf) - ((outputs_per_leaf - (to % outputs_per_leaf)) % outputs_per_leaf);

    return std::make_pair(begin, end);
}

template <typename node_t,
          typename Allocator = detail::aligned_allocator<node_t>>
struct basic_interval_level_memoizer
{
  public:
    using unique_ptr = typename Allocator::unique_ptr;

    explicit basic_interval_level_memoizer(std::size_t output_len, Allocator alloc = Allocator{})
      : pivot{(dpf::utils::msb_of_v<std::size_t> >> clz(output_len))/2},
        length{std::max(3*pivot, output_len)},
        buf{alloc.allocate_unique_ptr(length * sizeof(node_t))}
    {
        if (pivot < 32)  // check alignment (pivot is 0 or some power of 2)
        {
            throw std::domain_error("output_len must be at least 64");
        }
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    auto operator[](bool b) const noexcept
    {
        return Allocator::assume_aligned(&buf[b*pivot]);
    }

  private:
    static constexpr auto clz = utils::countl_zero<std::size_t>{};
    const std::size_t pivot;
  public:
    const std::size_t length;
    unique_ptr buf;
};

template <std::size_t I = 0,
          typename dpf_t,
          typename input_t>
auto eval_interval(const dpf_t & dpf, input_t from, input_t to)
{
    using exterior_node_t = typename dpf_t::exterior_node_t;
    using output_t = std::tuple_element_t<I, typename dpf_t::outputs_t>;

    auto memoizer = make_interval_level_memoizer(dpf, from, to);

    static constexpr auto outputs_per_leaf = outputs_per_leaf_v<output_t, exterior_node_t>;
    std::size_t from_node = utils::quotient_floor(from, (input_t)outputs_per_leaf), to_node = utils::quotient_ceiling(to, (input_t)outputs_per_leaf);
    std::size_t nodes_in_interval = std::max(std::size_t(0), std::size_t(to_node - from_node));
    dpf::output_buffer<output_t> outbuf(nodes_in_interval*outputs_per_leaf);
    auto [first, last] = eval_interval(dpf, from, to, memoizer, outbuf);
    return std::make_tuple(std::move(outbuf), std::move(first), std::move(last));
}

template <std::size_t I = 0,
          typename dpf_t,
          class buffer_t>
auto eval_full(const dpf_t & dpf, buffer_t & outbuf)
{
    using input_t = typename dpf_t::input_type;
    return eval_interval<I>(dpf, input_t(0), std::numeric_limits<input_t>::max(), outbuf);
}

template <std::size_t I = 0,
          typename dpf_t>
auto eval_full(const dpf_t & dpf)
{
    using input_t = typename dpf_t::input_type;
    return eval_interval<I>(dpf, input_t(0), std::numeric_limits<input_t>::max());
}

template <typename dpf_t,
          typename input_t>
auto make_interval_level_memoizer(const dpf_t &, input_t from = 0,
    input_t to = std::numeric_limits<input_t>::max())
{
    using interior_prg = typename dpf_t::interior_prg_t;
    using exterior_prg = typename dpf_t::exterior_prg_t;
    using output_t = std::tuple_element_t<0, typename dpf_t::outputs_t>;
    static constexpr auto outputs_per_leaf = outputs_per_leaf_v<output_t,
        typename exterior_prg::block_t>;
    auto from_node = from/outputs_per_leaf, to_node = 1+to/outputs_per_leaf;
    auto nodes_in_interval = std::max(std::size_t(1), to_node - from_node);

    using node_t = typename interior_prg::block_t;
    return basic_interval_level_memoizer<node_t>(nodes_in_interval);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_INTERVAL_HPP__
