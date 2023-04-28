#include <gtest/gtest.h>

#include "asio.hpp"
#include "dpf.hpp"

#include "helpers/eval_common_data.hpp"

template <typename T>
struct EvalIntervalTest : public testing::Test
{
  protected:
    EvalIntervalTest()
      : params{std::get<std::vector<T>>(allParams)}
    { }

    void SetUp() override
    { }

    void TearDown() override
    { }

    std::vector<T> params;
};

TYPED_TEST_SUITE_P(EvalIntervalTest);

TYPED_TEST_P(EvalIntervalTest, SurroundingInterval)
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
        integral_type x_int = to_integral_type(x),
                      msb_mask = to_integral_type(dpf::utils::msb_of_v<input_type>),
                      max_int = msb_mask | msb_mask-1, from_int, to_int;
        // set [from_int, to_int] to be centered around x_int if possible
        //   use [0, 2*range] or [max_int-2*range, max_int] as needed otherwise
        // range is selected to be at most 1 less than half the maximum range for input_type
        //   this ensures there are no overflow issues
        if (x_int < range)
        {
            from_int = 0;
            to_int = range << 1;
        }
        else if (x_int > max_int - range)
        {
            from_int = max_int - (range << 1);
            to_int = max_int;
        }
        else
        {
            from_int = x_int - range;
            to_int = x_int + range;
        }
        input_type from = from_integral_type(from_int), to = from_integral_type(to_int), cur = from;
        output_type zero_output = output_type(0);
        auto [buf0, iter0] = dpf::eval_interval(dpf0, from, to);
        auto [buf1, iter1] = dpf::eval_interval(dpf1, from, to);
        auto it0 = std::begin(iter0), it1 = std::begin(iter1);

        for (std::size_t i = 0; i <= range<<1; ++i, ++cur, ++it0, ++it1)
        {
            if (cur == x)
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

TYPED_TEST_P(EvalIntervalTest, BasicIntervalMemoizer)
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
        integral_type x_int = to_integral_type(x),
                      msb_mask = to_integral_type(dpf::utils::msb_of_v<input_type>),
                      max_int = msb_mask | msb_mask-1, from_int, to_int;
        // set [from_int, to_int] to be centered around x_int if possible
        //   use [0, 2*range] or [max_int-2*range, max_int] as needed otherwise
        // range is selected to be at most 1 less than half the maximum range for input_type
        //   this ensures there are no overflow issues
        if (x_int < range)
        {
            from_int = 0;
            to_int = range << 1;
        }
        else if (x_int > max_int - range)
        {
            from_int = max_int - (range << 1);
            to_int = max_int;
        }
        else
        {
            from_int = x_int - range;
            to_int = x_int + range;
        }
        input_type from = from_integral_type(from_int), to = from_integral_type(to_int), cur = from;
        output_type zero_output = output_type(0);
        auto buf0 = dpf::make_output_buffer_for_interval(dpf0, from, to),
             buf1 = dpf::make_output_buffer_for_interval(dpf1, from, to);
        auto memo0 = dpf::make_basic_interval_memoizer(dpf0, from, to),
             memo1 = dpf::make_basic_interval_memoizer(dpf1, from, to);
        auto iter0 = dpf::eval_interval(dpf0, from, to, buf0, memo0),
             iter1 = dpf::eval_interval(dpf1, from, to, buf1, memo1);
        auto it0 = std::begin(iter0), it1 = std::begin(iter1);

        for (std::size_t i = 0; i <= range<<1; ++i, ++cur, ++it0, ++it1)
        {
            if (cur == x)
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

TYPED_TEST_P(EvalIntervalTest, FullTreeIntervalMemoizer)
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
        integral_type x_int = to_integral_type(x),
                      msb_mask = to_integral_type(dpf::utils::msb_of_v<input_type>),
                      max_int = msb_mask | msb_mask-1, from_int, to_int;
        // set [from_int, to_int] to be centered around x_int if possible
        //   use [0, 2*range] or [max_int-2*range, max_int] as needed otherwise
        // range is selected to be at most 1 less than half the maximum range for input_type
        //   this ensures there are no overflow issues
        if (x_int < range)
        {
            from_int = 0;
            to_int = range << 1;
        }
        else if (x_int > max_int - range)
        {
            from_int = max_int - (range << 1);
            to_int = max_int;
        }
        else
        {
            from_int = x_int - range;
            to_int = x_int + range;
        }
        input_type from = from_integral_type(from_int), to = from_integral_type(to_int), cur = from;
        output_type zero_output = output_type(0);
        auto buf0 = dpf::make_output_buffer_for_interval(dpf0, from, to),
             buf1 = dpf::make_output_buffer_for_interval(dpf1, from, to);
        auto memo0 = dpf::make_full_tree_interval_memoizer(dpf0, from, to),
             memo1 = dpf::make_full_tree_interval_memoizer(dpf1, from, to);
        auto iter0 = dpf::eval_interval(dpf0, from, to, buf0, memo0),
             iter1 = dpf::eval_interval(dpf1, from, to, buf1, memo1);
        auto it0 = std::begin(iter0), it1 = std::begin(iter1);

        for (std::size_t i = 0; i <= range<<1; ++i, ++cur, ++it0, ++it1)
        {
            if (cur == x)
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

REGISTER_TYPED_TEST_SUITE_P(EvalIntervalTest,
    SurroundingInterval,
    BasicIntervalMemoizer,
    FullTreeIntervalMemoizer);
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
INSTANTIATE_TYPED_TEST_SUITE_P(EvalIntervalTestInstantiation, EvalIntervalTest, Types);
