#include <gtest/gtest.h>

#include <utility>

#include "dpf.hpp"

#include "helpers/eval_common_multi_data.hpp"

template <typename T>
struct EvalIntervalMultiTest : public testing::Test
{
  public:
    using input_type = typename std::tuple_element_t<0, T>;
    using output_type0 = typename std::tuple_element_t<1, T>;
    using output_type1 = typename std::tuple_element_t<2, T>;
    using output_type2 = typename std::tuple_element_t<3, T>;
    using output_type3 = typename std::tuple_element_t<4, T>;
    using integral_type = dpf::utils::integral_type_from_bitlength_t<dpf::utils::bitlength_of_v<input_type>>;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type0, output_type1, output_type2, output_type3>;

  protected:
    EvalIntervalMultiTest()
      : params{std::get<std::vector<T>>(allParams)},
        range{(std::size_t(1) << std::min(dpf::utils::bitlength_of_v<input_type>, std::size_t(10))-1)-1},
        zero_output0{from_integral_type_output0(0)},
        zero_output1{from_integral_type_output1(0)},
        zero_output2{from_integral_type_output2(0)},
        zero_output3{from_integral_type_output3(0)},
        max_from_to{get_max_from_to()}
    { }

    void SetUp() override
    { }

    void TearDown() override
    { }

    std::pair<input_type, input_type> get_from_to(const input_type & x)
    {
        integral_type x_int = to_integral_type(x),
                      min_int = to_integral_type(std::numeric_limits<input_type>::min()),
                      max_int = to_integral_type(std::numeric_limits<input_type>::max()),
                      from_int, to_int;
        // set [from_int, to_int] to be centered around x_int if possible
        //   use [min_int, min_int+2*range] or [max_int-2*range, max_int] as needed otherwise
        // range is selected to be at most 1 less than half the maximum range for input_type
        //   this ensures there are no overflow issues
        // for signed integral types, since the MSB is internally flipped,
        //   needed additional check that x_int was in the correct range for the given
        //   conditionals (note that these added checks are always try for unsigned types)
        if (x_int < min_int + range && x_int >= min_int)
        {
            from_int = min_int;
            to_int = min_int + (range << 1);
        }
        else if (x_int > max_int - range && x_int <= max_int)
        {
            from_int = max_int - (range << 1);
            to_int = max_int;
        }
        else
        {
            from_int = x_int - range;
            to_int = x_int + range;
        }
        return std::make_pair(from_integral_type(from_int), from_integral_type(to_int));
    }

    template <typename IterableT>
    void assert_wrapper(const input_type & x, const output_type0 & y0,
        const output_type1 & y1, const output_type2 & y2, const output_type3 & y3,
        input_type cur, const IterableT & iter0, const IterableT & iter1)
    {
        auto zip0 = dpf::tuple_as_zip(iter0),
             zip1 = dpf::tuple_as_zip(iter1);
        auto it0 = std::begin(zip0),
             it1 = std::begin(zip1);
        for (std::size_t i = 0; i <= range<<1; ++i, ++cur, ++it0, ++it1)
        {
            if (cur == x)
            {
                ASSERT_EQ(static_cast<output_type0>(std::get<0>(*it1) - std::get<0>(*it0)), y0);
                ASSERT_EQ(static_cast<output_type1>(std::get<1>(*it1) - std::get<1>(*it0)), y1);
                ASSERT_EQ(static_cast<output_type2>(std::get<2>(*it1) - std::get<2>(*it0)), y2);
                ASSERT_EQ(static_cast<output_type3>(std::get<3>(*it1) - std::get<3>(*it0)), y3);
            }
            else
            {
                ASSERT_EQ(static_cast<output_type0>(std::get<0>(*it1) - std::get<0>(*it0)), zero_output0);
                ASSERT_EQ(static_cast<output_type1>(std::get<1>(*it1) - std::get<1>(*it0)), zero_output1);
                ASSERT_EQ(static_cast<output_type2>(std::get<2>(*it1) - std::get<2>(*it0)), zero_output2);
                ASSERT_EQ(static_cast<output_type3>(std::get<3>(*it1) - std::get<3>(*it0)), zero_output3);
            }
        }
        ASSERT_EQ(it0, std::end(zip0));
        ASSERT_EQ(it1, std::end(zip1));
    }

