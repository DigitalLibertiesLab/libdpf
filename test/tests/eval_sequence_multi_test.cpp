#include <gtest/gtest.h>

#include <set>

#include "asio.hpp"
#include "dpf.hpp"

#include "helpers/eval_common_multi_data.hpp"

template <typename T>
struct EvalSequenceMultiTest : public testing::Test
{
  public:
    using input_type = typename std::tuple_element_t<0, T>;
    using output_type = typename std::tuple_element_t<1, T>;
    using integral_type = dpf::utils::integral_type_from_bitlength_t<dpf::utils::bitlength_of_v<input_type>>;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type, output_type, output_type, output_type>;

  protected:
    EvalSequenceMultiTest()
      : params{std::get<std::vector<T>>(allParams)},
        range{std::size_t(1) << std::min(dpf::utils::bitlength_of_v<input_type>, std::size_t(10))-1},
        zero_output{from_integral_type_output(0)},
        points{get_points()}
    { }

    void SetUp() override
    { }

    void TearDown() override
    { }

    std::set<input_type> get_points()
    {
        std::set<input_type> ret;
        // insert all `x`'s used in tests
        for (auto [x, y0, y1, y2, y3] : params)
        {
            ret.insert(x);
        }
        // insert additional points until sufficient random points
        while (ret.size() < range)
        {
            ret.emplace(from_integral_type(dpf::uniform_sample<integral_type>()));
        }
        return ret;
    }

