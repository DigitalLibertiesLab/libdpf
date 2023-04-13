#include <gtest/gtest.h>

#include <tuple>
#include <cxxabi.h>

template <typename InputT,
          typename OutputT>
using ParamT = std::vector<std::tuple<InputT, OutputT>>;

static std::tuple<ParamT<uint8_t, uint16_t>, ParamT<uint32_t, std::size_t>, ParamT<std::size_t, __int128>> allParams
{
    {
        std::make_tuple(uint8_t(0), uint16_t(1)),
        std::make_tuple(uint8_t(1), ~uint16_t(0)),
        std::make_tuple(uint8_t(2), uint16_t(0x5555))
    },
    {
        std::make_tuple(uint32_t(3), std::size_t(1)),
        std::make_tuple(uint32_t(4), ~std::size_t(0)),
        std::make_tuple(uint32_t(5), std::size_t(0x5555555555555555))
    },
    {
        std::make_tuple(std::size_t(6), __int128(1)),
        std::make_tuple(std::size_t(7), ~__int128(0))
    }
};

template <typename T>
struct TypeParamTest : public testing::Test
{
    // test fixture setup

    TypeParamTest()
      : params{std::get<std::vector<T>>(allParams)}
    { }

    std::vector<T> params;
};

TYPED_TEST_SUITE_P(TypeParamTest);

TYPED_TEST_P(TypeParamTest, One)
{
    for (auto [input, output] : this->params)
    {
        char *input_name = abi::__cxa_demangle(typeid(input).name(), NULL, NULL, NULL),
             *output_name = abi::__cxa_demangle(typeid(output).name(), NULL, NULL, NULL);

        std::cout << "----- Print 1 -----\n"
                  << "    Input:  " << input_name << " - " << static_cast<uint64_t>(input) << "\n"
                  << "    Output: " << output_name << " - " << static_cast<uint64_t>(output) << "\n";

        free(output_name);
        free(input_name);
    }
}

TYPED_TEST_P(TypeParamTest, Two)
{
    for (auto [input, output] : this->params)
    {
        char *input_name = abi::__cxa_demangle(typeid(input).name(), NULL, NULL, NULL),
             *output_name = abi::__cxa_demangle(typeid(output).name(), NULL, NULL, NULL);

        std::cout << "----- Print 1 -----\n"
                  << "    Input:  " << input_name << " - " << static_cast<uint64_t>(input) << "\n"
                  << "    Output: " << output_name << " - " << static_cast<uint64_t>(output) << "\n"
                  << "----- Print 2 -----\n";

        free(output_name);
        free(input_name);
    }
}

REGISTER_TYPED_TEST_SUITE_P(TypeParamTest, One, Two);
using Types = testing::Types<std::tuple<uint8_t, uint16_t>, std::tuple<uint32_t, std::size_t>, std::tuple<std::size_t, __int128>>;
INSTANTIATE_TYPED_TEST_SUITE_P(BaseTypeInstantiation, TypeParamTest, Types);
