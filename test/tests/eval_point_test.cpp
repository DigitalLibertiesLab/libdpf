#include <gtest/gtest.h>

#include "dpf.hpp"

TEST(EvalPointTest, EvalPoint00) {
    uint8_t x = 0x00;
    uint32_t y = 0xAAAAAAAA;
    auto [dpf0, dpf1] = dpf::make_dpf(x, y);

    for (int i = 0; i < (1 << 7); ++i) {
        auto curi = i;
        uint32_t y0 = dpf::eval_point(dpf0, i),
                 y1 = dpf::eval_point(dpf1, i);
        if (curi == x) {
            EXPECT_EQ(y1 - y0, y);
        } else {
            EXPECT_EQ(y1 - y0, 0) << "Error occured at index " << i;
        }
    }
}

TEST(EvalPointTest, EvalPointFF) {
    uint8_t x = 0xFF;
    uint32_t y = 0xAAAAAAAA;
    auto [dpf0, dpf1] = dpf::make_dpf(x, y);

    for (size_t i = 0; i < (1 << 7); ++i) {
        uint8_t curi = static_cast<uint8_t>(i);
        uint32_t y0 = dpf::eval_point(dpf0, curi),
                 y1 = dpf::eval_point(dpf1, curi);
        if (curi == x) {
            EXPECT_EQ(y1 - y0, y);
        } else {
            EXPECT_EQ(y1 - y0, 0) << "Error occured at index " << i;
        }
    }
}

// test multiple exterior nodes
// test modint?
