#include <gtest/gtest.h>

#include "dpf.hpp"

template <typename T>
static T fake_root_sampler()
{
    static T ret;
    return ++ret;
}

template <>
simde__m128i fake_root_sampler()
{
    static int64_t ret_int = 0x4;
    simde__m128i ret = {ret_int, 0};
    ret_int <<= 1;
    return ret;
}

TEST(DpfKeyTest, SimpleGen) {
    uint8_t x = 0xAA;  // = 0b 1010 1010
    uint32_t y = 0xAAAAAAAA;
    auto [dpf0, dpf1] = dpf::make_dpf<dpf::prg::aes128, dpf::prg::aes128, &fake_root_sampler>(x, y);

    // 128-bit representation of 0x4 with lowest bit unset
    EXPECT_EQ(dpf0.root[1], 0);
    EXPECT_EQ(dpf0.root[0], 0x4);
    // 128-bit representation of 0x8 with lowest bit set
    EXPECT_EQ(dpf1.root[1], 0);
    EXPECT_EQ(dpf1.root[0], 0x9);

    EXPECT_EQ(dpf0.interior_cws[0][1], 0x7ff85a65ce2111c9);
    EXPECT_EQ(dpf0.interior_cws[0][0], 0x36863b84ab3944d2);
    EXPECT_EQ(dpf0.interior_cws[0][1], dpf1.interior_cws[0][1]);
    EXPECT_EQ(dpf0.interior_cws[0][0], dpf1.interior_cws[0][0]);
    EXPECT_EQ(dpf0.correction_advice[0], 0b00);
    EXPECT_EQ(dpf0.correction_advice[0], dpf1.correction_advice[0]);

    // dpf0 after level 0:
    // 0xc4c4bd72d02958c541201f063e3c1173
    // dpf1 after level 0:
    // 0xdd09c23385ba379378631a3a9c46f52e

    EXPECT_EQ(dpf0.interior_cws[1][1], 0x9ca0f55370cf6bfe);
    EXPECT_EQ(dpf0.interior_cws[1][0], 0xc3b9e951c500d272);
    EXPECT_EQ(dpf0.interior_cws[1][1], dpf1.interior_cws[1][1]);
    EXPECT_EQ(dpf0.interior_cws[1][0], dpf1.interior_cws[1][0]);
    EXPECT_EQ(dpf0.correction_advice[1], 0b01);
    EXPECT_EQ(dpf0.correction_advice[1], dpf1.correction_advice[1]);

    // dpf0 after level 1:
    // 0x2bef771157872382accfcf2a5e2f7e57
    // dpf1 after level 1:
    // 0x7604b860b26e8586b0c6ad05ec6886ce

    EXPECT_EQ(dpf0.interior_cws[2][1], 0x886f1eb652b72eda);
    EXPECT_EQ(dpf0.interior_cws[2][0], 0x0ff98303eca43ab6);
    EXPECT_EQ(dpf0.interior_cws[2][1], dpf1.interior_cws[2][1]);
    EXPECT_EQ(dpf0.interior_cws[2][0], dpf1.interior_cws[2][0]);
    EXPECT_EQ(dpf0.correction_advice[2], 0b10);
    EXPECT_EQ(dpf0.correction_advice[2], dpf1.correction_advice[2]);


    // dpf0 after level 2:
    // 0x39adfa95d94a10fdff65a956019f0a6c
    // dpf1 after level 2:
    // 0x59be9dba7aa04f9a12d23cd995d90135

    EXPECT_EQ(dpf0.interior_cws[3][1], 0x4e69100f5b844cb9);
    EXPECT_EQ(dpf0.interior_cws[3][0], 0x9ac5b5baba9a193b);
    EXPECT_EQ(dpf0.interior_cws[3][1], dpf1.interior_cws[3][1]);
    EXPECT_EQ(dpf0.interior_cws[3][0], dpf1.interior_cws[3][0]);
    EXPECT_EQ(dpf0.correction_advice[3], 0b10);
    EXPECT_EQ(dpf0.correction_advice[3], dpf1.correction_advice[3]);

    // dpf0 after level 3:
    // 0x028922e3e5fca1a824a12136fc2ed7e3
    // dpf1 after level 3:
    // 0xd7699bb72bb9e8d42363e899692ecf36

    EXPECT_EQ(dpf0.interior_cws[4][1], 0xe701887629e08652);
    EXPECT_EQ(dpf0.interior_cws[4][0], 0xbd92c2853e1e2457);
    EXPECT_EQ(dpf0.interior_cws[4][1], dpf1.interior_cws[4][1]);
    EXPECT_EQ(dpf0.interior_cws[4][0], dpf1.interior_cws[4][0]);
    EXPECT_EQ(dpf0.correction_advice[4], 0b01);
    EXPECT_EQ(dpf0.correction_advice[4], dpf1.correction_advice[4]);

    // dpf0 after level 4:
    // 0xe0deacc7c5f61d83aebacde0bd97f61f
    // dpf1 after level 4:
    // 0x96be3cfb09b9bc84e0a6de756d9589f2

    EXPECT_EQ(dpf0.interior_cws[5][1], 0xc8edc84047a7b3df);
    EXPECT_EQ(dpf0.interior_cws[5][0], 0xbc0d1f614b01d608);
    EXPECT_EQ(dpf0.interior_cws[5][1], dpf1.interior_cws[5][1]);
    EXPECT_EQ(dpf0.interior_cws[5][0], dpf1.interior_cws[5][0]);
    EXPECT_EQ(dpf0.correction_advice[5], 0b01);
    EXPECT_EQ(dpf0.correction_advice[5], dpf1.correction_advice[5]);

    // dpf0 after level 5:
    // 0x3cb3c5060d58e866c703b4b7939725b8
    // dpf1 after level 5:
    // 0x1afcd5c2a2a3f4b9be5b9564585df4f3

    // TODO: Check exterior nodes and leaves
}
