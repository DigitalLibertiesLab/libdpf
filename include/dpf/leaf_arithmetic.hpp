/// @file dpf/leaf_arithmetic.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_LEAF_ARITHMETIC_HPP__
#define LIBDPF_INCLUDE_DPF_LEAF_ARITHMETIC_HPP__

#include <functional>
#include <cstring>

#include "simde/simde/x86/avx2.h"

#include "dpf/bit.hpp"
#include "dpf/wildcard.hpp"
#include "dpf/xor_wrapper.hpp"

namespace dpf
{

namespace leaf_arithmetic
{

template <typename OutputT, typename NodeT> struct add_t;
template <typename OutputT, typename NodeT> struct subtract_t;
template <typename OutputT, typename NodeT> struct multiply_t;

template <typename OutputT>
struct add_t<OutputT, void>
{
    template <typename NodeT>
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const NodeT & a, const NodeT & b) const
    {
        static constexpr auto adder = add_t<OutputT, NodeT>{};
        return adder(a, b);
    }
};

template <typename OutputT>
struct subtract_t<OutputT, void>
{
    template <typename NodeT>
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const NodeT & a, const NodeT & b) const
    {
        static constexpr auto subtracter = subtract_t<OutputT, NodeT>{};
        return subtracter(a, b);
    }
};

template <>
struct multiply_t<void, void>
{
    template <typename NodeT, typename OutputT>
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const NodeT & a, OutputT b) const
    {
        static constexpr auto multiplier = multiply_t<OutputT, NodeT>{};
        return multiplier(a, b);
    }
};

}

template <typename OutputT>
static constexpr auto add_leaf = leaf_arithmetic::add_t<OutputT, void>{};

template <typename OutputT>
static constexpr auto subtract_leaf = leaf_arithmetic::subtract_t<OutputT, void>{};

static constexpr auto multiply_leaf = leaf_arithmetic::multiply_t<void, void>{};

namespace leaf_arithmetic
{

namespace detail
{

// /// @brief adds vectors of 8-bit integral types
// template <typename NodeT> struct add8_t;
// /// @brief adds vectors of 16-bit integral types
// template <typename NodeT> struct add16_t;
// /// @brief adds vectors of 32-bit integral types
// template <typename NodeT> struct add32_t;
// /// @brief adds vectors of 64-bit integral types
// template <typename NodeT> struct add64_t;

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

/// @brief Function object for adding vectors of `32x8`-bit integral types
struct add32x8_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m256i & a, const simde__m256i & b) const
    {
        return simde_mm256_add_epi8(a, b);
    }
};

/// @brief Function object for adding vectors of `16x16`-bit integral types
struct add16x16_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m256i & a, const simde__m256i & b) const
    {
        return simde_mm256_add_epi16(a, b);
    }
};

/// @brief Function object for adding vectors of `8x32`-bit integral types
struct add8x32_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m256i & a, const simde__m256i & b) const
    {
        return simde_mm256_add_epi32(a, b);
    }
};

/// @brief Function object for adding vectors of `4x64`-bit integral types
struct add4x64_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m256i & a, const simde__m256i & b) const
    {
        return simde_mm256_add_epi64(a, b);
    }
};

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
struct add_array_t
{
    template <typename T, std::size_t N>
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const std::array<T, N> & a, const std::array<T, N> & b) const
    {
        std::array<simde__m128i, N> c;
        std::transform(std::begin(a), std::end(a), std::begin(b), std::begin(c),
            [](const T & a, const T & b)
            {
                return std::bit_xor<>{}(a, b);
            });
        return c;
    }
};
HEDLEY_PRAGMA(GCC diagnostic pop)

}  // namespace detail

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")

template <> struct add_t<bool, simde__m128i> final : public detail::add16x8_t {};
template <> struct add_t<char, simde__m128i> final : public detail::add16x8_t {};
// template <> struct add_t<unsigned char, simde__m128i> final : public detail::add16x8_t {};
template <> struct add_t<int8_t, simde__m128i> final : public detail::add16x8_t {};
template <> struct add_t<uint8_t, simde__m128i> final : public detail::add16x8_t {};

template <> struct add_t<int16_t, simde__m128i> final : public detail::add8x16_t {};
template <> struct add_t<uint16_t, simde__m128i> final : public detail::add8x16_t {};

template <> struct add_t<int32_t, simde__m128i> final : public detail::add4x32_t {};
template <> struct add_t<uint32_t, simde__m128i> final : public detail::add4x32_t {};

