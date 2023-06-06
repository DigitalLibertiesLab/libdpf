/// @file dpf/random.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_RANDOM_HPP__
#define LIBDPF_INCLUDE_DPF_RANDOM_HPP__

#include <bsd/stdlib.h>

#include <utility>

#include "thirdparty/hedley.h"

namespace dpf
{

template <typename T>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
auto & uniform_fill(T & buf) noexcept  // NOLINT(runtime/references)
{
    arc4random_buf(&buf, sizeof(buf));
    return buf;
}

template <typename T>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
auto uniform_sample() noexcept
{
    T buf;
    return uniform_fill(buf);
}

template <typename T>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
auto additively_share(T && secret) noexcept
{
    T tmp = uniform_sample<T>();
    return std::make_pair(tmp, secret-tmp);
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_RANDOM_HPP__
