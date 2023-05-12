/// @file dpf.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief includes all headers needed for basic libdpf++ functionality
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_HPP__
#define LIBDPF_INCLUDE_DPF_HPP__

#ifdef LIBDPF_HAS_ASIO
  #include <asio.hpp>
#endif  // LIBDPF_HAS_ASIO

#include "dpf/advice_bit_iterable.hpp"

#include "dpf/aligned_allocator.hpp"

#include "dpf/bit_array.hpp"

#include "dpf/bit.hpp"

#include "dpf/bitstring.hpp"

#include "dpf/dpf_key.hpp"

#include "dpf/eval_common.hpp"

#include "dpf/eval_interval.hpp"

#include "dpf/eval_full.hpp"

#include "dpf/eval_point.hpp"

#include "dpf/eval_sequence.hpp"

#include "dpf/interval_memoizer.hpp"

#ifdef LIBDPF_HAS_NLOHMANN_JSON
  #ifndef NLOHMANN_JSON_VERSION_MAJOR
    // was told you use nlohmann::json, but it's not available!
  #else
    #include "dpf/json.hpp"
  #endif  // NLOHMANN_JSON_VERSION_MAJOR
#endif  // LIBDPF_HAS_NLOHMANN_JSON

#include "dpf/keyword.hpp"

#include "dpf/leaf_arithmetic.hpp"

#include "dpf/leaf_node.hpp"

#include "dpf/modint.hpp"

#include "dpf/output_buffer.hpp"

#include "dpf/parallel_bit_iterable_helpers.hpp"

#include "dpf/parallel_bit_iterable.hpp"

#include "dpf/path_memoizer.hpp"

#include "dpf/prg_aes.hpp"

#include "dpf/prg_nonsecure_just_a_counter.hpp"

#include "dpf/random.hpp"

#include "dpf/sequence_memoizer.hpp"

#include "dpf/sequence_recipe.hpp"

#include "dpf/setbit_index_iterable.hpp"

#include "dpf/subinterval_iterable.hpp"

#include "dpf/subsequence_iterable.hpp"

#include "dpf/twiddle.hpp"

#include "dpf/utils.hpp"

#include "dpf/wildcard.hpp"

#include "dpf/xor_wrapper.hpp"

#include "dpf/zip_iterable.hpp"

#endif  // LIBDPF_INCLUDE_DPF_HPP__
