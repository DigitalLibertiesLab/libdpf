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

#include "dpf/sequence_recipe.hpp"

namespace dpf
{

template <typename DpfKey,
          typename ReturnT = typename DpfKey::interior_node *>
struct sequence_memoizer_base
{
  public:
    using dpf_type = DpfKey;
    using input_type = typename DpfKey::input_type;
    using return_type = ReturnT;
    using iterator_type = return_type;
    const sequence_recipe<input_type> & recipe;

    // level 0 should access the root
    // level goes up to (and including) depth
    virtual return_type operator[](std::size_t) const noexcept = 0;

    // iterators should access most recently completed level
    virtual return_type begin() const noexcept = 0;
    virtual return_type end() const noexcept = 0;

    virtual std::size_t assign_dpf(const dpf_type & dpf, const sequence_recipe<input_type> & r)
    {
        if (&recipe != &r)
        {
            throw std::logic_error("memoizer cannot be used with different recipe");
        }

        if (dpf_.has_value() == false || std::addressof(dpf_->get()) != std::addressof(dpf))
        {
            this->operator[](0)[0] = dpf.root;
            dpf_ = std::cref(dpf);
            level_index = 1;
        }

        return level_index;
    }

    std::size_t advance_level()
    {
        return ++level_index;
    }

    std::size_t get_nodes_at_level() const
    {
        return get_nodes_at_level(level_index);
    }

    std::size_t get_nodes_at_level(std::size_t level) const
    {
        if (level == size_t(-1))
        {
            return 0;
        }

        if (level == depth)
        {
            return recipe.num_leaf_nodes();
        }

        return recipe.level_endpoints()[level+1] - recipe.level_endpoints()[level];
    }

    // returns true if first traversal should be taken
    // this usually means traversing left, but for the inplace_reversing memoizer
    // if it is working in reverse, this could be a right traversal
    virtual bool traverse_first(std::size_t step) const
    {
        return recipe.recipe_steps()[step] > int8_t(-1);
    }

    // returns true if second traversal should be taken
    // this usually means traversing right, but for the inplace_reversing memoizer
    // if it is working in reverse, this could be a left traversal
    virtual bool traverse_second(std::size_t step) const
    {
        return recipe.recipe_steps()[step] < int8_t(1);
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
    std::size_t depth;
    std::size_t level_index;  // indicates current level being built

    explicit sequence_memoizer_base(const sequence_recipe<input_type> & r)
      : recipe{r},
        depth{recipe.level_endpoints().size()-1},
        level_index{0},
        dpf_{std::nullopt}
    { }

  private:
    std::optional<std::reference_wrapper<const dpf_type>> dpf_;
};

namespace detail
{

template <typename ForwardIterT,
          typename ReverseIterT>
struct pointer_facade
{
  public:
    using forward_iter = ForwardIterT;
    using reverse_iter = ReverseIterT;

    using value_type = typename std::iterator_traits<ForwardIterT>::value_type;
    using reference = value_type &;
    using const_reference = std::add_const_t<reference>;
    using pointer = std::add_pointer_t<value_type>;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::pair<std::ptrdiff_t, std::ptrdiff_t>;

