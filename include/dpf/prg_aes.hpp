/// @file dpf/prg/aes128.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2022 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see `LICENSE` for details.

#ifndef LIBDPF_INCLUDE_DPF_PRG_AES_HPP__
#define LIBDPF_INCLUDE_DPF_PRG_AES_HPP__

#include <array>

#include "hedley/hedley.h"
#include "simde/simde/x86/avx2.h"

#include "dpf/utils.hpp"
// #include "dpf/twiddle.hpp"

namespace dpf
{

namespace prg
{

#ifdef __ARM_NEON
// TODO
#else
#define simde_mm_aesenc_si128(x, y) _mm_aesenc_si128(x, y);
#define simde_mm_aesenclast_si128(x, y) _mm_aesenclast_si128(x, y);
#define simde_mm_aeskeygenassist_si128(x, y) _mm_aeskeygenassist_si128(x, y);
#endif

template <typename aeskey_t>
struct aes final
{
    using block_t = simde__m128i;

    HEDLEY_NO_THROW
    HEDLEY_CONST
    DPF_UNROLL_LOOPS
    static block_t eval(block_t seed, uint32_t pos) noexcept
    {
        block_t rd_key0 = simde_mm_xor_si128(key.rd_key[0],
            simde_mm_set_epi64x(pos, 0));
        block_t output = simde_mm_xor_si128(seed, rd_key0);

        for (std::size_t j = 1; j < key.rounds; ++j)
        {
            output = simde_mm_aesenc_si128(output, key.rd_key[j]);
        }
        output = simde_mm_aesenclast_si128(output, key.rd_key[key.rounds]);
        output = simde_mm_xor_si128(output, seed);

        return output;
    }

    HEDLEY_NO_THROW
    HEDLEY_CONST
    DPF_UNROLL_LOOPS
    static auto eval01(block_t seed) noexcept
    {
        block_t rd_key00 = key.rd_key[0];
        block_t rd_key01 = simde_mm_xor_si128(rd_key00,
            simde_mm_set_epi64x(1, 0));

        block_t output0 = simde_mm_xor_si128(seed, rd_key00);
        block_t output1 = simde_mm_xor_si128(seed, rd_key01);

        for (std::size_t j = 1; j < key.rounds; ++j)
        {
            output0 = simde_mm_aesenc_si128(output0, key.rd_key[j]);
            output1 = simde_mm_aesenc_si128(output1, key.rd_key[j]);
        }
        output0 = simde_mm_aesenclast_si128(output0, key.rd_key[key.rounds]);
        output1 = simde_mm_aesenclast_si128(output1, key.rd_key[key.rounds]);
        output0 = simde_mm_xor_si128(output0, seed);
        output1 = simde_mm_xor_si128(output1, seed);
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
        return std::array<block_t, 2>{output0, output1};
HEDLEY_PRAGMA(GCC diagnostic pop)
    }

    HEDLEY_NO_THROW
    DPF_UNROLL_LOOPS
    static void eval(block_t seed, block_t * HEDLEY_RESTRICT output,
        uint32_t count, uint32_t pos = 0) noexcept
    {
        static constexpr block_t one{1,0};
        auto pos_ = simde_mm_set_epi64x(pos, 0);
        block_t * HEDLEY_RESTRICT out =
            static_cast<block_t *>(__builtin_assume_aligned(output,
            alignof(block_t)));
        for (uint32_t i = 0; i < count; ++i)
        {
            out[i] = simde_mm_xor_si128(seed, pos_);
            pos_ = simde_mm_add_epi64(pos_, one);
            for (std::size_t j = 1; j < key.rounds; ++j)
            {
                out[i] = simde_mm_aesenc_si128(out[i], key.rd_key[j]);
            }
            out[i] = simde_mm_aesenclast_si128(out[i],
                key.rd_key[key.rounds]);
            out[i] = simde_mm_xor_si128(out[i], seed);
        }
    }

  private:
    static const aeskey_t key;
};  // struct aes

#define EXPAND_ASSIST(v1, v2, v3, v4, shuff_const, aes_const)  \
  v2 = simde_mm_aeskeygenassist_si128(v4, aes_const);          \
  v3 = simde_mm_castps_si128(_mm_shuffle_ps(                   \
            simde_mm_castsi128_ps(v3),                         \
            simde_mm_castsi128_ps(v1), 16));                   \
  v1 = simde_mm_xor_si128(v1, v3);                             \
  v3 = simde_mm_castps_si128(simde_mm_shuffle_ps(              \
            simde_mm_castsi128_ps(v3),                         \
            simde_mm_castsi128_ps(v1), 140));                  \
  v1 = simde_mm_xor_si128(v1, v3);                             \
  v2 = simde_mm_shuffle_epi32(v2, shuff_const);                \
  v1 = simde_mm_xor_si128(v1, v2)

struct aes128_key
{
  public:
    static constexpr std::size_t rounds = 10;
    HEDLEY_PRAGMA(GCC diagnostic push)
    HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using rd_key_array = std::array<simde__m128i, rounds+1>;
    HEDLEY_PRAGMA(GCC diagnostic pop)
    const rd_key_array rd_key;

