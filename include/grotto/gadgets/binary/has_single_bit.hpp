/// @file grotto/gadgets/binary/has_single_bit.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief gadgetized form of `std::has_single_bit`
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_BINARY_HAS_SINGLE_BIT_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_BINARY_HAS_SINGLE_BIT_HPP__

#include <array>
#include <limits>
#include <portable-snippets/builtin/builtin.h>

#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

struct has_single_bit
{
    template <typename T>
    T operator()(T x)
    {
        if (x == 0) return 0;
        uint64_t x_ = static_cast<uint64_t>(static_cast<double>(x > 0 ? x : 1.0 / x));
        return psnip_builtin_popcount64(x_) == 1 ? 1 : 0;
    }
};

template <>
struct gadget_hints<has_single_bit>
{
    static constexpr double min = std::numeric_limits<double>::min();
    static constexpr double max = std::numeric_limits<double>::max();
    static constexpr double poles[] = { };
    static constexpr double interesting_points[] = {
        -0x1.0000000000000p+0063, +0x1.0000000000000p-0063,
        +0x1.0000000000000p-0062, +0x1.0000000000000p-0061,
        +0x1.0000000000000p-0060, +0x1.0000000000000p-0059,
        +0x1.0000000000000p-0058, +0x1.0000000000000p-0057,
        +0x1.0000000000000p-0056, +0x1.0000000000000p-0055,
        +0x1.0000000000000p-0054, +0x1.0000000000000p-0053,
        +0x1.0000000000000p-0052, +0x1.0000000000000p-0051,
        +0x1.0000000000000p-0050, +0x1.0000000000000p-0049,
        +0x1.0000000000000p-0048, +0x1.0000000000000p-0047,
        +0x1.0000000000000p-0046, +0x1.0000000000000p-0045,
        +0x1.0000000000000p-0044, +0x1.0000000000000p-0043,
        +0x1.0000000000000p-0042, +0x1.0000000000000p-0041,
        +0x1.0000000000000p-0040, +0x1.0000000000000p-0039,
        +0x1.0000000000000p-0038, +0x1.0000000000000p-0037,
        +0x1.0000000000000p-0036, +0x1.0000000000000p-0035,
        +0x1.0000000000000p-0034, +0x1.0000000000000p-0033,
        +0x1.0000000000000p-0032, +0x1.0000000000000p-0031,
        +0x1.0000000000000p-0030, +0x1.0000000000000p-0029,
        +0x1.0000000000000p-0028, +0x1.0000000000000p-0027,
        +0x1.0000000000000p-0026, +0x1.0000000000000p-0025,
        +0x1.0000000000000p-0024, +0x1.0000000000000p-0023,
        +0x1.0000000000000p-0022, +0x1.0000000000000p-0021,
        +0x1.0000000000000p-0020, +0x1.0000000000000p-0019,
        +0x1.0000000000000p-0018, +0x1.0000000000000p-0017,
        +0x1.0000000000000p-0016, +0x1.0000000000000p-0015,
        +0x1.0000000000000p-0014, +0x1.0000000000000p-0013,
        +0x1.0000000000000p-0012, +0x1.0000000000000p-0011,
        +0x1.0000000000000p-0010, +0x1.0000000000000p-0009,
        +0x1.0000000000000p-0008, +0x1.0000000000000p-0007,
        +0x1.0000000000000p-0006, +0x1.0000000000000p-0005,
        +0x1.0000000000000p-0004, +0x1.0000000000000p-0003,
        +0x1.0000000000000p-0002, +0x1.0000000000000p-0001,
        +0x1.0000000000000p+0000, +0x1.0000000000000p+0001,
        +0x1.0000000000000p+0002, +0x1.0000000000000p+0003,
        +0x1.0000000000000p+0004, +0x1.0000000000000p+0005,
        +0x1.0000000000000p+0006, +0x1.0000000000000p+0007,
        +0x1.0000000000000p+0008, +0x1.0000000000000p+0009,
        +0x1.0000000000000p+0010, +0x1.0000000000000p+0011,
        +0x1.0000000000000p+0012, +0x1.0000000000000p+0013,
        +0x1.0000000000000p+0014, +0x1.0000000000000p+0015,
        +0x1.0000000000000p+0016, +0x1.0000000000000p+0017,
        +0x1.0000000000000p+0018, +0x1.0000000000000p+0019,
        +0x1.0000000000000p+0020, +0x1.0000000000000p+0021,
        +0x1.0000000000000p+0022, +0x1.0000000000000p+0023,
        +0x1.0000000000000p+0024, +0x1.0000000000000p+0025,
        +0x1.0000000000000p+0026, +0x1.0000000000000p+0027,
        +0x1.0000000000000p+0028, +0x1.0000000000000p+0029,
        +0x1.0000000000000p+0030, +0x1.0000000000000p+0031,
        +0x1.0000000000000p+0032, +0x1.0000000000000p+0033,
        +0x1.0000000000000p+0034, +0x1.0000000000000p+0035,
        +0x1.0000000000000p+0036, +0x1.0000000000000p+0037,
        +0x1.0000000000000p+0038, +0x1.0000000000000p+0039,
        +0x1.0000000000000p+0040, +0x1.0000000000000p+0041,
        +0x1.0000000000000p+0042, +0x1.0000000000000p+0043,
        +0x1.0000000000000p+0044, +0x1.0000000000000p+0045,
        +0x1.0000000000000p+0046, +0x1.0000000000000p+0047,
        +0x1.0000000000000p+0048, +0x1.0000000000000p+0049,
        +0x1.0000000000000p+0050, +0x1.0000000000000p+0051,
        +0x1.0000000000000p+0052, +0x1.0000000000000p+0053,
        +0x1.0000000000000p+0054, +0x1.0000000000000p+0055,
        +0x1.0000000000000p+0056, +0x1.0000000000000p+0057,
        +0x1.0000000000000p+0058, +0x1.0000000000000p+0059,
        +0x1.0000000000000p+0060, +0x1.0000000000000p+0061,
        +0x1.0000000000000p+0062 };
    static constexpr unsigned degree = 0;
    static constexpr bool has_canonical_representation = true;
    static constexpr double canonical_bounds[] = {
        -0x1.0000000000000p+0063, -0x1.0000000000000p+0063,
        +0x1.0000000000000p-0063, +0x1.0000000000000p-0063, 
        +0x1.0000000000000p-0062, +0x1.0000000000000p-0062, 
        +0x1.0000000000000p-0061, +0x1.0000000000000p-0061, 
        +0x1.0000000000000p-0060, +0x1.0000000000000p-0060, 
        +0x1.0000000000000p-0059, +0x1.0000000000000p-0059, 
        +0x1.0000000000000p-0058, +0x1.0000000000000p-0058, 
        +0x1.0000000000000p-0057, +0x1.0000000000000p-0057, 
        +0x1.0000000000000p-0056, +0x1.0000000000000p-0056, 
        +0x1.0000000000000p-0055, +0x1.0000000000000p-0055, 
        +0x1.0000000000000p-0054, +0x1.0000000000000p-0054, 
        +0x1.0000000000000p-0053, +0x1.0000000000000p-0053, 
        +0x1.0000000000000p-0052, +0x1.0000000000000p-0052, 
        +0x1.0000000000000p-0051, +0x1.0000000000000p-0051, 
        +0x1.0000000000000p-0050, +0x1.0000000000000p-0050, 
        +0x1.0000000000000p-0049, +0x1.0000000000000p-0049, 
        +0x1.0000000000000p-0048, +0x1.0000000000000p-0048, 
        +0x1.0000000000000p-0047, +0x1.0000000000000p-0047, 
        +0x1.0000000000000p-0046, +0x1.0000000000000p-0046, 
        +0x1.0000000000000p-0045, +0x1.0000000000000p-0045, 
        +0x1.0000000000000p-0044, +0x1.0000000000000p-0044, 
        +0x1.0000000000000p-0043, +0x1.0000000000000p-0043, 
        +0x1.0000000000000p-0042, +0x1.0000000000000p-0042, 
        +0x1.0000000000000p-0041, +0x1.0000000000000p-0041, 
        +0x1.0000000000000p-0040, +0x1.0000000000000p-0040, 
        +0x1.0000000000000p-0039, +0x1.0000000000000p-0039, 
        +0x1.0000000000000p-0038, +0x1.0000000000000p-0038, 
        +0x1.0000000000000p-0037, +0x1.0000000000000p-0037, 
        +0x1.0000000000000p-0036, +0x1.0000000000000p-0036, 
        +0x1.0000000000000p-0035, +0x1.0000000000000p-0035, 
        +0x1.0000000000000p-0034, +0x1.0000000000000p-0034, 
        +0x1.0000000000000p-0033, +0x1.0000000000000p-0033, 
        +0x1.0000000000000p-0032, +0x1.0000000000000p-0032, 
        +0x1.0000000000000p-0031, +0x1.0000000000000p-0031, 
        +0x1.0000000000000p-0030, +0x1.0000000000000p-0030, 
        +0x1.0000000000000p-0029, +0x1.0000000000000p-0029, 
        +0x1.0000000000000p-0028, +0x1.0000000000000p-0028, 
        +0x1.0000000000000p-0027, +0x1.0000000000000p-0027, 
        +0x1.0000000000000p-0026, +0x1.0000000000000p-0026, 
        +0x1.0000000000000p-0025, +0x1.0000000000000p-0025, 
        +0x1.0000000000000p-0024, +0x1.0000000000000p-0024, 
        +0x1.0000000000000p-0023, +0x1.0000000000000p-0023, 
        +0x1.0000000000000p-0022, +0x1.0000000000000p-0022, 
        +0x1.0000000000000p-0021, +0x1.0000000000000p-0021, 
        +0x1.0000000000000p-0020, +0x1.0000000000000p-0020, 
        +0x1.0000000000000p-0019, +0x1.0000000000000p-0019, 
        +0x1.0000000000000p-0018, +0x1.0000000000000p-0018, 
        +0x1.0000000000000p-0017, +0x1.0000000000000p-0017, 
        +0x1.0000000000000p-0016, +0x1.0000000000000p-0016, 
        +0x1.0000000000000p-0015, +0x1.0000000000000p-0015, 
        +0x1.0000000000000p-0014, +0x1.0000000000000p-0014, 
        +0x1.0000000000000p-0013, +0x1.0000000000000p-0013, 
        +0x1.0000000000000p-0012, +0x1.0000000000000p-0012, 
        +0x1.0000000000000p-0011, +0x1.0000000000000p-0011, 
        +0x1.0000000000000p-0010, +0x1.0000000000000p-0010, 
        +0x1.0000000000000p-0009, +0x1.0000000000000p-0009, 
        +0x1.0000000000000p-0008, +0x1.0000000000000p-0008, 
        +0x1.0000000000000p-0007, +0x1.0000000000000p-0007, 
        +0x1.0000000000000p-0006, +0x1.0000000000000p-0006, 
        +0x1.0000000000000p-0005, +0x1.0000000000000p-0005, 
        +0x1.0000000000000p-0004, +0x1.0000000000000p-0004, 
        +0x1.0000000000000p-0003, +0x1.0000000000000p-0003, 
        +0x1.0000000000000p-0002, +0x1.0000000000000p-0002, 
        +0x1.0000000000000p-0001, +0x1.0000000000000p-0001, 
        +0x1.0000000000000p+0000, +0x1.0000000000000p+0000, 
        +0x1.0000000000000p+0001, +0x1.0000000000000p+0001, 
        +0x1.0000000000000p+0002, +0x1.0000000000000p+0002, 
        +0x1.0000000000000p+0003, +0x1.0000000000000p+0003, 
        +0x1.0000000000000p+0004, +0x1.0000000000000p+0004, 
        +0x1.0000000000000p+0005, +0x1.0000000000000p+0005, 
        +0x1.0000000000000p+0006, +0x1.0000000000000p+0006, 
        +0x1.0000000000000p+0007, +0x1.0000000000000p+0007, 
        +0x1.0000000000000p+0008, +0x1.0000000000000p+0008, 
        +0x1.0000000000000p+0009, +0x1.0000000000000p+0009, 
        +0x1.0000000000000p+0010, +0x1.0000000000000p+0010, 
        +0x1.0000000000000p+0011, +0x1.0000000000000p+0011, 
        +0x1.0000000000000p+0012, +0x1.0000000000000p+0012, 
        +0x1.0000000000000p+0013, +0x1.0000000000000p+0013, 
        +0x1.0000000000000p+0014, +0x1.0000000000000p+0014, 
        +0x1.0000000000000p+0015, +0x1.0000000000000p+0015, 
        +0x1.0000000000000p+0016, +0x1.0000000000000p+0016, 
        +0x1.0000000000000p+0017, +0x1.0000000000000p+0017, 
        +0x1.0000000000000p+0018, +0x1.0000000000000p+0018, 
        +0x1.0000000000000p+0019, +0x1.0000000000000p+0019, 
        +0x1.0000000000000p+0020, +0x1.0000000000000p+0020, 
        +0x1.0000000000000p+0021, +0x1.0000000000000p+0021, 
        +0x1.0000000000000p+0022, +0x1.0000000000000p+0022, 
        +0x1.0000000000000p+0023, +0x1.0000000000000p+0023, 
        +0x1.0000000000000p+0024, +0x1.0000000000000p+0024, 
        +0x1.0000000000000p+0025, +0x1.0000000000000p+0025, 
        +0x1.0000000000000p+0026, +0x1.0000000000000p+0026, 
        +0x1.0000000000000p+0027, +0x1.0000000000000p+0027, 
        +0x1.0000000000000p+0028, +0x1.0000000000000p+0028, 
        +0x1.0000000000000p+0029, +0x1.0000000000000p+0029, 
        +0x1.0000000000000p+0030, +0x1.0000000000000p+0030, 
        +0x1.0000000000000p+0031, +0x1.0000000000000p+0031, 
        +0x1.0000000000000p+0032, +0x1.0000000000000p+0032, 
        +0x1.0000000000000p+0033, +0x1.0000000000000p+0033, 
        +0x1.0000000000000p+0034, +0x1.0000000000000p+0034, 
        +0x1.0000000000000p+0035, +0x1.0000000000000p+0035, 
        +0x1.0000000000000p+0036, +0x1.0000000000000p+0036, 
        +0x1.0000000000000p+0037, +0x1.0000000000000p+0037, 
        +0x1.0000000000000p+0038, +0x1.0000000000000p+0038, 
        +0x1.0000000000000p+0039, +0x1.0000000000000p+0039, 
        +0x1.0000000000000p+0040, +0x1.0000000000000p+0040, 
        +0x1.0000000000000p+0041, +0x1.0000000000000p+0041, 
        +0x1.0000000000000p+0042, +0x1.0000000000000p+0042, 
        +0x1.0000000000000p+0043, +0x1.0000000000000p+0043, 
        +0x1.0000000000000p+0044, +0x1.0000000000000p+0044, 
        +0x1.0000000000000p+0045, +0x1.0000000000000p+0045, 
        +0x1.0000000000000p+0046, +0x1.0000000000000p+0046, 
        +0x1.0000000000000p+0047, +0x1.0000000000000p+0047, 
        +0x1.0000000000000p+0048, +0x1.0000000000000p+0048, 
        +0x1.0000000000000p+0049, +0x1.0000000000000p+0049, 
        +0x1.0000000000000p+0050, +0x1.0000000000000p+0050, 
        +0x1.0000000000000p+0051, +0x1.0000000000000p+0051, 
        +0x1.0000000000000p+0052, +0x1.0000000000000p+0052, 
        +0x1.0000000000000p+0053, +0x1.0000000000000p+0053, 
        +0x1.0000000000000p+0054, +0x1.0000000000000p+0054, 
        +0x1.0000000000000p+0055, +0x1.0000000000000p+0055, 
        +0x1.0000000000000p+0056, +0x1.0000000000000p+0056, 
        +0x1.0000000000000p+0057, +0x1.0000000000000p+0057, 
        +0x1.0000000000000p+0058, +0x1.0000000000000p+0058, 
        +0x1.0000000000000p+0059, +0x1.0000000000000p+0059, 
        +0x1.0000000000000p+0060, +0x1.0000000000000p+0060, 
        +0x1.0000000000000p+0061, +0x1.0000000000000p+0061, 
        +0x1.0000000000000p+0062, +0x1.0000000000000p+0062
    };
    static constexpr std::array<double, degree+1> canonical_polys[] = {
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
        0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0
    };
};

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_BINARY_HAS_SINGLE_BIT_HPP__