    // calculate maximum node difference between from and to
    // this allows the memoizers to be created with the correct size in advance
    std::pair<input_type, input_type> get_max_from_to()
    {
        input_type max_from, max_to;
        std::size_t max_range = 0;

        for (auto [x, y0, y1, y2, y3] : this->params)
        {
            auto [from, to] = this->get_from_to(x);
            std::size_t cur_range = dpf::utils::get_nodes_in_interval<dpf_type>(from, to);
            if (cur_range > max_range)
            {
                max_range = cur_range;
                max_from = from;
                max_to = to;
            }
        }

        return std::make_pair(max_from, max_to);
    }

    static constexpr auto to_integral_type = dpf::utils::to_integral_type<input_type>{};
    static constexpr auto from_integral_type = dpf::utils::make_from_integral_value<input_type>{};
    static constexpr auto from_integral_type_output0 = dpf::utils::make_from_integral_value<output_type0>{};
    static constexpr auto from_integral_type_output1 = dpf::utils::make_from_integral_value<output_type1>{};
    static constexpr auto from_integral_type_output2 = dpf::utils::make_from_integral_value<output_type2>{};
    static constexpr auto from_integral_type_output3 = dpf::utils::make_from_integral_value<output_type3>{};

    std::vector<T> params;
    std::size_t range;
    output_type0 zero_output0;
    output_type1 zero_output1;
    output_type2 zero_output2;
    output_type3 zero_output3;
    std::pair<input_type, input_type> max_from_to;
};

TYPED_TEST_SUITE_P(EvalIntervalMultiTest);

TYPED_TEST_P(EvalIntervalMultiTest, Basic)
{
    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [from, to] = this->get_from_to(x);
        auto [buf0, iter0] = dpf::eval_interval<0, 1, 2, 3>(dpf0, from, to);
        auto [buf1, iter1] = dpf::eval_interval<0, 1, 2, 3>(dpf1, from, to);

        this->assert_wrapper(x, y0, y1, y2, y3, from, iter0, iter1);
    }
}

TYPED_TEST_P(EvalIntervalMultiTest, Outbuf)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto buf0 = dpf::make_output_buffer_for_interval<dpf_type, 0, 1, 2, 3>(this->max_from_to.first, this->max_from_to.second),
         buf1 = dpf::make_output_buffer_for_interval<dpf_type, 0, 1, 2, 3>(this->max_from_to.first, this->max_from_to.second);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [from, to] = this->get_from_to(x);
        auto iter0 = dpf::eval_interval<0, 1, 2, 3>(dpf0, from, to, buf0),
             iter1 = dpf::eval_interval<0, 1, 2, 3>(dpf1, from, to, buf1);

        this->assert_wrapper(x, y0, y1, y2, y3, from, iter0, iter1);
    }
}

TYPED_TEST_P(EvalIntervalMultiTest, BasicIntervalMemoizer)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto memo0 = dpf::make_basic_interval_memoizer<dpf_type>(this->max_from_to.first, this->max_from_to.second),
         memo1 = dpf::make_basic_interval_memoizer<dpf_type>(this->max_from_to.first, this->max_from_to.second);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [from, to] = this->get_from_to(x);
        auto [buf0, iter0] = dpf::eval_interval<0, 1, 2, 3>(dpf0, from, to, memo0);
        auto [buf1, iter1] = dpf::eval_interval<0, 1, 2, 3>(dpf1, from, to, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, from, iter0, iter1);
    }
}

TYPED_TEST_P(EvalIntervalMultiTest, FullTreeIntervalMemoizer)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto memo0 = dpf::make_full_tree_interval_memoizer<dpf_type>(this->max_from_to.first, this->max_from_to.second),
         memo1 = dpf::make_full_tree_interval_memoizer<dpf_type>(this->max_from_to.first, this->max_from_to.second);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [from, to] = this->get_from_to(x);
        auto [buf0, iter0] = dpf::eval_interval<0, 1, 2, 3>(dpf0, from, to, memo0);
        auto [buf1, iter1] = dpf::eval_interval<0, 1, 2, 3>(dpf1, from, to, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, from, iter0, iter1);
    }
}

