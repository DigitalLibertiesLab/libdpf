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

template <typename DpfKey,
          class SequenceMemoizer,
          typename InputT>
DPF_UNROLL_LOOPS
inline auto eval_interval_interior(const DpfKey & dpf, InputT from_node, InputT to_node,
    SequenceMemoizer & memoizer, std::size_t to_level = DpfKey::tree_depth)
{
    using node_t = typename DpfKey::interior_node_t;
    auto nodes_in_interval = std::max(std::size_t(0), to_node-from_node);

    InputT mask = dpf.msb_mask >> memoizer.level_index;
    std::size_t nodes_at_level = std::ceil(std::ldexp(nodes_in_interval, memoizer.level_index-dpf.tree_depth+2));

    if (memoizer.level_index == 0)
    {
        memoizer[-1][0] = dpf.root;
        nodes_at_level = 1;
    }

    for (; memoizer.level_index < to_level; ++memoizer.level_index, mask>>=1)
    {
        std::size_t i = !!(mask & from_node), j = i;
        const node_t cw[2] = {
            set_lo_bit(dpf.interior_cws[memoizer.level_index], dpf.correction_advice[memoizer.level_index]&1),
            set_lo_bit(dpf.interior_cws[memoizer.level_index], (dpf.correction_advice[memoizer.level_index]>>1)&1)
        };

        if (i == 1)
        {
            memoizer[memoizer.level_index][0] = DpfKey::traverse_interior(memoizer[memoizer.level_index-1][0], cw[1], 1);
        }
        for (; j < nodes_at_level-1; ++j)
        {
            memoizer[memoizer.level_index][i++] = DpfKey::traverse_interior(memoizer[memoizer.level_index-1][j], cw[0], 0);
            memoizer[memoizer.level_index][i++] = DpfKey::traverse_interior(memoizer[memoizer.level_index-1][j], cw[1], 1);
        }
        nodes_at_level = std::ceil(std::ldexp(nodes_in_interval, memoizer.level_index-dpf.tree_depth+2));
        memoizer[memoizer.level_index][i++] = DpfKey::traverse_interior(memoizer[memoizer.level_index-1][j], cw[0], 0);
        if (i < nodes_at_level)
        {
            memoizer[memoizer.level_index][i++] = DpfKey::traverse_interior(memoizer[memoizer.level_index-1][j], cw[1], 1);
        }
    }
}

template <std::size_t I = 0,
          typename DpfKey,
          class OutputBuffer,
          class SequenceMemoizer>
DPF_UNROLL_LOOPS
inline auto eval_interval_exterior(const DpfKey & dpf, std::size_t from_node, std::size_t to_node,
    OutputBuffer & outbuf, SequenceMemoizer & memoizer)
{
    assert_not_wildcard<I>(dpf);

    using exterior_node_t = typename DpfKey::exterior_node_t;
    using output_t = std::tuple_element_t<I, typename DpfKey::outputs_t>;

    auto nodes_in_interval = std::max(std::size_t(0), to_node - from_node);

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    auto cw = dpf.template exterior_cw<I>();
    auto rawbuf = reinterpret_cast<exterior_node_t *>(std::data(outbuf));
    for (std::size_t j = 0, k = 0; j < nodes_in_interval; ++j,
        k += dpf::block_length_of_leaf_v<output_t, exterior_node_t>)
    {
        auto leaf = DpfKey::template traverse_exterior<I>(memoizer[DpfKey::tree_depth-1][j],
            dpf::get_if_lo_bit(cw, memoizer[DpfKey::tree_depth-1][j]));
        std::memcpy(&rawbuf[k], &leaf, sizeof(exterior_node_t));
    }
HEDLEY_PRAGMA(GCC diagnostic pop)
}

template <std::size_t I = 0,
          typename DpfKey,
          class OutputBuffer,
          class SequenceMemoizer,
          typename InputT>
DPF_UNROLL_LOOPS
auto eval_interval(const DpfKey & dpf, InputT from, InputT to,
    OutputBuffer & outbuf, SequenceMemoizer & memoizer)
{
    assert_not_wildcard<I>(dpf);

    using exterior_node_t = typename DpfKey::exterior_node_t;
    using outputs_t = typename DpfKey::outputs_t;
    using output_t = std::tuple_element_t<0, outputs_t>;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    static constexpr auto outputs_per_leaf = outputs_per_leaf_v<output_t, exterior_node_t>;
HEDLEY_PRAGMA(GCC diagnostic pop)
    std::size_t from_node = utils::quotient_floor(from, (InputT)outputs_per_leaf), to_node = utils::quotient_ceiling(to, (InputT)outputs_per_leaf);

    eval_interval_interior(dpf, from_node, to_node, memoizer);
    eval_interval_exterior<I>(dpf, from_node, to_node, outbuf, memoizer);

    return dpf::clipped_iterable<OutputBuffer>(outbuf, from % outputs_per_leaf,
        outputs_per_leaf - (to % outputs_per_leaf));
}

