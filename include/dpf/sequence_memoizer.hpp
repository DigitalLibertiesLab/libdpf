/// @file dpf/sequence_memoizer.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

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
    const std::vector<std::size_t> level_endpoints;  // level_endpoints.size() = depth+1
};

template <typename RandomAccessIterator>
auto make_recipe(std::size_t outputs_per_leaf, RandomAccessIterator begin, RandomAccessIterator end)
{
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

    auto depth = dpf::utils::bitlength_of_v<input_type> - std::log2(outputs_per_leaf);
    auto mask = input_type(1) << (dpf::utils::bitlength_of_v<input_type>-1);

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

    return list_recipe<input_type>{recipe_steps, output_indices, leaf_index+1, level_endpoints};
}

template <typename InputT,
          typename NodeT>
struct sequence_memoizer_base
{
  public:
    using input_type = InputT;
    using node_type = NodeT;
    const list_recipe<InputT> & recipe;

    // level 0 should access the root
    // level goes up to (and including) depth
    // TODO: how should this be fixed?
    // virtual node_type * operator[](std::size_t) const noexcept = 0;

    // TODO: should get_level() and reset() be replaced with something like assign_recipe?
    std::size_t get_level()
    {
        return level_index;
    }

    void reset()
    {
        level_index = 0;
    }

    std::size_t advance_level()
    {
        return ++level_index;
    }

    std::size_t get_nodes_at_level(std::size_t level) const
    {
        if (level == -1)
        {
            return 0;
        }

        if (level == depth)
        {
            return recipe.num_leaf_nodes;
        }

        return recipe.level_endpoints[level+1] - recipe.level_endpoints[level];
    }

    // returns true if first traversal should be taken
    // this usually means traversing left, but for the inplace_reversing memoizer
    // if it is working in reverse, this could be a right traversal
    virtual bool traverse_first(std::size_t step) const
    {
        return recipe.recipe_steps[step] > int8_t(-1);
    }

    // returns true if second traversal should be taken
    // this usually means traversing right, but for the inplace_reversing memoizer
    // if it is working in reverse, this could be a left traversal
    virtual bool traverse_second(std::size_t step) const
    {
        return recipe.recipe_steps[step] < int8_t(1);
    }

    // returns true if traversal should be done to the right
    // this usually means traversing in the same direction as supplied, but for the
    // inplace_reversing memoizer if it is working in reverse, this could be
    // the opposite of the supplied direction
    virtual bool get_direction(bool right) const
    {
        return right;
    }

  protected:
    const std::size_t depth;
    std::size_t level_index;

    explicit sequence_memoizer_base(const list_recipe<input_type> & r)
      : recipe{r},
        depth{recipe.level_endpoints.size()-1},
        level_index{0}
    { }
};

template <typename InputT,
          typename NodeT,
          typename Allocator = aligned_allocator<NodeT>>
