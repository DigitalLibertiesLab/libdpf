/// @file dpf/random.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_RANDOM_HPP__
#define LIBDPF_INCLUDE_DPF_RANDOM_HPP__

#include <bsd/stdlib.h>

#include <utility>

#include "hedley/hedley.h"

namespace dpf
{

#if   defined(LIBDPF_USE_DEV_RANDOM)
    #define RANDOM_DEVICE "/dev/random"
    static FILE * random_device;
    void check_random_device() __attribute__((constructor))
    {
        random_device = fopen(RANDOM_DEVICE, "rb");
        if (random_device == nullptr) throw std::runtime_error("cannot open /dev/urandom");
    }
    void close_random_device() __attribute__((destructor))
    {
        fclose(random_device);
    }

    template <typename T>
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    auto & uniform_fill(T & buf) noexcept  // NOLINT(runtime/references)
    {
        while (fread(&buf, sizeof(buf), 1, random_device) != 1) continue;
        return buf;
    }
#elif defined(LIBDPF_USE_ARC4RANDOM)
    template <typename T>
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    auto & uniform_fill(T & buf) noexcept  // NOLINT(runtime/references)
    {
        arc4random_buf(&buf, sizeof(buf));
        return buf;
    }
#else
    #define URANDOM_DEVICE "/dev/urandom"
    static FILE * urandom_device;
    void check_urandom_device() __attribute__((constructor));
    void check_urandom_device()
    {
        urandom_device = fopen(URANDOM_DEVICE, "rb");
        if (urandom_device == nullptr) throw std::runtime_error("cannot open /dev/urandom");
    }
    void close_urandom_device() __attribute__((destructor));
    void close_urandom_device()
    {
        fclose(urandom_device);
    }

    template <typename T>
    HEDLEY_ALWAYS_INLINE
    HEDLEY_NO_THROW
    auto & uniform_fill(T & buf) noexcept  // NOLINT(runtime/references)
    {
        while (fread(&buf, sizeof(buf), 1, urandom_device) != 1) continue;
        return buf;
    }
#endif

template <typename T>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
auto uniform_sample() noexcept
{
    T buf;
    uniform_fill(buf);
    return buf;
}

template <typename T>
HEDLEY_ALWAYS_INLINE
HEDLEY_NO_THROW
auto additively_share(T secret) noexcept
{
    using T_ = std::remove_reference_t<T>;
    T_ tmp = uniform_sample<T_>();
    return std::make_pair(tmp, static_cast<T_>(secret-tmp));
}

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_RANDOM_HPP__
