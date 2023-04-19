#include <gtest/gtest.h>

#include "asio.hpp"
#include "dpf.hpp"

template <typename InputT,
          typename OutputT>
using test_type = std::tuple<InputT, OutputT>;

template <typename InputT,
          typename OutputT>
using param_type = std::vector<test_type<InputT, OutputT>>;

static std::tuple
<
    // base test
    param_type<uint64_t, uint64_t>,

    // test input types
    param_type<uint8_t, uint64_t>,
    param_type<simde_uint128, uint64_t>,
    param_type<dpf::bitstring<10>, uint64_t>,
    param_type<dpf::keyword<4, dpf::alphabets::hex>, uint64_t>,
    param_type<dpf::modint<10>, uint64_t>,
    param_type<dpf::xor_wrapper<uint64_t>, uint64_t>,

    // test output types
    param_type<uint64_t, uint8_t>,
    param_type<uint64_t, simde_uint128>,
    param_type<uint64_t, dpf::bit>,
    param_type<uint64_t, dpf::bitstring<10>>,
    param_type<uint64_t, dpf::xor_wrapper<uint64_t>>
> allParams
{
    {
        std::make_tuple(uint64_t(0x0000000000000000), uint64_t(0x0000000000000001)),
        std::make_tuple(uint64_t(0x0000000000000000), uint64_t(0x5555555555555555)),
        std::make_tuple(uint64_t(0x0000000000000000), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0x0000000000000000), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint64_t(0x5555555555555555), uint64_t(0x0000000000000001)),
        std::make_tuple(uint64_t(0x5555555555555555), uint64_t(0x5555555555555555)),
        std::make_tuple(uint64_t(0x5555555555555555), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0x5555555555555555), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), uint64_t(0x0000000000000001)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), uint64_t(0x5555555555555555)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), uint64_t(0x0000000000000001)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), uint64_t(0x5555555555555555)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), uint64_t(0xFFFFFFFFFFFFFFFF))
    },
    {
        std::make_tuple(uint8_t(0x00), uint64_t(0x0000000000000001)),
        std::make_tuple(uint8_t(0x00), uint64_t(0x5555555555555555)),
        std::make_tuple(uint8_t(0x00), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint8_t(0x00), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint8_t(0x55), uint64_t(0x0000000000000001)),
        std::make_tuple(uint8_t(0x55), uint64_t(0x5555555555555555)),
        std::make_tuple(uint8_t(0x55), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint8_t(0x55), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint8_t(0xAA), uint64_t(0x0000000000000001)),
        std::make_tuple(uint8_t(0xAA), uint64_t(0x5555555555555555)),
        std::make_tuple(uint8_t(0xAA), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint8_t(0xAA), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint8_t(0xFF), uint64_t(0x0000000000000001)),
        std::make_tuple(uint8_t(0xFF), uint64_t(0x5555555555555555)),
        std::make_tuple(uint8_t(0xFF), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint8_t(0xFF), uint64_t(0xFFFFFFFFFFFFFFFF))
    },
    {
        std::make_tuple(simde_uint128(0x0000000000000000) << 16 | simde_uint128(0x0000000000000000), uint64_t(0x0000000000000001)),
        std::make_tuple(simde_uint128(0x0000000000000000) << 16 | simde_uint128(0x0000000000000000), uint64_t(0x5555555555555555)),
        std::make_tuple(simde_uint128(0x0000000000000000) << 16 | simde_uint128(0x0000000000000000), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(simde_uint128(0x0000000000000000) << 16 | simde_uint128(0x0000000000000000), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(simde_uint128(0x5555555555555555) << 16 | simde_uint128(0x5555555555555555), uint64_t(0x0000000000000001)),
        std::make_tuple(simde_uint128(0x5555555555555555) << 16 | simde_uint128(0x5555555555555555), uint64_t(0x5555555555555555)),
        std::make_tuple(simde_uint128(0x5555555555555555) << 16 | simde_uint128(0x5555555555555555), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(simde_uint128(0x5555555555555555) << 16 | simde_uint128(0x5555555555555555), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(simde_uint128(0xAAAAAAAAAAAAAAAA) << 16 | simde_uint128(0xAAAAAAAAAAAAAAAA), uint64_t(0x0000000000000001)),
        std::make_tuple(simde_uint128(0xAAAAAAAAAAAAAAAA) << 16 | simde_uint128(0xAAAAAAAAAAAAAAAA), uint64_t(0x5555555555555555)),
        std::make_tuple(simde_uint128(0xAAAAAAAAAAAAAAAA) << 16 | simde_uint128(0xAAAAAAAAAAAAAAAA), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(simde_uint128(0xAAAAAAAAAAAAAAAA) << 16 | simde_uint128(0xAAAAAAAAAAAAAAAA), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(simde_uint128(0xFFFFFFFFFFFFFFFF) << 16 | simde_uint128(0xFFFFFFFFFFFFFFFF), uint64_t(0x0000000000000001)),
        std::make_tuple(simde_uint128(0xFFFFFFFFFFFFFFFF) << 16 | simde_uint128(0xFFFFFFFFFFFFFFFF), uint64_t(0x5555555555555555)),
        std::make_tuple(simde_uint128(0xFFFFFFFFFFFFFFFF) << 16 | simde_uint128(0xFFFFFFFFFFFFFFFF), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(simde_uint128(0xFFFFFFFFFFFFFFFF) << 16 | simde_uint128(0xFFFFFFFFFFFFFFFF), uint64_t(0xFFFFFFFFFFFFFFFF))
    },
    {
        std::make_tuple(dpf::bitstring<10>(0x000), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::bitstring<10>(0x000), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::bitstring<10>(0x000), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::bitstring<10>(0x000), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::bitstring<10>(0x155), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::bitstring<10>(0x155), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::bitstring<10>(0x155), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::bitstring<10>(0x155), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::bitstring<10>(0x2AA), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::bitstring<10>(0x2AA), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::bitstring<10>(0x2AA), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::bitstring<10>(0x2AA), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::bitstring<10>(0x3FF), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::bitstring<10>(0x3FF), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::bitstring<10>(0x3FF), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::bitstring<10>(0x3FF), uint64_t(0xFFFFFFFFFFFFFFFF))
    },
    {
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("0000"), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("0000"), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("0000"), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("0000"), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("5555"), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("5555"), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("5555"), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("5555"), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("aaaa"), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("aaaa"), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("aaaa"), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("aaaa"), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("ffff"), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("ffff"), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("ffff"), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::keyword<4, dpf::alphabets::hex>("ffff"), uint64_t(0xFFFFFFFFFFFFFFFF)),
    },
    {
        std::make_tuple(dpf::modint<10>(0x000), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::modint<10>(0x000), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::modint<10>(0x000), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::modint<10>(0x000), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::modint<10>(0x155), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::modint<10>(0x155), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::modint<10>(0x155), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::modint<10>(0x155), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::modint<10>(0x2AA), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::modint<10>(0x2AA), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::modint<10>(0x2AA), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::modint<10>(0x2AA), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::modint<10>(0x3FF), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::modint<10>(0x3FF), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::modint<10>(0x3FF), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::modint<10>(0x3FF), uint64_t(0xFFFFFFFFFFFFFFFF)),
    },
    {
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x0000000000000000)), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x0000000000000000)), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x0000000000000000)), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x0000000000000000)), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x5555555555555555)), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x5555555555555555)), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x5555555555555555)), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x5555555555555555)), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0xAAAAAAAAAAAAAAAA)), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0xAAAAAAAAAAAAAAAA)), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0xAAAAAAAAAAAAAAAA)), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0xAAAAAAAAAAAAAAAA)), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0xFFFFFFFFFFFFFFFF)), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0xFFFFFFFFFFFFFFFF)), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0xFFFFFFFFFFFFFFFF)), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0xFFFFFFFFFFFFFFFF)), uint64_t(0xFFFFFFFFFFFFFFFF))
    },
    {
        std::make_tuple(uint64_t(0x0000000000000000), uint8_t(0x01)),
        std::make_tuple(uint64_t(0x0000000000000000), uint8_t(0x55)),
        std::make_tuple(uint64_t(0x0000000000000000), uint8_t(0xAA)),
        std::make_tuple(uint64_t(0x0000000000000000), uint8_t(0xFF)),
        std::make_tuple(uint64_t(0x5555555555555555), uint8_t(0x01)),
        std::make_tuple(uint64_t(0x5555555555555555), uint8_t(0x55)),
        std::make_tuple(uint64_t(0x5555555555555555), uint8_t(0xAA)),
        std::make_tuple(uint64_t(0x5555555555555555), uint8_t(0xFF)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), uint8_t(0x01)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), uint8_t(0x55)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), uint8_t(0xAA)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), uint8_t(0xFF)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), uint8_t(0x01)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), uint8_t(0x55)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), uint8_t(0xAA)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), uint8_t(0xFF))
    },
    {
        std::make_tuple(uint64_t(0x0000000000000000), simde_uint128(0x0000000000000000) << 16 | simde_uint128(0x0000000000000001)),
        std::make_tuple(uint64_t(0x0000000000000000), simde_uint128(0x5555555555555555) << 16 | simde_uint128(0x5555555555555555)),
        std::make_tuple(uint64_t(0x0000000000000000), simde_uint128(0xAAAAAAAAAAAAAAAA) << 16 | simde_uint128(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0x0000000000000000), simde_uint128(0xFFFFFFFFFFFFFFFF) << 16 | simde_uint128(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint64_t(0x5555555555555555), simde_uint128(0x0000000000000000) << 16 | simde_uint128(0x0000000000000001)),
        std::make_tuple(uint64_t(0x5555555555555555), simde_uint128(0x5555555555555555) << 16 | simde_uint128(0x5555555555555555)),
        std::make_tuple(uint64_t(0x5555555555555555), simde_uint128(0xAAAAAAAAAAAAAAAA) << 16 | simde_uint128(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0x5555555555555555), simde_uint128(0xFFFFFFFFFFFFFFFF) << 16 | simde_uint128(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), simde_uint128(0x0000000000000000) << 16 | simde_uint128(0x0000000000000001)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), simde_uint128(0x5555555555555555) << 16 | simde_uint128(0x5555555555555555)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), simde_uint128(0xAAAAAAAAAAAAAAAA) << 16 | simde_uint128(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), simde_uint128(0xFFFFFFFFFFFFFFFF) << 16 | simde_uint128(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), simde_uint128(0x0000000000000000) << 16 | simde_uint128(0x0000000000000001)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), simde_uint128(0x5555555555555555) << 16 | simde_uint128(0x5555555555555555)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), simde_uint128(0xAAAAAAAAAAAAAAAA) << 16 | simde_uint128(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), simde_uint128(0xFFFFFFFFFFFFFFFF) << 16 | simde_uint128(0xFFFFFFFFFFFFFFFF))
    },
    {
        std::make_tuple(uint64_t(0x0000000000000000), dpf::bit::one),
        std::make_tuple(uint64_t(0x5555555555555555), dpf::bit::one),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), dpf::bit::one),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), dpf::bit::one)
    },
    {
        std::make_tuple(uint64_t(0x0000000000000000), dpf::bitstring<10>(0x001)),
        std::make_tuple(uint64_t(0x0000000000000000), dpf::bitstring<10>(0x155)),
        std::make_tuple(uint64_t(0x0000000000000000), dpf::bitstring<10>(0x2AA)),
        std::make_tuple(uint64_t(0x0000000000000000), dpf::bitstring<10>(0x3FF)),
        std::make_tuple(uint64_t(0x5555555555555555), dpf::bitstring<10>(0x001)),
        std::make_tuple(uint64_t(0x5555555555555555), dpf::bitstring<10>(0x155)),
        std::make_tuple(uint64_t(0x5555555555555555), dpf::bitstring<10>(0x2AA)),
        std::make_tuple(uint64_t(0x5555555555555555), dpf::bitstring<10>(0x3FF)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), dpf::bitstring<10>(0x001)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), dpf::bitstring<10>(0x155)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), dpf::bitstring<10>(0x2AA)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), dpf::bitstring<10>(0x3FF)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), dpf::bitstring<10>(0x001)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), dpf::bitstring<10>(0x155)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), dpf::bitstring<10>(0x2AA)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), dpf::bitstring<10>(0x3FF))
    },
    {
        std::make_tuple(uint64_t(0x0000000000000000), dpf::xor_wrapper<uint64_t>(uint64_t(0x0000000000000001))),
        std::make_tuple(uint64_t(0x0000000000000000), dpf::xor_wrapper<uint64_t>(uint64_t(0x5555555555555555))),
        std::make_tuple(uint64_t(0x0000000000000000), dpf::xor_wrapper<uint64_t>(uint64_t(0xAAAAAAAAAAAAAAAA))),
        std::make_tuple(uint64_t(0x0000000000000000), dpf::xor_wrapper<uint64_t>(uint64_t(0xFFFFFFFFFFFFFFFF))),
        std::make_tuple(uint64_t(0x5555555555555555), dpf::xor_wrapper<uint64_t>(uint64_t(0x0000000000000001))),
        std::make_tuple(uint64_t(0x5555555555555555), dpf::xor_wrapper<uint64_t>(uint64_t(0x5555555555555555))),
        std::make_tuple(uint64_t(0x5555555555555555), dpf::xor_wrapper<uint64_t>(uint64_t(0xAAAAAAAAAAAAAAAA))),
        std::make_tuple(uint64_t(0x5555555555555555), dpf::xor_wrapper<uint64_t>(uint64_t(0xFFFFFFFFFFFFFFFF))),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), dpf::xor_wrapper<uint64_t>(uint64_t(0x0000000000000001))),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), dpf::xor_wrapper<uint64_t>(uint64_t(0x5555555555555555))),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), dpf::xor_wrapper<uint64_t>(uint64_t(0xAAAAAAAAAAAAAAAA))),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), dpf::xor_wrapper<uint64_t>(uint64_t(0xFFFFFFFFFFFFFFFF))),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), dpf::xor_wrapper<uint64_t>(uint64_t(0x0000000000000001))),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), dpf::xor_wrapper<uint64_t>(uint64_t(0x5555555555555555))),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), dpf::xor_wrapper<uint64_t>(uint64_t(0xAAAAAAAAAAAAAAAA))),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), dpf::xor_wrapper<uint64_t>(uint64_t(0xFFFFFFFFFFFFFFFF)))
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
        ASSERT_EQ(static_cast<output_type>(y1 - y0), y);
    }
}

