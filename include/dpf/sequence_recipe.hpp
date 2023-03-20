/// @file dpf/sequence_recipe.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_RECIPE_HPP__
#define LIBDPF_INCLUDE_DPF_RECIPE_HPP__

#include <set>

namespace dpf
{

template <typename InputT>
struct sequence_recipe
{
    sequence_recipe(const std::vector<int8_t> & steps_,
                const std::vector<std::size_t> subsequence_indexes_,
                std::size_t leaf_index_,
                std::vector<std::size_t> level_endpoints_)
      : recipe_steps{steps_},
        output_indices{subsequence_indexes_},
        num_leaf_nodes{leaf_index_},
        level_endpoints{level_endpoints_}
    { }

    const std::vector<int8_t> recipe_steps;
    const std::vector<std::size_t> output_indices;
    const std::size_t num_leaf_nodes;
    const std::vector<std::size_t> level_endpoints;  // level_endpoints.size() = depth+1
};

namespace detail
{

template <typename DpfKey,
          typename RandomAccessIterator>
auto make_sequence_recipe(RandomAccessIterator begin, RandomAccessIterator end)
{
    static_assert(std::is_same_v<typename DpfKey::input_type, std::remove_reference_t<decltype(*begin)>>);

    using dpf_type = DpfKey;
    using input_type = std::remove_reference_t<decltype(*begin)>;

    struct IteratorComp
    {
        bool operator()(const RandomAccessIterator & lhs,
                        const RandomAccessIterator & rhs) const
        { 
            return *lhs < *rhs;
        }
    };

    if (!std::is_sorted(begin, end))
    {
        throw std::runtime_error("list must be sorted");
    }

    auto mask = dpf_type::msb_mask;

    std::set<RandomAccessIterator, IteratorComp> splits;
    splits.insert(begin);

    std::vector<std::size_t> level_endpoints;
    level_endpoints.push_back(0);
    std::vector<int8_t> recipe_steps;
    for (std::size_t level_index = 0; level_index < dpf_type::depth; ++level_index, mask>>=1)
    {
        for (auto upper = std::begin(splits), lower = upper++; ; lower = upper++)
        {
            bool at_end = (upper == std::end(splits));
            auto upper_ = at_end ? end : *upper;
            auto it = std::upper_bound(*lower, upper_, mask,
                [](auto a, auto b){ return a&b; });
            if (it == *lower) recipe_steps.push_back(-1);       // right only
            else if (it == upper_) recipe_steps.push_back(+1);  // left only
            else
            {
                recipe_steps.push_back(0);                      // both ways
                splits.insert(it);
            }
            if (at_end) break;
        }
        level_endpoints.push_back(recipe_steps.size());
    }

    std::vector<std::size_t> output_indices;
    // output_indices.push_back(*begin % outputs_per_leaf);
    std::size_t leaf_index = 0;//*begin/outputs_per_leaf < *(begin+1)/outs_per_leaf;
    for (auto curr = begin, prev = curr; curr != end; prev = curr++)
    {
        leaf_index += *prev/dpf_type::outputs_per_leaf < *curr/dpf_type::outputs_per_leaf;
        output_indices.push_back(leaf_index * dpf_type::outputs_per_leaf + (*curr % dpf_type::outputs_per_leaf));
    }

    return sequence_recipe<input_type>{recipe_steps, output_indices, leaf_index+1, level_endpoints};
}

}  // namespace detail

template <typename DpfKey,
          typename RandomAccessIterator>
auto make_sequence_recipe(const DpfKey &, RandomAccessIterator begin, RandomAccessIterator end)
{
    return detail::make_sequence_recipe<DpfKey>(begin, end);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_RECIPE_HPP__
