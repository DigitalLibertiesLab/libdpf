/// @file dpf/parallel_bit_iterable_helpers.hpp
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @brief
/// @details
/// @copyright Copyright (c) 2019-2023 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_PARALLEL_BIT_ITERABLE_HELPERS_HPP__
#define LIBDPF_INCLUDE_DPF_PARALLEL_BIT_ITERABLE_HELPERS_HPP__

#include <cstddef>
#include <limits>
#include <array>

#include "thirdparty/simde/x86/avx2.h"
#include "thirdparty/psnips/exact-int/exact-int.h"

#include "dpf/bit_array.hpp"

namespace dpf
{

namespace
{

template <std::size_t batch_size_log_2, typename ChildT>
struct parallel_bit_iterable_helper;

/// @brief for batch_size in 1..4
template <typename ChildT>
struct parallel_bit_iterable_helper<2, ChildT>
{
  public:
    using word_pointer = typename dpf::bit_array_base<ChildT>::word_pointer;
    using simde_type = simde__m256i;
    using simde_ptr = simde_type *;
    using element_type = psnip_uint64_t;
    static constexpr auto bits_per_word = dpf::bit_array_base<ChildT>::bits_per_word;
    static constexpr auto bits_per_element = std::numeric_limits<element_type>::digits;
    static constexpr auto elements_per_word = bits_per_word / bits_per_element;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using simde_array = std::array<simde_type, elements_per_word>;
HEDLEY_PRAGMA(GCC diagnostic pop)
    static constexpr auto left_shift = simde_mm256_slli_epi64;
    static constexpr auto right_shift = simde_mm256_srli_epi64;
    static constexpr auto bit_and = simde_mm256_and_si256;
    static auto get_mask() noexcept
    {
        return simde_mm256_set1_epi64x(1);
    }
    static simde_array build_vecs(word_pointer cur_word)
    {
        return { simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word)) };
    }
};  // struct parallel_bit_iterable_helper<2>

/// @brief for batch_size in 5..8
template <typename ChildT>
struct parallel_bit_iterable_helper<3, ChildT>
{
  public:
    using word_pointer = typename dpf::bit_array_base<ChildT>::word_pointer;
    using simde_type = simde__m256i;
    using simde_ptr = simde_type *;
    using element_type = psnip_uint32_t;
    static constexpr auto bits_per_word = dpf::bit_array_base<ChildT>::bits_per_word;
    static constexpr auto bits_per_element = std::numeric_limits<element_type>::digits;
    static constexpr auto elements_per_word = bits_per_word / bits_per_element;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using simde_array = std::array<simde_type, elements_per_word>;
HEDLEY_PRAGMA(GCC diagnostic pop)
    static constexpr auto left_shift = simde_mm256_slli_epi64;
    static constexpr auto right_shift = simde_mm256_srli_epi64;
    static constexpr auto bit_and = simde_mm256_and_si256;
    static auto get_mask() noexcept
    {
        return simde_mm256_set1_epi32(1);
    }
    static simde_array build_vecs(word_pointer cur_word)
    {
        simde_type vec1 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word));
        simde_type vec2 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word+4));

        vec1 = simde_mm256_permutevar8x32_epi32(vec1, mask1);
        vec2 = simde_mm256_permutevar8x32_epi32(vec2, mask1);

        return {
            simde_mm256_permute2x128_si256(vec1, vec2, 0b00100000),
            simde_mm256_permute2x128_si256(vec1, vec2, 0b00110001) };
    }
  private:
    static constexpr simde_type mask1 = {
        0b0000000000000000000000000000001000000000000000000000000000000000,
        0b0000000000000000000000000000011000000000000000000000000000000100,
        0b0000000000000000000000000000001100000000000000000000000000000001,
        0b0000000000000000000000000000011100000000000000000000000000000101};
};  // struct parallel_bit_iterable_helper<3>

