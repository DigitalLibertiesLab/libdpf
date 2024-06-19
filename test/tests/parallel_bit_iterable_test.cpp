#include <gtest/gtest.h>

#include <array>

#include "dpf.hpp"

TEST(ParallelBitIterableTest, BasicUsageBatchSize02)
{
    constexpr std::size_t test_size = 2;
    using input_type = uint16_t;
    using output_type = dpf::bit;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type>;
    using element_type = typename dpf::parallel_const_bit_iterator<test_size, dpf::dynamic_bit_array<>>::value_type::value_type;
    const std::size_t bitlength_of_element = dpf::utils::bitlength_of_v<element_type>;
    auto memo0 = dpf::make_basic_full_memoizer<dpf_type>(),
         memo1 = dpf::make_basic_full_memoizer<dpf_type>();
    output_type y = dpf::bit::one;
    constexpr std::array<input_type, test_size> x{0x5555, 0xAAAA};
    auto [dpf00, dpf01] = dpf::make_dpf(x[0], y);
    auto [dpf10, dpf11] = dpf::make_dpf(x[1], y);
    auto [buf00, iter00] = dpf::eval_full(dpf00, memo0);
    auto [buf01, iter01] = dpf::eval_full(dpf01, memo1);
    auto [buf10, iter10] = dpf::eval_full(dpf10, memo0);
    auto [buf11, iter11] = dpf::eval_full(dpf11, memo1);

    auto parallel0 = dpf::batch_of(buf00, buf10),
         parallel1 = dpf::batch_of(buf01, buf11);

    auto it0 = std::begin(parallel0), it1 = std::begin(parallel1);
    for (std::size_t i = 0, idx = 0; i < std::size_t(1)<<dpf::utils::bitlength_of_v<input_type>; ++i, ++it0, ++it1)
    {
        auto tmp = *it0;
        if (idx < test_size && i == x[idx])
        {
            tmp[idx] ^= element_type(1) << x[idx]%bitlength_of_element;
            ++idx;
        }
        ASSERT_EQ(tmp, *it1);
    }
    ASSERT_EQ(it0, std::end(parallel0));
    ASSERT_EQ(it1, std::end(parallel1));
}

TEST(ParallelBitIterableTest, BasicUsageBatchSize04)
{
    constexpr std::size_t test_size = 4;
    using input_type = uint16_t;
    using output_type = dpf::bit;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type>;
    using element_type = typename dpf::parallel_const_bit_iterator<test_size, dpf::dynamic_bit_array<>>::value_type::value_type;
    const std::size_t bitlength_of_element = dpf::utils::bitlength_of_v<element_type>;
    auto memo0 = dpf::make_basic_full_memoizer<dpf_type>(),
         memo1 = dpf::make_basic_full_memoizer<dpf_type>();
    output_type y = dpf::bit::one;
    constexpr std::array<input_type, test_size> x{0x0000, 0x5555, 0xAAAA, 0xFFFF};
    auto [dpf00, dpf01] = dpf::make_dpf(x[0], y);
    auto [dpf10, dpf11] = dpf::make_dpf(x[1], y);
    auto [dpf20, dpf21] = dpf::make_dpf(x[2], y);
    auto [dpf30, dpf31] = dpf::make_dpf(x[3], y);
    auto [buf00, iter00] = dpf::eval_full(dpf00, memo0);
    auto [buf01, iter01] = dpf::eval_full(dpf01, memo1);
    auto [buf10, iter10] = dpf::eval_full(dpf10, memo0);
    auto [buf11, iter11] = dpf::eval_full(dpf11, memo1);
    auto [buf20, iter20] = dpf::eval_full(dpf20, memo0);
    auto [buf21, iter21] = dpf::eval_full(dpf21, memo1);
    auto [buf30, iter30] = dpf::eval_full(dpf30, memo0);
    auto [buf31, iter31] = dpf::eval_full(dpf31, memo1);

    auto parallel0 = dpf::batch_of(buf00, buf10, buf20, buf30),
         parallel1 = dpf::batch_of(buf01, buf11, buf21, buf31);

    auto it0 = std::begin(parallel0), it1 = std::begin(parallel1);
    for (std::size_t i = 0, idx = 0; i < std::size_t(1)<<dpf::utils::bitlength_of_v<input_type>; ++i, ++it0, ++it1)
    {
        auto tmp = *it0;
        if (idx < test_size && i == x[idx])
        {
            tmp[idx] ^= element_type(1) << x[idx]%bitlength_of_element;
            ++idx;
        }
        ASSERT_EQ(tmp, *it1);
    }
    ASSERT_EQ(it0, std::end(parallel0));
    ASSERT_EQ(it1, std::end(parallel1));
}

