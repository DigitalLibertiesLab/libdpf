/// @file dpf/prg_aes.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_PRG_NONSECURE_JUST_A_COUNTER_HPP__
#define LIBDPF_INCLUDE_DPF_PRG_NONSECURE_JUST_A_COUNTER_HPP__

#include <array>

#include "hedley/hedley.h"

#include "dpf/utils.hpp"

namespace dpf
{

namespace prg
{

struct nonsecure_just_a_counter final
{
    using block_t = simde__m128i;

    static block_t eval(block_t seed, uint32_t)
    {
        count++;
        return seed;
    }

    static auto eval01(block_t seed)
    {
        count+=2;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
        return std::array<block_t, 2>{seed, seed};
HEDLEY_PRAGMA(GCC diagnostic pop)
    }

    static void eval(block_t seed, block_t * HEDLEY_RESTRICT output,
        uint32_t count_, uint32_t pos = 0)
    {
        count += count_;
        std::fill_n(output, count_, seed);
    }
  public:
    static std::size_t count;
};  // struct nonsecure_just_a_counter

}  // namespace prg

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_PRG_NONSECURE_JUST_A_COUNTER_HPP__
