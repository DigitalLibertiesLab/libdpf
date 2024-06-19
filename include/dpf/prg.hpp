/// @file dpf/prg.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_PRG_HPP__
#define LIBDPF_INCLUDE_DPF_PRG_HPP__

#include <atomic>

#include "dpf/prg_aes.hpp"
#include "dpf/prg_dummy.hpp"

namespace dpf
{

namespace prg
{

template <typename PRG>
struct counter_wrapper final
{
    using block_type = typename PRG::block_type;

    static block_type eval(block_type seed, psnip_uint32_t pos)
    {
        count_.fetch_add(1, std::memory_order::memory_order_relaxed);
        return prg_.eval(seed, pos);
    }

    static auto eval01(block_type seed)
    {
        count_.fetch_add(2, std::memory_order::memory_order_relaxed);
        return prg_.eval01(seed);
    }

    static void eval(block_type seed, block_type * HEDLEY_RESTRICT output,
        psnip_uint32_t count, psnip_uint32_t pos = 0)
    {
        count_.fetch_add(count, std::memory_order::memory_order_relaxed);
        prg_.eval(seed, output, count, pos);
    }

    static std::size_t count()
    {
        return count_;
    }

  private:
    static const PRG prg_;
    inline static std::atomic_size_t count_{0};
};  // struct counter_wrapper

}  // namespace prg

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_PRG_HPP__
