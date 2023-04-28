#include <gtest/gtest.h>

#include "asio.hpp"
#include "dpf.hpp"

#include "helpers/eval_common_data.hpp"

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

    std::size_t range_bitlength = std::min(dpf::utils::bitlength_of_v<input_type>, std::size_t(10)),
                range = std::size_t(1) << range_bitlength-1;

    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        input_type xp = x, xm = x; ++xp; --xm;
        output_type zero_output = output_type(0);

        for (std::size_t i = 1; i < range; ++i, ++xp, --xm)
        {
            output_type y0p = dpf::eval_point(dpf0, xp),
                        y1p = dpf::eval_point(dpf1, xp),
                        y0m = dpf::eval_point(dpf0, xm),
                        y1m = dpf::eval_point(dpf1, xm);
            ASSERT_EQ(static_cast<output_type>(y1p - y0p), zero_output);
            ASSERT_EQ(static_cast<output_type>(y1m - y0m), zero_output);
        }
    }
}

TYPED_TEST_P(EvalPointTest, BasicPathMemoizer)
{
    using input_type = typename std::tuple_element_t<0, TypeParam>;
    using output_type = typename std::tuple_element_t<1, TypeParam>;
    using integral_type = dpf::utils::integral_type_from_bitlength_t<dpf::utils::bitlength_of_v<input_type>>;

    auto to_integral_type = dpf::utils::to_integral_type<input_type>{};
    auto from_integral_type = dpf::utils::make_from_integral_value<input_type>{};

    std::size_t range_bitlength = std::min(dpf::utils::bitlength_of_v<input_type>, std::size_t(10)),
                range = (std::size_t(1) << range_bitlength-1)-1;

    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto memo0 = dpf::make_basic_path_memoizer(dpf0),
             memo1 = dpf::make_basic_path_memoizer(dpf1);
        integral_type x_int = to_integral_type(x),
                      msb_mask = to_integral_type(dpf::utils::msb_of_v<input_type>),
                      max_int = msb_mask | msb_mask-1, start_int;
        if (x_int < range)
        {
            start_int = 0;
        }
        else if (x_int > max_int - range)
        {
            start_int = max_int - (range << 1);
        }
        else
        {
            start_int = x_int - range;
        }
        input_type start = from_integral_type(start_int), cur = start;
        output_type zero_output = output_type(0);

        for (std::size_t i = 0; i <= range<<1; ++i, ++cur)
        {
            output_type y0 = dpf::eval_point(dpf0, cur, memo0),
                        y1 = dpf::eval_point(dpf1, cur, memo1);
            if (cur == x)
            {
                ASSERT_EQ(static_cast<output_type>(y1 - y0), y);
            }
            else
            {
                ASSERT_EQ(static_cast<output_type>(y1 - y0), zero_output);
            }
        }
    }
}

TYPED_TEST_P(EvalPointTest, NonmemoizingPathMemoizer)
{
    using input_type = typename std::tuple_element_t<0, TypeParam>;
    using output_type = typename std::tuple_element_t<1, TypeParam>;
    using integral_type = dpf::utils::integral_type_from_bitlength_t<dpf::utils::bitlength_of_v<input_type>>;

    auto to_integral_type = dpf::utils::to_integral_type<input_type>{};
    auto from_integral_type = dpf::utils::make_from_integral_value<input_type>{};

    std::size_t range_bitlength = std::min(dpf::utils::bitlength_of_v<input_type>, std::size_t(10)),
                range = (std::size_t(1) << range_bitlength-1)-1;

    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto memo0 = dpf::make_nonmemoizing_path_memoizer(dpf0),
             memo1 = dpf::make_nonmemoizing_path_memoizer(dpf1);
        integral_type x_int = to_integral_type(x),
                      msb_mask = to_integral_type(dpf::utils::msb_of_v<input_type>),
                      max_int = msb_mask | msb_mask-1, start_int;
        if (x_int < range)
        {
            start_int = 0;
        }
        else if (x_int > max_int - range)
        {
            start_int = max_int - (range << 1);
        }
        else
        {
            start_int = x_int - range;
        }
        input_type start = from_integral_type(start_int), cur = start;
        output_type zero_output = output_type(0);

        for (std::size_t i = 0; i <= range<<1; ++i, ++cur)
        {
            output_type y0 = dpf::eval_point(dpf0, cur, memo0),
                        y1 = dpf::eval_point(dpf1, cur, memo1);
            if (cur == x)
            {
                ASSERT_EQ(static_cast<output_type>(y1 - y0), y);
            }
            else
            {
                ASSERT_EQ(static_cast<output_type>(y1 - y0), zero_output);
            }
        }
    }
}

REGISTER_TYPED_TEST_SUITE_P(EvalPointTest,
    DistinguishedPoint,
    SurroundingPoints,
    BasicPathMemoizer,
    NonmemoizingPathMemoizer);
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
