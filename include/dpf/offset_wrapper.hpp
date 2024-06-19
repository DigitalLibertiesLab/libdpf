/// @file dpf/offset_wrapper.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_OFFSET_WRAPPER_HPP__
#define LIBDPF_INCLUDE_DPF_OFFSET_WRAPPER_HPP__

namespace dpf
{

template <typename InputT>
struct offset_wrapper final
{
  public:
    using input_type = dpf::concrete_type_t<InputT>;

    offset_wrapper(input_type = input_type{})
        : offset_{} { }

    template <typename InputType>
    HEDLEY_ALWAYS_INLINE
    HEDLEY_CONST
    HEDLEY_NO_THROW
    constexpr auto operator()(InputType && x) const noexcept
    {
        static_assert(std::is_convertible_v<InputType, input_type>);
        return std::forward<input_type>(x);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_PURE
    HEDLEY_NO_THROW
    constexpr bool is_ready() const noexcept { return true; }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_PURE
    HEDLEY_NO_THROW
    static constexpr bool is_wildcard() noexcept { return false; }

  private:
    input_type offset_;  // waste an `input_type` to make `sizeof` match up
};

template <typename ConcreteInputT>
struct offset_wrapper<dpf::wildcard_value<ConcreteInputT>>
{
  public:
    using input_type = ConcreteInputT;

    offset_wrapper(input_type x)
        : offset_{x},
          offset_state_(std::make_unique<std::atomic<offset_status>>(offset_status::notset)),
          ready_{false} 
    { }

    template <typename InputType>
    HEDLEY_INLINE
    input_type operator()(InputType && x) const
    {
        static_assert(std::is_convertible_v<InputType, input_type>);
        if (HEDLEY_UNLIKELY(!ready_))
        {
            throw std::runtime_error("offset not set");
        }
        return input_type(x) + offset_;
    }

    template <typename InputType>
    const input_type & compute_and_get_share(InputType && input_share)
    {
        static_assert(std::is_convertible_v<InputType, input_type>);
        offset_status notset = offset_status::notset;
        if (HEDLEY_UNLIKELY(!offset_state_->compare_exchange_strong(notset,
            offset_status::computing,
            std::memory_order_seq_cst, std::memory_order_relaxed)))
        {
            throw std::runtime_error("invalid state transition");
        }
        offset_ -= input_share;
        offset_state_->store(offset_status::waiting, std::memory_order_release);
        return offset_; 
    }

    template <typename InputType>
    const input_type & reconstruct(InputType && other_share)
    {
        static_assert(std::is_convertible_v<InputType, input_type>);
        offset_status waiting = offset_status::waiting;
        if (HEDLEY_UNLIKELY(!offset_state_->compare_exchange_strong(waiting,
            offset_status::computing,
            std::memory_order_acquire, std::memory_order_relaxed)))
        {
            throw std::runtime_error("invalid state transition");
        }
        offset_ += other_share;
        ready_ = true;
        offset_state_->store(offset_status::ready, std::memory_order_release);
        return offset_;
    }

    template <typename InputType>
    const input_type & set(InputType && offset)
    {
        static_assert(std::is_convertible_v<InputType, input_type>);
        offset_status notset = offset_status::notset;
        if (HEDLEY_UNLIKELY(!offset_state_->compare_exchange_strong(notset,
            offset_status::computing,
            std::memory_order_acquire, std::memory_order_relaxed)))
        {
            throw std::runtime_error("invalid state transition");
        }
        offset_ += offset;
        ready_ = true;
        offset_state_->store(offset_status::ready, std::memory_order_release);
        return offset_;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    bool is_ready() const noexcept { return ready_; }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_PURE
    HEDLEY_NO_THROW
    static constexpr bool is_wildcard() noexcept { return true; }
  private:
    enum class offset_status : psnip_uint8_t { ready = 0, waiting = 1, computing = 2, notset = 3 };
    input_type offset_;
    std::unique_ptr<std::atomic<offset_status>> offset_state_;
    bool ready_;
};

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_OFFSET_WRAPPER_HPP__