/// @brief for batch_size in 9..16
template <typename ChildT>
struct parallel_bit_iterable_helper<4, ChildT>
{
  public:
    using word_pointer = typename dpf::bit_array_base<ChildT>::word_pointer;
    using simde_type = simde__m256i;
    using simde_ptr = simde_type *;
    using element_type = psnip_uint16_t;
    static constexpr auto bits_per_word = dpf::bit_array_base<ChildT>::bits_per_word;
    static constexpr auto bits_per_element = std::numeric_limits<element_type>::digits;
    static constexpr auto elements_per_word = bits_per_word / bits_per_element;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using simde_array = std::array<simde_type, elements_per_word>;
HEDLEY_PRAGMA(GCC diagnostic pop)
    static constexpr auto left_shift = simde_mm256_slli_epi64;
    static constexpr auto right_shift = simde_mm256_srli_epi64;
    static constexpr auto bit_and = simde_mm256_and_si256;
    static auto get_mask() noexcept
    {
        return simde_mm256_set1_epi16(1);
    }
    static simde_array build_vecs(word_pointer cur_word)
    {
        simde_type vec1 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word));
        simde_type vec2 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word+4));
        simde_type vec3 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word+8));
        simde_type vec4 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word+12));

        vec1 = simde_mm256_permutevar8x32_epi32(vec1, mask1);
        vec2 = simde_mm256_permutevar8x32_epi32(vec2, mask1);
        vec3 = simde_mm256_permutevar8x32_epi32(vec3, mask1);
        vec4 = simde_mm256_permutevar8x32_epi32(vec4, mask1);

        vec1 = simde_mm256_shuffle_epi8(vec1, mask2);
        vec2 = simde_mm256_shuffle_epi8(vec2, mask2);
        vec3 = simde_mm256_shuffle_epi8(vec3, mask2);
        vec4 = simde_mm256_shuffle_epi8(vec4, mask2);

        simde_type bld_0002 = simde_mm256_unpacklo_epi64(vec1, vec2);
        simde_type bld_0406 = simde_mm256_unpacklo_epi64(vec3, vec4);
        simde_type bld_0103 = simde_mm256_unpackhi_epi64(vec1, vec2);
        simde_type bld_0507 = simde_mm256_unpackhi_epi64(vec3, vec4);

        return {
            simde_mm256_permute2x128_si256(bld_0002, bld_0406, 0b00100000),
            simde_mm256_permute2x128_si256(bld_0103, bld_0507, 0b00100000),
            simde_mm256_permute2x128_si256(bld_0002, bld_0406, 0b00110001),
            simde_mm256_permute2x128_si256(bld_0103, bld_0507, 0b00110001) };
    }
  private:
    static constexpr simde_type mask1 = {
        0b0000000000000000000000000000001000000000000000000000000000000000,
        0b0000000000000000000000000000011000000000000000000000000000000100,
        0b0000000000000000000000000000001100000000000000000000000000000001,
        0b0000000000000000000000000000011100000000000000000000000000000101};
    static constexpr simde_type mask2 = {
        0b0000110100001100000010010000100000000101000001000000000100000000,
        0b0000111100001110000010110000101000000111000001100000001100000010,
        0b0000110100001100000010010000100000000101000001000000000100000000,
        0b0000111100001110000010110000101000000111000001100000001100000010};
};  // struct parallel_bit_iterable_helper<4>