TYPED_TEST_P(EvalPointTest, SurroundingPoints)
{
    using input_type = typename std::tuple_element_t<0, TypeParam>;
    using output_type = typename std::tuple_element_t<1, TypeParam>;

    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        std::size_t range_bitlength = std::min(dpf::utils::bitlength_of_v<input_type>, std::size_t(10)),
                    range = std::size_t(1) << range_bitlength;
        input_type xp = x, xm = x; ++xp; --xm;
        for (std::size_t i = 1; i < range >> 1; ++i, ++xp, --xm)
        {
            output_type y0p = dpf::eval_point(dpf0, xp),
                        y1p = dpf::eval_point(dpf1, xp),
                        y0m = dpf::eval_point(dpf0, xm),
                        y1m = dpf::eval_point(dpf1, xm);
            ASSERT_EQ(static_cast<output_type>(y1p - y0p), output_type(0));
            ASSERT_EQ(static_cast<output_type>(y1m - y0m), output_type(0));
        }
    }
}

REGISTER_TYPED_TEST_SUITE_P(EvalPointTest, DistinguishedPoint, SurroundingPoints);
using Types = testing::Types
<
    // base test
    test_type<uint64_t, uint64_t>,

    // test input types
    test_type<uint8_t, uint64_t>,
    test_type<simde_uint128, uint64_t>,
    test_type<dpf::bitstring<10>, uint64_t>,
    test_type<dpf::keyword<4, dpf::alphabets::hex>, uint64_t>,
    test_type<dpf::modint<10>, uint64_t>,
    test_type<dpf::xor_wrapper<uint64_t>, uint64_t>,

    // test output types
    test_type<uint64_t, uint8_t>,
    test_type<uint64_t, simde_uint128>,
    test_type<uint64_t, dpf::bit>,
    // test_type<uint64_t, dpf::bitstring<10>>,
    test_type<uint64_t, dpf::xor_wrapper<uint64_t>>
>;
INSTANTIATE_TYPED_TEST_SUITE_P(EvalPointTestInstantiation, EvalPointTest, Types);
