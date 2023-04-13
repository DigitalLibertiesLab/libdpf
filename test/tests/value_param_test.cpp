#include <gtest/gtest.h>

#include <tuple>
#include <cxxabi.h>

template <typename InputT,
          typename OutputT>
struct ValueParamTest : public testing::TestWithParam<std::tuple<InputT, OutputT>>
{
    // test fixture setup
};

using ValueParamTest008016 = ValueParamTest<uint8_t, uint16_t>;
using ValueParamTest032szt = ValueParamTest<uint32_t, std::size_t>;
using ValueParamTestszt128 = ValueParamTest<std::size_t, __int128>;

template <typename T>
void test_one(T & param)
{
    auto [input, output] = param;

    char *input_name = abi::__cxa_demangle(typeid(input).name(), NULL, NULL, NULL),
         *output_name = abi::__cxa_demangle(typeid(output).name(), NULL, NULL, NULL);

    std::cout << "----- Print 1 -----\n"
                << "    Input:  " << input_name << " - " << static_cast<uint64_t>(input) << "\n"
                << "    Output: " << output_name << " - " << static_cast<uint64_t>(output) << "\n";

    free(output_name);
    free(input_name);
}

TEST_P(ValueParamTest008016, One)
{
    test_one(GetParam());
}
TEST_P(ValueParamTest032szt, One)
{
    test_one(GetParam());
}
TEST_P(ValueParamTestszt128, One)
{
    test_one(GetParam());
}

template <typename T>
void test_two(T & param)
{
    auto [input, output] = param;

    char *input_name = abi::__cxa_demangle(typeid(input).name(), NULL, NULL, NULL),
         *output_name = abi::__cxa_demangle(typeid(output).name(), NULL, NULL, NULL);

    std::cout << "----- Print 1 -----\n"
                << "    Input:  " << input_name << " - " << static_cast<uint64_t>(input) << "\n"
                << "    Output: " << output_name << " - " << static_cast<uint64_t>(output) << "\n"
                << "----- Print 2 -----\n";

    free(output_name);
    free(input_name);
}

TEST_P(ValueParamTest008016, Two)
{
    test_two(GetParam());
}
TEST_P(ValueParamTest032szt, Two)
{
    test_two(GetParam());
}
TEST_P(ValueParamTestszt128, Two)
{
    test_two(GetParam());
}

INSTANTIATE_TEST_SUITE_P(BaseValueInstantiation008016, ValueParamTest008016, testing::Values(
    std::make_tuple(uint8_t(0), uint16_t(1)),
    std::make_tuple(uint8_t(1), ~uint16_t(0)),
    std::make_tuple(uint8_t(2), uint16_t(0x5555))
));
INSTANTIATE_TEST_SUITE_P(BaseValueInstantiation032szt, ValueParamTest032szt, testing::Values(
    std::make_tuple(uint32_t(3), std::size_t(1)),
    std::make_tuple(uint32_t(4), ~std::size_t(0)),
    std::make_tuple(uint32_t(5), std::size_t(0x5555555555555555))
));
INSTANTIATE_TEST_SUITE_P(BaseValueInstantiationszt128, ValueParamTestszt128, testing::Values(
    std::make_tuple(std::size_t(6), __int128(1)),
    std::make_tuple(std::size_t(7), ~__int128(0))
));