    template <typename IterableT>
    void assert_wrapper(const input_type & x, const output_type & y0,
        const output_type & y1, const output_type & y2, const output_type & y3,
        const IterableT & iter0, const IterableT & iter1)
    {
        auto zip0 = dpf::tuple_as_zip(iter0),
             zip1 = dpf::tuple_as_zip(iter1);
        auto it0 = std::cbegin(zip0),
             it1 = std::cbegin(zip1);
        auto cur = points.cbegin();
        for (std::size_t i = 0; i < range; ++i, ++cur, ++it0, ++it1)
        {
            if (*cur == x)
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
    std::set<input_type> points;
};

TYPED_TEST_SUITE_P(EvalSequenceMultiTest);

TYPED_TEST_P(EvalSequenceMultiTest, NoRecipeBasic)
{
    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, this->points.begin(), this->points.end());
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, this->points.begin(), this->points.end());

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, NoRecipeBasicEntireNode)
{
    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, this->points.begin(), this->points.end(), dpf::return_entire_node_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, this->points.begin(), this->points.end(), dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, NoRecipeBasicOutputOnly)
{
    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, this->points.begin(), this->points.end(), dpf::return_output_only_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, this->points.begin(), this->points.end(), dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, NoRecipeOutbuf)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto buf0 = dpf::make_output_buffer_for_subsequence<dpf_type, 0, 1, 2, 3>(this->points.begin(), this->points.end()),
         buf1 = dpf::make_output_buffer_for_subsequence<dpf_type, 0, 1, 2, 3>(this->points.begin(), this->points.end());

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, this->points.begin(), this->points.end(), buf0),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, this->points.begin(), this->points.end(), buf1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, NoRecipeOutbufEntireNode)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto buf0 = dpf::make_output_buffer_for_subsequence<dpf_type, 0, 1, 2, 3>(this->points.begin(), this->points.end()),
         buf1 = dpf::make_output_buffer_for_subsequence<dpf_type, 0, 1, 2, 3>(this->points.begin(), this->points.end());

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, this->points.begin(), this->points.end(), buf0, dpf::return_entire_node_tag_{}),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, this->points.begin(), this->points.end(), buf1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, NoRecipeOutbufOutputOnly)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto buf0 = dpf::make_output_buffer_for_subsequence<dpf_type, 0, 1, 2, 3>(this->points.begin(), this->points.end()),
         buf1 = dpf::make_output_buffer_for_subsequence<dpf_type, 0, 1, 2, 3>(this->points.begin(), this->points.end());

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, this->points.begin(), this->points.end(), buf0, dpf::return_output_only_tag_{}),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, this->points.begin(), this->points.end(), buf1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeBasic)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0);
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeBasicEntireNode)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, dpf::return_entire_node_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeBasicOutputOnly)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, dpf::return_output_only_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeOutbuf)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto buf0 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe0),
         buf1 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, buf0),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, buf1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeOutbufEntireNode)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto buf0 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe0),
         buf1 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, buf0, dpf::return_entire_node_tag_{}),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, buf1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeOutbufOutputOnly)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto buf0 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe0),
         buf1 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, buf0, dpf::return_output_only_tag_{}),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, buf1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeInplaceReversingSequenceMemoizer)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto memo0 = dpf::make_inplace_reversing_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_inplace_reversing_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, memo0);
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeInplaceReversingSequenceMemoizerEntireNode)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto memo0 = dpf::make_inplace_reversing_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_inplace_reversing_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, memo0, dpf::return_entire_node_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, memo1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeInplaceReversingSequenceMemoizerOutputOnly)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto memo0 = dpf::make_inplace_reversing_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_inplace_reversing_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, memo0, dpf::return_output_only_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, memo1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeDoubleSpaceSequenceMemoizer)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto memo0 = dpf::make_double_space_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_double_space_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, memo0);
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeDoubleSpaceSequenceMemoizerEntireNode)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto memo0 = dpf::make_double_space_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_double_space_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, memo0, dpf::return_entire_node_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, memo1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeDoubleSpaceSequenceMemoizerOutputOnly)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto memo0 = dpf::make_double_space_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_double_space_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, memo0, dpf::return_output_only_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, memo1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeFullTreeSequenceMemoizer)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto memo0 = dpf::make_full_tree_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_full_tree_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, memo0);
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeFullTreeSequenceMemoizerEntireNode)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto memo0 = dpf::make_full_tree_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_full_tree_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, memo0, dpf::return_entire_node_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, memo1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeFullTreeSequenceMemoizerOutputOnly)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto memo0 = dpf::make_full_tree_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_full_tree_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto [buf0, iter0] = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, memo0, dpf::return_output_only_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, memo1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeInplaceReversingSequenceMemoizerOutbuf)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto buf0 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe0),
         buf1 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe1);
    auto memo0 = dpf::make_inplace_reversing_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_inplace_reversing_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, buf0, memo0),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, buf1, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeInplaceReversingSequenceMemoizerOutbufEntireNode)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto buf0 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe0),
         buf1 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe1);
    auto memo0 = dpf::make_inplace_reversing_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_inplace_reversing_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, buf0, memo0, dpf::return_entire_node_tag_{}),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, buf1, memo1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeInplaceReversingSequenceMemoizerOutbufOutputOnly)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto buf0 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe0),
         buf1 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe1);
    auto memo0 = dpf::make_inplace_reversing_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_inplace_reversing_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, buf0, memo0, dpf::return_output_only_tag_{}),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, buf1, memo1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeDoubleSpaceSequenceMemoizerOutbuf)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto buf0 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe0),
         buf1 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe1);
    auto memo0 = dpf::make_double_space_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_double_space_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, buf0, memo0),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, buf1, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeDoubleSpaceSequenceMemoizerOutbufEntireNode)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto buf0 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe0),
         buf1 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe1);
    auto memo0 = dpf::make_double_space_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_double_space_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, buf0, memo0, dpf::return_entire_node_tag_{}),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, buf1, memo1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeDoubleSpaceSequenceMemoizerOutbufOutputOnly)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto buf0 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe0),
         buf1 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe1);
    auto memo0 = dpf::make_double_space_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_double_space_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, buf0, memo0, dpf::return_output_only_tag_{}),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, buf1, memo1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeFullTreeSequenceMemoizerOutbuf)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto buf0 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe0),
         buf1 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe1);
    auto memo0 = dpf::make_full_tree_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_full_tree_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, buf0, memo0),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, buf1, memo1);

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeFullTreeSequenceMemoizerOutbufEntireNode)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto buf0 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe0),
         buf1 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe1);
    auto memo0 = dpf::make_full_tree_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_full_tree_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, buf0, memo0, dpf::return_entire_node_tag_{}),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, buf1, memo1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceMultiTest, RecipeFullTreeSequenceMemoizerOutbufOutputOnly)
{
    using dpf_type = typename TestFixture::dpf_type;
    auto recipe0 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end()),
         recipe1 = dpf::make_sequence_recipe<dpf_type>(this->points.begin(), this->points.end());
    auto buf0 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe0),
         buf1 = dpf::make_output_buffer_for_recipe_subsequence<dpf_type, 0, 1, 2, 3>(recipe1);
    auto memo0 = dpf::make_full_tree_sequence_memoizer<dpf_type>(recipe0),
         memo1 = dpf::make_full_tree_sequence_memoizer<dpf_type>(recipe1);

    for (auto [x, y0, y1, y2, y3] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y0, y1, y2, y3);
        auto iter0 = dpf::eval_sequence<0, 1, 2, 3>(dpf0, recipe0, buf0, memo0, dpf::return_output_only_tag_{}),
             iter1 = dpf::eval_sequence<0, 1, 2, 3>(dpf1, recipe1, buf1, memo1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y0, y1, y2, y3, iter0, iter1);
    }
}