TYPED_TEST_P(EvalIntervalMultiTest, BasicIntervalMemoizerOutbuf)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto buf0 = dpf::make_output_buffer_for_interval<dpf_type, 0, 1, 2, 3>(this->max_from_to.first, this->max_from_to.second),
         buf1 = dpf::make_output_buffer_for_interval<dpf_type, 0, 1, 2, 3>(this->max_from_to.first, this->max_from_to.second);
    auto memo0 = dpf::make_basic_interval_memoizer<dpf_type>(this->max_from_to.first, this->max_from_to.second),
         memo1 = dpf::make_basic_interval_memoizer<dpf_type>(this->max_from_to.first, this->max_from_to.second);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [from, to] = this->get_from_to(x);
        auto iter0 = dpf::eval_interval<0, 1, 2, 3>(dpf0, from, to, buf0, memo0),
             iter1 = dpf::eval_interval<0, 1, 2, 3>(dpf1, from, to, buf1, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, from, iter0, iter1);
    }
}

TYPED_TEST_P(EvalIntervalMultiTest, FullTreeIntervalMemoizerOutbuf)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto buf0 = dpf::make_output_buffer_for_interval<dpf_type, 0, 1, 2, 3>(this->max_from_to.first, this->max_from_to.second),
         buf1 = dpf::make_output_buffer_for_interval<dpf_type, 0, 1, 2, 3>(this->max_from_to.first, this->max_from_to.second);
    auto memo0 = dpf::make_full_tree_interval_memoizer<dpf_type>(this->max_from_to.first, this->max_from_to.second),
         memo1 = dpf::make_full_tree_interval_memoizer<dpf_type>(this->max_from_to.first, this->max_from_to.second);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [from, to] = this->get_from_to(x);
        auto iter0 = dpf::eval_interval<0, 1, 2, 3>(dpf0, from, to, buf0, memo0),
             iter1 = dpf::eval_interval<0, 1, 2, 3>(dpf1, from, to, buf1, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, from, iter0, iter1);
    }
}

REGISTER_TYPED_TEST_SUITE_P(EvalIntervalMultiTest,
    Basic,
    Outbuf,
    BasicIntervalMemoizer,
    FullTreeIntervalMemoizer,
    BasicIntervalMemoizerOutbuf,
    FullTreeIntervalMemoizerOutbuf);
using Types = testing::Types
<
    // base test
    test_type<uint16_t, uint64_t>,

    // test input types
    test_type<int16_t, uint64_t>,
    test_type<uint8_t, uint64_t>,
    test_type<uint64_t, uint64_t>,
    test_type<simde_int128, uint64_t>,
    test_type<simde_uint128, uint64_t>,
    test_type<dpf::bitstring<10>, uint64_t>,
    test_type<dpf::keyword<3, dpf::alphabets::hex>, uint64_t>,
    test_type<dpf::modint<10>, uint64_t>,
    test_type<dpf::xor_wrapper<int16_t>, uint64_t>,
    test_type<dpf::xor_wrapper<uint16_t>, uint64_t>,

    // test output types
    test_type<uint16_t, int64_t>,
    test_type<uint16_t, uint8_t>,
    test_type<uint16_t, simde_int128>,
    test_type<uint16_t, simde_uint128>,
    test_type<uint16_t, dpf::bit>,
    test_type<uint16_t, dpf::bitstring<20, uint8_t>>,
    test_type<uint16_t, dpf::bitstring<150>>,
    test_type<uint16_t, dpf::xor_wrapper<int64_t>>,
    test_type<uint16_t, dpf::xor_wrapper<uint64_t>>,

    // custom types
    test_type<custom_input_type, uint64_t>,
    test_type<uint16_t, custom_output_type_small>,
    test_type<uint16_t, custom_output_type_large_plus_minus>,
    test_type<uint16_t, custom_output_type_large_xor>,

    // distinct output types
    multi_test_type<uint16_t, uint32_t, dpf::xor_wrapper<uint32_t>, dpf::bitstring<20, uint8_t>, dpf::bitstring<32>>
>;
INSTANTIATE_TYPED_TEST_SUITE_P(EvalIntervalMultiTestInstantiation, EvalIntervalMultiTest, Types);
