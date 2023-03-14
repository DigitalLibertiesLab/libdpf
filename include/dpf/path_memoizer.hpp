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

template <typename DpfKey>
struct alignas(alignof(typename DpfKey::interior_node_t))
basic_path_memoizer final
    : public std::array<typename DpfKey::interior_node_t, DpfKey::depth+1>
{
  public:
    using dpf_type = DpfKey;
    using input_type = typename dpf_type::input_type;
    using node_type = typename dpf_type::interior_node_t;
    static constexpr auto depth = dpf_type::depth;
    
    basic_path_memoizer()
      : dpf_{std::nullopt}, x_{std::nullopt} { }
    basic_path_memoizer(basic_path_memoizer &&) = default;
    basic_path_memoizer(const basic_path_memoizer &) = default;

    inline std::size_t assign_x(const dpf_type & dpf, input_type new_x)
    {
        static constexpr auto clz_xor = utils::countl_zero_symmmetric_difference<input_type>{};
        if (std::addressof(dpf_->get()) == std::addressof(dpf))
        {
            static constexpr auto complement_of = std::bit_not{};
            input_type old_x = x_.value_or(complement_of(new_x));
            x_ = new_x;
            return clz_xor(old_x, new_x);
        }
        else
        {
            this->operator[](0) = dpf.root;
            dpf_ = std::cref(dpf);
            x_ = new_x;
            return 0;
        }
    }

  private:
    std::optional<std::reference_wrapper<const dpf_type>> dpf_;
    std::optional<input_type> x_;
};

template <typename DpfKey>
struct nonmemoizing_path_memoizer final
{
  public:
    using dpf_type = DpfKey;
    using input_type = typename dpf_type::input_type;
    using node_type = typename dpf_type::interior_node_t;

    nonmemoizing_path_memoizer()
      : dpf_{std::nullopt} { }
    nonmemoizing_path_memoizer(nonmemoizing_path_memoizer &&) = default;
    nonmemoizing_path_memoizer(const nonmemoizing_path_memoizer &) = default;

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    std::size_t assign_x(const dpf_type & dpf, input_type) noexcept
    {
        if (std::addressof(dpf_->get()) != std::addressof(dpf)) v = dpf.root;
        return 0;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    node_type & operator[](std::size_t) noexcept { return v; }

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
    return detail::make_interval_memoizer<basic_path_memoizer<DpfKey>>();
}

template <typename DpfKey>
auto make_nonmemoizing_path_memoizer(const DpfKey &)
{
    return detail::make_interval_memoizer<nonmemoizing_path_memoizer<DpfKey>>();
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_PATH_MEMOIZER_HPP__