template <> struct add_t<int64_t, simde__m128i> final : public detail::add2x64_t {};
template <> struct add_t<uint64_t, simde__m128i> final : public detail::add2x64_t {};

template <> struct add_t<simde_int128, simde__m128i> final
{
    auto operator()(const simde__m128i & lhs, const simde__m128i & rhs) const
    {
        simde__m128i ret;
        simde_int128 lhs_, rhs_;
        std::memcpy(&lhs_, &lhs, sizeof(simde_int128));
        std::memcpy(&rhs_, &rhs, sizeof(simde_int128));
        simde_int128 sum = lhs_ + rhs_;
        std::memcpy(&ret, &sum, sizeof(simde__m128i));
        return ret;
    }
};
template <> struct add_t<simde_uint128, simde__m128i> final
{
    auto operator()(const simde__m128i & lhs, const simde__m128i & rhs) const
    {
        simde__m128i ret;
        simde_uint128 lhs_, rhs_;
        std::memcpy(&lhs_, &lhs, sizeof(simde_uint128));
        std::memcpy(&rhs_, &rhs, sizeof(simde_uint128));
        simde_uint128 sum = lhs_ + rhs_;
        std::memcpy(&ret, &sum, sizeof(simde__m128i));
        return ret;
    }
};

template <> struct add_t<bool, simde__m256i> final : public detail::add32x8_t {};
// template <> struct add_t<unsigned char, simde__m256i> final : public detail::add32x8_t {};
template <> struct add_t<int8_t, simde__m256i> final : public detail::add32x8_t {};
template <> struct add_t<uint8_t, simde__m256i> final : public detail::add32x8_t {};

template <> struct add_t<int16_t, simde__m256i> final : public detail::add16x16_t {};
template <> struct add_t<uint16_t, simde__m256i> final : public detail::add16x16_t {};

template <> struct add_t<int32_t, simde__m256i> final : public detail::add8x32_t {};
template <> struct add_t<uint32_t, simde__m256i> final : public detail::add8x32_t {};

template <> struct add_t<int64_t, simde__m256i> final : public detail::add4x64_t {};
template <> struct add_t<uint64_t, simde__m256i> final : public detail::add4x64_t {};

template <> struct add_t<simde_int128, simde__m256i> final
{
    auto operator()(const simde__m256i & lhs, const simde__m256i & rhs) const
    {
        simde__m256i ret;
        simde_int128 lhs_[2], rhs_[2];
        std::memcpy(&lhs_, &lhs, sizeof(simde_int128) * 2);
        std::memcpy(&rhs_, &rhs, sizeof(simde_int128) * 2);
        simde_int128 sum[2] = { lhs_[0] + rhs_[0], lhs_[1] + rhs_[1] };
        std::memcpy(&ret, &sum, sizeof(simde__m128i) * 2);
        return ret;
    }
};
template <> struct add_t<simde_uint128, simde__m256i> final
{
    auto operator()(const simde__m256i & lhs, const simde__m256i & rhs) const
    {
        simde__m256i ret;
        simde_uint128 lhs_[2], rhs_[2];
        std::memcpy(&lhs_, &lhs, sizeof(simde_uint128) * 2);
        std::memcpy(&rhs_, &rhs, sizeof(simde_uint128) * 2);
        simde_uint128 sum[2] = { lhs_[0] + rhs_[0], lhs_[1] + rhs_[1] };
        std::memcpy(&ret, &sum, sizeof(simde__m128i) * 2);
        return ret;
    }
};

template <typename OutputT, typename NodeT, std::size_t N>
struct add_t<OutputT, std::array<NodeT, N>> final : public detail::add_array_t {};

template <typename NodeT> struct add_t<float, NodeT> final : public std::bit_xor<> {};
template <typename NodeT> struct add_t<double, NodeT> final : public std::bit_xor<> {};
template <> struct add_t<dpf::bit, void> final : public std::bit_xor<> {};
template <typename NodeT> struct add_t<dpf::bit, NodeT> final : public std::bit_xor<> {};
template <typename T> struct add_t<xor_wrapper<T>, void> final : public std::bit_xor<> {};

HEDLEY_PRAGMA(GCC diagnostic pop)

namespace detail
{

// /// @brief subtracts vectors of 8-bit integral types
// template <typename NodeT> struct sub8_t;
// /// @brief subtracts vectors of 16-bit integral types
// template <typename NodeT> struct sub16_t;
// /// @brief subtracts vectors of 32-bit integral types
// template <typename NodeT> struct sub32_t;
// /// @brief subtracts vectors of 64-bit integral types
// template <typename NodeT> struct sub64_t;

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

/// @brief Function object for subtracting vectors of `32x8`-bit integral types
struct sub32x8_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m256i & a, const simde__m256i & b) const
    {
        return simde_mm256_sub_epi8(a, b);
    }
};