template <typename NodeT,
          typename Allocator = detail::aligned_allocator<NodeT>>
struct basic_interval_memoizer
{
  public:
    using unique_ptr = typename Allocator::unique_ptr;

    explicit basic_interval_memoizer(std::size_t output_len, std::size_t depth, Allocator alloc = Allocator{})
      : pivot{(dpf::utils::msb_of_v<std::size_t> >> clz(output_len))/2},
        tree_depth{depth},
        length{std::max(3*pivot, output_len)},
        buf{alloc.allocate_unique_ptr(length * sizeof(NodeT))},
        level_index{0}
    {
        if (pivot < 32)  // check alignment (pivot is 0 or some power of 2)
        {
            throw std::domain_error("output_len must be at least 64");
        }
    }

    void reset()
    {
        level_index = 0;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    auto operator[](std::size_t level) const noexcept
    {
        auto b = !((tree_depth ^ level) & 1);
        return Allocator::assume_aligned(&buf[b*pivot]);
    }

  private:
    static constexpr auto clz = utils::countl_zero<std::size_t>{};
    const std::size_t pivot;
    const std::size_t tree_depth;
  public:
    const std::size_t length;
    unique_ptr buf;
    std::size_t level_index;
};

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          typename OutputBuffer>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to, OutputBuffer & outbuf)
{
    auto memoizer = make_interval_memoizer(dpf, from, to);
    return eval_interval(dpf, from, to, outbuf, memoizer);
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT>
auto eval_interval(const DpfKey & dpf, InputT from, InputT to)
{
    using exterior_node_t = typename DpfKey::exterior_node_t;
    using output_t = std::tuple_element_t<I, typename DpfKey::outputs_t>;

    auto memoizer = make_interval_memoizer(dpf, from, to);

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    static constexpr auto outputs_per_leaf = outputs_per_leaf_v<output_t, exterior_node_t>;
HEDLEY_PRAGMA(GCC diagnostic pop)
    std::size_t from_node = utils::quotient_floor(from, (InputT)outputs_per_leaf), to_node = utils::quotient_ceiling(to, (InputT)outputs_per_leaf);
    std::size_t nodes_in_interval = std::max(std::size_t(0), std::size_t(to_node - from_node));
    dpf::output_buffer<output_t> outbuf(nodes_in_interval*outputs_per_leaf);
    auto clipped_iterable = eval_interval(dpf, from, to, outbuf, memoizer);
    return std::make_tuple(std::move(outbuf), std::move(clipped_iterable));
}

template <std::size_t I = 0,
          typename DpfKey,
          class OutputBuffer,
          class SequenceMemoizer>
auto eval_full(const DpfKey & dpf, OutputBuffer & outbuf, SequenceMemoizer & memoizer)
{
    using input_t = typename DpfKey::InputTypeype;
    return eval_interval<I>(dpf, input_t(0), std::numeric_limits<input_t>::max(), outbuf, memoizer);
}

template <std::size_t I = 0,
          typename DpfKey,
          class OutputBuffer>
auto eval_full(const DpfKey & dpf, OutputBuffer & outbuf)
{
    using input_t = typename DpfKey::input_type;
    auto memoizer = make_interval_memoizer(dpf, input_t(0), std::numeric_limits<input_t>::max());
    return eval_interval<I>(dpf, input_t(0), std::numeric_limits<input_t>::max(), outbuf, memoizer);
}

template <std::size_t I = 0,
          typename DpfKey>
auto eval_full(const DpfKey & dpf)
{
    using input_t = typename DpfKey::input_type;
    return eval_interval<I>(dpf, input_t(0), std::numeric_limits<input_t>::max());
}

template <typename DpfKey,
          typename InputT>
auto make_interval_memoizer(const DpfKey &, InputT from = 0,
    InputT to = std::numeric_limits<InputT>::max())
{
    using interior_prg = typename DpfKey::interior_prg_t;
    using exterior_prg = typename DpfKey::exterior_prg_t;
    using output_t = std::tuple_element_t<0, typename DpfKey::outputs_t>;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    static constexpr auto outputs_per_leaf = outputs_per_leaf_v<output_t,
        typename exterior_prg::block_t>;
    auto from_node = from/outputs_per_leaf, to_node = 1+to/outputs_per_leaf;
    auto nodes_in_interval = std::max(std::size_t(1), to_node - from_node);

    using node_t = typename interior_prg::block_t;
    return basic_interval_memoizer<node_t>(nodes_in_interval, DpfKey::tree_depth);
HEDLEY_PRAGMA(GCC diagnostic pop)
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_INTERVAL_HPP__
