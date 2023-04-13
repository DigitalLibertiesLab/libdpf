#include <gtest/gtest.h>

#include "dpf.hpp"

simde__m128i fake_root_sampler()
{
    static int64_t ret_int = 0x4;
    simde__m128i ret = {ret_int, 0};
    ret_int <<= 1;
    return ret;
}

TEST(DpfKeyTest, SimpleGen)
{
    uint8_t x = 0xAA;  // = 0b 1010 1010
    uint32_t y0 = 0xAAAAAAAA;  // additive / subtractive share
    dpf::xor_wrapper<uint32_t> y1 = dpf::xor_wrapper<uint32_t>(uint32_t(0x55555555));  // xor share
    dpf::wildcard_value<uint32_t> y2 = dpf::wildcard_value<uint32_t>();  // wildcard
    auto [dpf0, dpf1] = dpf::make_dpf<dpf::prg::aes128, dpf::prg::aes128, &fake_root_sampler>(x, y0, y1, y2);

    // 128-bit representation of 0x4 with lowest bit unset
    EXPECT_EQ(dpf0.root[1], 0);
    EXPECT_EQ(dpf0.root[0], 0x4);
    // 128-bit representation of 0x8 with lowest bit set
    EXPECT_EQ(dpf1.root[1], 0);
    EXPECT_EQ(dpf1.root[0], 0x9);

    EXPECT_EQ(dpf0.correction_words[0][1], 0x7ff85a65ce2111c9);
    EXPECT_EQ(dpf0.correction_words[0][0], 0x36863b84ab3944d2);
    EXPECT_EQ(dpf0.correction_words[0][1], dpf1.correction_words[0][1]);
    EXPECT_EQ(dpf0.correction_words[0][0], dpf1.correction_words[0][0]);
    EXPECT_EQ(dpf0.correction_advice[0], 0b00);
    EXPECT_EQ(dpf0.correction_advice[0], dpf1.correction_advice[0]);

    // dpf0 after level 0:
    // 0xc4c4bd72d02958c541201f063e3c1173
    // dpf1 after level 0:
    // 0xdd09c23385ba379378631a3a9c46f52e

    EXPECT_EQ(dpf0.correction_words[1][1], 0x9ca0f55370cf6bfe);
    EXPECT_EQ(dpf0.correction_words[1][0], 0xc3b9e951c500d272);
    EXPECT_EQ(dpf0.correction_words[1][1], dpf1.correction_words[1][1]);
    EXPECT_EQ(dpf0.correction_words[1][0], dpf1.correction_words[1][0]);
    EXPECT_EQ(dpf0.correction_advice[1], 0b01);
    EXPECT_EQ(dpf0.correction_advice[1], dpf1.correction_advice[1]);

    // dpf0 after level 1:
    // 0x2bef771157872382accfcf2a5e2f7e57
    // dpf1 after level 1:
    // 0x7604b860b26e8586b0c6ad05ec6886ce

    EXPECT_EQ(dpf0.correction_words[2][1], 0x886f1eb652b72eda);
    EXPECT_EQ(dpf0.correction_words[2][0], 0x0ff98303eca43ab6);
    EXPECT_EQ(dpf0.correction_words[2][1], dpf1.correction_words[2][1]);
    EXPECT_EQ(dpf0.correction_words[2][0], dpf1.correction_words[2][0]);
    EXPECT_EQ(dpf0.correction_advice[2], 0b10);
    EXPECT_EQ(dpf0.correction_advice[2], dpf1.correction_advice[2]);

    // dpf0 after level 2:
    // 0x39adfa95d94a10fdff65a956019f0a6c
    // dpf1 after level 2:
    // 0x59be9dba7aa04f9a12d23cd995d90135

    EXPECT_EQ(dpf0.correction_words[3][1], 0x4e69100f5b844cb9);
    EXPECT_EQ(dpf0.correction_words[3][0], 0x9ac5b5baba9a193b);
    EXPECT_EQ(dpf0.correction_words[3][1], dpf1.correction_words[3][1]);
    EXPECT_EQ(dpf0.correction_words[3][0], dpf1.correction_words[3][0]);
    EXPECT_EQ(dpf0.correction_advice[3], 0b10);
    EXPECT_EQ(dpf0.correction_advice[3], dpf1.correction_advice[3]);

    // dpf0 after level 3:
    // 0x028922e3e5fca1a824a12136fc2ed7e3
    // dpf1 after level 3:
    // 0xd7699bb72bb9e8d42363e899692ecf36

    EXPECT_EQ(dpf0.correction_words[4][1], 0xe701887629e08652);
    EXPECT_EQ(dpf0.correction_words[4][0], 0xbd92c2853e1e2457);
    EXPECT_EQ(dpf0.correction_words[4][1], dpf1.correction_words[4][1]);
    EXPECT_EQ(dpf0.correction_words[4][0], dpf1.correction_words[4][0]);
    EXPECT_EQ(dpf0.correction_advice[4], 0b01);
    EXPECT_EQ(dpf0.correction_advice[4], dpf1.correction_advice[4]);

    // dpf0 after level 4:
    // 0xe0deacc7c5f61d83aebacde0bd97f61f
    // dpf1 after level 4:
    // 0x96be3cfb09b9bc84e0a6de756d9589f2

    EXPECT_EQ(dpf0.correction_words[5][1], 0xc8edc84047a7b3df);
    EXPECT_EQ(dpf0.correction_words[5][0], 0xbc0d1f614b01d608);
    EXPECT_EQ(dpf0.correction_words[5][1], dpf1.correction_words[5][1]);
    EXPECT_EQ(dpf0.correction_words[5][0], dpf1.correction_words[5][0]);
    EXPECT_EQ(dpf0.correction_advice[5], 0b01);
    EXPECT_EQ(dpf0.correction_advice[5], dpf1.correction_advice[5]);

    // dpf0 after level 5:
    // 0x3cb3c5060d58e866c703b4b7939725b8
    // dpf1 after level 5:
    // 0x1afcd5c2a2a3f4b9be5b9564585df4f3

    // Leaf layer

    // dpf0 make leaf mask inner:
    // 0: bb994bbd eba3cbb2 39b39032 e5f31930
    // 1: d32db0c1 3da76455 961fadd7 4b5d7350
    // 2: 5de4be73 fd14043f 19b22bba be0ff8f8
    // dpf1 make leaf mask inner:
    // 0: 921bb1c5 b0a6c8c2 484ae275 9a752740
    // 1: 279a2459 0d9d913f f1bf8700 fc603f6a
    // 2: cb839afd 6a68b9cf b0c6aac6 7dd6f9ad
    // naked masks:
    // 0: 00000000 aaaaaaaa 00000000 00000000
    // 1: 00000000 55555555 00000000 00000000
    // 2: 00000000 00000000 00000000 00000000
    // correction words:
    // 0: d6826608 1a585266 0e975243 b4820e10
    // 1: 546c7398 7aa0d795 5b9fd929 b102cc1a
    // 2: 6d9edc8a 6d54b590 97147f0c bfc700b5

    EXPECT_EQ(dpf0.leaf<0>()[1], 0xd68266081a585266);
    EXPECT_EQ(dpf0.leaf<0>()[0], 0x0e975243b4820e10);
    EXPECT_EQ(dpf0.leaf<0>()[1], dpf1.leaf<0>()[1]);
    EXPECT_EQ(dpf0.leaf<0>()[0], dpf1.leaf<0>()[0]);

    EXPECT_EQ(dpf0.leaf<1>()[1], 0xf4b79498656fa03f);
    EXPECT_EQ(dpf0.leaf<1>()[0], 0x67a02ad7b73d4c3a);
    EXPECT_EQ(dpf0.leaf<1>()[1], dpf1.leaf<1>()[1]);
    EXPECT_EQ(dpf0.leaf<1>()[0], dpf1.leaf<1>()[0]);

    simde__m128i wildcard = simde_mm_add_epi32(dpf0.leaf<2>(), dpf1.leaf<2>());
    EXPECT_EQ(wildcard[1], 0x6d9edc8a6d54b590);
    EXPECT_EQ(wildcard[0], 0x97147f0cbfc700b5);

    EXPECT_EQ(dpf0.is_wildcard(0), false);
    EXPECT_EQ(dpf0.is_wildcard(1), false);
    EXPECT_EQ(dpf0.is_wildcard(2), true);
    EXPECT_EQ(dpf1.is_wildcard(0), false);
    EXPECT_EQ(dpf1.is_wildcard(1), false);
    EXPECT_EQ(dpf1.is_wildcard(2), true);
}
