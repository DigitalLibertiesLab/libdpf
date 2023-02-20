/// @file dpf/leaf_arithmetic.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief miscellaneous helper functions, structs, preprocessor directives
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_LEAF_ARITHMETIC_HPP__
#define LIBDPF_INCLUDE_DPF_LEAF_ARITHMETIC_HPP__

#include <functional>

#include "simde/simde/x86/avx2.h"

#include "dpf/bit.hpp"
#include "dpf/wildcard.hpp"
#include "dpf/xor_wrapper.hpp"

namespace dpf
{

template <typename output_t, typename node_t> struct add_t;
template <typename output_t, typename node_t> struct subtract_t;

template <typename output_t, typename node_t>
static constexpr auto add = add_t<output_t, node_t>{};

template <typename output_t, typename node_t>
static constexpr auto subtract = subtract_t<dpf::actual_type_t<output_t>, node_t>{};

namespace detail
{

// /// @brief adds vectors of 8-bit integral types
// template <typename node_t> struct add8_t;
// /// @brief adds vectors of 16-bit integral types
// template <typename node_t> struct add16_t;
// /// @brief adds vectors of 32-bit integral types
// template <typename node_t> struct add32_t;
// /// @brief adds vectors of 64-bit integral types
// template <typename node_t> struct add64_t;

/// @brief Function object for adding vectors of `16x8`-bit integral types
struct add16x8_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m128i & a, const simde__m128i & b) const
    {
        return simde_mm_add_epi8(a, b);
    }
};

/// @brief Function object for adding vectors of `8x16`-bit integral types
struct add8x16_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m128i & a, const simde__m128i & b) const
    {
        return simde_mm_add_epi16(a, b);
    }
};

/// @brief Function object for adding vectors of `4x32`-bit integral types
struct add4x32_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m128i & a, const simde__m128i & b) const
    {
        return simde_mm_add_epi32(a, b);
    }
};

/// @brief Function object for adding vectors of `2x64`-bit integral types
struct add2x64_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m128i & a, const simde__m128i & b) const
    {
        return simde_mm_add_epi64(a, b);
    }
};

}  // namespace detail

template <typename node_t, typename T>
struct add_t<node_t, dpf::xor_wrapper<T>> : public std::bit_xor<> {};
template <typename node_t> struct add_t<dpf::bit, node_t> : public std::bit_xor<> {};

template <typename node_t> struct add_t<bool, node_t> : public detail::add16x8_t {};
template <typename node_t> struct add_t<char, node_t> : public detail::add16x8_t {};
// template <typename node_t> struct add_t<unsigned char, node_t> : public detail::add16x8_t {};
template <typename node_t> struct add_t<int8_t, node_t> : public detail::add16x8_t {};
template <typename node_t> struct add_t<uint8_t, node_t> : public detail::add16x8_t {};

template <typename node_t> struct add_t<int16_t, node_t> : public detail::add8x16_t {};
template <typename node_t> struct add_t<uint16_t, node_t> : public detail::add8x16_t {};

template <typename node_t> struct add_t<int32_t, node_t> : public detail::add4x32_t {};
template <typename node_t> struct add_t<uint32_t, node_t> : public detail::add4x32_t {};

template <typename node_t> struct add_t<int64_t, node_t> : public detail::add2x64_t {};
template <typename node_t> struct add_t<uint64_t, node_t> : public detail::add2x64_t {};

template <typename node_t> struct add_t<simde_int128, node_t> : public std::plus<simde_int128> {};
template <typename node_t> struct add_t<simde_uint128, node_t> : public std::plus<simde_uint128> {};

template <typename node_t> struct add_t<float, node_t> : public std::bit_xor<> {};
template <typename node_t> struct add_t<double, node_t> : public std::bit_xor<> {};

namespace detail
{

// /// @brief subtracts vectors of 8-bit integral types
// template <typename node_t> struct sub8_t;
// /// @brief subtracts vectors of 16-bit integral types
// template <typename node_t> struct sub16_t;
// /// @brief subtracts vectors of 32-bit integral types
// template <typename node_t> struct sub32_t;
// /// @brief subtracts vectors of 64-bit integral types
// template <typename node_t> struct sub64_t;

/// @brief Function object for subtracting vectors of `16x8`-bit integral types
struct sub16x8_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m128i & a, const simde__m128i & b) const
    {
        return simde_mm_sub_epi8(a, b);
    }
};

/// @brief Function object for subtracting vectors of `8x16`-bit integral types
struct sub8x16_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m128i & a, const simde__m128i & b) const
    {
        return simde_mm_sub_epi16(a, b);
    }
};

/// @brief Function object for subtracting vectors of `4x32`-bit integral types
struct sub4x32_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m128i & a, const simde__m128i & b) const
    {
        return simde_mm_sub_epi32(a, b);
    }
};

/// @brief Function object for subtracting vectors of `2x64`-bit integral types
struct sub2x64_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m128i & a, const simde__m128i & b) const
    {
        return simde_mm_sub_epi64(a, b);
    }
};

}  // namespace detail

template <typename node_t, typename T>
struct subtract_t<node_t, dpf::xor_wrapper<T>> : public std::bit_xor<> {};
template <typename node_t> struct subtract_t<dpf::bit, node_t> : public std::bit_xor<> {};

template <typename node_t> struct subtract_t<bool, node_t> : public detail::sub16x8_t {};
template <typename node_t> struct subtract_t<char, node_t> : public detail::sub16x8_t {};
// template <typename node_t> struct subtract_t<unsigned char, node_t> : public detail::sub16x8_t {};
template <typename node_t> struct subtract_t<int8_t, node_t> : public detail::sub16x8_t {};
template <typename node_t> struct subtract_t<uint8_t, node_t> : public detail::sub16x8_t {};

template <typename node_t> struct subtract_t<int16_t, node_t> : public detail::sub8x16_t {};
template <typename node_t> struct subtract_t<uint16_t, node_t> : public detail::sub8x16_t {};

template <typename node_t> struct subtract_t<int32_t, node_t> : public detail::sub4x32_t {};
template <typename node_t> struct subtract_t<uint32_t, node_t> : public detail::sub4x32_t {};

template <typename node_t> struct subtract_t<int64_t, node_t> : public detail::sub2x64_t {};
template <typename node_t> struct subtract_t<uint64_t, node_t> : public detail::sub2x64_t {};

template <typename node_t> struct subtract_t<simde_int128, node_t> : public std::minus<simde_int128> {};
template <typename node_t> struct subtract_t<simde_uint128, node_t> : public std::minus<simde_uint128> {};

template <typename node_t> struct subtract_t<float, node_t> : public std::bit_xor<> {};
template <typename node_t> struct subtract_t<double, node_t> : public std::bit_xor<> {};

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_LEAF_ARITHMETIC_HPP__
