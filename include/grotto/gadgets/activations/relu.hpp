/// @file grotto/gadgets/activations/relu.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_RELU_HPP__
#define LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_RELU_HPP__

#include <cmath>
#include <array>
#include <limits>

#include "leakyrelu.hpp"
#include "grotto/gadget_hints.hpp"

namespace grotto
{

namespace gadgets
{

using relu = leakyrelu<leakyrelu_zero_negative_slope>;

}  // namespace gadgets

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_GADGETS_ACTIVATIONS_RELU_HPP__
