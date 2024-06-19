/// @file dpf/xor_wrapper.hpp
/// @brief defines the `xor_wrapper` class and associated helpers
/// @details A `xor_wrapper` is a struct template that adapts integral
///          types to use bitwise arithmetic; that is, it makes an `N`-bit
///          integer type behave as it it were an element of `GF(2)^N`.
///          Specifically,
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_XOR_WRAPPER_HPP__
#define LIBDPF_INCLUDE_DPF_XOR_WRAPPER_HPP__

#include <cstddef>
#include <type_traits>
#include <functional>
#include <limits>
#include <ostream>
#include <istream>

#include "simde/simde/x86/avx2.h"
#include "portable-snippets/exact-int/exact-int.h"

#include "dpf/modint.hpp"
#include "dpf/literals.hpp"

namespace dpf
{

template <typename T>
struct xor_wrapper
{
  public:
    using value_type = utils::make_unsigned_t<T>;
    static constexpr auto bit_xor = std::bit_xor<value_type>{};
    static constexpr auto bit_and = std::bit_and<value_type>{};
    static constexpr auto bit_or = std::bit_or<value_type>{};
    static constexpr auto bit_not = std::bit_not<value_type>{};

    static constexpr std::size_t bits = utils::bitlength_of_v<value_type>;
    using integral_type = utils::integral_type_from_bitlength_t<bits>;

    /// @{

    /// @brief Default c'tor
    constexpr xor_wrapper() = default;

    /// @brief Copy c'tor
    constexpr xor_wrapper(const xor_wrapper &) noexcept = default;

    /// @brief Move c'tor
    constexpr xor_wrapper(xor_wrapper &&) noexcept = default;

    /// @brief Value c'tor
    // cppcheck-suppress noExplicitConstructor
    constexpr xor_wrapper(value_type v) noexcept : value{v} { }  // NOLINT(runtime/explicit)

    /// @}

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr xor_wrapper & operator=(const xor_wrapper &) noexcept = default;

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr xor_wrapper & operator=(xor_wrapper &&) noexcept = default;

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr xor_wrapper & operator=(value_type v) noexcept
    {
        value = v;
        return *this;
    }