/// @brief Function object for subtracting vectors of `16x16`-bit integral types
struct sub16x16_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m256i & a, const simde__m256i & b) const
    {
        return simde_mm256_sub_epi16(a, b);
    }
};

/// @brief Function object for subtracting vectors of `8x32`-bit integral types
struct sub8x32_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m256i & a, const simde__m256i & b) const
    {
        return simde_mm256_sub_epi32(a, b);
    }
};

/// @brief Function object for subtracting vectors of `4x64`-bit integral types
struct sub4x64_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m256i & a, const simde__m256i & b) const
    {
        return simde_mm256_sub_epi64(a, b);
    }
};

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
struct sub_array_t
{
    template <typename T, std::size_t N>
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const std::array<T, N> & a, const std::array<T, N> & b) const
    {
        std::array<T, N> c;
        std::transform(std::begin(a), std::end(a), std::begin(b), std::begin(c),
            [](const T & a, const T & b) { return std::bit_xor<>{}(a, b); });
        return c;
    }
};
HEDLEY_PRAGMA(GCC diagnostic pop)

}  // namespace detail

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")

template <> struct subtract_t<bool, simde__m128i> final : public detail::sub16x8_t {};
template <> struct subtract_t<char, simde__m128i> final : public detail::sub16x8_t {};
// template <> struct subtract_t<unsigned char, simde__m128i> final : public detail::sub16x8_t {};
template <> struct subtract_t<int8_t, simde__m128i> final : public detail::sub16x8_t {};
template <> struct subtract_t<uint8_t, simde__m128i> final : public detail::sub16x8_t {};

template <> struct subtract_t<int16_t, simde__m128i> final : public detail::sub8x16_t {};
template <> struct subtract_t<uint16_t, simde__m128i> final : public detail::sub8x16_t {};

template <> struct subtract_t<int32_t, simde__m128i> final : public detail::sub4x32_t {};
template <> struct subtract_t<uint32_t, simde__m128i> final : public detail::sub4x32_t {};

template <> struct subtract_t<int64_t, simde__m128i> final : public detail::sub2x64_t {};
template <> struct subtract_t<uint64_t, simde__m128i> final : public detail::sub2x64_t {};

template <> struct subtract_t<simde_int128, simde__m128i> final
{
    auto operator()(const simde__m128i & lhs, const simde__m128i & rhs) const
    {
        simde__m128i ret;
        simde_int128 lhs_, rhs_;
        std::memcpy(&lhs_, &lhs, sizeof(simde_int128));
        std::memcpy(&rhs_, &rhs, sizeof(simde_int128));
        simde_int128 sum = lhs_ - rhs_;
        std::memcpy(&ret, &sum, sizeof(simde__m128i));
        return ret;
    }
};
template <> struct subtract_t<simde_uint128, simde__m128i> final
{
    auto operator()(const simde__m128i & lhs, const simde__m128i & rhs) const
    {
        simde__m128i ret;
        simde_uint128 lhs_, rhs_;
        std::memcpy(&lhs_, &lhs, sizeof(simde_uint128));
        std::memcpy(&rhs_, &rhs, sizeof(simde_uint128));
        simde_uint128 sum = lhs_ - rhs_;
        std::memcpy(&ret, &sum, sizeof(simde__m128i));
        return ret;
    }
};

template <> struct subtract_t<bool, simde__m256i> final : public detail::sub32x8_t {};
template <> struct subtract_t<char, simde__m256i> final : public detail::sub32x8_t {};
// template <> struct subtract_t<unsigned char, simde__m256i> final : public detail::sub32x8_t {};
template <> struct subtract_t<int8_t, simde__m256i> final : public detail::sub32x8_t {};
template <> struct subtract_t<uint8_t, simde__m256i> final : public detail::sub32x8_t {};

template <> struct subtract_t<int16_t, simde__m256i> final : public detail::sub16x16_t {};
template <> struct subtract_t<uint16_t, simde__m256i> final : public detail::sub16x16_t {};

template <> struct subtract_t<int32_t, simde__m256i> final : public detail::sub8x32_t {};
template <> struct subtract_t<uint32_t, simde__m256i> final : public detail::sub8x32_t {};