TEST(ParallelBitIterableTest, BasicUsageBatchSize08)
{
    constexpr std::size_t test_size = 8;
    using input_type = uint16_t;
    using output_type = dpf::bit;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type>;
    using element_type = typename dpf::parallel_const_bit_iterator<test_size, dpf::dynamic_bit_array<>>::value_type::value_type;
    const std::size_t bitlength_of_element = dpf::utils::bitlength_of_v<element_type>;
    auto memo0 = dpf::make_basic_full_memoizer<dpf_type>(),
         memo1 = dpf::make_basic_full_memoizer<dpf_type>();
    output_type y = dpf::bit::one;
    constexpr std::array<input_type, test_size> x{0x0000, 0x3333, 0x5555, 0x7FFF,
                                                  0x8000, 0xAAAA, 0xCCCC, 0xFFFF};
    auto [dpf00, dpf01] = dpf::make_dpf(x[0], y);
    auto [dpf10, dpf11] = dpf::make_dpf(x[1], y);
    auto [dpf20, dpf21] = dpf::make_dpf(x[2], y);
    auto [dpf30, dpf31] = dpf::make_dpf(x[3], y);
    auto [dpf40, dpf41] = dpf::make_dpf(x[4], y);
    auto [dpf50, dpf51] = dpf::make_dpf(x[5], y);
    auto [dpf60, dpf61] = dpf::make_dpf(x[6], y);
    auto [dpf70, dpf71] = dpf::make_dpf(x[7], y);
    auto [buf00, iter00] = dpf::eval_full(dpf00, memo0);
    auto [buf01, iter01] = dpf::eval_full(dpf01, memo1);
    auto [buf10, iter10] = dpf::eval_full(dpf10, memo0);
    auto [buf11, iter11] = dpf::eval_full(dpf11, memo1);
    auto [buf20, iter20] = dpf::eval_full(dpf20, memo0);
    auto [buf21, iter21] = dpf::eval_full(dpf21, memo1);
    auto [buf30, iter30] = dpf::eval_full(dpf30, memo0);
    auto [buf31, iter31] = dpf::eval_full(dpf31, memo1);
    auto [buf40, iter40] = dpf::eval_full(dpf40, memo0);
    auto [buf41, iter41] = dpf::eval_full(dpf41, memo1);
    auto [buf50, iter50] = dpf::eval_full(dpf50, memo0);
    auto [buf51, iter51] = dpf::eval_full(dpf51, memo1);
    auto [buf60, iter60] = dpf::eval_full(dpf60, memo0);
    auto [buf61, iter61] = dpf::eval_full(dpf61, memo1);
    auto [buf70, iter70] = dpf::eval_full(dpf70, memo0);
    auto [buf71, iter71] = dpf::eval_full(dpf71, memo1);

    auto parallel0 = dpf::batch_of(buf00, buf10, buf20, buf30, buf40, buf50, buf60, buf70),
         parallel1 = dpf::batch_of(buf01, buf11, buf21, buf31, buf41, buf51, buf61, buf71);

    auto it0 = std::begin(parallel0), it1 = std::begin(parallel1);
    for (std::size_t i = 0, idx = 0; i < std::size_t(1)<<dpf::utils::bitlength_of_v<input_type>; ++i, ++it0, ++it1)
    {
        auto tmp = *it0;
        if (idx < test_size && i == x[idx])
        {
            tmp[idx] ^= element_type(1) << x[idx]%bitlength_of_element;
            ++idx;
        }
        ASSERT_EQ(tmp, *it1);
    }
    ASSERT_EQ(it0, std::end(parallel0));
    ASSERT_EQ(it1, std::end(parallel1));
}

