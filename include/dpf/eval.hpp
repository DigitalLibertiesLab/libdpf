#include <hedley/hedley.h>

#include "dpf/leaf_node.hpp"

namespace dpf
{

template <typename output_t, typename node_t>
struct alignas(utils::max_align_v) dpf_output
{
    dpf_output(const dpf_output &) = default;
    dpf_output(dpf_output &&) = default;

    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    constexpr operator output_t() const
    {
        return extract_leaf<node_t, output_t>(node, offset);;
    }
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    auto operator*() const
    {
        return static_cast<output_t>(*this);
    }
    const node_t node;
    const std::size_t offset;

  private:
    dpf_output(node_t leaf_node, std::size_t off)
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
    return dpf_output<Output, Node>{node,
        offset_within_block<Output, Node>(x)};
}

template <std::size_t I = 0,
          typename dpf_t>
void assert_not_wildcard(const dpf_t & dpf)
{
    if (dpf.is_wildcard(I))
    {
        throw std::runtime_error("cannot evaluate to wildcards");
    }
}


}  // namespace dpf

#include "eval_point.hpp"
#include "eval_interval.hpp"
#include "eval_sequence.hpp"