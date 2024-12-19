#include <type_traits>
#include <limits>

#include "uint256_t/uint256_t.hpp"

#include "dpf/utils.hpp"
#include "dpf/leaf_arithmetic.hpp"



namespace dpf
{

namespace leaf_arithmetic
{

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
template <> struct add_t<uint128_t, simde__m128i> final
{
    auto operator()(const simde__m128i & lhs, const simde__m128i & rhs) const
    {
        simde__m128i ret;
        uint128_t lhs_, rhs_;
        std::memcpy(&lhs_, &lhs, sizeof(uint128_t));
        std::memcpy(&rhs_, &rhs, sizeof(uint128_t));
        uint128_t sum = lhs_ + rhs_;
        std::memcpy(&ret, &sum, sizeof(simde__m128i));
        return ret;
    }
};

template <> struct add_t<uint128_t, simde__m256i> final
{
    auto operator()(const simde__m256i & lhs, const simde__m256i & rhs) const
    {
        simde__m256i ret;
        uint128_t lhs_[2], rhs_[2];
        std::memcpy(&lhs_, &lhs, sizeof(uint128_t) * 2);
        std::memcpy(&rhs_, &rhs, sizeof(uint128_t) * 2);
        uint128_t sum[2] = { lhs_[0] + rhs_[0], lhs_[1] + rhs_[1] };
        std::memcpy(&ret, &sum, sizeof(simde__m256i));
        return ret;
    }
};
template <> struct add_t<uint256_t, simde__m256i> final
{
    auto operator()(const simde__m256i & lhs, const simde__m256i & rhs) const
    {
        simde__m256i ret;
        uint256_t lhs_, rhs_;
        std::memcpy(&lhs_, &lhs, sizeof(uint256_t));
        std::memcpy(&rhs_, &rhs, sizeof(uint256_t));
        uint256_t sum = lhs_ + rhs_;
        std::memcpy(&ret, &sum, sizeof(simde__m256i));
        return ret;
    }
};
template <> struct add_t<uint256_t, std::array<simde__m128i, 2>> final
{
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_CONST
    auto operator()(const std::array<simde__m128i, 2> & a, const std::array<simde__m128i, 2> & b) const
    {
        std::array<simde__m128i, 2> c;
        uint256_t a_, b_;
        std::memcpy(&a_, std::data(a), sizeof(uint256_t));
        std::memcpy(&b_, std::data(b), sizeof(uint256_t));
        uint256_t c_ = a_ + b_;
        std::memcpy(std::data(c), &c_, sizeof(std::array<simde__m128i, 2>));
        return c;
    }
};

template <> struct subtract_t<uint128_t, simde__m128i> final
{
    auto operator()(const simde__m128i & lhs, const simde__m128i & rhs) const
    {
        simde__m128i ret;
        uint128_t lhs_, rhs_;
        std::memcpy(&lhs_, &lhs, sizeof(uint128_t));
        std::memcpy(&rhs_, &rhs, sizeof(uint128_t));
        uint128_t sum = lhs_ - rhs_;
        std::memcpy(&ret, &sum, sizeof(simde__m128i));
        return ret;
    }
};

template <> struct subtract_t<uint128_t, simde__m256i> final
{
    auto operator()(const simde__m256i & lhs, const simde__m256i & rhs) const
    {
        simde__m256i ret;
        uint128_t lhs_[2], rhs_[2];
        std::memcpy(&lhs_, &lhs, sizeof(simde_uint128) * 2);
        std::memcpy(&rhs_, &rhs, sizeof(simde_uint128) * 2);
        uint128_t sum[2] = { lhs_[0] - rhs_[0], lhs_[1] - rhs_[1] };
        std::memcpy(&ret, &sum, sizeof(simde__m256i));
        return ret;
    }
};
template <> struct subtract_t<uint256_t, simde__m256i> final
{
    auto operator()(const simde__m256i & lhs, const simde__m256i & rhs) const
    {
        simde__m256i ret;
        uint256_t lhs_, rhs_;
        std::memcpy(&lhs_, &lhs, sizeof(uint256_t));
        std::memcpy(&rhs_, &rhs, sizeof(uint256_t));
        uint256_t sum = lhs_ - rhs_;
        std::memcpy(&ret, &sum, sizeof(simde__m256i));
        return ret;
    }
};
template <> struct subtract_t<uint256_t, std::array<simde__m128i, 2>> final
{
    auto operator()(const std::array<simde__m128i, 2> & a, const std::array<simde__m128i, 2> & b) const
    {
        std::array<simde__m128i, 2> c;
        uint256_t a_, b_;
        std::memcpy(&a_, std::data(a), sizeof(uint256_t));
        std::memcpy(&b_, std::data(b), sizeof(uint256_t));
        uint256_t c_ = a_ - b_;
        std::memcpy(std::data(c), &c_, sizeof(std::array<simde__m128i, 2>));
        return c;
    }
};

template <> struct multiply_t<uint128_t, simde__m128i> final
{
    auto operator()(const simde__m128i & a, uint128_t b) const
    {
        uint128_t a_;
        simde__m128i c;
        std::memcpy(&a_, &a, sizeof(uint128_t));
        uint128_t c_ = a_ * b;
        std::memcpy(&c, &c_, sizeof(simde__m128i));
        return c;
    }
};

template <> struct multiply_t<uint256_t, std::array<simde__m128i, 2>> final
{
    auto operator()(const std::array<simde__m128i, 2> & a, uint256_t b) const
    {
        uint256_t a_;
        std::memcpy(&a_, &a, sizeof(uint256_t));
        uint256_t c_ = a_ * b;
        std::array<simde__m128i, 2> c;
        std::memcpy(&c, &c_, sizeof(std::array<simde__m128i, 2>));
        return c;
    }
};

template <> struct multiply_t<uint128_t, simde__m256i> final
{
    auto operator()(const simde__m256i & a, uint128_t b) const
    {
        uint256_t a_;
        simde__m256i c;
        std::memcpy(&a_, &a, sizeof(uint256_t));
        uint256_t c_{a_.upper() * b, a_.lower() * b};
        std::memcpy(&c, &c_, sizeof(simde__m256i));
        return c;
    }
};

template <> struct multiply_t<uint256_t, simde__m256i> final
{
    auto operator()(const simde__m256i & a, uint256_t b) const
    {
        uint256_t a_;
        simde__m256i c;
        std::memcpy(&a_, &a, sizeof(uint256_t));
        uint256_t c_ = a_ * b;
        std::memcpy(&c, &c_, sizeof(simde__m256i));
        return c;
    }
};
HEDLEY_PRAGMA(GCC diagnostic pop)

}  // namespace leaf_arithmetic

namespace utils
{

template <>
struct msb_of<uint128_t>
{
    constexpr static uint128_t value{1ul << 63, 0ul};
};

template <>
struct msb_of<uint256_t>
{
    constexpr static uint256_t value{uint128_t{1ul << 63, 0ul}, uint128_t{0ul, 0ul}};
};

}

}