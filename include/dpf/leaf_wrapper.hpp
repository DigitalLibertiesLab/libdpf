/// @file dpf/leaf_wrapper.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_LEAF_WRAPPER_HPP__
#define LIBDPF_INCLUDE_DPF_LEAF_WRAPPER_HPP__

namespace dpf
{

template <typename OutputT,
          typename NodeT>
struct leaf_wrapper
{
  public:
    using leaf_type = dpf::leaf_node_t<NodeT, OutputT>;
    using output_type = OutputT;

    leaf_wrapper() = delete;
    leaf_wrapper(leaf_type leaf, dpf::beaver<false, NodeT, OutputT> = dpf::beaver<false, NodeT, OutputT>{})
        : leaf_{std::forward<leaf_type>(leaf)} { }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr const leaf_type & get() const noexcept { return leaf_; }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_PURE
    HEDLEY_NO_THROW
    constexpr bool is_ready() const noexcept { return true; }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_PURE
    HEDLEY_NO_THROW
    static constexpr bool is_wildcard() noexcept { return false; }

  private:
    leaf_type leaf_;
};

// // unpacked wildcard reconstruction
// template <typename ConcreteOutputT,
//           typename NodeT>
// struct leaf_wrapper<wildcard_value<ConcreteOutputT>, NodeT, false>
// {
//   public:
//     using leaf_type = dpf::leaf_node_t<NodeT, ConcreteOutputT>;
//     using output_type = ConcreteOutputT;

//     leaf_wrapper() = delete;
//     leaf_wrapper(leaf_type leaf_share, dpf::beaver<NodeT, output_type> = dpf::beaver<NodeT, output_type>{})
//         : leaf_{leaf_share},
//           leaf_state_(std::make_unique<std::atomic<leaf_status>>(leaf_status::notset)),
//           ready_{false} { }

//     HEDLEY_ALWAYS_INLINE
//     const leaf_type & get() const
//     {
//         if (HEDLEY_UNLIKELY(!ready_))
//         {
//             throw std::runtime_error("offset not set");
//         }
//         return leaf_;
//     }

//     const leaf_type compute_and_get_leaf_share(output_type output_share)
//     {
//         leaf_status notset = leaf_status::notset;
//         if (HEDLEY_UNLIKELY(!leaf_state_->compare_exchange_strong(notset,
//             leaf_status::computing,
//             std::memory_order_seq_cst, std::memory_order_relaxed)))
//         {
//             throw std::runtime_error("invalid state transition");
//         }
//         leaf_type tmp;
//         std::memcpy(&tmp, &output_share, sizeof(output_type));
//         leaf_ = add_leaf<output_type>(leaf_, tmp);
//         leaf_state_->store(leaf_status::waiting, std::memory_order_release);
//         return leaf_;
//     }

//     const leaf_type reconstruct_correction_word(leaf_type other_share)
//     {
//         leaf_status waiting = leaf_status::waiting;
//         if (HEDLEY_UNLIKELY(!leaf_state_->compare_exchange_strong(waiting,
//             leaf_status::computing,
//             std::memory_order_acquire, std::memory_order_relaxed)))
//         {
//             throw std::runtime_error("invalid state transition");
//         }
//         leaf_ = add_leaf<output_type>(leaf_, other_share);
//         ready_ = true;
//         leaf_state_->store(leaf_status::ready, std::memory_order_relaxed);
//         return leaf_;
//     }

//     HEDLEY_ALWAYS_INLINE
//     HEDLEY_NO_THROW
//     bool is_ready() const noexcept { return ready_; }

//     HEDLEY_ALWAYS_INLINE
//     HEDLEY_PURE
//     HEDLEY_NO_THROW
//     static constexpr bool is_wildcard() noexcept { return true; }

// //   private:
//     enum class leaf_status : psnip_uint8_t { ready = 0, waiting = 1, computing = 2, notset = 3 };
//     leaf_type leaf_;
//     std::unique_ptr<std::atomic<leaf_status>> leaf_state_;
//     bool ready_;
// };

template <typename ConcreteOutputT,
          typename NodeT>
struct leaf_wrapper<wildcard_value<ConcreteOutputT>, NodeT>
{
  public:
    using node_type = NodeT;
    using output_type = ConcreteOutputT;
    using leaf_type = dpf::leaf_node_t<node_type, output_type>;
    using beaver_type = dpf::beaver<true, node_type, output_type>;

    leaf_wrapper() = delete;
    leaf_wrapper(leaf_type leaf_share, beaver_type beaver)
        : leaf_{std::forward<leaf_type>(leaf_share)},
          beaver_{beaver},
          output_share_{},
          leaf_state_(std::make_unique<std::atomic<leaf_status>>(leaf_status::notset)),
          ready_{false}
    { }

    HEDLEY_ALWAYS_INLINE
    const leaf_type & get() const
    {
        if (HEDLEY_UNLIKELY(!ready_))
        {
            throw std::runtime_error("offset not set");
        }
        return leaf_;
    }

    const output_type compute_and_get_blinded_output_share(output_type output_share)
    {
        leaf_status notset = leaf_status::notset;
        if (HEDLEY_UNLIKELY(!leaf_state_->compare_exchange_strong(notset,
            leaf_status::computing,
            std::memory_order_seq_cst, std::memory_order_relaxed)))
        {
            throw std::runtime_error("invalid state transition");
        }
        output_share_ = output_share;
        auto blinded_output_share = output_share_ + beaver_.output_blind;
        leaf_state_->store(leaf_status::blinded, std::memory_order_release);
        return blinded_output_share;
    }

    const leaf_type compute_and_get_leaf_share(output_type other_output_share)
    {
        leaf_status blinded = leaf_status::blinded;
        if (HEDLEY_UNLIKELY(!leaf_state_->compare_exchange_strong(blinded,
            leaf_status::computing,
            std::memory_order_acquire, std::memory_order_relaxed)))
        {
            throw std::runtime_error("invalid state transition");
        }
        leaf_ = add_leaf<output_type>(leaf_, subtract_leaf<output_type>(
            multiply_leaf(beaver_.blinded_vector, output_share_),
            multiply_leaf(beaver_.vector_blind, other_output_share)));
        leaf_state_->store(leaf_status::waiting, std::memory_order_release);
        return leaf_;
    }

    const leaf_type reconstruct_correction_word(leaf_type other_share)
    {
        leaf_status waiting = leaf_status::waiting;
        if (HEDLEY_UNLIKELY(!leaf_state_->compare_exchange_strong(waiting,
            leaf_status::computing,
            std::memory_order_acquire, std::memory_order_relaxed)))
        {
            throw std::runtime_error("invalid state transition");
        }
        leaf_ = add_leaf<output_type>(leaf_, other_share);
        ready_ = true;
        leaf_state_->store(leaf_status::ready, std::memory_order_relaxed);
        return leaf_;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    bool is_ready() const noexcept { return ready_; }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_PURE
    HEDLEY_NO_THROW
    static constexpr bool is_wildcard() noexcept { return true; }

  private:
    enum class leaf_status : psnip_uint8_t { ready = 0, waiting = 1, computing = 2, blinded = 3, notset = 4 };
    leaf_type leaf_;
    beaver_type beaver_;
    output_type output_share_;
    std::unique_ptr<std::atomic<leaf_status>> leaf_state_;
    bool ready_;
};

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_LEAF_WRAPPER_HPP__
