/// @file dpf/path_memoizer.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_PATH_MEMOIZER_HPP__
#define LIBDPF_INCLUDE_DPF_PATH_MEMOIZER_HPP__

namespace dpf
{

template <typename DpfKey,
          typename ReturnT = const typename DpfKey::interior_node *>
struct path_memoizer_base
{
  public:
    using dpf_type = DpfKey;
    using input_type = typename DpfKey::input_type;
    using node_type = typename DpfKey::interior_node;
    using return_type = ReturnT;
    using iterator_type = return_type;

    virtual std::size_t assign_x(const dpf_type &, input_type) noexcept = 0;
    virtual node_type & operator[](std::size_t) noexcept = 0;

    virtual return_type begin() const noexcept = 0;
    virtual return_type end() const noexcept = 0;
};

template <typename DpfKey>
struct alignas(alignof(typename DpfKey::interior_node))
basic_path_memoizer final
    : public path_memoizer_base<DpfKey>
{
  public:
    using dpf_type = DpfKey;
    using input_type = typename DpfKey::input_type;
    using node_type = typename DpfKey::interior_node;
    using return_type = std::add_pointer_t<std::add_const_t<node_type>>;
    using iterator_type = return_type;
    static constexpr auto depth = DpfKey::depth;

    basic_path_memoizer()
      : dpf_{std::nullopt}, x_{std::nullopt} { }
    basic_path_memoizer(basic_path_memoizer &&) noexcept = default;
    basic_path_memoizer(const basic_path_memoizer &) = default;
    basic_path_memoizer & operator=(basic_path_memoizer &&) noexcept = default;
    basic_path_memoizer & operator=(const basic_path_memoizer &) = default;
    ~basic_path_memoizer() = default;

    std::size_t assign_x(const dpf_type & dpf, input_type new_x) noexcept override
    {
        static constexpr auto clz_xor = utils::countl_zero_symmmetric_difference<input_type>{};
        if (dpf_.has_value() == true && std::addressof(dpf_->get()) == std::addressof(dpf))
        {
            static constexpr auto complement_of = std::bit_not{};
            input_type old_x = x_.value_or(complement_of(new_x));
            x_ = new_x;
            return clz_xor(old_x, new_x)+1;
        }

        this->operator[](0) = dpf.root;
        dpf_ = std::cref(dpf);
        x_ = new_x;
        return 1;
    }

    node_type & operator[](std::size_t i) noexcept override
    {
        return arr_[i];
    }

    return_type begin() const noexcept override
    {
        if (x_.has_value() == true)
        {
            return std::addressof(arr_[depth]);
        }
        else
        {
            return end();
        }
    }

    return_type end() const noexcept override
    {
        return std::addressof(arr_[depth+1]);
    }

  private:
    std::optional<std::reference_wrapper<const dpf_type>> dpf_;
    std::optional<input_type> x_;
    std::array<node_type, depth+1> arr_;
};

template <typename DpfKey>
struct nonmemoizing_path_memoizer final : public path_memoizer_base<DpfKey>
{
  public:
    using dpf_type = DpfKey;
    using input_type = typename DpfKey::input_type;
    using node_type = typename DpfKey::interior_node;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using return_type = std::add_pointer_t<std::add_const_t<node_type>>;
HEDLEY_PRAGMA(GCC diagnostic pop)
    using iterator_type = return_type;

    nonmemoizing_path_memoizer()
      : dpf_{std::nullopt} { }
    nonmemoizing_path_memoizer(nonmemoizing_path_memoizer &&) noexcept = default;
    nonmemoizing_path_memoizer(const nonmemoizing_path_memoizer &) = default;
    nonmemoizing_path_memoizer & operator=(nonmemoizing_path_memoizer &&) noexcept = default;
    nonmemoizing_path_memoizer & operator=(const nonmemoizing_path_memoizer &) = default;
    ~nonmemoizing_path_memoizer() = default;

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    std::size_t assign_x(const dpf_type & dpf, input_type) noexcept override
    {
        if (dpf_.has_value() == false || std::addressof(dpf_->get()) != std::addressof(dpf))
        {
            dpf_ = dpf;
            v = dpf.root;
        }
        return 1;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    node_type & operator[](std::size_t) noexcept override
    {
        return v;
    }

    return_type begin() const noexcept override
    {
        return std::addressof(v);
    }

    return_type end() const noexcept override
    {
        return std::addressof(v) + 1;
    }

  private:
    std::optional<std::reference_wrapper<const dpf_type>> dpf_;
    node_type v;
};

namespace detail
{

template <typename MemoizerT>
HEDLEY_ALWAYS_INLINE
auto make_path_memoizer()
{
    return MemoizerT();
}

}  // namespace dpf::detail

template <typename DpfKey>
auto make_basic_path_memoizer(const DpfKey &)
{
    return detail::make_path_memoizer<basic_path_memoizer<DpfKey>>();
}

template <typename DpfKey>
auto make_nonmemoizing_path_memoizer(const DpfKey &)
{
    return detail::make_path_memoizer<nonmemoizing_path_memoizer<DpfKey>>();
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_PATH_MEMOIZER_HPP__
