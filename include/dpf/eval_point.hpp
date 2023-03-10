/// @file dpf/memoization.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief miscellaneous helper functions, structs, preprocessor directives
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_POINT_HPP__
#define LIBDPF_INCLUDE_DPF_POINT_HPP__

#include <iomanip>

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

namespace internal
{

template <typename DpfKey,
          typename InputT,
          class PathMemoizer>
DPF_UNROLL_LOOPS
inline auto eval_point_interior(const DpfKey & dpf, InputT x, PathMemoizer & path)
{
    std::size_t level_index = path.assign_x(x);
    for (InputT mask = dpf.msb_mask>>level_index;
        level_index < dpf.tree_depth; ++level_index, mask>>=1)
    {
        bool bit = !!(mask & x);
        auto cw = set_lo_bit(dpf.interior_cws[level_index],
            dpf.correction_advice[level_index]>>bit);
        path[level_index+1] = DpfKey::traverse_interior(path[level_index], cw, bit);
    }
}

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          class PathMemoizer>
inline auto eval_point_exterior(const DpfKey & dpf, InputT x, const PathMemoizer & path)
{
    assert_not_wildcard<I>(dpf);

    using output_t = std::tuple_element_t<I, typename DpfKey::outputs_t>;

    auto interior = path[dpf.tree_depth];
    auto ext = dpf.template exterior_cw<I>();
    return dpf::make_dpf_output<output_t>(
        DpfKey::template traverse_exterior<I>(interior, ext), x);
}

}  // namespace internal

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT,
          class PathMemoizer>
auto eval_point(const DpfKey & dpf, InputT x, PathMemoizer & path)
{
    assert_not_wildcard<I>(dpf);
    internal::eval_point_interior(dpf, x, path);
    return internal::eval_point_exterior<I>(dpf, x, path);
}

template <std::size_t depth,
          typename InputT,
          typename NodeT>
struct alignas(alignof(NodeT)) basic_path_memoizer
    : public std::array<NodeT, depth+1>
{
  public:
    using input_type = InputT;
    using node_type = NodeT;
    
    explicit basic_path_memoizer(node_type root)
      : std::array<node_type, depth+1>{root}, x{std::nullopt} { }
    basic_path_memoizer(basic_path_memoizer &&) = default;
    basic_path_memoizer(const basic_path_memoizer &) = default;

    inline std::size_t assign_x(input_type new_x)
    {
        static constexpr auto complement_of = std::bit_not{};
        input_type old_x = x.value_or(complement_of(new_x));
        x = new_x;
        return clzx(old_x, new_x);
    }

  private:
    std::optional<input_type> x;
    static constexpr auto clzx = utils::countl_zero_symmmetric_difference<InputT>{};
};

template <typename NodeT>
struct nonmemoizing_path_memoizer
{
  public:
    using node_type = NodeT;

    HEDLEY_ALWAYS_INLINE
    explicit nonmemoizing_path_memoizer(node_type root)
      : v{root} { }
    nonmemoizing_path_memoizer(nonmemoizing_path_memoizer &&) = default;
    nonmemoizing_path_memoizer(const nonmemoizing_path_memoizer &) = default;

    template <typename InputT>
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    std::size_t assign_x(InputT) noexcept { return 0; }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const node_type & operator[](std::size_t) const noexcept { return v; }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    node_type & operator[](std::size_t) noexcept { return v; }

  private:
    node_type v;
};

template <std::size_t I = 0,
          typename DpfKey,
          typename InputT>
auto eval_point(const DpfKey & dpf, InputT x)
{
    auto memoizer = nonmemoizing_path_memoizer(dpf.root);
    return eval_point<I>(dpf, x, memoizer);
}

template <typename DpfKey>
auto make_path_memoizer(const DpfKey & dpf)
{
    using input_t = typename DpfKey::input_type;
    using node_t = typename DpfKey::interior_node_t;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    return basic_path_memoizer<DpfKey::tree_depth, input_t, node_t>(dpf.root);
HEDLEY_PRAGMA(GCC diagnostic pop)
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_POINT_HPP__
