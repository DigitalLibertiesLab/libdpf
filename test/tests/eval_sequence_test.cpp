#include <gtest/gtest.h>

#include <set>

#include "asio.hpp"
#include "dpf.hpp"

#include "helpers/eval_common_data.hpp"

template <typename T>
struct EvalSequenceTest : public testing::Test
{
  public:
    using input_type = typename std::tuple_element_t<0, T>;
    using output_type = typename std::tuple_element_t<1, T>;
    using integral_type = dpf::utils::integral_type_from_bitlength_t<dpf::utils::bitlength_of_v<input_type>>;

  protected:
    EvalSequenceTest()
      : params{std::get<std::vector<T>>(allParams)},
        range{std::size_t(1) << std::min(dpf::utils::bitlength_of_v<input_type>, std::size_t(10))-1},
        zero_output(output_type(0)),
        points(get_points())
    { }

    void SetUp() override
    { }

    void TearDown() override
    { }

    std::set<input_type> get_points()
    {
        std::set<input_type> ret;
        // insert all `x`'s used in tests
        for (auto [x, y] : params)
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
    void assert_wrapper(const input_type & x, const output_type & y,
        const IterableT & iter0, const IterableT & iter1)
    {
        auto it0 = std::begin(iter0),
             it1 = std::begin(iter1);
        auto cur = points.cbegin();
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
        ASSERT_EQ(it0, std::end(iter0));
        ASSERT_EQ(it1, std::end(iter1));
    }

    static constexpr auto from_integral_type = dpf::utils::make_from_integral_value<input_type>{};

    std::vector<T> params;
    std::size_t range;
    output_type zero_output;
    std::set<input_type> points;
};

TYPED_TEST_SUITE_P(EvalSequenceTest);

TYPED_TEST_P(EvalSequenceTest, NoRecipeBasic)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, this->points.begin(), this->points.end());
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, this->points.begin(), this->points.end());

        this->assert_wrapper(x, y, iter0, iter1);
    }
}


TYPED_TEST_P(EvalSequenceTest, NoRecipeBasicEntireNode)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, this->points.begin(), this->points.end(), dpf::return_entire_node_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, this->points.begin(), this->points.end(), dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}


