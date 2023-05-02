#include <gtest/gtest.h>

#include "asio.hpp"
#include "dpf.hpp"

#include "helpers/eval_common_data.hpp"

template <typename T>
struct EvalFullTest : public testing::Test
{
  public:
    using input_type = typename std::tuple_element_t<0, T>;
    using output_type = typename std::tuple_element_t<1, T>;

  protected:
    EvalFullTest()
      : params{std::get<std::vector<T>>(allParams)},
        range{std::size_t(1) << dpf::utils::bitlength_of_v<input_type>},
        zero_output{from_integral_type_output(0)}
    { }

    void SetUp() override
    { }

    void TearDown() override
    { }

    template <typename IterableT>
    void assert_wrapper(const input_type & x, const output_type & y,
        const IterableT & iter0, const IterableT & iter1)
    {
        auto it0 = std::cbegin(iter0),
             it1 = std::cbegin(iter1);
        input_type cur = from_integral_type(0);
        for (std::size_t i = 0; i < range; ++i, ++cur, ++it0, ++it1)
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
        ASSERT_EQ(it0, std::end(iter0));
        ASSERT_EQ(it1, std::end(iter1));
    }

    static constexpr auto from_integral_type = dpf::utils::make_from_integral_value<input_type>{};
    static constexpr auto from_integral_type_output = dpf::utils::make_from_integral_value<output_type>{};

    std::vector<T> params;
    std::size_t range;
    output_type zero_output;
};

TYPED_TEST_SUITE_P(EvalFullTest);

TYPED_TEST_P(EvalFullTest, Basic)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto [buf0, iter0] = dpf::eval_full(dpf0);
        auto [buf1, iter1] = dpf::eval_full(dpf1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalFullTest, Outbuf)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto buf0 = dpf::make_output_buffer_for_full(dpf0),
             buf1 = dpf::make_output_buffer_for_full(dpf1);
        auto iter0 = dpf::eval_full(dpf0, buf0),
             iter1 = dpf::eval_full(dpf1, buf1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalFullTest, BasicIntervalMemoizer)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto memo0 = dpf::make_basic_full_memoizer(dpf0),
             memo1 = dpf::make_basic_full_memoizer(dpf1);
        auto [buf0, iter0] = dpf::eval_full(dpf0, memo0);
        auto [buf1, iter1] = dpf::eval_full(dpf1, memo1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalFullTest, FullTreeIntervalMemoizer)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto memo0 = dpf::make_full_tree_full_memoizer(dpf0),
             memo1 = dpf::make_full_tree_full_memoizer(dpf1);
        auto [buf0, iter0] = dpf::eval_full(dpf0, memo0);
        auto [buf1, iter1] = dpf::eval_full(dpf1, memo1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalFullTest, BasicIntervalMemoizerOutbuf)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto buf0 = dpf::make_output_buffer_for_full(dpf0),
             buf1 = dpf::make_output_buffer_for_full(dpf1);
        auto memo0 = dpf::make_basic_full_memoizer(dpf0),
             memo1 = dpf::make_basic_full_memoizer(dpf1);
        auto iter0 = dpf::eval_full(dpf0, buf0, memo0),
             iter1 = dpf::eval_full(dpf1, buf1, memo1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalFullTest, FullTreeIntervalMemoizerOutbuf)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto buf0 = dpf::make_output_buffer_for_full(dpf0),
             buf1 = dpf::make_output_buffer_for_full(dpf1);
        auto memo0 = dpf::make_full_tree_full_memoizer(dpf0),
             memo1 = dpf::make_full_tree_full_memoizer(dpf1);
        auto iter0 = dpf::eval_full(dpf0, buf0, memo0),
             iter1 = dpf::eval_full(dpf1, buf1, memo1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

REGISTER_TYPED_TEST_SUITE_P(EvalFullTest,
    Basic,
    Outbuf,
    BasicIntervalMemoizer,
    FullTreeIntervalMemoizer,
    BasicIntervalMemoizerOutbuf,
    FullTreeIntervalMemoizerOutbuf);

using Types = testing::Types
<
    test_type<uint8_t, uint64_t>,
    test_type<dpf::bitstring<10>, uint64_t>,
    test_type<dpf::keyword<3, dpf::alphabets::hex>, uint64_t>,
    test_type<dpf::modint<10>, uint64_t>
>;
INSTANTIATE_TYPED_TEST_SUITE_P(EvalFullTestInstantiation, EvalFullTest, Types);
