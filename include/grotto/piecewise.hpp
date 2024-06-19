/// @file grotto/piecewise.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @details
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_GROTTO_PIECEWISE_HPP__
#define LIBDPF_INCLUDE_GROTTO_PIECEWISE_HPP__

#include <array>
#include <iterator>
#include <algorithm>

namespace grotto
{

namespace polynomials
{

template <typename T> using poly_constant  = std::array<T, 1>;
template <typename T> using poly_linear    = std::array<T, 2>;
template <typename T> using poly_quadratic = std::array<T, 3>;
template <typename T> using poly_cubic     = std::array<T, 4>;

template <typename T>
auto eval_horner(const poly_constant<T> & f, T x) { return f[0]; }
template <typename T>
auto eval_horner(const poly_linear<T> & f, T x) { return f[1] * x + f[0]; }
template <typename T>
auto eval_horner(const poly_quadratic<T> & f, T x) { return (f[2] * x + f[1]) * x + f[0]; }
template <typename T>
auto eval_horner(const poly_cubic<T> & f, T x) { return ((f[3] * x + f[2]) * x + f[1]) * x + f[0]; }

template <typename T, std::size_t D, std::size_t N1, std::size_t N2>
auto piecewise_eval(const std::array<std::array<T, D>, N1> & polys, const std::array<T, N2> & bounds, T x)
{
    auto it = std::upper_bound(std::cbegin(bounds), std::cend(bounds), x,
        [](const double & lhs, const double & rhs){ return static_cast<T>(lhs) < rhs; });
    auto i = std::distance(std::cbegin(bounds), it);
    return eval_horner(polys[i], x);
}

}  // namespace polynomials

}  // namespace grotto

#endif  // LIBDPF_INCLUDE_GROTTO_PIECEWISE_HPP__
