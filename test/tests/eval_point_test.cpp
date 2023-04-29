#include <gtest/gtest.h>

#include "asio.hpp"
#include "dpf.hpp"

#include "helpers/eval_common_data.hpp"

template <typename T>
struct EvalPointTest : public testing::Test
{
  public:
    using input_type = typename std::tuple_element_t<0, T>;
    using output_type = typename std::tuple_element_t<1, T>;
    using integral_type = dpf::utils::integral_type_from_bitlength_t<dpf::utils::bitlength_of_v<input_type>>;

  protected:
    EvalPointTest()
      : params{std::get<std::vector<T>>(allParams)},
        range{(std::size_t(1) << std::min(dpf::utils::bitlength_of_v<input_type>, std::size_t(10))-1)-1},
        zero_output(output_type(0))
    { }

    void SetUp() override
    { }

    void TearDown() override
    { }

    input_type get_start(const input_type & x)
    {
        integral_type x_int = to_integral_type(x),
                      msb_mask = to_integral_type(dpf::utils::msb_of_v<input_type>),
                      max_int = msb_mask | msb_mask-1, start_int;
        // set start_int so that the tested range is centered around x_int if possible
        //   use start_int = 0 if x_int smaller than 2*range
        //   or start_int = max_int-2*range if x_int larger than max_int-2*range
        // range is selected to be at most 1 less than half the maximum range for input_type
        //   this ensures there are no overflow issues
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
        return from_integral_type(start_int);
    }

    template <typename UnaryFunction0, typename UnaryFunction1>
    void assert_wrapper(const input_type & x, const output_type & y,
        UnaryFunction0 f0, UnaryFunction1 f1)
    {
        input_type cur = get_start(x);
        for (std::size_t i = 0; i <= range<<1; ++i, ++cur)
        {
            output_type y0 = f0(cur),
                        y1 = f1(cur);
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

    static constexpr auto to_integral_type = dpf::utils::to_integral_type<input_type>{};
    static constexpr auto from_integral_type = dpf::utils::make_from_integral_value<input_type>{};

    std::vector<T> params;
    std::size_t range;
    output_type zero_output;
};

TYPED_TEST_SUITE_P(EvalPointTest);

TYPED_TEST_P(EvalPointTest, SurroundingPoints)
{
    using input_type = typename std::tuple_element_t<0, TypeParam>;

    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);

        this->assert_wrapper(x, y,
            [&dpf0](input_type cur)
            {
                return dpf::eval_point(dpf0, cur);
            },
            [&dpf1](input_type cur)
            {
                return dpf::eval_point(dpf1, cur);
            }
        );
    }
}

TYPED_TEST_P(EvalPointTest, BasicPathMemoizer)
{
    using input_type = typename std::tuple_element_t<0, TypeParam>;

    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto memo0 = dpf::make_basic_path_memoizer(dpf0),
             memo1 = dpf::make_basic_path_memoizer(dpf1);

        this->assert_wrapper(x, y,
            [&dpf0, &memo0](input_type cur)
            {
                return dpf::eval_point(dpf0, cur, memo0);
            },
            [&dpf1, &memo1](input_type cur)
            {
                return dpf::eval_point(dpf1, cur, memo1);
            }
        );
    }
}

TYPED_TEST_P(EvalPointTest, NonmemoizingPathMemoizer)
{
    using input_type = typename std::tuple_element_t<0, TypeParam>;

    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto memo0 = dpf::make_nonmemoizing_path_memoizer(dpf0),
             memo1 = dpf::make_nonmemoizing_path_memoizer(dpf1);

        this->assert_wrapper(x, y,
            [&dpf0, &memo0](input_type cur)
            {
                return dpf::eval_point(dpf0, cur, memo0);
            },
            [&dpf1, &memo1](input_type cur)
            {
                return dpf::eval_point(dpf1, cur, memo1);
            }
        );
    }
}

REGISTER_TYPED_TEST_SUITE_P(EvalPointTest,
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
