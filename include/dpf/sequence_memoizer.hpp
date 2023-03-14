/// @file dpf/sequence_memoizer.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_SEQUENCE_MEMOIZER_HPP__
#define LIBDPF_INCLUDE_DPF_SEQUENCE_MEMOIZER_HPP__

namespace dpf
{

template <typename InputT>
struct list_recipe
{
    list_recipe(const std::vector<int8_t> & steps_,
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
    const std::vector<std::size_t> level_endpoints;
};

template <typename RandomAccessIterator>
auto make_recipe(std::size_t outputs_per_leaf, RandomAccessIterator begin, RandomAccessIterator end)
{
    using input_t = std::remove_reference_t<decltype(*begin)>;
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

    auto depth = dpf::utils::bitlength_of_v<input_t> - std::log2(outputs_per_leaf);
    auto mask = input_t(1) << (dpf::utils::bitlength_of_v<input_t>-1);

    std::set<RandomAccessIterator, IteratorComp> splits;
    splits.insert(begin);

    std::vector<std::size_t> level_endpoints;
    level_endpoints.push_back(0);
    std::vector<int8_t> recipe_steps;
    for (std::size_t level_index = 0; level_index < depth; ++level_index, mask>>=1)
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
        leaf_index += *prev/outputs_per_leaf < *curr/outputs_per_leaf;
        output_indices.push_back(leaf_index * outputs_per_leaf + (*curr % outputs_per_leaf));
    }

    return list_recipe<input_t>{recipe_steps, output_indices, leaf_index+1, level_endpoints};
}

template <typename InputT,
          typename NodeT,
          typename Allocator>
struct sequence_memoizer_base
{
  public:
    using unique_ptr = typename Allocator::unique_ptr;
    const list_recipe<InputT> & recipe;

    const unique_ptr buf;
  protected:
    explicit sequence_memoizer_base(const list_recipe<InputT> & r,
        std::size_t buffer_size_in_nodes, Allocator alloc)
      : recipe{r},
        buf{alloc.make_unique(buffer_size_in_nodes)} { }
};

template <typename InputT,
          typename NodeT,
          typename Allocator = aligned_allocator<NodeT>>
struct reversing_sequence_memoizer
  : public sequence_memoizer_base<InputT, NodeT, Allocator>
{
  private:
    using parent = sequence_memoizer_base<InputT, NodeT, Allocator>;
  public:
    using unique_ptr = typename Allocator::unique_ptr;
    using forward_iter = NodeT *;
    using reverse_iter = std::reverse_iterator<forward_iter>;

    explicit reversing_sequence_memoizer(const list_recipe<InputT> & r,
        Allocator alloc = Allocator{})
      : parent::sequence_memoizer_base(r, r.num_leaf_nodes, alloc) { }

    struct pointer_facade
    {
        HEDLEY_ALWAYS_INLINE
        pointer_facade(bool flip, forward_iter it, reverse_iter rit)
          : flip_{flip}, it_{it}, rit_{rit} { }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        HEDLEY_PURE
        auto & operator[](std::size_t i)
        {
            return flip_ ? rit_[i] : it_[i];
        }

        HEDLEY_ALWAYS_INLINE
        HEDLEY_NO_THROW
        HEDLEY_PURE
        const auto & operator[](std::size_t i) const
        {
            return flip_ ? rit_[i] : it_[i];
        }

      private:
        const bool flip_;
        forward_iter it_;
        reverse_iter rit_;
    };

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    auto get_iterators(bool b, InputT x) const noexcept
    {
        return std::make_pair(
          pointer_facade(b, &parent::buf[parent::recipe.num_leaf_nodes-x],
              std::make_reverse_iterator(&parent::buf[x])),
          pointer_facade(b, &parent::buf[0],
              std::make_reverse_iterator(&parent::buf[parent::recipe.num_leaf_nodes])));
    }

    auto get_step(bool b, std::size_t level, std::size_t step)
    {
        step = b ? step : parent::recipe.level_endpoints[level+1] - step - 1 + parent::recipe.level_endpoints[level];
        return parent::recipe.recipe_steps(step);
    }
};

template <typename InputT,
          typename NodeT,
          typename Allocator = aligned_allocator<NodeT>>
struct double_space_sequence_memoizer
{
  public:
    using unique_ptr = typename Allocator::unique_ptr;

    explicit double_space_sequence_memoizer(const list_recipe<InputT> & r, Allocator alloc = Allocator{})
      : recipe{r},
        buf{alloc.allocate_unique_ptr(2*recipe.num_leaf_nodes * sizeof(NodeT))} { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    auto get_iterators(bool b, InputT) const noexcept
    {
        return std::make_pair(
          Allocator::assume_aligned(&buf[recipe.num_leaf_nodes*(!b)]),
          Allocator::assume_aligned(&buf[recipe.num_leaf_nodes*b]));
    }

    auto get_step(bool, std::size_t, std::size_t step)
    {
        return recipe.recipe_steps[step];
    }

    const list_recipe<InputT> & recipe;
    const unique_ptr buf;
};

template <typename InputT,
          typename NodeT,
          typename Allocator = aligned_allocator<NodeT>>
struct full_tree_sequence_memoizer
{
  public:
    using unique_ptr = typename Allocator::unique_ptr;

    explicit full_tree_sequence_memoizer(const list_recipe<InputT> & r, Allocator alloc = Allocator{})
      : recipe{r},
        buf{alloc.allocate_unique_ptr(recipe.level_endpoints[recipe.level_endpoints.size()-1] + recipe.num_leaf_nodes)} { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    auto get_iterators(std::size_t level, InputT) const noexcept
    {
        return std::make_pair(
          Allocator::assume_aligned(&buf[recipe.level_endpoints[level]]),
          Allocator::assume_aligned(&buf[recipe.level_endpoints[level+1]]));
    }

    auto get_step(bool, std::size_t, std::size_t step)
    {
        return recipe.recipe_steps[step];
    }

    const list_recipe<InputT> & recipe;
    const unique_ptr buf;
};

template <typename DpfKey,
          typename InputT>
auto make_inplace_reversing_sequence_memoizer(const DpfKey &, const list_recipe<InputT> & recipe)
{
    return reversing_sequence_memoizer<InputT, typename DpfKey::interior_node_t>(recipe);
}

template <typename DpfKey,
          typename InputT>
auto make_double_space_sequence_memoizer(const DpfKey &, const list_recipe<InputT> & recipe)
{
    return double_space_sequence_memoizer<InputT, typename DpfKey::interior_node_t>(recipe);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_SEQUENCE_MEMOIZER_HPP__
