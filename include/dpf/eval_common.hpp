/// @file dpf/eval.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_EVAL_HPP__
#define LIBDPF_INCLUDE_DPF_EVAL_HPP__

#include <hedley/hedley.h>

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

template <std::size_t ...Is,
          typename DpfKey>
void assert_not_wildcard(const DpfKey & dpf)
{
    if ((dpf.is_wildcard(Is) || ...))
    {
        throw std::runtime_error("cannot evaluate to wildcards");
    }
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_EVAL_HPP__
