/// @file dpf.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief includes all headers needed for basic libdpf++ functionality
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_HPP__
#define LIBDPF_INCLUDE_DPF_HPP__

#include "dpf/bit.hpp"

#include "dpf/bit_array.hpp"

#include "dpf/bitstring.hpp"

#include "dpf/modint.hpp"

#include "dpf/keyword.hpp"

#include "dpf/fixedpoint.hpp"

#include "dpf/setbit_index_iterable.hpp"

#include "dpf/parallel_bit_iterable.hpp"

#include "dpf/output_buffer.hpp"

#include "dpf/prg_aes.hpp"

#include "dpf/eval.hpp"
  // #include "dpf/eval_point.hpp"
  // #include "dpf/eval_interval.hpp"
  // #include "dpf/eval_sequence.hpp"

#include "dpf/wildcard.hpp"

#include "dpf/xor_wrapper.hpp"

#ifdef HAS_NLOHMANN_JSON
  #ifndef NLOHMANN_JSON_VERSION_MAJOR
    // was told you use nlohmann::json, but it's not available!
  #else
    #include "dpf/json.hpp"
  #endif
#endif

#endif  // LIBDPF_INCLUDE_DPF_HPP__