template <> struct subtract_t<int64_t, simde__m256i> final : public detail::sub4x64_t {};
template <> struct subtract_t<uint64_t, simde__m256i> final : public detail::sub4x64_t {};

template <> struct subtract_t<simde_int128, simde__m256i> final
{
    auto operator()(const simde__m256i & lhs, const simde__m256i & rhs) const
    {
        simde__m256i ret;
        simde_int128 lhs_[2], rhs_[2];
        std::memcpy(&lhs_, &lhs, sizeof(simde_int128) * 2);
        std::memcpy(&rhs_, &rhs, sizeof(simde_int128) * 2);
        simde_int128 sum[2] = { lhs_[0] - rhs_[0], lhs_[1] - rhs_[1] };
        std::memcpy(&ret, &sum, sizeof(simde__m128i) * 2);
        return ret;
    }
};
template <> struct subtract_t<simde_uint128, simde__m256i> final
{
    auto operator()(const simde__m256i & lhs, const simde__m256i & rhs) const
    {
        simde__m256i ret;
        simde_uint128 lhs_[2], rhs_[2];
        std::memcpy(&lhs_, &lhs, sizeof(simde_uint128) * 2);
        std::memcpy(&rhs_, &rhs, sizeof(simde_uint128) * 2);
        simde_uint128 sum[2] = { lhs_[0] - rhs_[0], lhs_[1] - rhs_[1] };
        std::memcpy(&ret, &sum, sizeof(simde__m128i) * 2);
        return ret;
    }
};

template <typename OutputT, typename NodeT, std::size_t N>
struct subtract_t<OutputT, std::array<NodeT, N>> final : public detail::sub_array_t {};

template <typename NodeT> struct subtract_t<float, NodeT> final : public std::bit_xor<> {};
template <typename NodeT> struct subtract_t<double, NodeT> final : public std::bit_xor<> {};
template <typename NodeT> struct subtract_t<dpf::bit, NodeT> final : public std::bit_xor<> {};
template <> struct subtract_t<dpf::bit, void> final : public std::bit_xor<> {};
template <typename T> struct subtract_t<xor_wrapper<T>, void> final : public std::bit_xor<> {};

HEDLEY_PRAGMA(GCC diagnostic pop)

namespace detail
{

/// @brief Function object for multiplying a vector of `16x8`-bit integral types by an unsigned scalar of the same size
struct mul16x8_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m128i & a, uint8_t b) const
    {
        auto bb = simde_mm_set1_epi8(b);
        auto lo_bytes = simde_mm_mullo_epi16(a, bb);
        auto hi_bytes = simde_mm_mullo_epi16(simde_mm_srli_epi16(a, 8),
                                             simde_mm_srli_epi16(bb, 8));

        return simde_mm_or_si128(
            simde_mm_slli_epi16(hi_bytes, 8),
            simde_mm_and_si128(lo_bytes, simde_mm_set1_epi16(0xff))
        );
    }
};

/// @brief Function object for multiplying a vector of `8x16`-bit integral types by an unsigned scalar of the same size
struct mul8x16_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m128i & a, uint16_t b) const
    {
        return simde_mm_mullo_epi16(a, simde_mm_set1_epi16(b));
    }
};

/// @brief Function object for multiplying a vector of `4x32`-bit integral types by an unsigned scalar of the same size
struct mul4x32_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m128i & a, uint32_t b) const
    {
        return simde_mm_mullo_epi32(a, simde_mm_set1_epi32(b));
    }
};

/// @brief Function object for multiplying a vector of `2x64`-bit integral types by an unsigned scalar of the same size
struct mul2x64_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m128i & a, uint64_t b) const
    {
        return simde__m128i{static_cast<int64_t>(a[0]*b),
                            static_cast<int64_t>(a[1]*b)};
    }
};

/// @brief Function object for multiplying a vector of `32x8`-bit integral types by an unsigned scalar of the same size
struct mul32x8_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m256i & a, uint8_t b) const
    {
        auto bb = simde_mm256_set1_epi8(b);
        auto lo_bytes = simde_mm256_mullo_epi16(a, bb);
        auto hi_bytes = simde_mm256_mullo_epi16(simde_mm256_srli_epi16(a, 8),
                                                simde_mm256_srli_epi16(bb, 8));

        return simde_mm256_or_si256(
            simde_mm256_slli_epi16(hi_bytes, 8),
            simde_mm256_and_si256(lo_bytes, simde_mm256_set1_epi16(0xff))
        );
    }
};

