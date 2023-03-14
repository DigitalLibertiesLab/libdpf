/// @file dpf/path_memoizer.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_PATH_MEMOIZER_HPP__
#define LIBDPF_INCLUDE_DPF_PATH_MEMOIZER_HPP__

namespace dpf
{

template <typename DpfKey>
struct alignas(alignof(typename DpfKey::interior_node_t)) basic_path_memoizer
    : public std::array<typename DpfKey::interior_node_t, DpfKey::depth+1>
{
  public:
    using dpf_type = DpfKey;
    using input_type = typename dpf_type::input_type;
    using node_type = typename dpf_type::interior_node_t;
    static constexpr auto depth = dpf_type::depth;
    
    HEDLEY_ALWAYS_INLINE
    explicit basic_path_memoizer(const dpf_type & dpf)
      : std::array<node_type, depth+1>{dpf.root}, dpf_{std::cref(dpf)}, x_{std::nullopt} { }
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
struct nonmemoizing_path_memoizer
{
  public:
    using dpf_type = DpfKey;
    using input_type = typename dpf_type::input_type;
    using node_type = typename dpf_type::interior_node_t;

    HEDLEY_ALWAYS_INLINE
    explicit nonmemoizing_path_memoizer(const dpf_type & dpf)
      : dpf_{std::cref(dpf)}, v{dpf.root} { }
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
    const node_type & operator[](std::size_t) const noexcept { return v; }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    node_type & operator[](std::size_t) noexcept { return v; }

  private:
    std::optional<std::reference_wrapper<const dpf_type>> dpf_;
    node_type v;
};

template <typename DpfKey>
auto make_path_memoizer(const DpfKey & dpf)
{
    return basic_path_memoizer(dpf);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_PATH_MEMOIZER_HPP__