    ~xor_wrapper() = default;

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr xor_wrapper operator-() const noexcept
    {
        return xor_wrapper{value};
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr operator bool() const noexcept
    {
        return static_cast<bool>(value);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr operator T() const noexcept {
        if constexpr (std::is_same_v<T, value_type> == true)
        {
            return value;
        }
        return static_cast<T>(value);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr bool operator==(xor_wrapper rhs) const noexcept
    {
        return value == rhs.value;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr bool operator!=(xor_wrapper rhs) const noexcept
    {
        return value != rhs.value;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr bool operator<(xor_wrapper rhs) const noexcept
    {
        return value < rhs.value;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr bool operator<=(xor_wrapper rhs) const noexcept
    {
        return value <= rhs.value;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr bool operator>(xor_wrapper rhs) const noexcept
    {
        return value > rhs.value;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    HEDLEY_PURE
    constexpr bool operator>=(xor_wrapper rhs) const noexcept
    {
        return value >= rhs.value;
    }

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr value_type data() const noexcept
    {
        return value;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr xor_wrapper & operator<<=(std::size_t amount)
    {
        this->value <<= amount;
        return *this;
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    constexpr xor_wrapper & operator>>=(std::size_t amount)
    {
        this->value >>= amount;
        return *this;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr xor_wrapper & operator++() noexcept
    {
        ++value;
        return *this;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr xor_wrapper operator++(int) noexcept
    {
        auto ret = *this;
        this->operator++();
        return ret;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr xor_wrapper & operator--() noexcept
    {
        --value;
        return *this;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr xor_wrapper operator--(int) noexcept
    {
        auto ret = *this;
        this->operator--();
        return ret;
    }

  private:
    value_type value;

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator+(const xor_wrapper & lhs,
        const xor_wrapper & rhs) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_xor(lhs.value, rhs.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator-(const xor_wrapper & lhs,
        const xor_wrapper & rhs) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_xor(lhs.value, rhs.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator*(const xor_wrapper & lhs,
        const xor_wrapper & rhs) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_and(lhs.value, rhs.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator&(const xor_wrapper & lhs,
        const xor_wrapper & rhs) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_and(lhs.value, rhs.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator|(const xor_wrapper & lhs,
        const xor_wrapper & rhs) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_or(lhs.value, rhs.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator^(const xor_wrapper & lhs,
        const xor_wrapper & rhs) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_xor(lhs.value, rhs.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator~(const xor_wrapper & val) noexcept
    {
        return xor_wrapper(xor_wrapper::bit_not(val.value));
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator<<(const xor_wrapper & val, std::size_t amount)
    {
        return xor_wrapper(val.value << amount);
    }

    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    friend constexpr xor_wrapper operator>>(const xor_wrapper & val, std::size_t amount)
    {
        return xor_wrapper(val.value >> amount);
    }

    friend std::ostream & operator<<(std::ostream & os, xor_wrapper<T> val)
    {
        return os << val.value;
    }

    friend std::istream & operator<<(std::istream & is, xor_wrapper<T> & val)
    {
        return is >> val.value;
    }

    friend struct utils::to_integral_type<xor_wrapper>;
    friend struct utils::mod_pow_2<xor_wrapper>;
};

template <std::size_t Nbits>
using xint = xor_wrapper<dpf::modint<Nbits>>;

namespace xints
{

// 1--9
using xint1_t = xint<1>;
using xint2_t = xint<2>;
using xint3_t = xint<3>;
using xint4_t = xint<4>;
using xint5_t = xint<5>;
using xint6_t = xint<6>;
using xint7_t = xint<7>;
using xint8_t = xint<8>;
using xint9_t = xint<9>;
// 10--19
using xint10_t = xint<10>;
using xint11_t = xint<11>;
using xint12_t = xint<12>;
using xint13_t = xint<13>;
using xint14_t = xint<14>;
using xint15_t = xint<15>;
using xint16_t = xint<16>;
using xint17_t = xint<17>;
using xint18_t = xint<18>;
using xint19_t = xint<19>;
// 20--29
using xint20_t = xint<20>;
using xint21_t = xint<21>;
using xint22_t = xint<22>;
using xint23_t = xint<23>;
using xint24_t = xint<24>;
using xint25_t = xint<25>;
using xint26_t = xint<26>;
using xint27_t = xint<27>;
using xint28_t = xint<28>;
using xint29_t = xint<29>;
// 30--39
using xint30_t = xint<30>;
using xint31_t = xint<31>;
using xint32_t = xint<32>;
using xint33_t = xint<33>;
using xint34_t = xint<34>;
using xint35_t = xint<35>;
using xint36_t = xint<36>;
using xint37_t = xint<37>;
using xint38_t = xint<38>;
using xint39_t = xint<39>;
// 40--49
using xint40_t = xint<40>;
using xint41_t = xint<41>;
using xint42_t = xint<42>;
using xint43_t = xint<43>;
using xint44_t = xint<44>;
using xint45_t = xint<45>;
using xint46_t = xint<46>;
using xint47_t = xint<47>;
using xint48_t = xint<48>;
using xint49_t = xint<49>;
// 50--59
using xint50_t = xint<50>;
using xint51_t = xint<51>;
using xint52_t = xint<52>;
using xint53_t = xint<53>;
using xint54_t = xint<54>;
using xint55_t = xint<55>;
using xint56_t = xint<56>;
using xint57_t = xint<57>;
using xint58_t = xint<58>;
using xint59_t = xint<59>;
// 60--69
using xint60_t = xint<60>;
using xint61_t = xint<61>;
using xint62_t = xint<62>;
using xint63_t = xint<63>;
using xint64_t = xint<64>;
using xint65_t = xint<65>;
using xint66_t = xint<66>;
using xint67_t = xint<67>;
using xint68_t = xint<68>;
using xint69_t = xint<69>;
// 70--79
using xint70_t = xint<70>;
using xint71_t = xint<71>;
using xint72_t = xint<72>;
using xint73_t = xint<73>;
using xint74_t = xint<74>;
using xint75_t = xint<75>;
using xint76_t = xint<76>;
using xint77_t = xint<77>;
using xint78_t = xint<78>;
using xint79_t = xint<79>;
// 80--89
using xint80_t = xint<80>;
using xint81_t = xint<81>;
using xint82_t = xint<82>;
using xint83_t = xint<83>;
using xint84_t = xint<84>;
using xint85_t = xint<85>;
using xint86_t = xint<86>;
using xint87_t = xint<87>;
using xint88_t = xint<88>;
using xint89_t = xint<89>;
// 90--99
using xint90_t = xint<90>;
using xint91_t = xint<91>;
using xint92_t = xint<92>;
using xint93_t = xint<93>;
using xint94_t = xint<94>;
using xint95_t = xint<95>;
using xint96_t = xint<96>;
using xint97_t = xint<97>;
using xint98_t = xint<98>;
using xint99_t = xint<99>;
// 100--109
using xint100_t = xint<100>;
using xint101_t = xint<101>;
using xint102_t = xint<102>;
using xint103_t = xint<103>;
using xint104_t = xint<104>;
using xint105_t = xint<105>;
using xint106_t = xint<106>;
using xint107_t = xint<107>;
using xint108_t = xint<108>;
using xint109_t = xint<109>;
// 110--119
using xint110_t = xint<110>;
using xint111_t = xint<111>;
using xint112_t = xint<112>;
using xint113_t = xint<113>;
using xint114_t = xint<114>;
using xint115_t = xint<115>;
using xint116_t = xint<116>;
using xint117_t = xint<117>;
using xint118_t = xint<118>;
using xint119_t = xint<119>;
// 120--129
using xint120_t = xint<120>;
using xint121_t = xint<121>;
using xint122_t = xint<122>;
using xint123_t = xint<123>;
using xint124_t = xint<124>;
using xint125_t = xint<125>;
using xint126_t = xint<126>;
using xint127_t = xint<127>;
using xint128_t = xint<128>;
using xint129_t = xint<129>;
// 130--139
using xint130_t = xint<130>;
using xint131_t = xint<131>;
using xint132_t = xint<132>;
using xint133_t = xint<133>;
using xint134_t = xint<134>;
using xint135_t = xint<135>;
using xint136_t = xint<136>;
using xint137_t = xint<137>;
using xint138_t = xint<138>;
using xint139_t = xint<139>;
// 140--149
using xint140_t = xint<140>;
using xint141_t = xint<141>;
using xint142_t = xint<142>;
using xint143_t = xint<143>;
using xint144_t = xint<144>;
using xint145_t = xint<145>;
using xint146_t = xint<146>;
using xint147_t = xint<147>;
using xint148_t = xint<148>;
using xint149_t = xint<149>;
// 150--159
using xint150_t = xint<150>;
using xint151_t = xint<151>;
using xint152_t = xint<152>;
using xint153_t = xint<153>;
using xint154_t = xint<154>;
using xint155_t = xint<155>;
using xint156_t = xint<156>;
using xint157_t = xint<157>;
using xint158_t = xint<158>;
using xint159_t = xint<159>;
// 160--169
using xint160_t = xint<160>;
using xint161_t = xint<161>;
using xint162_t = xint<162>;
using xint163_t = xint<163>;
using xint164_t = xint<164>;
using xint165_t = xint<165>;
using xint166_t = xint<166>;
using xint167_t = xint<167>;
using xint168_t = xint<168>;
using xint169_t = xint<169>;
// 170--179
using xint170_t = xint<170>;
using xint171_t = xint<171>;
using xint172_t = xint<172>;
using xint173_t = xint<173>;
using xint174_t = xint<174>;
using xint175_t = xint<175>;
using xint176_t = xint<176>;
using xint177_t = xint<177>;
using xint178_t = xint<178>;
using xint179_t = xint<179>;
// 180--189
using xint180_t = xint<180>;
using xint181_t = xint<181>;
using xint182_t = xint<182>;
using xint183_t = xint<183>;
using xint184_t = xint<184>;
using xint185_t = xint<185>;
using xint186_t = xint<186>;
using xint187_t = xint<187>;
using xint188_t = xint<188>;
using xint189_t = xint<189>;
// 190--199
using xint190_t = xint<190>;
using xint191_t = xint<191>;
using xint192_t = xint<192>;
using xint193_t = xint<193>;
using xint194_t = xint<194>;
using xint195_t = xint<195>;
using xint196_t = xint<196>;
using xint197_t = xint<197>;
using xint198_t = xint<198>;
using xint199_t = xint<199>;
// 200--209
using xint200_t = xint<200>;
using xint201_t = xint<201>;
using xint202_t = xint<202>;
using xint203_t = xint<203>;
using xint204_t = xint<204>;
using xint205_t = xint<205>;
using xint206_t = xint<206>;
using xint207_t = xint<207>;
using xint208_t = xint<208>;
using xint209_t = xint<209>;
// 210--219
using xint210_t = xint<210>;
using xint211_t = xint<211>;
using xint212_t = xint<212>;
using xint213_t = xint<213>;
using xint214_t = xint<214>;
using xint215_t = xint<215>;
using xint216_t = xint<216>;
using xint217_t = xint<217>;
using xint218_t = xint<218>;
using xint219_t = xint<219>;
// 220--229
using xint220_t = xint<220>;
using xint221_t = xint<221>;
using xint222_t = xint<222>;
using xint223_t = xint<223>;
using xint224_t = xint<224>;
using xint225_t = xint<225>;
using xint226_t = xint<226>;
using xint227_t = xint<227>;
using xint228_t = xint<228>;
using xint229_t = xint<229>;
// 230--239
using xint230_t = xint<230>;
using xint231_t = xint<231>;
using xint232_t = xint<232>;
using xint233_t = xint<233>;
using xint234_t = xint<234>;
using xint235_t = xint<235>;
using xint236_t = xint<236>;
using xint237_t = xint<237>;
using xint238_t = xint<238>;
using xint239_t = xint<239>;
// 240--249
using xint240_t = xint<240>;
using xint241_t = xint<241>;
using xint242_t = xint<242>;
using xint243_t = xint<243>;
using xint244_t = xint<244>;
using xint245_t = xint<245>;
using xint246_t = xint<246>;
using xint247_t = xint<247>;
using xint248_t = xint<248>;
using xint249_t = xint<249>;
// 250--256
using xint250_t = xint<250>;
using xint251_t = xint<251>;
using xint252_t = xint<252>;
using xint253_t = xint<253>;
using xint254_t = xint<254>;
using xint255_t = xint<255>;
using xint256_t = xint<256>;

namespace literals = dpf::literals::xints;

}  // namespace xints

namespace literals
{

namespace xints
{

// 1--9
constexpr static auto operator "" _x1(unsigned long long int x) { return dpf::xints::xint1_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _x2(unsigned long long int x) { return dpf::xints::xint2_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _x3(unsigned long long int x) { return dpf::xints::xint3_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _x4(unsigned long long int x) { return dpf::xints::xint4_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _x5(unsigned long long int x) { return dpf::xints::xint5_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _x6(unsigned long long int x) { return dpf::xints::xint6_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _x7(unsigned long long int x) { return dpf::xints::xint7_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _x8(unsigned long long int x) { return dpf::xints::xint8_t{static_cast<psnip_uint8_t>(x)}; }
constexpr static auto operator "" _x9(unsigned long long int x) { return dpf::xints::xint9_t{static_cast<psnip_uint16_t>(x)}; }
// 10--19
constexpr static auto operator "" _x10(unsigned long long int x) { return dpf::xints::xint10_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _x11(unsigned long long int x) { return dpf::xints::xint11_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _x12(unsigned long long int x) { return dpf::xints::xint12_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _x13(unsigned long long int x) { return dpf::xints::xint13_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _x14(unsigned long long int x) { return dpf::xints::xint14_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _x15(unsigned long long int x) { return dpf::xints::xint15_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _x16(unsigned long long int x) { return dpf::xints::xint16_t{static_cast<psnip_uint16_t>(x)}; }
constexpr static auto operator "" _x17(unsigned long long int x) { return dpf::xints::xint17_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x18(unsigned long long int x) { return dpf::xints::xint18_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x19(unsigned long long int x) { return dpf::xints::xint19_t{static_cast<psnip_uint32_t>(x)}; }
// 20--29
constexpr static auto operator "" _x20(unsigned long long int x) { return dpf::xints::xint20_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x21(unsigned long long int x) { return dpf::xints::xint21_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x22(unsigned long long int x) { return dpf::xints::xint22_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x23(unsigned long long int x) { return dpf::xints::xint23_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x24(unsigned long long int x) { return dpf::xints::xint24_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x25(unsigned long long int x) { return dpf::xints::xint25_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x26(unsigned long long int x) { return dpf::xints::xint26_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x27(unsigned long long int x) { return dpf::xints::xint27_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x28(unsigned long long int x) { return dpf::xints::xint28_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x29(unsigned long long int x) { return dpf::xints::xint29_t{static_cast<psnip_uint32_t>(x)}; }
// 30--39
constexpr static auto operator "" _x30(unsigned long long int x) { return dpf::xints::xint30_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x31(unsigned long long int x) { return dpf::xints::xint31_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x32(unsigned long long int x) { return dpf::xints::xint32_t{static_cast<psnip_uint32_t>(x)}; }
constexpr static auto operator "" _x33(unsigned long long int x) { return dpf::xints::xint33_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x34(unsigned long long int x) { return dpf::xints::xint34_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x35(unsigned long long int x) { return dpf::xints::xint35_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x36(unsigned long long int x) { return dpf::xints::xint36_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x37(unsigned long long int x) { return dpf::xints::xint37_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x38(unsigned long long int x) { return dpf::xints::xint38_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x39(unsigned long long int x) { return dpf::xints::xint39_t{static_cast<psnip_uint64_t>(x)}; }
// 40--49
constexpr static auto operator "" _x40(unsigned long long int x) { return dpf::xints::xint40_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x41(unsigned long long int x) { return dpf::xints::xint41_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x42(unsigned long long int x) { return dpf::xints::xint42_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x43(unsigned long long int x) { return dpf::xints::xint43_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x44(unsigned long long int x) { return dpf::xints::xint44_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x45(unsigned long long int x) { return dpf::xints::xint45_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x46(unsigned long long int x) { return dpf::xints::xint46_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x47(unsigned long long int x) { return dpf::xints::xint47_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x48(unsigned long long int x) { return dpf::xints::xint48_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x49(unsigned long long int x) { return dpf::xints::xint49_t{static_cast<psnip_uint64_t>(x)}; }
// 50--59
constexpr static auto operator "" _x50(unsigned long long int x) { return dpf::xints::xint50_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x51(unsigned long long int x) { return dpf::xints::xint51_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x52(unsigned long long int x) { return dpf::xints::xint52_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x53(unsigned long long int x) { return dpf::xints::xint53_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x54(unsigned long long int x) { return dpf::xints::xint54_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x55(unsigned long long int x) { return dpf::xints::xint55_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x56(unsigned long long int x) { return dpf::xints::xint56_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x57(unsigned long long int x) { return dpf::xints::xint57_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x58(unsigned long long int x) { return dpf::xints::xint58_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x59(unsigned long long int x) { return dpf::xints::xint59_t{static_cast<psnip_uint64_t>(x)}; }
// 60--69
constexpr static auto operator "" _x60(unsigned long long int x) { return dpf::xints::xint60_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x61(unsigned long long int x) { return dpf::xints::xint61_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x62(unsigned long long int x) { return dpf::xints::xint62_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x63(unsigned long long int x) { return dpf::xints::xint63_t{static_cast<psnip_uint64_t>(x)}; }
constexpr static auto operator "" _x64(unsigned long long int x) { return dpf::xints::xint64_t{static_cast<psnip_uint64_t>(x)}; }
template <char ...digits> constexpr static auto operator "" _x65() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint65_t{x}; }
template <char ...digits> constexpr static auto operator "" _x66() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint66_t{x}; }
template <char ...digits> constexpr static auto operator "" _x67() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint67_t{x}; }
template <char ...digits> constexpr static auto operator "" _x68() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint68_t{x}; }
template <char ...digits> constexpr static auto operator "" _x69() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint69_t{x}; }
// 70--79
template <char ...digits> constexpr static auto operator "" _x70() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint70_t{x}; }
template <char ...digits> constexpr static auto operator "" _x71() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint71_t{x}; }
template <char ...digits> constexpr static auto operator "" _x72() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint72_t{x}; }
template <char ...digits> constexpr static auto operator "" _x73() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint73_t{x}; }
template <char ...digits> constexpr static auto operator "" _x74() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint74_t{x}; }
template <char ...digits> constexpr static auto operator "" _x75() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint75_t{x}; }
template <char ...digits> constexpr static auto operator "" _x76() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint76_t{x}; }
template <char ...digits> constexpr static auto operator "" _x77() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint77_t{x}; }
template <char ...digits> constexpr static auto operator "" _x78() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint78_t{x}; }
template <char ...digits> constexpr static auto operator "" _x79() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint79_t{x}; }
// 80--89
template <char ...digits> constexpr static auto operator "" _x80() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint80_t{x}; }
template <char ...digits> constexpr static auto operator "" _x81() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint81_t{x}; }
template <char ...digits> constexpr static auto operator "" _x82() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint82_t{x}; }
template <char ...digits> constexpr static auto operator "" _x83() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint83_t{x}; }
template <char ...digits> constexpr static auto operator "" _x84() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint84_t{x}; }
template <char ...digits> constexpr static auto operator "" _x85() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint85_t{x}; }
template <char ...digits> constexpr static auto operator "" _x86() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint86_t{x}; }
template <char ...digits> constexpr static auto operator "" _x87() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint87_t{x}; }
template <char ...digits> constexpr static auto operator "" _x88() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint88_t{x}; }
template <char ...digits> constexpr static auto operator "" _x89() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint89_t{x}; }
// 90--99
template <char ...digits> constexpr static auto operator "" _x90() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint90_t{x}; }
template <char ...digits> constexpr static auto operator "" _x91() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint91_t{x}; }
template <char ...digits> constexpr static auto operator "" _x92() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint92_t{x}; }
template <char ...digits> constexpr static auto operator "" _x93() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint93_t{x}; }
template <char ...digits> constexpr static auto operator "" _x94() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint94_t{x}; }
template <char ...digits> constexpr static auto operator "" _x95() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint95_t{x}; }
template <char ...digits> constexpr static auto operator "" _x96() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint96_t{x}; }
template <char ...digits> constexpr static auto operator "" _x97() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint97_t{x}; }
template <char ...digits> constexpr static auto operator "" _x98() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint98_t{x}; }
template <char ...digits> constexpr static auto operator "" _x99() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint99_t{x}; }
// 100--109
template <char ...digits> constexpr static auto operator "" _x100() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint100_t{x}; }
template <char ...digits> constexpr static auto operator "" _x101() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint101_t{x}; }
template <char ...digits> constexpr static auto operator "" _x102() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint102_t{x}; }
template <char ...digits> constexpr static auto operator "" _x103() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint103_t{x}; }
template <char ...digits> constexpr static auto operator "" _x104() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint104_t{x}; }
template <char ...digits> constexpr static auto operator "" _x105() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint105_t{x}; }
template <char ...digits> constexpr static auto operator "" _x106() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint106_t{x}; }
template <char ...digits> constexpr static auto operator "" _x107() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint107_t{x}; }
template <char ...digits> constexpr static auto operator "" _x108() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint108_t{x}; }
template <char ...digits> constexpr static auto operator "" _x109() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint109_t{x}; }
// 110--119
template <char ...digits> constexpr static auto operator "" _x110() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint110_t{x}; }
template <char ...digits> constexpr static auto operator "" _x111() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint111_t{x}; }
template <char ...digits> constexpr static auto operator "" _x112() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint112_t{x}; }
template <char ...digits> constexpr static auto operator "" _x113() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint113_t{x}; }
template <char ...digits> constexpr static auto operator "" _x114() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint114_t{x}; }
template <char ...digits> constexpr static auto operator "" _x115() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint115_t{x}; }
template <char ...digits> constexpr static auto operator "" _x116() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint116_t{x}; }
template <char ...digits> constexpr static auto operator "" _x117() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint117_t{x}; }
template <char ...digits> constexpr static auto operator "" _x118() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint118_t{x}; }
template <char ...digits> constexpr static auto operator "" _x119() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint119_t{x}; }
// 120--129
template <char ...digits> constexpr static auto operator "" _x120() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint120_t{x}; }
template <char ...digits> constexpr static auto operator "" _x121() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint121_t{x}; }
template <char ...digits> constexpr static auto operator "" _x122() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint122_t{x}; }
template <char ...digits> constexpr static auto operator "" _x123() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint123_t{x}; }
template <char ...digits> constexpr static auto operator "" _x124() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint124_t{x}; }
template <char ...digits> constexpr static auto operator "" _x125() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint125_t{x}; }
template <char ...digits> constexpr static auto operator "" _x126() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint126_t{x}; }
template <char ...digits> constexpr static auto operator "" _x127() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint127_t{x}; }
template <char ...digits> constexpr static auto operator "" _x128() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); simde_uint128 x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint128_t{x}; }
template <char ...digits> constexpr static auto operator "" _x129() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint129_t{x}; }
// 130--139
template <char ...digits> constexpr static auto operator "" _x130() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint130_t{x}; }
template <char ...digits> constexpr static auto operator "" _x131() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint131_t{x}; }
template <char ...digits> constexpr static auto operator "" _x132() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint132_t{x}; }
template <char ...digits> constexpr static auto operator "" _x133() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint133_t{x}; }
template <char ...digits> constexpr static auto operator "" _x134() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint134_t{x}; }
template <char ...digits> constexpr static auto operator "" _x135() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint135_t{x}; }
template <char ...digits> constexpr static auto operator "" _x136() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint136_t{x}; }
template <char ...digits> constexpr static auto operator "" _x137() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint137_t{x}; }
template <char ...digits> constexpr static auto operator "" _x138() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint138_t{x}; }
template <char ...digits> constexpr static auto operator "" _x139() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint139_t{x}; }
// 140--149
template <char ...digits> constexpr static auto operator "" _x140() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint140_t{x}; }
template <char ...digits> constexpr static auto operator "" _x141() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint141_t{x}; }
template <char ...digits> constexpr static auto operator "" _x142() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint142_t{x}; }
template <char ...digits> constexpr static auto operator "" _x143() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint143_t{x}; }
template <char ...digits> constexpr static auto operator "" _x144() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint144_t{x}; }
template <char ...digits> constexpr static auto operator "" _x145() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint145_t{x}; }
template <char ...digits> constexpr static auto operator "" _x146() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint146_t{x}; }
template <char ...digits> constexpr static auto operator "" _x147() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint147_t{x}; }
template <char ...digits> constexpr static auto operator "" _x148() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint148_t{x}; }
template <char ...digits> constexpr static auto operator "" _x149() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint149_t{x}; }
// 150--159
template <char ...digits> constexpr static auto operator "" _x150() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint150_t{x}; }
template <char ...digits> constexpr static auto operator "" _x151() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint151_t{x}; }
template <char ...digits> constexpr static auto operator "" _x152() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint152_t{x}; }
template <char ...digits> constexpr static auto operator "" _x153() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint153_t{x}; }
template <char ...digits> constexpr static auto operator "" _x154() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint154_t{x}; }
template <char ...digits> constexpr static auto operator "" _x155() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint155_t{x}; }
template <char ...digits> constexpr static auto operator "" _x156() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint156_t{x}; }
template <char ...digits> constexpr static auto operator "" _x157() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint157_t{x}; }
template <char ...digits> constexpr static auto operator "" _x158() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint158_t{x}; }
template <char ...digits> constexpr static auto operator "" _x159() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint159_t{x}; }
// 160--169
template <char ...digits> constexpr static auto operator "" _x160() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint160_t{x}; }
template <char ...digits> constexpr static auto operator "" _x161() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint161_t{x}; }
template <char ...digits> constexpr static auto operator "" _x162() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint162_t{x}; }
template <char ...digits> constexpr static auto operator "" _x163() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint163_t{x}; }
template <char ...digits> constexpr static auto operator "" _x164() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint164_t{x}; }
template <char ...digits> constexpr static auto operator "" _x165() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint165_t{x}; }
template <char ...digits> constexpr static auto operator "" _x166() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint166_t{x}; }
template <char ...digits> constexpr static auto operator "" _x167() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint167_t{x}; }
template <char ...digits> constexpr static auto operator "" _x168() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint168_t{x}; }
template <char ...digits> constexpr static auto operator "" _x169() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint169_t{x}; }
// 170--179
template <char ...digits> constexpr static auto operator "" _x170() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint170_t{x}; }
template <char ...digits> constexpr static auto operator "" _x171() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint171_t{x}; }
template <char ...digits> constexpr static auto operator "" _x172() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint172_t{x}; }
template <char ...digits> constexpr static auto operator "" _x173() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint173_t{x}; }
template <char ...digits> constexpr static auto operator "" _x174() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint174_t{x}; }
template <char ...digits> constexpr static auto operator "" _x175() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint175_t{x}; }
template <char ...digits> constexpr static auto operator "" _x176() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint176_t{x}; }
template <char ...digits> constexpr static auto operator "" _x177() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint177_t{x}; }
template <char ...digits> constexpr static auto operator "" _x178() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint178_t{x}; }
template <char ...digits> constexpr static auto operator "" _x179() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint179_t{x}; }
// 180--189
template <char ...digits> constexpr static auto operator "" _x180() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint180_t{x}; }
template <char ...digits> constexpr static auto operator "" _x181() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint181_t{x}; }
template <char ...digits> constexpr static auto operator "" _x182() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint182_t{x}; }
template <char ...digits> constexpr static auto operator "" _x183() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint183_t{x}; }
template <char ...digits> constexpr static auto operator "" _x184() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint184_t{x}; }
template <char ...digits> constexpr static auto operator "" _x185() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint185_t{x}; }
template <char ...digits> constexpr static auto operator "" _x186() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint186_t{x}; }
template <char ...digits> constexpr static auto operator "" _x187() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint187_t{x}; }
template <char ...digits> constexpr static auto operator "" _x188() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint188_t{x}; }
template <char ...digits> constexpr static auto operator "" _x189() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint189_t{x}; }
// 190--199
template <char ...digits> constexpr static auto operator "" _x190() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint190_t{x}; }
template <char ...digits> constexpr static auto operator "" _x191() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint191_t{x}; }
template <char ...digits> constexpr static auto operator "" _x192() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint192_t{x}; }
template <char ...digits> constexpr static auto operator "" _x193() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint193_t{x}; }
template <char ...digits> constexpr static auto operator "" _x194() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint194_t{x}; }
template <char ...digits> constexpr static auto operator "" _x195() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint195_t{x}; }
template <char ...digits> constexpr static auto operator "" _x196() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint196_t{x}; }
template <char ...digits> constexpr static auto operator "" _x197() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint197_t{x}; }
template <char ...digits> constexpr static auto operator "" _x198() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint198_t{x}; }
template <char ...digits> constexpr static auto operator "" _x199() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint199_t{x}; }
// 200--209
template <char ...digits> constexpr static auto operator "" _x200() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint200_t{x}; }
template <char ...digits> constexpr static auto operator "" _x201() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint201_t{x}; }
template <char ...digits> constexpr static auto operator "" _x202() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint202_t{x}; }
template <char ...digits> constexpr static auto operator "" _x203() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint203_t{x}; }
template <char ...digits> constexpr static auto operator "" _x204() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint204_t{x}; }
template <char ...digits> constexpr static auto operator "" _x205() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint205_t{x}; }
template <char ...digits> constexpr static auto operator "" _x206() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint206_t{x}; }
template <char ...digits> constexpr static auto operator "" _x207() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint207_t{x}; }
template <char ...digits> constexpr static auto operator "" _x208() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint208_t{x}; }
template <char ...digits> constexpr static auto operator "" _x209() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint209_t{x}; }
// 210--219
template <char ...digits> constexpr static auto operator "" _x210() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint210_t{x}; }
template <char ...digits> constexpr static auto operator "" _x211() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint211_t{x}; }
template <char ...digits> constexpr static auto operator "" _x212() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint212_t{x}; }
template <char ...digits> constexpr static auto operator "" _x213() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint213_t{x}; }
template <char ...digits> constexpr static auto operator "" _x214() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint214_t{x}; }
template <char ...digits> constexpr static auto operator "" _x215() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint215_t{x}; }
template <char ...digits> constexpr static auto operator "" _x216() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint216_t{x}; }
template <char ...digits> constexpr static auto operator "" _x217() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint217_t{x}; }
template <char ...digits> constexpr static auto operator "" _x218() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint218_t{x}; }
template <char ...digits> constexpr static auto operator "" _x219() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint219_t{x}; }
// 220--229
template <char ...digits> constexpr static auto operator "" _x220() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint220_t{x}; }
template <char ...digits> constexpr static auto operator "" _x221() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint221_t{x}; }
template <char ...digits> constexpr static auto operator "" _x222() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint222_t{x}; }
template <char ...digits> constexpr static auto operator "" _x223() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint223_t{x}; }
template <char ...digits> constexpr static auto operator "" _x224() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint224_t{x}; }
template <char ...digits> constexpr static auto operator "" _x225() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint225_t{x}; }
template <char ...digits> constexpr static auto operator "" _x226() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint226_t{x}; }
template <char ...digits> constexpr static auto operator "" _x227() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint227_t{x}; }
template <char ...digits> constexpr static auto operator "" _x228() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint228_t{x}; }
template <char ...digits> constexpr static auto operator "" _x229() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint229_t{x}; }
// 230--239
template <char ...digits> constexpr static auto operator "" _x230() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint230_t{x}; }
template <char ...digits> constexpr static auto operator "" _x231() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint231_t{x}; }
template <char ...digits> constexpr static auto operator "" _x232() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint232_t{x}; }
template <char ...digits> constexpr static auto operator "" _x233() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint233_t{x}; }
template <char ...digits> constexpr static auto operator "" _x234() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint234_t{x}; }
template <char ...digits> constexpr static auto operator "" _x235() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint235_t{x}; }
template <char ...digits> constexpr static auto operator "" _x236() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint236_t{x}; }
template <char ...digits> constexpr static auto operator "" _x237() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint237_t{x}; }
template <char ...digits> constexpr static auto operator "" _x238() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint238_t{x}; }
template <char ...digits> constexpr static auto operator "" _x239() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint239_t{x}; }
// 240--249
template <char ...digits> constexpr static auto operator "" _x240() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint240_t{x}; }
template <char ...digits> constexpr static auto operator "" _x241() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint241_t{x}; }
template <char ...digits> constexpr static auto operator "" _x242() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint242_t{x}; }
template <char ...digits> constexpr static auto operator "" _x243() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint243_t{x}; }
template <char ...digits> constexpr static auto operator "" _x244() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint244_t{x}; }
template <char ...digits> constexpr static auto operator "" _x245() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint245_t{x}; }
template <char ...digits> constexpr static auto operator "" _x246() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint246_t{x}; }
template <char ...digits> constexpr static auto operator "" _x247() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint247_t{x}; }
template <char ...digits> constexpr static auto operator "" _x248() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint248_t{x}; }
template <char ...digits> constexpr static auto operator "" _x249() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint249_t{x}; }
// 250--259
template <char ...digits> constexpr static auto operator "" _x250() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint250_t{x}; }
template <char ...digits> constexpr static auto operator "" _x251() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint251_t{x}; }
template <char ...digits> constexpr static auto operator "" _x252() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint252_t{x}; }
template <char ...digits> constexpr static auto operator "" _x253() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint253_t{x}; }
template <char ...digits> constexpr static auto operator "" _x254() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint254_t{x}; }
template <char ...digits> constexpr static auto operator "" _x255() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint255_t{x}; }
template <char ...digits> constexpr static auto operator "" _x256() { utils::constexpr_maybe_throw<std::runtime_error>(!(std::isdigit(digits) && ...), "invalid char"); uint256_t x{0}; (~((x *= 10) | (x += digits - '0')), ...); return dpf::xints::xint256_t{x}; }

}  // namespace xints

}  // namespace literals