REGISTER_TYPED_TEST_SUITE_P(EvalSequenceMultiTest,
    NoRecipeBasic,
    NoRecipeBasicEntireNode,
    NoRecipeBasicOutputOnly,
    NoRecipeOutbuf,
    NoRecipeOutbufEntireNode,
    NoRecipeOutbufOutputOnly,
    RecipeBasic,
    RecipeBasicEntireNode,
    RecipeBasicOutputOnly,
    RecipeOutbuf,
    RecipeOutbufEntireNode,
    RecipeOutbufOutputOnly,
    RecipeInplaceReversingSequenceMemoizer,
    RecipeInplaceReversingSequenceMemoizerEntireNode,
    RecipeInplaceReversingSequenceMemoizerOutputOnly,
    RecipeDoubleSpaceSequenceMemoizer,
    RecipeDoubleSpaceSequenceMemoizerEntireNode,
    RecipeDoubleSpaceSequenceMemoizerOutputOnly,
    RecipeFullTreeSequenceMemoizer,
    RecipeFullTreeSequenceMemoizerEntireNode,
    RecipeFullTreeSequenceMemoizerOutputOnly,
    RecipeInplaceReversingSequenceMemoizerOutbuf,
    RecipeInplaceReversingSequenceMemoizerOutbufEntireNode,
    RecipeInplaceReversingSequenceMemoizerOutbufOutputOnly,
    RecipeDoubleSpaceSequenceMemoizerOutbuf,
    RecipeDoubleSpaceSequenceMemoizerOutbufEntireNode,
    RecipeDoubleSpaceSequenceMemoizerOutbufOutputOnly,
    RecipeFullTreeSequenceMemoizerOutbuf,
    RecipeFullTreeSequenceMemoizerOutbufEntireNode,
    RecipeFullTreeSequenceMemoizerOutbufOutputOnly);
using Types = testing::Types
<
    // base test
    test_type<uint16_t, uint64_t>,

    // test input types
    test_type<uint8_t, uint64_t>,
    test_type<uint64_t, uint64_t>,
    test_type<simde_uint128, uint64_t>,
    test_type<dpf::bitstring<10>, uint64_t>,
    test_type<dpf::keyword<3, dpf::alphabets::hex>, uint64_t>,
    test_type<dpf::modint<10>, uint64_t>,
    test_type<dpf::xor_wrapper<uint16_t>, uint64_t>,

    // test output types
    test_type<uint16_t, uint8_t>,
    test_type<uint16_t, simde_uint128>,
    test_type<uint16_t, dpf::bit>,
    // test_type<uint16_t, dpf::bitstring<10>>,
    test_type<uint16_t, dpf::xor_wrapper<uint64_t>>,

    // custom types
    test_type<uint16_t, output_type_large>
>;
INSTANTIATE_TYPED_TEST_SUITE_P(EvalSequenceMultiTestInstantiation, EvalSequenceMultiTest, Types);
