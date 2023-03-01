/// @file dpf/memoization.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief miscellaneous helper functions, structs, preprocessor directives
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_MEMOIZE_HPP__
#define LIBDPF_INCLUDE_DPF_MEMOIZE_HPP__

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

template <std::size_t I = 0,
          typename dpf_t,
          typename input_t,
          class memoizer_t>
auto eval_point(const dpf_t & dpf, input_t x, memoizer_t & buf)
{
    if (dpf.is_wildcard(I))
    {
        throw std::runtime_error("cannot evaluate wildcards");
    }
    using output_t = std::tuple_element_t<I, typename dpf_t::outputs_t>;

    std::size_t i = buf.assign_x(x);
<<<<<<< HEAD
    for (input_t mask = dpf.msb_mask>>i; i < dpf.tree_depth; ++i, mask>>=1)
    {
        bool bit = !!(x & mask);
=======
    for (input_t mask = dpf.msb_mask>>i; i < dpf.tree_depth; ++i, mask >>= 1)
    {
        bool bit = !!(mask & x);
>>>>>>> 6b74f66b91c0a65ecbdfa6dd15da724296846dd6
        auto cw = set_lo_bit(dpf.interior_cws[i],
            dpf.correction_advice[i]>>bit);
        buf[i+1] = dpf_t::traverse_interior(buf[i], cw, bit);
    }

    auto interior = buf[dpf.tree_depth-1];
    auto ext = dpf.template exterior_cw<I>();
    return dpf::make_dpf_output<output_t>(
        dpf_t::template traverse_exterior<I>(interior, ext), x);
}

template <std::size_t depth,
          typename input_t,
          typename node_t>
struct alignas(alignof(node_t)) basic_path_memoizer
    : public std::array<node_t, depth+1>
{
  public:
    explicit basic_path_memoizer(node_t root)
      : std::array<node_t, depth+1>{root}, x{std::nullopt} { }
    basic_path_memoizer(basic_path_memoizer &&) = default;
    basic_path_memoizer(const basic_path_memoizer &) = default;

    inline std::size_t assign_x(input_t new_x)
    {
        static constexpr auto complement_of = std::bit_not{};
        input_t old_x = x.value_or(complement_of(new_x));
        x = new_x;
        return utils::countl_zero_symmmetric_difference(old_x, new_x);
    }

  private:
    std::optional<input_t> x;
};

template <typename node_t>
struct nonmemoizing_path_memoizer
{
  public:
    HEDLEY_ALWAYS_INLINE
    explicit nonmemoizing_path_memoizer(node_t root)
      : v{root} { }
    nonmemoizing_path_memoizer(nonmemoizing_path_memoizer &&) = default;
    nonmemoizing_path_memoizer(const nonmemoizing_path_memoizer &) = default;

    template <typename input_t>
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    std::size_t assign_x(input_t) noexcept { return 0; }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const node_t & operator[](std::size_t) const noexcept { return v; }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    node_t & operator[](std::size_t) noexcept { return v; }

  private:
    node_t v;
};

template <std::size_t I = 0,
          typename dpf_t,
          typename input_t>
auto eval_point(const dpf_t & dpf, input_t x)
{
    using node_t = typename dpf_t::interior_node_t;
    auto buf = nonmemoizing_path_memoizer<node_t>(dpf.root);
    return eval_point<I>(dpf, x, buf);
}

template <typename dpf_t>
auto make_path_memoizer(const dpf_t & dpf)
{
    using input_t = typename dpf_t::input_t;
    using node_t = typename dpf_t::interior_node_t;
    return basic_path_memoizer<dpf_t::tree_depth, input_t, node_t>(dpf.root);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_MEMOIZE_HPP__
