#include <gtest/gtest.h>

#include "asio.hpp"
#include "dpf.hpp"

TEST(SetbitIndexIterableTest, BasicUsage)
{
    using input_type = uint16_t;
    using output_type = dpf::bit;
    input_type x = 0xAAAA;
    output_type y = dpf::bit::one;
    auto [dpf0, dpf1] = dpf::make_dpf(x, y);
    auto [buf0, iter0] = dpf::eval_full(dpf0);
    auto [buf1, iter1] = dpf::eval_full(dpf1);

    auto setbit0 = dpf::indices_set_in(buf0),
         setbit1 = dpf::indices_set_in(buf1);

    auto it0 = std::begin(setbit0), it0end = std::end(setbit0),
         it1 = std::begin(setbit1), it1end = std::end(setbit1);
    for (; it0 != it0end && it1 != it1end; ++it0, ++it1)
    {
        if (*it0 == x)
        {
            ASSERT_EQ(*(++it0), *it1);
        }
        else if (*it1 == x)
        {
            ASSERT_EQ(*it0, *(++it1));
        }
        else
        {
            ASSERT_EQ(*it0, *it1);
        }
    }
    ASSERT_EQ(it0, it0end);
    ASSERT_EQ(it1, it1end);
}
