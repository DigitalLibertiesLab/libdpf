/// @file dpf/prg_dummy.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_PRG_DUMMY_HPP__
#define LIBDPF_INCLUDE_DPF_PRG_DUMMY_HPP__

#include <array>
#include <algorithm>

#include "hedley/hedley.h"
#include "portable-snippets/exact-int/exact-int.h"

#include "dpf/utils.hpp"

namespace dpf
{

namespace prg
{

struct dummy final
{
    using block_type = simde__m128i;

    static block_type eval(block_type seed, psnip_uint32_t)
    {
        return seed;
    }

    static auto eval01(block_type seed)
    {
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
        return std::array<block_type, 2>{seed, seed};
HEDLEY_PRAGMA(GCC diagnostic pop)
    }

    static void eval(block_type seed, block_type * HEDLEY_RESTRICT output,
        psnip_uint32_t count_, psnip_uint32_t = 0)
    {
        std::fill_n(output, count_, seed);
    }
};  // struct dummy

}  // namespace prg

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_PRG_DUMMY_HPP__