TYPED_TEST_P(EvalSequenceTest, NoRecipeBasicOutputOnly)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, this->points.begin(), this->points.end(), dpf::return_output_only_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, this->points.begin(), this->points.end(), dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, NoRecipeOutbuf)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto buf0 = dpf::make_output_buffer_for_subsequence(dpf0, this->points.begin(), this->points.end()),
             buf1 = dpf::make_output_buffer_for_subsequence(dpf1, this->points.begin(), this->points.end());
        auto iter0 = dpf::eval_sequence(dpf0, this->points.begin(), this->points.end(), buf0),
             iter1 = dpf::eval_sequence(dpf1, this->points.begin(), this->points.end(), buf1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, NoRecipeOutbufEntireNode)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto buf0 = dpf::make_output_buffer_for_subsequence(dpf0, this->points.begin(), this->points.end()),
             buf1 = dpf::make_output_buffer_for_subsequence(dpf1, this->points.begin(), this->points.end());
        auto iter0 = dpf::eval_sequence(dpf0, this->points.begin(), this->points.end(), buf0, dpf::return_entire_node_tag_{}),
             iter1 = dpf::eval_sequence(dpf1, this->points.begin(), this->points.end(), buf1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, NoRecipeOutbufOutputOnly)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto buf0 = dpf::make_output_buffer_for_subsequence(dpf0, this->points.begin(), this->points.end()),
             buf1 = dpf::make_output_buffer_for_subsequence(dpf1, this->points.begin(), this->points.end());
        auto iter0 = dpf::eval_sequence(dpf0, this->points.begin(), this->points.end(), buf0, dpf::return_output_only_tag_{}),
             iter1 = dpf::eval_sequence(dpf1, this->points.begin(), this->points.end(), buf1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, BreadthFirstBasic)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto [buf0, iter0] = dpf::eval_sequence_breadth_first(dpf0, this->points.begin(), this->points.end());
        auto [buf1, iter1] = dpf::eval_sequence_breadth_first(dpf1, this->points.begin(), this->points.end());

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, BreadthFirstOutbuf)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto buf0 = dpf::make_output_buffer_for_subsequence(dpf0, this->points.begin(), this->points.end()),
             buf1 = dpf::make_output_buffer_for_subsequence(dpf1, this->points.begin(), this->points.end());
        auto iter0 = dpf::eval_sequence_breadth_first(dpf0, this->points.begin(), this->points.end(), buf0),
             iter1 = dpf::eval_sequence_breadth_first(dpf1, this->points.begin(), this->points.end(), buf1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeBasic)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, recipe0);
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, recipe1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeBasicEntireNode)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, recipe0, dpf::return_entire_node_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, recipe1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeBasicOutputOnly)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, recipe0, dpf::return_output_only_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, recipe1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeOutbuf)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeOutbufEntireNode)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, dpf::return_entire_node_tag_{}),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeOutbufOutputOnly)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, dpf::return_output_only_tag_{}),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeInplaceReversingSequenceMemoizer)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto memo0 = dpf::make_inplace_reversing_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_inplace_reversing_sequence_memoizer(dpf1, recipe1);
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, recipe0, memo0);
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, recipe1, memo1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeInplaceReversingSequenceMemoizerEntireNode)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto memo0 = dpf::make_inplace_reversing_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_inplace_reversing_sequence_memoizer(dpf1, recipe1);
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, recipe0, memo0, dpf::return_entire_node_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, recipe1, memo1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeInplaceReversingSequenceMemoizerOutputOnly)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto memo0 = dpf::make_inplace_reversing_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_inplace_reversing_sequence_memoizer(dpf1, recipe1);
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, recipe0, memo0, dpf::return_output_only_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, recipe1, memo1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeDoubleSpaceSequenceMemoizer)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto memo0 = dpf::make_double_space_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_double_space_sequence_memoizer(dpf1, recipe1);
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, recipe0, memo0);
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, recipe1, memo1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeDoubleSpaceSequenceMemoizerEntireNode)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto memo0 = dpf::make_double_space_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_double_space_sequence_memoizer(dpf1, recipe1);
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, recipe0, memo0, dpf::return_entire_node_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, recipe1, memo1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeDoubleSpaceSequenceMemoizerOutputOnly)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto memo0 = dpf::make_double_space_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_double_space_sequence_memoizer(dpf1, recipe1);
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, recipe0, memo0, dpf::return_output_only_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, recipe1, memo1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeFullTreeSequenceMemoizer)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto memo0 = dpf::make_full_tree_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_full_tree_sequence_memoizer(dpf1, recipe1);
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, recipe0, memo0);
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, recipe1, memo1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeFullTreeSequenceMemoizerEntireNode)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto memo0 = dpf::make_full_tree_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_full_tree_sequence_memoizer(dpf1, recipe1);
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, recipe0, memo0, dpf::return_entire_node_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, recipe1, memo1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeFullTreeSequenceMemoizerOutputOnly)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto memo0 = dpf::make_full_tree_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_full_tree_sequence_memoizer(dpf1, recipe1);
        auto [buf0, iter0] = dpf::eval_sequence(dpf0, recipe0, memo0, dpf::return_output_only_tag_{});
        auto [buf1, iter1] = dpf::eval_sequence(dpf1, recipe1, memo1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeInplaceReversingSequenceMemoizerOutbuf)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto memo0 = dpf::make_inplace_reversing_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_inplace_reversing_sequence_memoizer(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, memo0),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, memo1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeInplaceReversingSequenceMemoizerOutbufEntireNode)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto memo0 = dpf::make_inplace_reversing_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_inplace_reversing_sequence_memoizer(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, memo0, dpf::return_entire_node_tag_{}),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, memo1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeInplaceReversingSequenceMemoizerOutbufOutputOnly)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto memo0 = dpf::make_inplace_reversing_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_inplace_reversing_sequence_memoizer(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, memo0, dpf::return_output_only_tag_{}),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, memo1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeDoubleSpaceSequenceMemoizerOutbuf)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto memo0 = dpf::make_double_space_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_double_space_sequence_memoizer(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, memo0),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, memo1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeDoubleSpaceSequenceMemoizerOutbufEntireNode)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto memo0 = dpf::make_double_space_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_double_space_sequence_memoizer(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, memo0, dpf::return_entire_node_tag_{}),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, memo1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeDoubleSpaceSequenceMemoizerOutbufOutputOnly)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto memo0 = dpf::make_double_space_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_double_space_sequence_memoizer(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, memo0, dpf::return_output_only_tag_{}),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, memo1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeFullTreeSequenceMemoizerOutbuf)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto memo0 = dpf::make_full_tree_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_full_tree_sequence_memoizer(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, memo0),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, memo1);

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeFullTreeSequenceMemoizerOutbufEntireNode)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto memo0 = dpf::make_full_tree_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_full_tree_sequence_memoizer(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, memo0, dpf::return_entire_node_tag_{}),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, memo1, dpf::return_entire_node_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

TYPED_TEST_P(EvalSequenceTest, RecipeFullTreeSequenceMemoizerOutbufOutputOnly)
{
    for (auto [x, y] : this->params)
    {
        auto [dpf0, dpf1] = dpf::make_dpf(x, y);
        auto recipe0 = dpf::make_sequence_recipe(dpf0, this->points.begin(), this->points.end()),
             recipe1 = dpf::make_sequence_recipe(dpf1, this->points.begin(), this->points.end());
        auto buf0 = dpf::make_output_buffer_for_recipe_subsequence(dpf0, recipe0),
             buf1 = dpf::make_output_buffer_for_recipe_subsequence(dpf1, recipe1);
        auto memo0 = dpf::make_full_tree_sequence_memoizer(dpf0, recipe0),
             memo1 = dpf::make_full_tree_sequence_memoizer(dpf1, recipe1);
        auto iter0 = dpf::eval_sequence(dpf0, recipe0, buf0, memo0, dpf::return_output_only_tag_{}),
             iter1 = dpf::eval_sequence(dpf1, recipe1, buf1, memo1, dpf::return_output_only_tag_{});

        this->assert_wrapper(x, y, iter0, iter1);
    }
}

REGISTER_TYPED_TEST_SUITE_P(EvalSequenceTest,
    NoRecipeBasic,
    NoRecipeBasicEntireNode,
    NoRecipeBasicOutputOnly,
    NoRecipeOutbuf,
    NoRecipeOutbufEntireNode,
    NoRecipeOutbufOutputOnly,
    BreadthFirstBasic,
    BreadthFirstOutbuf,
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