/// @brief for batch_size in 17..32
template <typename ChildT>
struct parallel_bit_iterable_helper<5, ChildT>
{
  public:
    using word_pointer = typename dpf::bit_array_base<ChildT>::word_pointer;
    using simde_type = simde__m256i;
    using simde_ptr = simde_type *;
    using element_type = psnip_uint8_t;
    static constexpr auto bits_per_word = dpf::bit_array_base<ChildT>::bits_per_word;
    static constexpr auto bits_per_element = std::numeric_limits<element_type>::digits;
    static constexpr auto elements_per_word = bits_per_word / bits_per_element;
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using simde_array = std::array<simde_type, elements_per_word>;
HEDLEY_PRAGMA(GCC diagnostic pop)
    static constexpr auto left_shift = simde_mm256_slli_epi64;
    static constexpr auto right_shift = simde_mm256_srli_epi64;
    static constexpr auto bit_and = simde_mm256_and_si256;
    static auto get_mask() noexcept
    {
        return simde_mm256_set1_epi8(1);
    }
    static simde_array build_vecs(word_pointer cur_word)
    {
        simde_type vec1 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word));
        simde_type vec2 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word+4));
        simde_type vec3 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word+8));
        simde_type vec4 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word+12));
        simde_type vec5 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word+16));
        simde_type vec6 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word+20));
        simde_type vec7 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word+24));
        simde_type vec8 = simde_mm256_loadu_si256(reinterpret_cast<simde_ptr>(cur_word+28));

        vec1 = simde_mm256_permutevar8x32_epi32(vec1, mask1);
        vec2 = simde_mm256_permutevar8x32_epi32(vec2, mask1);
        vec3 = simde_mm256_permutevar8x32_epi32(vec3, mask1);
        vec4 = simde_mm256_permutevar8x32_epi32(vec4, mask1);
        vec5 = simde_mm256_permutevar8x32_epi32(vec5, mask1);
        vec6 = simde_mm256_permutevar8x32_epi32(vec6, mask1);
        vec7 = simde_mm256_permutevar8x32_epi32(vec7, mask1);
        vec8 = simde_mm256_permutevar8x32_epi32(vec8, mask1);

        vec1 = simde_mm256_shuffle_epi8(vec1, mask2);
        vec2 = simde_mm256_shuffle_epi8(vec2, mask2);
        vec3 = simde_mm256_shuffle_epi8(vec3, mask2);
        vec4 = simde_mm256_shuffle_epi8(vec4, mask2);
        vec5 = simde_mm256_shuffle_epi8(vec5, mask2);
        vec6 = simde_mm256_shuffle_epi8(vec6, mask2);
        vec7 = simde_mm256_shuffle_epi8(vec7, mask2);
        vec8 = simde_mm256_shuffle_epi8(vec8, mask2);

        simde_type bld_00010405 = simde_mm256_unpacklo_epi32(vec1, vec2);
        simde_type bld_08091213 = simde_mm256_unpacklo_epi32(vec3, vec4);
        simde_type bld_16172021 = simde_mm256_unpacklo_epi32(vec5, vec6);
        simde_type bld_24252829 = simde_mm256_unpacklo_epi32(vec7, vec8);
        simde_type bld_02030607 = simde_mm256_unpackhi_epi32(vec1, vec2);
        simde_type bld_10111415 = simde_mm256_unpackhi_epi32(vec3, vec4);
        simde_type bld_18192223 = simde_mm256_unpackhi_epi32(vec5, vec6);
        simde_type bld_26273031 = simde_mm256_unpackhi_epi32(vec7, vec8);

        simde_type bld_0004 = simde_mm256_unpacklo_epi64(bld_00010405, bld_08091213);
        simde_type bld_1620 = simde_mm256_unpacklo_epi64(bld_16172021, bld_24252829);
        simde_type bld_0206 = simde_mm256_unpacklo_epi64(bld_02030607, bld_10111415);
        simde_type bld_1822 = simde_mm256_unpacklo_epi64(bld_18192223, bld_26273031);
        simde_type bld_0105 = simde_mm256_unpackhi_epi64(bld_00010405, bld_08091213);
        simde_type bld_1721 = simde_mm256_unpackhi_epi64(bld_16172021, bld_24252829);
        simde_type bld_0307 = simde_mm256_unpackhi_epi64(bld_02030607, bld_10111415);
        simde_type bld_1923 = simde_mm256_unpackhi_epi64(bld_18192223, bld_26273031);

        return {
            simde_mm256_permute2x128_si256(bld_0004, bld_1620, 0b00100000),
            simde_mm256_permute2x128_si256(bld_0105, bld_1721, 0b00100000),
            simde_mm256_permute2x128_si256(bld_0206, bld_1822, 0b00100000),
            simde_mm256_permute2x128_si256(bld_0307, bld_1923, 0b00100000),
            simde_mm256_permute2x128_si256(bld_0004, bld_1620, 0b00110001),
            simde_mm256_permute2x128_si256(bld_0105, bld_1721, 0b00110001),
            simde_mm256_permute2x128_si256(bld_0206, bld_1822, 0b00110001),
            simde_mm256_permute2x128_si256(bld_0307, bld_1923, 0b00110001) };
    }
  private:
    static constexpr simde_type mask1 = {
        0b0000000000000000000000000000001000000000000000000000000000000000,
        0b0000000000000000000000000000011000000000000000000000000000000100,
        0b0000000000000000000000000000001100000000000000000000000000000001,
        0b0000000000000000000000000000011100000000000000000000000000000101};
    static constexpr simde_type mask2 = {
        0b0000110100001001000001010000000100001100000010000000010000000000,
        0b0000111100001011000001110000001100001110000010100000011000000010,
        0b0000110100001001000001010000000100001100000010000000010000000000,
        0b0000111100001011000001110000001100001110000010100000011000000010};
};  // struct parallel_bit_iterable_helper<5>

}  // anonymous namespace

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_PARALLEL_BIT_ITERABLE_HELPERS_HPP__