/// @brief Function object for multiplying a vector of `16x16`-bit integral types by an unsigned scalar of the same size
struct mul16x16_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m256i & a, uint16_t b) const
    {
        return simde_mm256_mullo_epi16(a, simde_mm256_set1_epi16(b));
    }
};

/// @brief Function object for multiplying a vector of `8x32`-bit integral types by an unsigned scalar of the same size
struct mul8x32_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m256i & a, uint32_t b) const
    {
        return simde_mm256_add_epi32(a, simde_mm256_set1_epi32(b));
    }
};

/// @brief Function object for multiplying a vector of `4x64`-bit integral types by an unsigned scalar of the same size
struct mul4x64_t
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const simde__m256i & a, uint64_t b) const
    {
        return simde__m256i{static_cast<int64_t>(a[0]*b),
                            static_cast<int64_t>(a[1]*b),
                            static_cast<int64_t>(a[2]*b),
                            static_cast<int64_t>(a[3]*b)};
    }
};
HEDLEY_PRAGMA(GCC diagnostic pop)

}  // namespace detail

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")

template <> struct multiply_t<bool, simde__m128i> final : public detail::mul16x8_t {};
template <> struct multiply_t<char, simde__m128i> final : public detail::mul16x8_t {};
// template <> struct multiply_t<unsigned char, simde__m128i> final : public detail::mul16x8_t {};
template <> struct multiply_t<int8_t, simde__m128i> final : public detail::mul16x8_t {};
template <> struct multiply_t<uint8_t, simde__m128i> final : public detail::mul16x8_t {};

template <> struct multiply_t<int16_t, simde__m128i> final : public detail::mul8x16_t {};
template <> struct multiply_t<uint16_t, simde__m128i> final : public detail::mul8x16_t {};

template <> struct multiply_t<int32_t, simde__m128i> final : public detail::mul4x32_t {};
template <> struct multiply_t<uint32_t, simde__m128i> final : public detail::mul4x32_t {};

template <> struct multiply_t<int64_t, simde__m128i> final : public detail::mul2x64_t {};
template <> struct multiply_t<uint64_t, simde__m128i> final : public detail::mul2x64_t {};

template <> struct multiply_t<bool, simde__m256i> final : public detail::mul32x8_t {};
// template <> struct multiply_t<unsigned char, simde__m256i> final : public detail::mul32x8_t {};
template <> struct multiply_t<int8_t, simde__m256i> final : public detail::mul32x8_t {};
template <> struct multiply_t<uint8_t, simde__m256i> final : public detail::mul32x8_t {};

template <> struct multiply_t<int16_t, simde__m256i> final : public detail::mul16x16_t {};
template <> struct multiply_t<uint16_t, simde__m256i> final : public detail::mul16x16_t {};

template <> struct multiply_t<int32_t, simde__m256i> final : public detail::mul8x32_t {};
template <> struct multiply_t<uint32_t, simde__m256i> final : public detail::mul8x32_t {};

template <> struct multiply_t<int64_t, simde__m256i> final : public detail::mul4x64_t {};
template <> struct multiply_t<uint64_t, simde__m256i> final : public detail::mul4x64_t {};

// todo(ryan): finish these (and check that the corresponding ones for add and subtract work properly for 128-bit and non-integer types)
// template <typename NodeT> struct multiply_t<simde_int128, NodeT> final : public std::multiplies<simde_int128> {};
// template <typename NodeT> struct multiply_t<simde_uint128, NodeT> final : public std::multiplies<simde_uint128> {};

// template <typename NodeT> struct multiply_t<float, NodeT> final : public std::bit_and<> {};
// template <typename NodeT> struct multiply_t<double, NodeT> final : public std::bit_and<> {};
template <> struct multiply_t<dpf::bit, simde__m128i> final
{
    auto operator()(const simde__m128i & a, const dpf::bit & b) const
    {
        simde__m128i bb = simde_mm_set1_epi8(-uint8_t(b));
        return simde_mm_and_si128(a, bb);
    }
};
template <> struct multiply_t<dpf::bit, simde__m256i> final
{
    auto operator()(const simde__m256i & a, const dpf::bit & b) const
    {
        simde__m256i bb = simde_mm256_set1_epi8(-uint8_t(b));
        return simde_mm256_and_si256(a, bb);
    }
};
template <typename T, typename NodeT> struct multiply_t<xor_wrapper<T>, NodeT> final : public std::bit_and<xor_wrapper<T>> {};


HEDLEY_PRAGMA(GCC diagnostic pop)
}  // namespace dpf::leaf_arithmetic

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_LEAF_ARITHMETIC_HPP__
