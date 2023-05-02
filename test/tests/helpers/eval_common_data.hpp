#ifndef LIBDPF_TEST_TESTS_HELPERS_EVAL_COMMON_DATA_HPP__
#define LIBDPF_TEST_TESTS_HELPERS_EVAL_COMMON_DATA_HPP__

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
    param_type<dpf::keyword<3, dpf::alphabets::hex>, uint64_t>,
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
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), uint64_t(0x0000000000000001)),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), uint64_t(0x5555555555555555)),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint64_t(0X8000000000000000), uint64_t(0x0000000000000001)),
        std::make_tuple(uint64_t(0X8000000000000000), uint64_t(0x5555555555555555)),
        std::make_tuple(uint64_t(0X8000000000000000), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0X8000000000000000), uint64_t(0xFFFFFFFFFFFFFFFF)),
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
        std::make_tuple(uint8_t(0x7F), uint64_t(0x0000000000000001)),
        std::make_tuple(uint8_t(0x7F), uint64_t(0x5555555555555555)),
        std::make_tuple(uint8_t(0x7F), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint8_t(0x7F), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint8_t(0x80), uint64_t(0x0000000000000001)),
        std::make_tuple(uint8_t(0x80), uint64_t(0x5555555555555555)),
        std::make_tuple(uint8_t(0x80), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint8_t(0x80), uint64_t(0xFFFFFFFFFFFFFFFF)),
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
        std::make_tuple(simde_uint128(0x0000000000000000) << 64 | simde_uint128(0x0000000000000000), uint64_t(0x0000000000000001)),
        std::make_tuple(simde_uint128(0x0000000000000000) << 64 | simde_uint128(0x0000000000000000), uint64_t(0x5555555555555555)),
        std::make_tuple(simde_uint128(0x0000000000000000) << 64 | simde_uint128(0x0000000000000000), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(simde_uint128(0x0000000000000000) << 64 | simde_uint128(0x0000000000000000), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(simde_uint128(0x5555555555555555) << 64 | simde_uint128(0x5555555555555555), uint64_t(0x0000000000000001)),
        std::make_tuple(simde_uint128(0x5555555555555555) << 64 | simde_uint128(0x5555555555555555), uint64_t(0x5555555555555555)),
        std::make_tuple(simde_uint128(0x5555555555555555) << 64 | simde_uint128(0x5555555555555555), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(simde_uint128(0x5555555555555555) << 64 | simde_uint128(0x5555555555555555), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(simde_uint128(0x7FFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF), uint64_t(0x0000000000000001)),
        std::make_tuple(simde_uint128(0x7FFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF), uint64_t(0x5555555555555555)),
        std::make_tuple(simde_uint128(0x7FFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(simde_uint128(0x7FFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(simde_uint128(0x8000000000000000) << 64 | simde_uint128(0x0000000000000000), uint64_t(0x0000000000000001)),
        std::make_tuple(simde_uint128(0x8000000000000000) << 64 | simde_uint128(0x0000000000000000), uint64_t(0x5555555555555555)),
        std::make_tuple(simde_uint128(0x8000000000000000) << 64 | simde_uint128(0x0000000000000000), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(simde_uint128(0x8000000000000000) << 64 | simde_uint128(0x0000000000000000), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(simde_uint128(0xAAAAAAAAAAAAAAAA) << 64 | simde_uint128(0xAAAAAAAAAAAAAAAA), uint64_t(0x0000000000000001)),
        std::make_tuple(simde_uint128(0xAAAAAAAAAAAAAAAA) << 64 | simde_uint128(0xAAAAAAAAAAAAAAAA), uint64_t(0x5555555555555555)),
        std::make_tuple(simde_uint128(0xAAAAAAAAAAAAAAAA) << 64 | simde_uint128(0xAAAAAAAAAAAAAAAA), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(simde_uint128(0xAAAAAAAAAAAAAAAA) << 64 | simde_uint128(0xAAAAAAAAAAAAAAAA), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(simde_uint128(0xFFFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF), uint64_t(0x0000000000000001)),
        std::make_tuple(simde_uint128(0xFFFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF), uint64_t(0x5555555555555555)),
        std::make_tuple(simde_uint128(0xFFFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(simde_uint128(0xFFFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF), uint64_t(0xFFFFFFFFFFFFFFFF))
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
        std::make_tuple(dpf::bitstring<10>(0x1FF), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::bitstring<10>(0x1FF), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::bitstring<10>(0x1FF), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::bitstring<10>(0x1FF), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::bitstring<10>(0x200), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::bitstring<10>(0x200), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::bitstring<10>(0x200), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::bitstring<10>(0x200), uint64_t(0xFFFFFFFFFFFFFFFF)),
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
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("000"), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("000"), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("000"), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("000"), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("555"), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("555"), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("555"), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("555"), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("7ff"), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("7ff"), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("7ff"), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("7ff"), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("800"), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("800"), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("800"), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("800"), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("aaa"), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("aaa"), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("aaa"), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("aaa"), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("fff"), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("fff"), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("fff"), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::keyword<3, dpf::alphabets::hex>("fff"), uint64_t(0xFFFFFFFFFFFFFFFF)),
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
        std::make_tuple(dpf::modint<10>(0x1FF), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::modint<10>(0x1FF), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::modint<10>(0x1FF), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::modint<10>(0x1FF), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::modint<10>(0x200), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::modint<10>(0x200), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::modint<10>(0x200), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::modint<10>(0x200), uint64_t(0xFFFFFFFFFFFFFFFF)),
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
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x7FFFFFFFFFFFFFFF)), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x7FFFFFFFFFFFFFFF)), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x7FFFFFFFFFFFFFFF)), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x7FFFFFFFFFFFFFFF)), uint64_t(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x8000000000000000)), uint64_t(0x0000000000000001)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x8000000000000000)), uint64_t(0x5555555555555555)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x8000000000000000)), uint64_t(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(dpf::xor_wrapper<uint64_t>(uint64_t(0x8000000000000000)), uint64_t(0xFFFFFFFFFFFFFFFF)),
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
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), uint8_t(0x01)),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), uint8_t(0x55)),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), uint8_t(0xAA)),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), uint8_t(0xFF)),
        std::make_tuple(uint64_t(0x8000000000000000), uint8_t(0x01)),
        std::make_tuple(uint64_t(0x8000000000000000), uint8_t(0x55)),
        std::make_tuple(uint64_t(0x8000000000000000), uint8_t(0xAA)),
        std::make_tuple(uint64_t(0x8000000000000000), uint8_t(0xFF)),
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
        std::make_tuple(uint64_t(0x0000000000000000), simde_uint128(0x0000000000000000) << 64 | simde_uint128(0x0000000000000001)),
        std::make_tuple(uint64_t(0x0000000000000000), simde_uint128(0x5555555555555555) << 64 | simde_uint128(0x5555555555555555)),
        std::make_tuple(uint64_t(0x0000000000000000), simde_uint128(0xAAAAAAAAAAAAAAAA) << 64 | simde_uint128(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0x0000000000000000), simde_uint128(0xFFFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint64_t(0x5555555555555555), simde_uint128(0x0000000000000000) << 64 | simde_uint128(0x0000000000000001)),
        std::make_tuple(uint64_t(0x5555555555555555), simde_uint128(0x5555555555555555) << 64 | simde_uint128(0x5555555555555555)),
        std::make_tuple(uint64_t(0x5555555555555555), simde_uint128(0xAAAAAAAAAAAAAAAA) << 64 | simde_uint128(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0x5555555555555555), simde_uint128(0xFFFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), simde_uint128(0x0000000000000000) << 64 | simde_uint128(0x0000000000000001)),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), simde_uint128(0x5555555555555555) << 64 | simde_uint128(0x5555555555555555)),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), simde_uint128(0xAAAAAAAAAAAAAAAA) << 64 | simde_uint128(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), simde_uint128(0xFFFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint64_t(0x8000000000000000), simde_uint128(0x0000000000000000) << 64 | simde_uint128(0x0000000000000001)),
        std::make_tuple(uint64_t(0x8000000000000000), simde_uint128(0x5555555555555555) << 64 | simde_uint128(0x5555555555555555)),
        std::make_tuple(uint64_t(0x8000000000000000), simde_uint128(0xAAAAAAAAAAAAAAAA) << 64 | simde_uint128(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0x8000000000000000), simde_uint128(0xFFFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), simde_uint128(0x0000000000000000) << 64 | simde_uint128(0x0000000000000001)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), simde_uint128(0x5555555555555555) << 64 | simde_uint128(0x5555555555555555)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), simde_uint128(0xAAAAAAAAAAAAAAAA) << 64 | simde_uint128(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0xAAAAAAAAAAAAAAAA), simde_uint128(0xFFFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), simde_uint128(0x0000000000000000) << 64 | simde_uint128(0x0000000000000001)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), simde_uint128(0x5555555555555555) << 64 | simde_uint128(0x5555555555555555)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), simde_uint128(0xAAAAAAAAAAAAAAAA) << 64 | simde_uint128(0xAAAAAAAAAAAAAAAA)),
        std::make_tuple(uint64_t(0xFFFFFFFFFFFFFFFF), simde_uint128(0xFFFFFFFFFFFFFFFF) << 64 | simde_uint128(0xFFFFFFFFFFFFFFFF))
    },
    {
        std::make_tuple(uint64_t(0x0000000000000000), dpf::bit::one),
        std::make_tuple(uint64_t(0x5555555555555555), dpf::bit::one),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), dpf::bit::one),
        std::make_tuple(uint64_t(0x8000000000000000), dpf::bit::one),
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
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), dpf::bitstring<10>(0x001)),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), dpf::bitstring<10>(0x155)),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), dpf::bitstring<10>(0x2AA)),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), dpf::bitstring<10>(0x3FF)),
        std::make_tuple(uint64_t(0x8000000000000000), dpf::bitstring<10>(0x001)),
        std::make_tuple(uint64_t(0x8000000000000000), dpf::bitstring<10>(0x155)),
        std::make_tuple(uint64_t(0x8000000000000000), dpf::bitstring<10>(0x2AA)),
        std::make_tuple(uint64_t(0x8000000000000000), dpf::bitstring<10>(0x3FF)),
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
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), dpf::xor_wrapper<uint64_t>(uint64_t(0x0000000000000001))),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), dpf::xor_wrapper<uint64_t>(uint64_t(0x5555555555555555))),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), dpf::xor_wrapper<uint64_t>(uint64_t(0xAAAAAAAAAAAAAAAA))),
        std::make_tuple(uint64_t(0x7FFFFFFFFFFFFFFF), dpf::xor_wrapper<uint64_t>(uint64_t(0xFFFFFFFFFFFFFFFF))),
        std::make_tuple(uint64_t(0x8000000000000000), dpf::xor_wrapper<uint64_t>(uint64_t(0x0000000000000001))),
        std::make_tuple(uint64_t(0x8000000000000000), dpf::xor_wrapper<uint64_t>(uint64_t(0x5555555555555555))),
        std::make_tuple(uint64_t(0x8000000000000000), dpf::xor_wrapper<uint64_t>(uint64_t(0xAAAAAAAAAAAAAAAA))),
        std::make_tuple(uint64_t(0x8000000000000000), dpf::xor_wrapper<uint64_t>(uint64_t(0xFFFFFFFFFFFFFFFF))),
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

#endif  // LIBDPF_TEST_TESTS_HELPERS_EVAL_COMMON_DATA_HPP__
