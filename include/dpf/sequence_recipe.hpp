/// @file dpf/sequence_recipe.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_SEQUENCE_RECIPE_HPP__
#define LIBDPF_INCLUDE_DPF_SEQUENCE_RECIPE_HPP__

#include <cstddef>
#include <type_traits>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <list>
#include <iterator>

namespace dpf
{

struct sequence_recipe
{
  public:
    sequence_recipe(const std::vector<int8_t> & steps,
                const std::vector<std::size_t> & subsequence_indexes,
                std::size_t leaf_index,
                const std::vector<std::size_t> & level_endpoints)
      : recipe_steps_{steps},
        output_indices_{subsequence_indexes},
        num_leaf_nodes_{leaf_index},
        level_endpoints_{level_endpoints}
    { }

    const std::vector<int8_t> & recipe_steps() const { return recipe_steps_; }
    const std::vector<std::size_t> & output_indices() const { return output_indices_; }
    std::size_t num_leaf_nodes() const { return num_leaf_nodes_; }
    const std::vector<std::size_t> & level_endpoints() const { return level_endpoints_; }
    std::size_t depth() const { return level_endpoints_.size()-1; }

  private:
    std::vector<int8_t> recipe_steps_;
    std::vector<std::size_t> output_indices_;
    std::size_t num_leaf_nodes_;
    std::vector<std::size_t> level_endpoints_;  // level_endpoints.size() = depth+1
};

namespace detail
{

template <typename DpfKey,
          typename ForwardIterator>
auto make_sequence_recipe(ForwardIterator begin, ForwardIterator end)
{
    static_assert(std::is_same_v<typename DpfKey::input_type, std::decay_t<decltype(*begin)>>);

    using dpf_type = DpfKey;
    using input_type = typename DpfKey::input_type;

    if (!std::is_sorted(begin, end))
    {
        throw std::runtime_error("list must be sorted");
    }

    auto mask = dpf_type::msb_mask;

    std::list<ForwardIterator> splits{begin, end};

    std::vector<std::size_t> level_endpoints;
    level_endpoints.push_back(0);
    std::vector<int8_t> recipe_steps;

    auto func = [&](const bool flip = false)
    {
        // `lower` and `upper` are always adjacent elements of `splits` with `lower` < `upper`
        // [lower, upper) = "block"
        for (auto upper = std::begin(splits), lower = upper++; upper != std::end(splits); lower = upper++)
        {
            // `upper_bound()` returns iterator to first element where the relevant bit (based on `mask`) is set
            auto it = std::upper_bound(*lower, *upper, mask,
                [&flip](auto a, auto b){ return static_cast<bool>(a&b) ^ flip; });
            if (it == *lower) recipe_steps.push_back(-1);       // right only since first element in "block" requires right traversal
            else if (it == *upper) recipe_steps.push_back(+1);  // left only since no element in "block" requires right traversal
            else
            {
                recipe_steps.push_back(0);                      // both ways since some (non-lower) element within "block" requires right traversal
                splits.insert(upper, it);
            }
        }
        level_endpoints.push_back(recipe_steps.size());
    };
    if (dpf_type::depth > 0)
    {
        func(utils::is_signed_integral_v<input_type>);
        mask >>= 1;
    }
    for (std::size_t level_index = 1; level_index < dpf_type::depth; ++level_index, mask>>=1)
    {
        func();
    }

    std::vector<std::size_t> output_indices;
    // output_indices.push_back(*begin % outputs_per_leaf);
    std::size_t leaf_index = 0;  // *begin/outputs_per_leaf < *(begin+1)/outs_per_leaf;
    constexpr auto mod = utils::mod_pow_2<input_type>{};
    constexpr auto clz = utils::countl_zero_symmetric_difference<input_type>{};
    for (auto curr = begin, prev = curr; curr != end; prev = curr++)
    {
        leaf_index += (clz(*prev, *curr)) < dpf_type::depth;
        output_indices.push_back(leaf_index * dpf_type::outputs_per_leaf + mod(*curr, dpf_type::lg_outputs_per_leaf));
    }

    return sequence_recipe{recipe_steps, output_indices, leaf_index+1, level_endpoints};
}

}  // namespace detail

template <typename DpfKey,
          typename ForwardIterator>
auto make_sequence_recipe(ForwardIterator begin, ForwardIterator end)
{
    return detail::make_sequence_recipe<DpfKey>(begin, end);
}

template <typename DpfKey,
          typename ForwardIterator>
auto make_sequence_recipe(const DpfKey &, ForwardIterator begin, ForwardIterator end)
{
    return make_sequence_recipe<DpfKey>(begin, end);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_SEQUENCE_RECIPE_HPP__
