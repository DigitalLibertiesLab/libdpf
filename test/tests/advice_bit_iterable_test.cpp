#include <gtest/gtest.h>

#include "dpf.hpp"

TEST(AdviceBitIterableTest, BasicUsage)
{
    using input_type = uint16_t;
    using output_type = dpf::bit;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type>;
    auto memo0 = dpf::make_basic_full_memoizer<dpf_type>(),
         memo1 = dpf::make_basic_full_memoizer<dpf_type>();
    input_type x = 0xAAAA;
    output_type y = dpf::bit::one;
    auto [dpf0, dpf1] = dpf::make_dpf(x, y);
    auto [buf0, iter0] = dpf::eval_full(dpf0, memo0);
    auto [buf1, iter1] = dpf::eval_full(dpf1, memo1);

    auto advice0 = dpf::advice_bits_of(memo0),
         advice1 = dpf::advice_bits_of(memo1);

    auto it0 = std::begin(advice0), it1 = std::begin(advice1);
    for (std::size_t i = 0; i < std::size_t(1)<<dpf_type::depth; ++i, ++it0, ++it1)
    {
        if (i == x/dpf_type::outputs_per_leaf)
        {
            ASSERT_NE(*it0, *it1);
        }
        else
        {
            ASSERT_EQ(*it0, *it1);
        }
    }
    ASSERT_EQ(it0, std::end(advice0));
    ASSERT_EQ(it1, std::end(advice1));
}
