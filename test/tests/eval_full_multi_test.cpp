#include <gtest/gtest.h>

#include "asio.hpp"
#include "dpf.hpp"

#include "helpers/eval_common_multi_data.hpp"

template <typename T>
struct EvalFullMultiTest : public testing::Test
{
  public:
    using input_type = typename std::tuple_element_t<0, T>;
    using output_type = typename std::tuple_element_t<1, T>;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type, output_type, output_type, output_type>;

  protected:
    EvalFullMultiTest()
      : params{std::get<std::vector<T>>(allParams)},
        range{std::size_t(1) << dpf::utils::bitlength_of_v<input_type>},
        zero_output{from_integral_type_output(0)}
    { }

    void SetUp() override
    { }

    void TearDown() override
    { }

    template <typename IterableT>
    void assert_wrapper(const input_type & x, const output_type & y0,
        const output_type & y1, const output_type & y2, const output_type & y3,
        const IterableT & iter0, const IterableT & iter1)
    {
        auto zip0 = dpf::tuple_as_zip(iter0),
             zip1 = dpf::tuple_as_zip(iter1);
        auto it0 = std::cbegin(zip0),
             it1 = std::cbegin(zip1);
        input_type cur = from_integral_type(0);
        for (std::size_t i = 0; i < range; ++i, ++cur, ++it0, ++it1)
        {
            if (cur == x)
            {
                ASSERT_EQ(static_cast<output_type>(std::get<0>(*it1) - std::get<0>(*it0)), y0);
                ASSERT_EQ(static_cast<output_type>(std::get<1>(*it1) - std::get<1>(*it0)), y1);
                ASSERT_EQ(static_cast<output_type>(std::get<2>(*it1) - std::get<2>(*it0)), y2);
                ASSERT_EQ(static_cast<output_type>(std::get<3>(*it1) - std::get<3>(*it0)), y3);
            }
            else
            {
                ASSERT_EQ(static_cast<output_type>(std::get<0>(*it1) - std::get<0>(*it0)), zero_output);
                ASSERT_EQ(static_cast<output_type>(std::get<1>(*it1) - std::get<1>(*it0)), zero_output);
                ASSERT_EQ(static_cast<output_type>(std::get<2>(*it1) - std::get<2>(*it0)), zero_output);
                ASSERT_EQ(static_cast<output_type>(std::get<3>(*it1) - std::get<3>(*it0)), zero_output);
            }
        }
        ASSERT_EQ(it0, std::end(zip0));
        ASSERT_EQ(it1, std::end(zip1));
    }

    static constexpr auto from_integral_type = dpf::utils::make_from_integral_value<input_type>{};
    static constexpr auto from_integral_type_output = dpf::utils::make_from_integral_value<output_type>{};

    std::vector<T> params;
    std::size_t range;
    output_type zero_output;
};

TYPED_TEST_SUITE_P(EvalFullMultiTest);

TYPED_TEST_P(EvalFullMultiTest, Basic)
{
    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_full<0, 1, 2, 3>(dpf0);
        auto [buf1, iter1] = dpf::eval_full<0, 1, 2, 3>(dpf1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalFullMultiTest, Outbuf)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto buf0 = dpf::make_output_buffer_for_full<dpf_type, 0, 1, 2, 3>(),
         buf1 = dpf::make_output_buffer_for_full<dpf_type, 0, 1, 2, 3>();

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_full<0, 1, 2, 3>(dpf0, buf0),
             iter1 = dpf::eval_full<0, 1, 2, 3>(dpf1, buf1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalFullMultiTest, BasicFullMemoizer)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto memo0 = dpf::make_basic_full_memoizer<dpf_type>(),
         memo1 = dpf::make_basic_full_memoizer<dpf_type>();

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_full<0, 1, 2, 3>(dpf0, memo0);
        auto [buf1, iter1] = dpf::eval_full<0, 1, 2, 3>(dpf1, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalFullMultiTest, FullTreeFullMemoizer)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto memo0 = dpf::make_full_tree_full_memoizer<dpf_type>(),
         memo1 = dpf::make_full_tree_full_memoizer<dpf_type>();

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_full<0, 1, 2, 3>(dpf0, memo0);
        auto [buf1, iter1] = dpf::eval_full<0, 1, 2, 3>(dpf1, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalFullMultiTest, BasicFullMemoizerOutbuf)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto buf0 = dpf::make_output_buffer_for_full<dpf_type, 0, 1, 2, 3>(),
         buf1 = dpf::make_output_buffer_for_full<dpf_type, 0, 1, 2, 3>();
    auto memo0 = dpf::make_basic_full_memoizer<dpf_type>(),
         memo1 = dpf::make_basic_full_memoizer<dpf_type>();

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_full<0, 1, 2, 3>(dpf0, buf0, memo0),
             iter1 = dpf::eval_full<0, 1, 2, 3>(dpf1, buf1, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalFullMultiTest, FullTreeFullMemoizerOutbuf)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto buf0 = dpf::make_output_buffer_for_full<dpf_type, 0, 1, 2, 3>(),
         buf1 = dpf::make_output_buffer_for_full<dpf_type, 0, 1, 2, 3>();
    auto memo0 = dpf::make_full_tree_full_memoizer<dpf_type>(),
         memo1 = dpf::make_full_tree_full_memoizer<dpf_type>();

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_full<0, 1, 2, 3>(dpf0, buf0, memo0),
             iter1 = dpf::eval_full<0, 1, 2, 3>(dpf1, buf1, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

REGISTER_TYPED_TEST_SUITE_P(EvalFullMultiTest,
    Basic,
    Outbuf,
    BasicFullMemoizer,
    FullTreeFullMemoizer,
    BasicFullMemoizerOutbuf,
    FullTreeFullMemoizerOutbuf);
using Types = testing::Types
<
    // base test
    test_type<uint16_t, uint64_t>,

    // test input types
    test_type<uint8_t, uint64_t>,
    test_type<dpf::bitstring<10>, uint64_t>,
    test_type<dpf::keyword<3, dpf::alphabets::hex>, uint64_t>,
    test_type<dpf::modint<10>, uint64_t>,
    test_type<dpf::xor_wrapper<uint16_t>, uint64_t>,

    // test output types
    test_type<uint16_t, uint8_t>,
    test_type<uint16_t, simde_uint128>,
    test_type<uint16_t, dpf::bit>,
    // test_type<uint16_t, dpf::bitstring<10>>,
    test_type<uint16_t, dpf::xor_wrapper<uint64_t>>
>;
INSTANTIATE_TYPED_TEST_SUITE_P(EvalFullMultiTestInstantiation, EvalFullMultiTest, Types);
