#include <gtest/gtest.h>

#include <set>

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
struct EvalSequenceTest : public testing::Test
{
  protected:
    EvalSequenceTest()
      : params{std::get<std::vector<T>>(allParams)}
    { }

    void SetUp() override
    { }

    void TearDown() override
    { }

    std::vector<T> params;
};

TYPED_TEST_SUITE_P(EvalSequenceTest);

TYPED_TEST_P(EvalSequenceTest, RandomPointsRecipe)
{
    using input_type = typename std::tuple_element_t<0, TypeParam>;
    using output_type = typename std::tuple_element_t<1, TypeParam>;
    using integral_type = dpf::utils::integral_type_from_bitlength_t<dpf::utils::bitlength_of_v<input_type>>;

    auto from_integral_type = dpf::utils::make_from_integral_value<input_type>{};

    std::size_t range_bitlength = std::min(dpf::utils::bitlength_of_v<input_type>, std::size_t(10)),
                range = std::size_t(1) << range_bitlength-1;

    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        std::set<input_type> points{x};
        while (points.size() < range)
        {
            points.emplace(from_integral_type(dpf::uniform_sample<integral_type>()));
        }
        output_type zero_output = output_type(0);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, points.begin(), points.end());
        auto recipe1 = dpf::make_sequence_recipe(dpf1, points.begin(), points.end());
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, recipe0);
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, recipe1);
        auto it0 = std::begin(iter0), it1 = std::begin(iter1);

        auto cur = points.begin();
        for (std::size_t i = 0; i < range; ++i, ++cur, ++it0, ++it1)
        {
            if (*cur == x)
            {
                ASSERT_EQ(static_cast<output_type>(*it1 - *it0), y);
            }
            else
            {
                ASSERT_EQ(static_cast<output_type>(*it1 - *it0), zero_output);
            }
        }
    }
}

TYPED_TEST_P(EvalSequenceTest, RandomPointsNoRecipe)
{
    using input_type = typename std::tuple_element_t<0, TypeParam>;
    using output_type = typename std::tuple_element_t<1, TypeParam>;
    using integral_type = dpf::utils::integral_type_from_bitlength_t<dpf::utils::bitlength_of_v<input_type>>;

    auto from_integral_type = dpf::utils::make_from_integral_value<input_type>{};

    std::size_t range_bitlength = std::min(dpf::utils::bitlength_of_v<input_type>, std::size_t(10)),
                range = std::size_t(1) << range_bitlength-1;

    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        std::set<input_type> points{x};
        while (points.size() < range)
        {
            points.emplace(from_integral_type(dpf::uniform_sample<integral_type>()));
        }
        output_type zero_output = output_type(0);
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, points.begin(), points.end());
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, points.begin(), points.end());
        auto it0 = std::begin(iter0), it1 = std::begin(iter1);

        auto cur = points.begin();
        for (std::size_t i = 0; i < range; ++i, ++cur, ++it0, ++it1)
        {
            if (*cur == x)
            {
                ASSERT_EQ(static_cast<output_type>(*it1 - *it0), y);
            }
            else
            {
                ASSERT_EQ(static_cast<output_type>(*it1 - *it0), zero_output);
            }
        }
    }
}

REGISTER_TYPED_TEST_SUITE_P(EvalSequenceTest, RandomPointsRecipe, RandomPointsNoRecipe);
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
INSTANTIATE_TYPED_TEST_SUITE_P(EvalSequenceTestInstantiation, EvalSequenceTest, Types);
