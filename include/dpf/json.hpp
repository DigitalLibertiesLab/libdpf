/// @file dpf/json.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_JSON_HPP__
#define LIBDPF_INCLUDE_DPF_JSON_HPP__

#include <array>
#include <bitset>

#include "dpf/dpf_key.hpp"

#include "json/include/nlohmann/json.hpp"

namespace nlohmann
{

template <typename NodeT,
          typename OutputT,
          std::size_t outputs_per_leaf>
struct adl_serializer<beaver<true, NodeT, OutputT, outputs_per_leaf>>
{
    static void from_json(const nlohmann::json &, beaver<true, NodeT, OutputT, outputs_per_leaf> & beaver)
    {
        j.get_to(beaver.output_blind);
        j.get_to(beaver.vector_blind);
        j.get_to(beaver.blinded_vector);
    }

    static void to_json(nlohmann::json &, beaver<true, NodeT, OutputT, outputs_per_leaf> & beaver)
    {
        j = nlohmann::json{
            {"output_blind", beaver.output_blind},
            {"vector_blind", beaver.vector_blind},
            {"blinded_vector", beaver.blinded_vector}
        };
    }
};

template <>
struct adl_serializer<simde__m128i>
{
    static void from_json(const nlohmann::json & j, simde__m128i & a)
    {
        std::array<psnip_uint64_t, 2> A;
        j.get_to(A);
        a = simde_mm_set_epi64x(A[1], A[0]);
    }

    static void to_json(nlohmann::json & j, const simde__m128i & a)
    {
        j = nlohmann::json{a[0], a[1]};
    }
};

template <>
struct adl_serializer<simde__m256i>
{
    static void from_json(const nlohmann::json & j, simde__m256i & a)
    {
        std::array<psnip_uint64_t, 4> A;
        j.get_to(A);
        a = simde_mm256_set_epi64x(A[3], A[2], A[1], A[0]);
    }

    static void to_json(nlohmann::json & j, const simde__m256i & a)
    {
        j = nlohmann::json{a[0], a[1], a[2], a[3]};
    }
};

template <typename InteriorPRG,
          typename ExteriorPRG,
          typename InputT,
          typename OutputT,
          typename ...OutputTs>
struct adl_serializer<dpf::dpf_key<InteriorPRG, ExteriorPRG, InputT, OutputT, OutputTs...>>
{
    using dpf_type = dpf::dpf_key<InteriorPRG, ExteriorPRG, InputT, OutputT, OutputTs...>;
    using interior_node = typename dpf_type::interior_node;
    using leaf_tuple = typename dpf_type::leaf_tuple;
    using beaver_tuple = typename dpf_type::beaver_tuple;

    static dpf_type from_json(const nlohmann::json & j)
    {
        interior_node root;
        j.at("root").get_to(root);
        std::array<interior_node, dpf_type::depth> correction_words;
        j.at("correction_words").get_to(correction_words);
        std::array<uint8_t, dpf_type::depth> correction_advice;
        j.at("correction_advice").get_to(correction_advice);
        leaf_tuple leaves;
        j.at("leaves").get_to(leaves);
        std::string wildcard_mask_str;
        j.at("wildcards").get_to(wildcard_mask_str);
        beaver_tuple beavers;
        j.at("beavers").get_to(beavers);

        return dpf_type{
            root,
            correction_words,
            correction_advice,
            leaves,
            std::bitset<std::tuple_size_v<leaf_tuple>>(wildcard_mask_str),
            beavers
        };
    }

    static void to_json(nlohmann::json & j, const dpf_type & dpf)
    {
        j = nlohmann::json{
            {"root", dpf.root},
            {"correction_words", dpf.correction_words},
            {"correction_advice", dpf.correction_advice},
            {"leaves", dpf.mutable_leaf_tuple()},
            {"wildcards", dpf.mutable_wildcard_mask()},
            {"beavers", dpf.mutable_beaver_tuple()}
        };
    }
};

}  // namespace nlohmann

namespace dpf
{

namespace json
{

template <typename DpfKey>
static std::string to_json(const DpfKey & dpf)
{
    nlohmann::json json = dpf;
    return json.dump();
}

template <typename DpfType>
static auto from_json(std::string json_string)
{
    nlohmann::json json = nlohmann::json::parse(json_string);
    return static_cast<DpfType>(json);
}

}  // namespace dpf::json

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_JSON_HPP__