namespace utils
{

template <typename T>
struct is_xor_wrapper<xor_wrapper<T>> : std::true_type {};

/// @brief specializes `dpf::utils::bitlength_of` for `xor_wrapper`
template <typename T>
struct bitlength_of<xor_wrapper<T>>
  : public bitlength_of<T>
{ };

template <typename T>
struct msb_of<xor_wrapper<T>>
{
    static constexpr xor_wrapper<T> value = xor_wrapper<T>{msb_of_v<T>};
};

template <typename T>
struct countl_zero_symmetric_difference<xor_wrapper<T>>
  : public countl_zero_symmetric_difference<T>
{ };

template <typename T>
struct to_integral_type<xor_wrapper<T>> : public to_integral_type_base<T>
{
    using parent = to_integral_type_base<T>;
    using typename parent::integral_type;
    static constexpr auto to_int = to_integral_type<T> {};

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr integral_type operator()(const xor_wrapper<T> & input) const noexcept
    {
        return to_int(input.value);
    }
};

template <typename T>
struct mod_pow_2<xor_wrapper<T>>
{
    static constexpr auto mod = mod_pow_2<T>{};
    std::size_t operator()(xor_wrapper<T> val, std::size_t n) const noexcept
    {
        return mod(val.value, n);
    }
};

template <typename T>
struct has_characteristic_two<xor_wrapper<T>> : public std::true_type {};

}  // namespace utils

}  // namespace dpf

namespace std
{

/// @brief specializes `std::numeric_limits` for CV-qualified `xor_wrapper`s
/// @{

/// @details specializes `std::numeric_limits` for `xor_wrapper<T>`
template<typename T>
class numeric_limits<dpf::xor_wrapper<T>>
  : public numeric_limits<dpf::utils::make_unsigned_t<T>> {};

/// @details specializes `std::numeric_limits` for `xor_wrapper<T> const`
template<typename T>
class numeric_limits<dpf::xor_wrapper<T> const>
  : public numeric_limits<dpf::xor_wrapper<T>> {};

/// @details specializes `std::numeric_limits` for
///          `xor_wrapper<T> volatile`
template<typename T>
class numeric_limits<dpf::xor_wrapper<T> volatile>
  : public numeric_limits<dpf::xor_wrapper<T>> {};

/// @details specializes `std::numeric_limits` for
///          `xor_wrapper<T> const volatile`
template<typename T>
class numeric_limits<dpf::xor_wrapper<T> const volatile>
  : public numeric_limits<dpf::xor_wrapper<T>> {};

/// @}

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_XOR_WRAPPER_HPP__
