#include <gtest/gtest.h>

#include "dpf.hpp"

template <typename InputT,
          typename OutputT>
using test_type = std::tuple<InputT, OutputT>;

template <typename InputT,
          typename OutputT>
using param_type = std::vector<test_type<InputT, OutputT>>;

static std::tuple<
        param_type<uint8_t, uint16_t>,
        param_type<uint64_t, __int128>,
        param_type<uint64_t, dpf::bit>,
        param_type<uint64_t, dpf::xor_wrapper<uint64_t>>,
        param_type<dpf::modint<10>, uint64_t>,
        param_type<dpf::keyword<4, dpf::alphabets::hex>, uint64_t>
       > allParams
{
    {
        std::make_tuple(uint8_t(0x00), uint16_t(0x0001)),
        std::make_tuple(uint8_t(0x00), uint16_t(0xFFFF)),
        std::make_tuple(uint8_t(0x55), uint16_t(0x5555)),
        std::make_tuple(uint8_t(0xFF), uint16_t(0x0001)),
        std::make_tuple(uint8_t(0xFF), uint16_t(0xFFFF))
    },
    {
        std::make_tuple(uint64_t(0), __int128(1)),
        std::make_tuple(uint64_t(0), ~__int128(0)),
        std::make_tuple(~uint64_t(0), __int128(1)),
        std::make_tuple(~uint64_t(0), ~__int128(0))
    },
    {
        std::make_tuple(uint64_t(0), dpf::bit::one),
        std::make_tuple(~uint64_t(0), dpf::bit::one)
    },
    {
        std::make_tuple(uint64_t(0), dpf::xor_wrapper<uint64_t>(uint64_t(1))),
        std::make_tuple(uint64_t(0), dpf::xor_wrapper<uint64_t>(~uint64_t(0))),
        std::make_tuple(~uint64_t(0), dpf::xor_wrapper<uint64_t>(uint64_t(1))),
        std::make_tuple(~uint64_t(0), dpf::xor_wrapper<uint64_t>(~uint64_t(0)))
    },
    {
        std::make_tuple(dpf::modint<10>(uint16_t(0)), uint64_t(1)),
        std::make_tuple(dpf::modint<10>(uint16_t(0)), ~uint64_t(0)),
        std::make_tuple(dpf::modint<10>(~uint16_t(0)), uint64_t(1)),
        std::make_tuple(dpf::modint<10>(~uint16_t(0)), ~uint64_t(0))
    },
    {
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("0000"), uint64_t(1)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("0000"), ~uint64_t(0)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("ffff"), uint64_t(1)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("ffff"), ~uint64_t(0))
    }
};

template <typename T>
struct EvalPointTest : public testing::Test
{
  protected:
    EvalPointTest()
      : params{std::get<std::vector<T>>(allParams)}
    { }

    void SetUp() override
    { }

    void TearDown() override
    { }

    std::vector<T> params;
};

TYPED_TEST_SUITE_P(EvalPointTest);

TYPED_TEST_P(EvalPointTest, DistinguishedPoint)
{
    using input_type = typename std::tuple_element_t<0, TypeParam>;
    using output_type = typename std::tuple_element_t<1, TypeParam>;

    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        output_type y0 = dpf::eval_point(dpf0, x),
                    y1 = dpf::eval_point(dpf1, x);
        EXPECT_EQ(static_cast<output_type>(y1 - y0), y);
    }
}

TYPED_TEST_P(EvalPointTest, SurroundingPoints)
{
    using input_type = typename std::tuple_element_t<0, TypeParam>;
    using output_type = typename std::tuple_element_t<1, TypeParam>;

    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        std::size_t range_bitlength = std::min(dpf::utils::bitlength_of_v<input_type>, std::size_t(16)),
                    range = std::size_t(1) << range_bitlength;
        for (std::size_t i = 1; i < range >> 1; ++i)
        {
            input_type xp = x + i,
                       xm = x - i;
            output_type y0p = dpf::eval_point(dpf0, xp),
                        y1p = dpf::eval_point(dpf1, xp),
                        y0m = dpf::eval_point(dpf0, xm),
                        y1m = dpf::eval_point(dpf1, xm);
            EXPECT_EQ(static_cast<output_type>(y1p - y0p), output_type(0));
            EXPECT_EQ(static_cast<output_type>(y1m - y0m), output_type(0));
        }
    }
}

REGISTER_TYPED_TEST_SUITE_P(EvalPointTest, DistinguishedPoint, SurroundingPoints);
using Types = testing::Types<
                test_type<uint8_t, uint16_t>,
                test_type<uint64_t, __int128>,
                test_type<uint64_t, dpf::bit>,
                test_type<uint64_t, dpf::xor_wrapper<uint64_t>>,
                test_type<dpf::modint<10>, uint64_t>
                // test_type<dpf::keyword<4, dpf::alphabets::hex>, uint64_t>
              >;
INSTANTIATE_TYPED_TEST_SUITE_P(EvalPointTestInstantiation, EvalPointTest, Types);
