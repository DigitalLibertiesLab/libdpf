/// @file grotto/gadgets/elementary/nonnegative.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_NONNEGATIVE_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_NONNEGATIVE_HPP__

#include "geq.hpp"

namespace grotto
{

namespace gadgets
{

static constexpr double geq_target_zero = 0;
using nonnegative = geq<geq_target_zero>;

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ELEMENTARY_NONNEGATIVE_HPP__
