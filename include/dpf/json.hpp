/// @file dpf/json.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_JSON_HPP__
#define LIBDPF_INCLUDE_DPF_JSON_HPP__

#include <array>
#include <bitset>

#include "dpf/dpf_key.hpp"

#include "json/include/nlohmann/json.hpp"

namespace nlohmann
{

template <typename InteriorPRG,
          typename ExteriorPRG,
          typename InputT,
          typename OutputT,
          typename ...OutputTs>
struct adl_serializer<dpf::dpf_key<InteriorPRG, ExteriorPRG, InputT, OutputT, OutputTs...>>
{
    using dpf_type = dpf::dpf_key<InteriorPRG, ExteriorPRG, InputT, OutputT, OutputTs...>;

    // the "normal" method for serializing involves overloading `to_json` and `from_json` for the generic specialization of 
    // adl_serializer to use. This doesn't work for non-default constructible types.
    // But for arbitrary movable types, the docs say you can just specialize adl_serializer
    // like the sigs below [https://json.nlohmann.me/features/arbitrary_types/#how-do-i-convert-third-party-types]
    static dpf_type from_json(const nlohmann::json & j);

    static void to_json(nlohmann::json & j, const dpf_type & dpf);
};

}  // namespace nlohmann

namespace dpf
{

namespace json
{

template <typename DpfKey>
static void to_json(nlohmann::json & j, const DpfKey & dpf);

template <typename DpfType>
static auto from_json(const nlohmann::json & j);

}  // namespace dpf::json

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_JSON_HPP__
