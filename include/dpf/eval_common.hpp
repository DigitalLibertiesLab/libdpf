/// @file dpf/eval_common.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_EVAL_COMMON_HPP__
#define LIBDPF_INCLUDE_DPF_EVAL_COMMON_HPP__

#include <hedley/hedley.h>

#include <cstddef>
#include <stdexcept>

#include "dpf/leaf_node.hpp"

namespace dpf
{

template <typename OutputT,
          typename NodeT>
struct alignas(utils::max_align_v) dpf_output
{
    dpf_output(const dpf_output &) = default;
    dpf_output(dpf_output &&) noexcept = default;
    dpf_output & operator=(const dpf_output &) = default;
    dpf_output & operator=(dpf_output &&) noexcept = default;
    ~dpf_output() = default;

    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    constexpr operator OutputT() const
    {
        return extract_leaf<NodeT, OutputT>(node, offset);
    }
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    auto operator*() const
    {
        return static_cast<OutputT>(*this);
    }
    NodeT node;
    std::size_t offset;

  private:
    dpf_output(NodeT leaf_node, std::size_t off)
      : node{leaf_node}, offset{off} { }

  public:
    template <typename Output,
              typename Input,
              typename Node>
    friend auto make_dpf_output(const Node & node, Input x);
};

template <typename Output,
          typename Input,
          typename Node>
auto make_dpf_output(const Node & node, Input x)
{
    return dpf_output<concrete_type_t<Output>, Node>{node,
        offset_within_block<concrete_type_t<Output>, Node>(x)};
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_COMMON_HPP__