    aes128_key(const simde__m128i & userkey)
      : rd_key{compute_round_keys(userkey)} { }

  private:
    rd_key_array compute_round_keys(const simde__m128i & userkey)
    {
        rd_key_array rd_key;
        simde__m128i x0, x1, x2;
        rd_key[0] = x0 = userkey;
        x2 = simde_mm_setzero_si128();
        EXPAND_ASSIST(x0, x1, x2, x0, 255, 1);
        rd_key[1] = x0;
        EXPAND_ASSIST(x0, x1, x2, x0, 255, 2);
        rd_key[2] = x0;
        EXPAND_ASSIST(x0, x1, x2, x0, 255, 4);
        rd_key[3] = x0;
        EXPAND_ASSIST(x0, x1, x2, x0, 255, 8);
        rd_key[4] = x0;
        EXPAND_ASSIST(x0, x1, x2, x0, 255, 16);
        rd_key[5] = x0;
        EXPAND_ASSIST(x0, x1, x2, x0, 255, 32);
        rd_key[6] = x0;
        EXPAND_ASSIST(x0, x1, x2, x0, 255, 64);
        rd_key[7] = x0;
        EXPAND_ASSIST(x0, x1, x2, x0, 255, 128);
        rd_key[8] = x0;
        EXPAND_ASSIST(x0, x1, x2, x0, 255, 27);
        rd_key[9] = x0;
        EXPAND_ASSIST(x0, x1, x2, x0, 255, 54);
        rd_key[10] = x0;

        return rd_key;
    }
};  // struct aes128_key    

struct aes256_key
{
  public:
    static constexpr std::size_t rounds = 14;

  HEDLEY_PRAGMA(GCC diagnostic push)
  HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
    using rd_key_array = std::array<simde__m128i, rounds+1>;
  HEDLEY_PRAGMA(GCC diagnostic pop)
    const rd_key_array rd_key;

    aes256_key(const simde__m256i & userkey)
      : rd_key{compute_round_keys(userkey)} { }

  private:
    rd_key_array compute_round_keys(const simde__m256i & userkey)
    {
        rd_key_array rd_key;
        simde__m128i x0, x1, x2, x3;
        rd_key[0] = x0 = simde_mm256_extracti128_si256(userkey, 0);
        rd_key[1] = x3 = simde_mm256_extracti128_si256(userkey, 0);
        x2 = simde_mm_setzero_si128();

        EXPAND_ASSIST(x0, x1, x2, x3, 255, 1);
        rd_key[2] = x0;
        EXPAND_ASSIST(x3, x1, x2, x0, 170, 1);
        rd_key[3] = x3;
        EXPAND_ASSIST(x0, x1, x2, x3, 255, 2);
        rd_key[4] = x0;
        EXPAND_ASSIST(x3, x1, x2, x0, 170, 2);
        rd_key[5] = x3;
        EXPAND_ASSIST(x0, x1, x2, x3, 255, 4);
        rd_key[6] = x0;
        EXPAND_ASSIST(x3, x1, x2, x0, 170, 4);
        rd_key[7] = x3;
        EXPAND_ASSIST(x0, x1, x2, x3, 255, 8);
        rd_key[8] = x0;
        EXPAND_ASSIST(x3, x1, x2, x0, 170, 8);
        rd_key[9] = x3;
        EXPAND_ASSIST(x0, x1, x2, x3, 255, 16);
        rd_key[10] = x0;
        EXPAND_ASSIST(x3, x1, x2, x0, 170, 16);
        rd_key[11] = x3;
        EXPAND_ASSIST(x0, x1, x2, x3, 255, 32);
        rd_key[12] = x0;
        EXPAND_ASSIST(x3, x1, x2, x0, 170, 32);
        rd_key[13] = x3;
        EXPAND_ASSIST(x0, x1, x2, x3, 255, 64);
        rd_key[14] = x0;

        return rd_key;
    }
};  // struct aes256_key

using aes128 = aes<aes128_key>;
using aes256 = aes<aes256_key>;

template <>
const aes128_key aes128::key = simde__m128i{0, 0};

template <>
const aes256_key aes256::key = simde__m256i{0, 0, 0, 0};

}  // namespace prg

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_PRG_AES_HPP__