struct inplace_reversing_sequence_memoizer final
  : public sequence_memoizer_base<InputT, NodeT>
{
  private:
    using parent = sequence_memoizer_base<InputT, NodeT>;
  public:
    using input_type = InputT;
    using node_type = NodeT;
    using unique_ptr = typename Allocator::unique_ptr;
    using forward_iter = node_type *;
    using reverse_iter = std::reverse_iterator<forward_iter>;
    using parent::recipe;
    using parent::depth;
    using parent::level_index;
    using parent::get_nodes_at_level;

    explicit inplace_reversing_sequence_memoizer(const list_recipe<input_type> & r,
        Allocator alloc = Allocator{})
      : parent::sequence_memoizer_base(r),
        buf{alloc.allocate_unique_ptr(r.num_leaf_nodes)}
    { }

    struct pointer_facade
    {
      public:
        HEDLEY_ALWAYS_INLINE
        pointer_facade(bool flip, forward_iter it, reverse_iter rit)
          : flip_{flip}, it_{it}, rit_{rit}
        { }

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
    pointer_facade operator[](std::size_t level) const noexcept
    {
        // flip false => forward traversal
        bool flip = (depth ^ level) & 1;
        // first check used to determine if previous or current level is being requested
        // second check used to determine if the last layer is being requested
        //     in which case it is setup to always return buf in normal order
        if (level == level_index-1 && level != depth)
        {
            std::size_t nodes_at_level = get_nodes_at_level(level);
            return pointer_facade(!flip, &buf[recipe.num_leaf_nodes-nodes_at_level],
                std::make_reverse_iterator(&buf[nodes_at_level]));
        }

        return pointer_facade(flip, &buf[0],
            std::make_reverse_iterator(&buf[recipe.num_leaf_nodes]));
    }

    bool traverse_first(std::size_t step) const override
    {
        // flip false => forward traversal
        bool flip = (depth ^ level_index) & 1;
        step = !flip ? step : recipe.level_endpoints[level_index] - step - 1 + recipe.level_endpoints[level_index-1];
        return !flip ? (recipe.recipe_steps[step] > int8_t(-1)) : (recipe.recipe_steps[step] < int8_t(1));
    }

    bool traverse_second(std::size_t step) const override
    {
        // flip false => forward traversal
        bool flip = (depth ^ level_index) & 1;
        step = !flip ? step : recipe.level_endpoints[level_index] - step - 1 + recipe.level_endpoints[level_index-1];
        return !flip ? (recipe.recipe_steps[step] < int8_t(1)) : (recipe.recipe_steps[step] > int8_t(-1));
    }

    bool get_direction(bool right) const override
    {
        // flip false => forward traversal
        bool flip = (depth ^ level_index) & 1;
        return !flip ? right : !right;
    }

  private:
    const unique_ptr buf;
};

template <typename InputT,
          typename NodeT,
          typename Allocator = aligned_allocator<NodeT>>
struct double_space_sequence_memoizer final
  : public sequence_memoizer_base<InputT, NodeT>
{
  private:
    using parent = sequence_memoizer_base<InputT, NodeT>;
  public:
    using input_type = InputT;
    using node_type = NodeT;
    using unique_ptr = typename Allocator::unique_ptr;
    using parent::recipe;
    using parent::depth;

    explicit double_space_sequence_memoizer(const list_recipe<input_type> & r, Allocator alloc = Allocator{})
      : parent::sequence_memoizer_base(r),
        buf{alloc.allocate_unique_ptr(2*recipe.num_leaf_nodes)}
    { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    node_type * operator[](std::size_t level) const noexcept
    {
        auto b = (depth ^ level) & 1;
        return Allocator::assume_aligned(&buf[recipe.num_leaf_nodes*b]);
    }

  private:
    const unique_ptr buf;
};

template <typename InputT,
          typename NodeT,
          typename Allocator = aligned_allocator<NodeT>>
struct full_tree_sequence_memoizer final
  : public sequence_memoizer_base<InputT, NodeT>
{
  private:
    using parent = sequence_memoizer_base<InputT, NodeT>;
  public:
    using input_type = InputT;
    using node_type = NodeT;
    using unique_ptr = typename Allocator::unique_ptr;
    using parent::recipe;

    explicit full_tree_sequence_memoizer(const list_recipe<input_type> & r, Allocator alloc = Allocator{})
      : parent::sequence_memoizer_base(r),
        buf{alloc.allocate_unique_ptr(recipe.level_endpoints[recipe.level_endpoints.size()-1] + recipe.num_leaf_nodes)}
    { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    node_type * operator[](std::size_t level) const noexcept
    {
        return Allocator::assume_aligned(&buf[recipe.level_endpoints[level]]);
    }

  private:
    const unique_ptr buf;
};

namespace detail
{

template <typename DpfKey,
          typename MemoizerT,
          typename InputT>
HEDLEY_ALWAYS_INLINE
auto make_sequence_memoizer(const list_recipe<InputT> & recipe)
{
    return MemoizerT(recipe);
}

}  // namespace dpf::detail

template <typename DpfKey,
          typename InputT = typename DpfKey::input_type>
auto make_inplace_reversing_sequence_memoizer(const DpfKey &, const list_recipe<InputT> & recipe)
{
    return detail::make_sequence_memoizer<DpfKey, inplace_reversing_sequence_memoizer<InputT, typename DpfKey::interior_node_t>, InputT>(recipe);
}

template <typename DpfKey,
          typename InputT = typename DpfKey::input_type>
auto make_double_space_sequence_memoizer(const DpfKey &, const list_recipe<InputT> & recipe)
{
    return detail::make_sequence_memoizer<DpfKey, double_space_sequence_memoizer<InputT, typename DpfKey::interior_node_t>, InputT>(recipe);
}

template <typename DpfKey,
          typename InputT>
auto make_full_tree_sequence_memoizer(const DpfKey &, const list_recipe<InputT> & recipe)
{
    return detail::make_sequence_memoizer<DpfKey, full_tree_sequence_memoizer<InputT, typename DpfKey::interior_node_t>, InputT>(recipe);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_SEQUENCE_MEMOIZER_HPP__
