#include <gtest/gtest.h>

#include <set>

#include "asio.hpp"
#include "dpf.hpp"

#include "helpers/eval_common_data.hpp"

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
        auto recipe0 = dpf::make_sequence_recipe(dpf0, points.begin(), points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, points.begin(), points.end());
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

TYPED_TEST_P(EvalSequenceTest, InplaceReversingSequenceMemoizer)
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
        auto recipe0 = dpf::make_sequence_recipe(dpf0, points.begin(), points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, points.begin(), points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto memo0 = dpf::make_inplace_reversing_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_inplace_reversing_sequence_memoizer(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, memo0),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, memo1);
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

TYPED_TEST_P(EvalSequenceTest, DoubleSpaceSequenceMemoizer)
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
        auto recipe0 = dpf::make_sequence_recipe(dpf0, points.begin(), points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, points.begin(), points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto memo0 = dpf::make_double_space_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_double_space_sequence_memoizer(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, memo0),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, memo1);
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

TYPED_TEST_P(EvalSequenceTest, FullTreeSequenceMemoizer)
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
        auto recipe0 = dpf::make_sequence_recipe(dpf0, points.begin(), points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, points.begin(), points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto memo0 = dpf::make_full_tree_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_full_tree_sequence_memoizer(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, memo0),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, memo1);
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

REGISTER_TYPED_TEST_SUITE_P(EvalSequenceTest,
    RandomPointsRecipe,
    RandomPointsNoRecipe,
    InplaceReversingSequenceMemoizer,
    DoubleSpaceSequenceMemoizer,
    FullTreeSequenceMemoizer);
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
