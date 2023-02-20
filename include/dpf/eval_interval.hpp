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



template <typename node_t,
          typename Allocator = detail::aligned_allocator<node_t>>
struct basic_interval_level_memoizer
{
  public:
    using unique_ptr = typename Allocator::unique_ptr;

    explicit basic_interval_level_memoizer(std::size_t output_len)
      : pivot{(dpf::utils::msb_of_v<std::size_t> >> clz(output_len))/2},
        length{std::max(3*pivot, output_len)},
        buf{Allocator::make_unique(length * sizeof(node_t))}
    {
        if (pivot < 32)  // check alignment (n.b.: pivot is power of 2 or 0)
        {
            throw std::domain_error("output_len must be at least 64");
        }
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    auto operator[](bool b) const noexcept
    {
        return assume_aligned(&buf[b*pivot]);
    }

    const std::size_t length;
    const unique_ptr buf;

  private:
    static constexpr auto clz = utils::countl_zero<std::size_t>{};
    const std::size_t pivot;
};

template <class interior_prg,
          class exterior_prg,
          typename input_t,
          typename output_t,
          typename... output_ts>
auto make_interval_level_memoizer(const dpf_key<interior_prg, exterior_prg, input_t,
    output_t, output_ts...> &, input_t from = 0,
    input_t to = std::numeric_limits<input_t>::max())
{
    static constexpr auto outputs_per_leaf = outputs_per_leaf_v<output_t,
        typename exterior_prg::block_t>;
    auto from_node = from/outputs_per_leaf, to_node = 1+to/outputs_per_leaf;
    auto nodes_in_interval = std::max(std::size_t(1), to_node - from_node);

    using node_t = typename interior_prg::block_t;
    return basic_interval_level_memoizer<input_t, node_t>(nodes_in_interval);
}


}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_INTERVAL_HPP__