    HEDLEY_ALWAYS_INLINE
    pointer_facade(bool flip, forward_iter it, reverse_iter rit)
        : flip_{flip}, it_{it}, rit_{rit}
    { }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    reference operator*() const noexcept
    {
        return flip_ ? *rit_ : *it_;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    pointer_facade & operator++() noexcept
    {
        ++it_;
        ++rit_;
        return *this;
    }

    HEDLEY_NO_THROW
    pointer_facade operator++(int) noexcept
    {
        auto tmp = *this;
        pointer_facade::operator++();
        return tmp;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    pointer_facade & operator--() noexcept
    {
        --it_;
        --rit_;
        return *this;
    }

    HEDLEY_NO_THROW
    pointer_facade operator--(int) noexcept
    {
        auto tmp = *this;
        pointer_facade::operator--();
        return tmp;
    }

    pointer_facade & operator+=(std::size_t n) noexcept
    {
        it_ += n;
        rit_ += n;
        return *this;
    }

    pointer_facade operator+(std::size_t n) const noexcept
    {
        return pointer_facade(flip_, it_ + n, rit_ + n);
    }

    pointer_facade & operator-=(std::size_t n) noexcept
    {
        it_ -= n;
        rit_ -= n;
        return *this;
    }

    pointer_facade operator-(std::size_t n) const noexcept
    {
        return pointer_facade(flip_, it_ - n, rit_ - n);
    }

    difference_type operator-(pointer_facade rhs) const noexcept
    {
        return std::make_pair(it_ - rhs.it_, rit_ - rhs.rit_);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    reference operator[](std::size_t i)
    {
        return flip_ ? rit_[i] : it_[i];
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    const_reference operator[](std::size_t i) const
    {
        return flip_ ? rit_[i] : it_[i];
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator==(const pointer_facade & rhs) const noexcept
    {
        return flip_ == rhs.flip_ && it_ == rhs.it_ && rit_ == rhs.rit_;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator!=(const pointer_facade & rhs) const noexcept
    {
        return !(*this == rhs);
    }

  private:
    bool flip_;
    forward_iter it_;
    reverse_iter rit_;
};

}  // namespace dpf::detail

template <typename DpfKey,
          typename Allocator = aligned_allocator<typename DpfKey::interior_node>>
struct inplace_reversing_sequence_memoizer final
  : public sequence_memoizer_base<DpfKey,
        detail::pointer_facade<typename DpfKey::interior_node *, std::reverse_iterator<typename DpfKey::interior_node *>>>
{
  public:
    using input_type = typename DpfKey::input_type;
    using unique_ptr = typename Allocator::unique_ptr;
    using forward_iter = typename DpfKey::interior_node *;
    using reverse_iter = std::reverse_iterator<forward_iter>;
    using return_type = detail::pointer_facade<forward_iter, reverse_iter>;
  private:
    using parent = sequence_memoizer_base<DpfKey, return_type>;
  public:
    using parent::recipe;
    using parent::depth;
    using parent::level_index;
    using parent::get_nodes_at_level;

    explicit inplace_reversing_sequence_memoizer(const sequence_recipe<input_type> & r,
        Allocator alloc = Allocator{})
      : parent::sequence_memoizer_base(r),
        buf{alloc.allocate_unique_ptr(r.num_leaf_nodes())}
    { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type operator[](std::size_t level) const noexcept override
    {
        // flip false => forward traversal
        bool flip = (depth ^ level) & 1;
        // first check used to determine if previous or current level is being requested
        // second check used to determine if the last layer is being requested
        //     in which case it is setup to always return buf in normal order
        if (level == level_index-1 && level != depth)
        {
            std::size_t nodes_at_level = get_nodes_at_level(level);
            return return_type(!flip, &buf[recipe.num_leaf_nodes()-nodes_at_level],
                std::make_reverse_iterator(&buf[nodes_at_level]));
        }

        return return_type(flip, &buf[0],
            std::make_reverse_iterator(&buf[recipe.num_leaf_nodes()]));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type begin() const noexcept override
    {
        std::size_t level = level_index-1;
        // flip false => forward traversal
        bool flip = (depth ^ level) & 1;
        return return_type(flip, &buf[0],
            std::make_reverse_iterator(&buf[recipe.num_leaf_nodes()]));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type end() const noexcept override
    {
        std::size_t level = level_index-1;
        // flip false => forward traversal
        bool flip = (depth ^ level) & 1;
        std::size_t nodes_at_level = get_nodes_at_level(level);
        return return_type(flip, &buf[nodes_at_level],
            std::make_reverse_iterator(&buf[recipe.num_leaf_nodes()-nodes_at_level]));
    }

    bool traverse_first(std::size_t step) const override
    {
        // flip false => forward traversal
        bool flip = (depth ^ level_index) & 1;
        step = !flip ? step : recipe.level_endpoints()[level_index] - step - 1 + recipe.level_endpoints()[level_index-1];
        return !flip ? (recipe.recipe_steps()[step] > int8_t(-1)) : (recipe.recipe_steps()[step] < int8_t(1));
    }

    bool traverse_second(std::size_t step) const override
    {
        // flip false => forward traversal
        bool flip = (depth ^ level_index) & 1;
        step = !flip ? step : recipe.level_endpoints()[level_index] - step - 1 + recipe.level_endpoints()[level_index-1];
        return !flip ? (recipe.recipe_steps()[step] < int8_t(1)) : (recipe.recipe_steps()[step] > int8_t(-1));
    }

    bool get_direction(bool right) const override
    {
        // flip false => forward traversal
        bool flip = (depth ^ level_index) & 1;
        return !flip ? right : !right;
    }

  private:
    unique_ptr buf;
};

template <typename DpfKey,
          typename Allocator = aligned_allocator<typename DpfKey::interior_node>>
struct double_space_sequence_memoizer final
  : public sequence_memoizer_base<DpfKey>
{
  private:
    using parent = sequence_memoizer_base<DpfKey>;
  public:
    using input_type = typename DpfKey::input_type;
    using unique_ptr = typename Allocator::unique_ptr;
    using return_type = typename DpfKey::interior_node *;
    using parent::recipe;
    using parent::depth;
    using parent::level_index;
    using parent::get_nodes_at_level;

    explicit double_space_sequence_memoizer(const sequence_recipe<input_type> & r, Allocator alloc = Allocator{})
      : parent::sequence_memoizer_base(r),
        buf{alloc.allocate_unique_ptr(2*recipe.num_leaf_nodes())}
    { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type operator[](std::size_t level) const noexcept override
    {
        auto b = (depth ^ level) & 1;
        return Allocator::assume_aligned(&buf[recipe.num_leaf_nodes()*b]);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type begin() const noexcept override
    {
        return this->operator[](level_index - 1);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type end() const noexcept override
    {
        return this->operator[](level_index - 1) + get_nodes_at_level(level_index - 1);
    }

  private:
    unique_ptr buf;
};

template <typename DpfKey,
          typename Allocator = aligned_allocator<typename DpfKey::interior_node>>
struct full_tree_sequence_memoizer final
  : public sequence_memoizer_base<DpfKey>
{
  private:
    using parent = sequence_memoizer_base<DpfKey>;
  public:
    using input_type = typename DpfKey::input_type;
    using unique_ptr = typename Allocator::unique_ptr;
    using return_type = typename DpfKey::interior_node *;
    using parent::recipe;
    using parent::level_index;
    using parent::get_nodes_at_level;

    explicit full_tree_sequence_memoizer(const sequence_recipe<input_type> & r, Allocator alloc = Allocator{})
      : parent::sequence_memoizer_base(r),
        buf{alloc.allocate_unique_ptr(recipe.level_endpoints()[recipe.level_endpoints().size()-1] + recipe.num_leaf_nodes())}
    { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type operator[](std::size_t level) const noexcept override
    {
        return Allocator::assume_aligned(&buf[recipe.level_endpoints()[level]]);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type begin() const noexcept override
    {
        return this->operator[](level_index - 1);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    return_type end() const noexcept override
    {
        return this->operator[](level_index - 1) + get_nodes_at_level(level_index - 1);
    }

  private:
    unique_ptr buf;
};

namespace detail
{

template <typename MemoizerT,
          typename InputT>
HEDLEY_ALWAYS_INLINE
auto make_sequence_memoizer(const sequence_recipe<InputT> & recipe)
{
    return MemoizerT(recipe);
}

}  // namespace dpf::detail

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
template <typename DpfKey,
          typename InputT = typename DpfKey::input_type>
auto make_inplace_reversing_sequence_memoizer(const DpfKey &, const sequence_recipe<InputT> & recipe)
{
    return detail::make_sequence_memoizer<inplace_reversing_sequence_memoizer<DpfKey>, InputT>(recipe);
}

template <typename DpfKey,
          typename InputT = typename DpfKey::input_type>
auto make_double_space_sequence_memoizer(const DpfKey &, const sequence_recipe<InputT> & recipe)
{
    return detail::make_sequence_memoizer<double_space_sequence_memoizer<DpfKey>, InputT>(recipe);
}

template <typename DpfKey,
          typename InputT = typename DpfKey::input_type>
auto make_full_tree_sequence_memoizer(const DpfKey &, const sequence_recipe<InputT> & recipe)
{
    return detail::make_sequence_memoizer<full_tree_sequence_memoizer<DpfKey>, InputT>(recipe);
}
HEDLEY_PRAGMA(GCC diagnostic pop)

}  // namespace dpf

namespace std
{

template <typename Iterator>
struct iterator_traits<dpf::detail::pointer_facade<Iterator, std::reverse_iterator<Iterator>>>
{
  private:
    using type = dpf::detail::pointer_facade<Iterator, std::reverse_iterator<Iterator>>;
  public:
    using iterator_category = typename type::iterator_category;
    using difference_type = typename type::difference_type;
    using value_type = typename type::value_type;
    using reference = typename type::reference;
    using const_reference = typename type::const_reference;
    using pointer = typename type::pointer;
};

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_SEQUENCE_MEMOIZER_HPP__
