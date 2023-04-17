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
#include "dpf/xor_wrapper.hpp"

namespace dpf
{

namespace leaf_arithmetic
{

template <typename OutputT, typename NodeT> struct add_t;
template <typename OutputT, typename NodeT> struct subtract_t;

}

template <typename OutputT>
static constexpr auto add_leaf = leaf_arithmetic::add_t<OutputT, void>{};

template <typename OutputT>
static constexpr auto subtract_leaf = leaf_arithmetic::subtract_t<OutputT, void>{};

namespace leaf_arithmetic
{

template <typename OutputT>
struct add_t<OutputT, void>
{
    template <typename NodeT>
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const NodeT & a, const NodeT & b) const
    {
        static constexpr auto adder = add_t<dpf::concrete_type_t<OutputT>, NodeT>{};
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
        static constexpr auto subtracter = subtract_t<dpf::concrete_type_t<OutputT>, NodeT>{};
        return subtracter(a, b);
    }
};

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

}  // namespace dpf::leaf_arithmetic

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_LEAF_ARITHMETIC_HPP__