TEST(ParallelBitIterableTest, BasicUsageBatchSize16)
{
    constexpr std::size_t test_size = 16;
    using input_type = uint16_t;
    using output_type = dpf::bit;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type>;
    using element_type = typename dpf::parallel_const_bit_iterator<test_size, dpf::dynamic_bit_array<>>::value_type::value_type;
    const std::size_t bitlength_of_element = dpf::utils::bitlength_of_v<element_type>;
    auto memo0 = dpf::make_basic_full_memoizer<dpf_type>(),
         memo1 = dpf::make_basic_full_memoizer<dpf_type>();
    output_type y = dpf::bit::one;
    constexpr std::array<input_type, test_size> x{0x0000, 0x1111, 0x2222, 0x3333,
                                                  0x4444, 0x5555, 0x6666, 0x7FFF,
                                                  0x8000, 0x9999, 0xAAAA, 0xBBBB,
                                                  0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF};
    auto [dpf000, dpf001] = dpf::make_dpf(x[ 0], y);
    auto [dpf010, dpf011] = dpf::make_dpf(x[ 1], y);
    auto [dpf020, dpf021] = dpf::make_dpf(x[ 2], y);
    auto [dpf030, dpf031] = dpf::make_dpf(x[ 3], y);
    auto [dpf040, dpf041] = dpf::make_dpf(x[ 4], y);
    auto [dpf050, dpf051] = dpf::make_dpf(x[ 5], y);
    auto [dpf060, dpf061] = dpf::make_dpf(x[ 6], y);
    auto [dpf070, dpf071] = dpf::make_dpf(x[ 7], y);
    auto [dpf080, dpf081] = dpf::make_dpf(x[ 8], y);
    auto [dpf090, dpf091] = dpf::make_dpf(x[ 9], y);
    auto [dpf100, dpf101] = dpf::make_dpf(x[10], y);
    auto [dpf110, dpf111] = dpf::make_dpf(x[11], y);
    auto [dpf120, dpf121] = dpf::make_dpf(x[12], y);
    auto [dpf130, dpf131] = dpf::make_dpf(x[13], y);
    auto [dpf140, dpf141] = dpf::make_dpf(x[14], y);
    auto [dpf150, dpf151] = dpf::make_dpf(x[15], y);
    auto [buf000, iter000] = dpf::eval_full(dpf000, memo0);
    auto [buf001, iter001] = dpf::eval_full(dpf001, memo1);
    auto [buf010, iter010] = dpf::eval_full(dpf010, memo0);
    auto [buf011, iter011] = dpf::eval_full(dpf011, memo1);
    auto [buf020, iter020] = dpf::eval_full(dpf020, memo0);
    auto [buf021, iter021] = dpf::eval_full(dpf021, memo1);
    auto [buf030, iter030] = dpf::eval_full(dpf030, memo0);
    auto [buf031, iter031] = dpf::eval_full(dpf031, memo1);
    auto [buf040, iter040] = dpf::eval_full(dpf040, memo0);
    auto [buf041, iter041] = dpf::eval_full(dpf041, memo1);
    auto [buf050, iter050] = dpf::eval_full(dpf050, memo0);
    auto [buf051, iter051] = dpf::eval_full(dpf051, memo1);
    auto [buf060, iter060] = dpf::eval_full(dpf060, memo0);
    auto [buf061, iter061] = dpf::eval_full(dpf061, memo1);
    auto [buf070, iter070] = dpf::eval_full(dpf070, memo0);
    auto [buf071, iter071] = dpf::eval_full(dpf071, memo1);
    auto [buf080, iter080] = dpf::eval_full(dpf080, memo0);
    auto [buf081, iter081] = dpf::eval_full(dpf081, memo1);
    auto [buf090, iter090] = dpf::eval_full(dpf090, memo0);
    auto [buf091, iter091] = dpf::eval_full(dpf091, memo1);
    auto [buf100, iter100] = dpf::eval_full(dpf100, memo0);
    auto [buf101, iter101] = dpf::eval_full(dpf101, memo1);
    auto [buf110, iter110] = dpf::eval_full(dpf110, memo0);
    auto [buf111, iter111] = dpf::eval_full(dpf111, memo1);
    auto [buf120, iter120] = dpf::eval_full(dpf120, memo0);
    auto [buf121, iter121] = dpf::eval_full(dpf121, memo1);
    auto [buf130, iter130] = dpf::eval_full(dpf130, memo0);
    auto [buf131, iter131] = dpf::eval_full(dpf131, memo1);
    auto [buf140, iter140] = dpf::eval_full(dpf140, memo0);
    auto [buf141, iter141] = dpf::eval_full(dpf141, memo1);
    auto [buf150, iter150] = dpf::eval_full(dpf150, memo0);
    auto [buf151, iter151] = dpf::eval_full(dpf151, memo1);

    auto parallel0 = dpf::batch_of(buf000, buf010, buf020, buf030, buf040, buf050, buf060, buf070,
                                   buf080, buf090, buf100, buf110, buf120, buf130, buf140, buf150),
         parallel1 = dpf::batch_of(buf001, buf011, buf021, buf031, buf041, buf051, buf061, buf071,
                                   buf081, buf091, buf101, buf111, buf121, buf131, buf141, buf151);

    auto it0 = std::begin(parallel0), it1 = std::begin(parallel1);
    for (std::size_t i = 0, idx = 0; i < std::size_t(1)<<dpf::utils::bitlength_of_v<input_type>; ++i, ++it0, ++it1)
    {
        auto tmp = *it0;
        if (idx < test_size && i == x[idx])
        {
            tmp[idx] ^= element_type(1) << x[idx]%bitlength_of_element;
            ++idx;
        }
        ASSERT_EQ(tmp, *it1);
    }
    ASSERT_EQ(it0, std::end(parallel0));
    ASSERT_EQ(it1, std::end(parallel1));
}
