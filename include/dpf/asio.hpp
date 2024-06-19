/// @file dpf/asio.hpp
/// @brief
/// @details
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @copyright Copyright (c) 2019-2023 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_ASIO_HPP__
#define LIBDPF_INCLUDE_DPF_ASIO_HPP__

#include <asio.hpp>

#include "dpf/prg.hpp"
#include "dpf/dpf_key.hpp"

extern bool do_quickack;

using quickack = asio::detail::socket_option::boolean<IPPROTO_TCP, TCP_QUICKACK>;
quickack quickack_toggle{false};

namespace dpf
{
namespace asio
{

namespace detail
{
template <class ...> using void_t = void;
template <typename T, typename = void> struct has_lowest_layer : std::false_type {};

template <typename T> struct has_lowest_layer<T, void_t<decltype(std::declval<T>().get_lowest_layer())>> : std::true_type {};

template <typename T> static constexpr bool has_lowest_layer_v = has_lowest_layer<T>::value;
}

template <typename ExecutorT,
          typename Function,
          typename CompletionToken>
auto async_post(ExecutorT executor, Function && func, CompletionToken && token)
{
#include <asio/yield.hpp>
    return ::asio::async_compose<CompletionToken, void()>(
        [
            executor,
            func = std::move(func)
        ]
        (auto & self)
        mutable
        {
            ::asio::post(executor,
            [
                func = std::move(func),
                self = std::move(self)
            ]
            ()
            mutable
            {
                func();
                self.complete();
            });
        }, token, executor);
#include <asio/unyield.hpp>
}

//
// make_dpf
//

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          typename PeerT,
          typename InputT,
          typename OutputT,
          typename ...OutputTs>
auto make_dpf(PeerT & peer0, PeerT & peer1, std::size_t count, dpfargs<InputT, OutputT, OutputTs...> & args, ::asio::error_code & error, root_sampler_t<InteriorPRG> && root_sampler = dpf::uniform_sample<typename InteriorPRG::block_type>)
{
    using dpf_type = utils::dpf_type_t<InteriorPRG, ExteriorPRG, InputT, OutputT, OutputTs...>;
    using correction_words_array = typename dpf_type::correction_words_array;
    using correction_advice_array = typename dpf_type::correction_advice_array;
    using interior_node = typename dpf_type::interior_node;
    using leaf_tuple = typename dpf_type::leaf_tuple;
    using beaver_tuple = typename dpf_type::beaver_tuple;
    using input_type = typename dpf_type::input_type;

    std::size_t bytes_written0 = 0, bytes_written1 = 0;
    for (std::size_t num_written = 0; num_written < count; ++num_written)
    {
        auto [correction_words, correction_advice, priv0, priv1]
            = dpf::detail::make_dpf_impl<InteriorPRG, ExteriorPRG>(args, std::forward<root_sampler_t<InteriorPRG>>(root_sampler));
        auto & [root0, leaves0, beavers0, offset_share0] = priv0;
        auto & [root1, leaves1, beavers1, offset_share1] = priv1;

        bytes_written0 += ::asio::write(peer0,
            std::array<::asio::const_buffer, 6>{
                ::asio::buffer(&correction_words,  sizeof(correction_words_array)),
                ::asio::buffer(&correction_advice, sizeof(correction_advice_array)),
                ::asio::buffer(&root0,             sizeof(interior_node)),
                ::asio::buffer(&leaves0,           sizeof(leaf_tuple)),
                ::asio::buffer(&beavers0,          sizeof(beaver_tuple)),
                ::asio::buffer(&offset_share0,     sizeof(input_type))
            }, error);
        if (error)
        {
            return std::make_tuple(bytes_written0, bytes_written1, num_written);
        }
        
        bytes_written1 += ::asio::write(peer1,
            std::array<::asio::const_buffer, 6>{
                ::asio::buffer(&correction_words,  sizeof(correction_words_array)),
                ::asio::buffer(&correction_advice, sizeof(correction_advice_array)),
                ::asio::buffer(&root1,             sizeof(interior_node)),
                ::asio::buffer(&leaves1,           sizeof(leaf_tuple)),
                ::asio::buffer(&beavers1,          sizeof(beaver_tuple)),
                ::asio::buffer(&offset_share1,     sizeof(input_type))
            }, error);
        if (error)
        {
            return std::make_tuple(bytes_written0, bytes_written1, num_written);
        }
    }
    return std::make_tuple(bytes_written0, bytes_written1, count);
}

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          typename PeerT,
          typename InputT,
          typename OutputT,
          typename ...OutputTs>
HEDLEY_ALWAYS_INLINE
auto make_dpf(PeerT & peer0, PeerT & peer1, std::size_t count, dpfargs<InputT, OutputT, OutputTs...> & args, root_sampler_t<InteriorPRG> && root_sampler = dpf::uniform_sample<typename InteriorPRG::block_type>)
{
    ::asio::error_code error{};
    auto ret = dpf::asio::make_dpf(peer0, peer1, count, args, error, std::forward<root_sampler_t<InteriorPRG>>(root_sampler));
    if (error) throw error;
    return ret;
}

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          typename PeerT,
          typename InputT,
          typename OutputT,
          typename ...OutputTs>
HEDLEY_ALWAYS_INLINE
auto make_dpf(PeerT & peer0, PeerT & peer1, dpfargs<InputT, OutputT, OutputTs...> args, ::asio::error_code & error, root_sampler_t<InteriorPRG> && root_sampler = dpf::uniform_sample<typename InteriorPRG::block_type>)
{
    auto [bytes_written0, bytes_written1, num_written]
        = dpf::asio::make_dpf<InteriorPRG, ExteriorPRG>(peer0, peer1, static_cast<std::size_t>(1), args, error, std::forward<root_sampler_t<InteriorPRG>>(root_sampler));
    return std::make_tuple(bytes_written0, bytes_written1);
}

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          typename PeerT,
          typename InputT,
          typename OutputT,
          typename ...OutputTs>
HEDLEY_ALWAYS_INLINE
auto make_dpf(PeerT & peer0, PeerT & peer1, dpfargs<InputT, OutputT, OutputTs...> args, root_sampler_t<InteriorPRG> && root_sampler = dpf::uniform_sample<typename InteriorPRG::block_type>)
{
    auto [bytes_written0, bytes_written1, num_written]
        = dpf::asio::make_dpf<InteriorPRG, ExteriorPRG>(peer0, peer1, static_cast<std::size_t>(1), args, std::forward<root_sampler_t<InteriorPRG>>(root_sampler));
    return std::make_tuple(bytes_written0, bytes_written1);
}

//
// async_make_dpf
//

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          typename PeerT,
          typename ExecutorT,
          typename InputT,
          typename OutputT,
          typename ...OutputTs,
          typename CompletionToken>
[[nodiscard]]
auto async_make_dpf(PeerT & peer0, PeerT & peer1, ExecutorT work_executor,
    std::size_t count, dpfargs<InputT, OutputT, OutputTs...> args,
    CompletionToken && token, root_sampler_t<InteriorPRG> && root_sampler = dpf::uniform_sample<typename InteriorPRG::block_type>)
{
    using dpf_type = utils::dpf_type_t<InteriorPRG, ExteriorPRG, InputT, OutputT, OutputTs...>;
    using correction_words_array = typename dpf_type::correction_words_array;
    using correction_advice_array = typename dpf_type::correction_advice_array;
    using interior_node = typename dpf_type::interior_node;
    using leaf_tuple = typename dpf_type::leaf_tuple;
    using beaver_tuple = typename dpf_type::beaver_tuple;
    using input_type = typename dpf_type::input_type;

    using dpf_priv_values = std::tuple<interior_node, leaf_tuple, beaver_tuple, input_type>;
    using dpf_values = std::tuple<correction_words_array, correction_advice_array, dpf_priv_values, dpf_priv_values>;

#include <asio/yield.hpp>
    return ::asio::async_compose<
        CompletionToken, void(::asio::error_code,  // error status
                              std::size_t,         // bytes_written0
                              std::size_t,         // bytes_written1
                              std::size_t)>(       // num_written
            [
                &peer0,
                &peer1,
                work_executor,
                args,
                count = std::size_t(count),
                dpf_data = std::make_shared<dpf_values>(),
                num_written = std::size_t(0),
                bytes_written0 = std::size_t(0),
                bytes_written1 = std::size_t(0),
                coro = ::asio::coroutine()
            ]
            (
                auto & self,
                const ::asio::error_code & error = {},
                std::size_t bytes_just_written = 0
            )
            mutable
            {
                reenter (coro)
                {
                    while (num_written++ < count)
                    {
                        yield async_post(work_executor, [dpf_data, args]()
                        {
                            *dpf_data = dpf::detail::make_dpf_impl<InteriorPRG, ExteriorPRG>(args);
                        }, std::move(self));

                        yield ::asio::async_write(peer0, std::array<::asio::const_buffer, 6>{
                            ::asio::buffer(&utils::get<0>(*dpf_data), sizeof(correction_words_array)),
                            ::asio::buffer(&utils::get<1>(*dpf_data), sizeof(correction_advice_array)),
                            ::asio::buffer(&utils::get<0>(utils::get<2>(*dpf_data)), sizeof(interior_node)),
                            ::asio::buffer(&utils::get<1>(utils::get<2>(*dpf_data)), sizeof(leaf_tuple)),
                            ::asio::buffer(&utils::get<2>(utils::get<2>(*dpf_data)), sizeof(beaver_tuple)),
                            ::asio::buffer(&utils::get<3>(utils::get<2>(*dpf_data)), sizeof(input_type))},
                            std::move(self));

                        bytes_written0 += bytes_just_written;

                        if (error)
                        {
                            self.complete(error, bytes_written0, bytes_written1, num_written);
                            break;
                        }

                        yield ::asio::async_write(peer1, std::array<::asio::const_buffer, 6>{
                            ::asio::buffer(&utils::get<0>(*dpf_data), sizeof(correction_words_array)),
                            ::asio::buffer(&utils::get<1>(*dpf_data), sizeof(correction_advice_array)),
                            ::asio::buffer(&utils::get<0>(utils::get<3>(*dpf_data)), sizeof(interior_node)),
                            ::asio::buffer(&utils::get<1>(utils::get<3>(*dpf_data)), sizeof(leaf_tuple)),
                            ::asio::buffer(&utils::get<2>(utils::get<3>(*dpf_data)), sizeof(beaver_tuple)),
                            ::asio::buffer(&utils::get<3>(utils::get<3>(*dpf_data)), sizeof(input_type))},
                            std::move(self));

                        bytes_written1 += bytes_just_written;

                        if (error)
                        {
                            self.complete(error, bytes_written0, bytes_written1, num_written);
                            break;
                        }
                    }
                    self.complete(error, bytes_written0, bytes_written1, count);
                }
            },
        token, peer0, peer1, work_executor);
#include <asio/unyield.hpp>
}

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          typename PeerT,
          typename ExecutorT,
          typename InputT,
          typename OutputT,
          typename ...OutputTs,
          typename CompletionToken,
          std::enable_if_t<!std::is_integral_v<ExecutorT>, bool> = false>
[[nodiscard]]
HEDLEY_ALWAYS_INLINE
auto async_make_dpf(PeerT & peer0, PeerT & peer1, ExecutorT work_executor,
    dpfargs<InputT, OutputT, OutputTs...> args, CompletionToken && token,
    root_sampler_t<InteriorPRG> && root_sampler = dpf::uniform_sample<typename InteriorPRG::block_type>)
{
    std::size_t count = 1;
    return async_make_dpf<InteriorPRG, ExteriorPRG>(peer0, peer1, work_executor, count, args, std::forward<CompletionToken>(token), std::forward<root_sampler_t<InteriorPRG>>(root_sampler));
}

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          typename PeerT,
          typename InputT,
          typename OutputT,
          typename ...OutputTs,
          typename CompletionToken>
[[nodiscard]]
HEDLEY_ALWAYS_INLINE
auto async_make_dpf(PeerT & peer0, PeerT & peer1, std::size_t count, dpfargs<InputT, OutputT, OutputTs...> args, CompletionToken && token,
    root_sampler_t<InteriorPRG> && root_sampler = dpf::uniform_sample<typename InteriorPRG::block_type>)
{
    auto work_executor = ::asio::system_executor();
    return async_make_dpf<InteriorPRG, ExteriorPRG>(peer0, peer1, work_executor, count, args, std::forward<CompletionToken>(token), std::forward<root_sampler_t<InteriorPRG>>(root_sampler));
}

template <typename InteriorPRG = dpf::prg::aes128,
          typename ExteriorPRG = InteriorPRG,
          typename PeerT,
          typename InputT,
          typename OutputT,
          typename ...OutputTs,
          typename CompletionToken>
[[nodiscard]]
HEDLEY_ALWAYS_INLINE
auto async_make_dpf(PeerT & peer0, PeerT & peer1, dpfargs<InputT, OutputT, OutputTs...> args, CompletionToken && token,
    root_sampler_t<InteriorPRG> && root_sampler = dpf::uniform_sample<typename InteriorPRG::block_type>)
{
    auto work_executor = ::asio::system_executor();
    std::size_t count = 1;
    return dpf::asio::async_make_dpf<InteriorPRG, ExteriorPRG>(peer0, peer1, work_executor, count, args, std::forward<CompletionToken>(token), std::forward<root_sampler_t<InteriorPRG>>(root_sampler));
}

//
// read_dpf
//

template <typename DpfKey,
          typename DealerT,
          typename BackEmplaceable>
auto read_dpf(DealerT & dealer, BackEmplaceable & output, std::size_t count, ::asio::error_code & error)
{
    using dpf_type = DpfKey;
    using interior_node = typename dpf_type::interior_node;
    using leaf_tuple = typename dpf_type::leaf_tuple;
    using beaver_tuple = typename dpf_type::beaver_tuple;
    using input_type = typename dpf_type::input_type;
    using correction_words_array = typename dpf_type::correction_words_array;
    using correction_advice_array = typename dpf_type::correction_advice_array;

    interior_node root;
    correction_words_array correction_words;
    correction_advice_array correction_advice;
    leaf_tuple leaves;
    beaver_tuple beavers;
    input_type offset_share;

    std::size_t bytes_read = 0;
    for (std::size_t num_read = 0; num_read < count; ++num_read)
    {
        bytes_read += ::asio::read(dealer,
            std::array<::asio::mutable_buffer, 6>{
                ::asio::buffer(&correction_words,  sizeof(correction_words_array)),
                ::asio::buffer(&correction_advice, sizeof(correction_advice_array)),
                ::asio::buffer(&root,              sizeof(interior_node)),
                ::asio::buffer(&leaves,            sizeof(leaf_tuple)),
                ::asio::buffer(&beavers,           sizeof(beaver_tuple)),
                ::asio::buffer(&offset_share,      sizeof(input_type))
            }, error);
            if constexpr(detail::has_lowest_layer_v<DealerT>)
            {
                if (do_quickack) dealer.get_lowest_layer().set_option(quickack_toggle);
            }

        if (error)
        {
            return std::make_pair(bytes_read, num_read);
        }

        dpf_type::emplace_back(output, root, correction_words, correction_advice, leaves, beavers, offset_share);
    }

    return std::make_pair(bytes_read, count);
}

template <typename DpfKey,
          typename DealerT,
          typename BackEmplaceable>
HEDLEY_ALWAYS_INLINE
auto read_dpf(DealerT & dealer, BackEmplaceable & output, std::size_t count)
{
    ::asio::error_code error{};
    auto ret = dpf::asio::read_dpf<DpfKey>(dealer, output, count, error);
    if constexpr(detail::has_lowest_layer_v<DealerT>)
    {
        if (do_quickack) dealer.get_lowest_layer().set_option(quickack_toggle);
    }
    if (error) throw error;
    return ret;
}

template <typename DpfKey,
          typename DealerT,
          typename Emplaceable>
auto read_dpf(DealerT & dealer, Emplaceable & output, ::asio::error_code & error)
{
    using dpf_type = DpfKey;
    using interior_node = typename dpf_type::interior_node;
    using leaf_tuple = typename dpf_type::leaf_tuple;
    using beaver_tuple = typename dpf_type::beaver_tuple;
    using input_type = typename dpf_type::input_type;
    using correction_words_array = typename dpf_type::correction_words_array;
    using correction_advice_array = typename dpf_type::correction_advice_array;

    interior_node root;
    correction_words_array correction_words;
    correction_advice_array correction_advice;
    leaf_tuple leaves;
    beaver_tuple beavers;
    input_type offset_share;

    std::size_t bytes_read = ::asio::read(dealer,
        std::array<::asio::mutable_buffer, 6>{
            ::asio::buffer(&correction_words,  sizeof(correction_words_array)),
            ::asio::buffer(&correction_advice, sizeof(correction_advice_array)),
            ::asio::buffer(&root,              sizeof(interior_node)),
            ::asio::buffer(&leaves,            sizeof(leaf_tuple)),
            ::asio::buffer(&beavers,           sizeof(beaver_tuple)),
            ::asio::buffer(&offset_share,      sizeof(input_type))
        }, error);
        if constexpr(detail::has_lowest_layer_v<DealerT>)
        {
            if (do_quickack) dealer.get_lowest_layer().set_option(quickack_toggle);
        }

    if (error)
    {
        return bytes_read;
    }

    dpf_type::emplace(output, root, correction_words, correction_advice, leaves, beavers, offset_share);

    return bytes_read;
}

template <typename DpfKey,
          typename DealerT,
          typename Emplaceable>
HEDLEY_ALWAYS_INLINE
auto read_dpf(DealerT & dealer, Emplaceable & output)
{
    ::asio::error_code error{};
    auto ret = dpf::asio::read_dpf<DpfKey>(dealer, output, error);
    if constexpr(detail::has_lowest_layer_v<DealerT>)
    {
        if (do_quickack) dealer.get_lowest_layer().set_option(quickack_toggle);
    }
    if (error) throw error;
    return ret;
}

//
// async_read_dpf
//

template <typename DpfKey,
          typename DealerT,
          typename ExecutorT,
          typename BackEmplaceable,
          typename CompletionToken>
[[nodiscard]]
auto async_read_dpf(DealerT & dealer, ExecutorT work_executor, BackEmplaceable & output, std::size_t count, CompletionToken && token)
{
    using dpf_type = DpfKey;
    using correction_words_array = typename dpf_type::correction_words_array;
    using correction_advice_array = typename dpf_type::correction_advice_array;
    using interior_node = typename dpf_type::interior_node;
    using leaf_tuple = typename dpf_type::leaf_tuple;
    using beaver_tuple = typename dpf_type::beaver_tuple;
    using input_type = typename dpf_type::input_type;

    using dpf_priv_values = std::tuple<interior_node, leaf_tuple, beaver_tuple, input_type>;
    using dpf_values = std::tuple<correction_words_array, correction_advice_array, dpf_priv_values>;

#include <asio/yield.hpp>
    return ::asio::async_compose<
        CompletionToken, void(::asio::error_code,  // error status
                              std::size_t,         // bytes_read
                              std::size_t)>(       // num_read
            [
                &dealer,
                work_executor,
                &output,
                count,
                dpf_data = std::make_shared<dpf_values>(),
                num_read = std::size_t(0),
                bytes_read = std::size_t(0),
                coro = ::asio::coroutine()
            ]
            (
                auto & self,
                const ::asio::error_code & error = {},
                std::size_t bytes_just_read = 0
            )
            mutable
            {
                reenter (coro)
                {
                    while (num_read++ < count)
                    {
                        yield ::asio::async_read(dealer, std::array<::asio::mutable_buffer, 6>{
                            ::asio::buffer(&std::get<0>(*dpf_data), sizeof(correction_words_array)),
                            ::asio::buffer(&std::get<1>(*dpf_data), sizeof(correction_advice_array)),
                            ::asio::buffer(&std::get<0>(std::get<2>(*dpf_data)), sizeof(interior_node)),
                            ::asio::buffer(&std::get<1>(std::get<2>(*dpf_data)), sizeof(leaf_tuple)),
                            ::asio::buffer(&std::get<2>(std::get<2>(*dpf_data)), sizeof(beaver_tuple)),
                            ::asio::buffer(&std::get<3>(std::get<2>(*dpf_data)), sizeof(input_type))}, std::move(self));

                        if constexpr(detail::has_lowest_layer_v<DealerT>)
                        {
                            if (do_quickack) dealer.get_lowest_layer().set_option(quickack_toggle);
                        }

                        bytes_read += bytes_just_read;

                        if (error)
                        {
                            self.complete(error, bytes_read, num_read);
                            break;
                        }

                        yield async_post(work_executor, [dpf_data, &output]() mutable
                        {
                            auto & [correction_words, correction_advice, priv]
                                = *dpf_data;
                            auto & [root, leaves, beavers, offset_share] = priv;

                            dpf_type::emplace_back(output, root, correction_words, correction_advice, leaves, beavers, offset_share);
                        }, std::move(self));
                    }
                    self.complete(error, bytes_read, count);
                }
            },
        token, dealer, work_executor);
#include <asio/unyield.hpp>
}

template <typename DpfKey,
          typename DealerT,
          typename ExecutorT,
          typename Emplaceable,
          typename CompletionToken>
[[nodiscard]]
auto async_read_dpf(DealerT & dealer, ExecutorT work_executor, Emplaceable & output, CompletionToken && token)
{
    using dpf_type = DpfKey;
    using correction_words_array = typename dpf_type::correction_words_array;
    using correction_advice_array = typename dpf_type::correction_advice_array;
    using interior_node = typename dpf_type::interior_node;
    using leaf_tuple = typename dpf_type::leaf_tuple;
    using beaver_tuple = typename dpf_type::beaver_tuple;
    using input_type = typename dpf_type::input_type;

    using dpf_priv_values = std::tuple<interior_node, leaf_tuple, beaver_tuple, input_type>;
    using dpf_values = std::tuple<correction_words_array, correction_advice_array, dpf_priv_values>;

#include <asio/yield.hpp>
    return ::asio::async_compose<
        CompletionToken, void(::asio::error_code,  // error status
                              std::size_t)>(       // bytes_read
            [
                &dealer,
                work_executor,
                &output,
                dpf_data = std::make_shared<dpf_values>(),
                bytes_read = std::size_t(0),
                coro = ::asio::coroutine()
            ]
            (
                auto & self,
                const ::asio::error_code & error = {},
                std::size_t bytes_just_read = 0
            )
            mutable
            {
                reenter (coro)
                {
                    yield ::asio::async_read(dealer, std::array<::asio::mutable_buffer, 6>{
                        ::asio::buffer(&std::get<0>(*dpf_data), sizeof(correction_words_array)),
                        ::asio::buffer(&std::get<1>(*dpf_data), sizeof(correction_advice_array)),
                        ::asio::buffer(&std::get<0>(std::get<2>(*dpf_data)), sizeof(interior_node)),
                        ::asio::buffer(&std::get<1>(std::get<2>(*dpf_data)), sizeof(leaf_tuple)),
                        ::asio::buffer(&std::get<2>(std::get<2>(*dpf_data)), sizeof(beaver_tuple)),
                        ::asio::buffer(&std::get<3>(std::get<2>(*dpf_data)), sizeof(input_type))}, std::move(self));

                    if constexpr(detail::has_lowest_layer_v<DealerT>)
                    {
                        if (do_quickack) dealer.get_lowest_layer().set_option(quickack_toggle);
                    }

                    bytes_read = bytes_just_read;

                    if (error)
                    {
                        self.complete(error, bytes_read);
                        break;
                    }

                    yield async_post(work_executor, [dpf_data, &output]()
                    {
                        auto & [correction_words, correction_advice, priv]
                            = *dpf_data;
                        auto & [root, leaves, beavers, offset_share] = priv;

                        dpf_type::emplace(output,
                            std::get<0>(std::get<2>(*dpf_data)),
                            std::get<0>(*dpf_data),
                            std::get<1>(*dpf_data),
                            std::get<1>(std::get<2>(*dpf_data)),
                            std::get<2>(std::get<2>(*dpf_data)),
                            std::get<3>(std::get<2>(*dpf_data)));
                    }, std::move(self));

                    self.complete(error, bytes_read);
                }
            },
        token, dealer, work_executor);
#include <asio/unyield.hpp>
}

template <typename DpfKey,
          typename DealerT,
          typename BackEmplaceable,
          typename CompletionToken>
[[nodiscard]]
HEDLEY_ALWAYS_INLINE
auto async_read_dpf(DealerT & dealer, BackEmplaceable & output, std::size_t count, CompletionToken && token)
{
    auto work_executor = ::asio::system_executor();
    return async_read_dpf<DpfKey>(dealer, work_executor, output, count, std::forward<CompletionToken>(token));
}

template <typename DpfKey,
          typename DealerT,
          typename Emplaceable,
          typename CompletionToken>
[[nodiscard]]
HEDLEY_ALWAYS_INLINE
auto async_read_dpf(DealerT & dealer, Emplaceable & output, CompletionToken && token)
{
    auto work_executor = ::asio::system_executor();
    return dpf::asio::async_read_dpf<DpfKey>(dealer, work_executor, output, std::forward<CompletionToken>(token));
}

//
// assign_wildcard_input
//

template <typename PeerT,
          typename DpfKey,
          typename InputType>
auto assign_wildcard_input(PeerT & peer_in, PeerT & peer_out, DpfKey & dpf,
    InputType && input_share, ::asio::error_code & error)
{
    using input_type = typename DpfKey::input_type;
    static_assert(std::is_convertible_v<InputType, input_type>);
    std::size_t bytes_written = 0, bytes_read = 0;

    input_type offset_share = dpf.offset_x.compute_and_get_share(input_share);
    bytes_written = ::asio::write(peer_out, ::asio::buffer(&offset_share, sizeof(input_type)), error);
    if (error)
    {
        return std::make_tuple(offset_share, bytes_written, bytes_read);
    }
	bytes_read = ::asio::read(peer_in, ::asio::buffer(&offset_share, sizeof(input_type)), error);
    if constexpr(detail::has_lowest_layer_v<PeerT>)
    {
        if (do_quickack) peer_in.get_lowest_layer().set_option(quickack_toggle);
    }
    if (!error)
    {
        offset_share = dpf.offset_x.reconstruct(offset_share);
    }

    return std::make_tuple(offset_share, bytes_written, bytes_read);
}

template <typename PeerT,
          typename DpfKey,
          typename InputType>
HEDLEY_ALWAYS_INLINE
auto assign_wildcard_input(PeerT & peer, DpfKey & dpf, InputType && input_share,
    ::asio::error_code & error)
{
    return dpf::asio::assign_wildcard_input(peer, peer, dpf,
        std::forward<InputType>(input_share), error);
}

template <typename PeerT,
          typename DpfKey,
          typename InputType>
HEDLEY_ALWAYS_INLINE
auto assign_wildcard_input(PeerT & peer_in, PeerT & peer_out, DpfKey & dpf,
    InputType && input_share)
{
    ::asio::error_code error{};
    auto ret = dpf::asio::assign_wildcard_input(peer_in, peer_out, dpf,
        std::forward<InputType>(input_share), error);
    if (error) throw error;
    return ret;
}

template <typename PeerT,
          typename DpfKey,
          typename InputType>
HEDLEY_ALWAYS_INLINE
auto assign_wildcard_input(PeerT & peer, DpfKey & dpf, InputType && input_share)
{
    ::asio::error_code error{};
    auto ret = dpf::asio::assign_wildcard_input(peer, dpf,
        std::forward<InputType>(input_share), error);
    if (error) throw error;
    return ret;
}

//
// async_assign_wildcard_input
//

template <typename PeerT,
          typename ExecutorT,
          typename DpfKey,
          typename InputType,
          typename CompletionToken>
[[nodiscard]]
auto async_assign_wildcard_input(PeerT & peer_in, PeerT & peer_out,
    ExecutorT work_executor, DpfKey & dpf, InputType && input_share,
    CompletionToken && token)
{
    using input_type = typename DpfKey::input_type;
    static_assert(std::is_convertible_v<InputType, input_type>);

#include <asio/yield.hpp>
    return ::asio::async_compose<
        CompletionToken, void(::asio::error_code,  // error status
                              input_type,          // offset
                              std::size_t,         // bytes_written
                              std::size_t)>(       // bytes_read
            [
                &peer_in,
                &peer_out,
                work_executor,
                &dpf,
                offset_share = std::make_shared<input_type>(input_share),
                bytes_written = std::size_t(0),
                bytes_read = std::size_t(0),
                coro = ::asio::coroutine()
            ]
            (
                auto & self,
                const ::asio::error_code & error = {},
                std::size_t bytes_just_transmitted = 0
            )
            mutable
            {
                reenter (coro)
                {
                    yield async_post(work_executor, [&dpf, offset_share]()
                    {
                        *offset_share = dpf.offset_x.compute_and_get_share(*offset_share);
                    }, std::move(self));

                    yield ::asio::async_write(peer_out,
                        ::asio::buffer(offset_share.get(), sizeof(input_type)), std::move(self));
                    
                    bytes_written = bytes_just_transmitted;

                    if (error)
                    {
                        self.complete(error, *offset_share, bytes_written, bytes_read);
                        break;
                    }

                    yield ::asio::async_read(peer_in,
                        ::asio::buffer(offset_share.get(), sizeof(input_type)), std::move(self));
                    if constexpr(detail::has_lowest_layer_v<PeerT>)
                    {
                        if (do_quickack) peer_in.get_lowest_layer().set_option(quickack_toggle);
                    }

                    bytes_read = bytes_just_transmitted;

                    if (error)
                    {
                        self.complete(error, *offset_share, bytes_written, bytes_read);
                        break;
                    }

                    yield async_post(work_executor, [&dpf, offset_share]() mutable
                    {
                        *offset_share = dpf.offset_x.reconstruct(*offset_share);
                    }, std::move(self));
                   self.complete(error, *offset_share, bytes_written, bytes_read);
                }
            }, token, peer_in, peer_out, work_executor);
#include <asio/unyield.hpp>
}

template <typename PeerT,
          typename ExecutorT,
          typename DpfKey,
          typename InputType,
          typename CompletionToken>
[[nodiscard]]
HEDLEY_ALWAYS_INLINE
auto async_assign_wildcard_input(PeerT & peer, ExecutorT work_executor,
    DpfKey & dpf, InputType && input_share, CompletionToken && token)
{
    return async_assign_wildcard_input(peer, peer, work_executor, dpf,
        std::forward<InputType>(input_share),
        std::forward<CompletionToken>(token));
}

template <typename PeerT,
          typename DpfKey,
          typename InputType,
          typename CompletionToken>
[[nodiscard]]
HEDLEY_ALWAYS_INLINE
auto async_assign_wildcard_input(PeerT & peer_in, PeerT & peer_out,
    DpfKey & dpf, InputType && input_share, CompletionToken && token)
{
    auto work_executor = ::asio::system_executor();
    return async_assign_wildcard_input(peer_in, peer_out, work_executor, dpf,
        std::forward<InputType>(input_share),
        std::forward<CompletionToken>(token));
}

template <typename PeerT,
          typename DpfKey,
          typename InputType,
          typename CompletionToken>
[[nodiscard]]
HEDLEY_ALWAYS_INLINE
auto async_assign_wildcard_input(PeerT & peer, DpfKey & dpf, InputType && input_share, CompletionToken && token)
{
    auto work_executor = ::asio::system_executor();
    return async_assign_wildcard_input(peer, peer, work_executor, dpf,
        std::forward<InputType>(input_share),
        std::forward<CompletionToken>(token));
}

//
// assign_wildcard_output
//

template <std::size_t I = 0,
          typename PeerT,
          typename DpfKey,
          typename OutputType>
auto assign_wildcard_output(PeerT & peer_in, PeerT & peer_out, DpfKey & dpf,
    OutputType && output_share, ::asio::error_code & error)
{
    using dpf_type = DpfKey;
    using leaf_type = std::tuple_element_t<I, typename dpf_type::leaf_tuple>;
    using output_type = typename dpf_type::concrete_output_type<I>;
    static_assert(std::is_convertible_v<OutputType, output_type>);
    std::size_t bytes_written = 0, bytes_read = 0;

    leaf_type leaf_share;
    constexpr bool is_packed = true;

    auto & leaf_wrapper = utils::get<I>(dpf.leaf_nodes);
    
    auto blinded_output = leaf_wrapper.compute_and_get_blinded_output_share(output_share);
    bytes_written += ::asio::write(peer_out, ::asio::buffer(&blinded_output, sizeof(output_type)), error);
    if (error)
    {
        return std::make_tuple(leaf_share, bytes_written, bytes_read);
    }
    bytes_read += ::asio::read(peer_in, ::asio::buffer(&blinded_output, sizeof(output_type)), error);
    if constexpr(detail::has_lowest_layer_v<PeerT>)
    {
        if (do_quickack) peer_in.get_lowest_layer().set_option(quickack_toggle);
    }
    if (error)
    {
        return std::make_tuple(leaf_share, bytes_written, bytes_read);
    }

    leaf_share = leaf_wrapper.compute_and_get_leaf_share(blinded_output);

    bytes_written += ::asio::write(peer_out, ::asio::buffer(&leaf_share, sizeof(leaf_type)), error);
    if (error)
    {
        return std::make_tuple(leaf_share, bytes_written, bytes_read);
    }
    bytes_read += ::asio::read(peer_in, ::asio::buffer(&leaf_share, sizeof(leaf_type)), error);
    if constexpr(detail::has_lowest_layer_v<PeerT>)
    {
        if (do_quickack) peer_in.get_lowest_layer().set_option(quickack_toggle);
    }
    if (!error)
    {
        leaf_share = leaf_wrapper.reconstruct_correction_word(leaf_share);
    }

    return std::make_tuple(leaf_share, bytes_written, bytes_read);
}

template <std::size_t I = 0,
          typename PeerT,
          typename DpfKey,
          typename OutputType>
HEDLEY_ALWAYS_INLINE
auto assign_wildcard_output(PeerT & peer, DpfKey & dpf,
    OutputType && output_share, ::asio::error_code & error)
{
    return assign_wildcard_output<I>(peer, peer, dpf,
        std::forward<OutputType>(output_share), error);
}

template <std::size_t I = 0,
          typename PeerT,
          typename DpfKey,
          typename OutputType>
HEDLEY_ALWAYS_INLINE
auto assign_wildcard_output(PeerT & peer_in, PeerT & peer_out, DpfKey & dpf,
    OutputType && output_share)
{
    ::asio::error_code error{};
    auto ret = dpf::asio::assign_wildcard_output<I>(peer_in, peer_out, dpf,
        std::forward<OutputType>(output_share), error);
    if (error) throw error;
    return ret;
}

template <std::size_t I = 0,
          typename PeerT,
          typename DpfKey,
          typename OutputType>
HEDLEY_ALWAYS_INLINE
auto assign_wildcard_output(PeerT & peer, DpfKey & dpf, OutputType && output_share)
{
    ::asio::error_code error{};
    auto ret = dpf::asio::assign_wildcard_output<I>(peer, peer, dpf,
        std::forward<OutputType>(output_share), error);
    if (error) throw error;
    return ret;
}

//
// async_assign_wildcard_output
//

template <std::size_t I = 0,
          typename PeerT,
          typename ExecutorT,
          typename DpfKey,
          typename OutputType,
          typename CompletionToken>
[[nodiscard]]
auto async_assign_wildcard_output(PeerT & peer_in, PeerT & peer_out,
    ExecutorT work_executor, DpfKey & dpf, OutputType && output_share,
    CompletionToken && token)
{
    using leaf_type = std::tuple_element_t<I, typename DpfKey::leaf_tuple>;
    using output_type = typename DpfKey::concrete_output_type<I>;
    static_assert(std::is_convertible_v<OutputType, output_type>);

#include <asio/yield.hpp>
    return ::asio::async_compose<
        CompletionToken, void(::asio::error_code,  // error status
                              leaf_type,           // assigned leaf
                              std::size_t,         // bytes_written
                              std::size_t)>(       // bytes_read
            [
                &peer_in,
                &peer_out,
                work_executor,
                &leaf = utils::get<I>(dpf.leaf_nodes),
                &dpf,
                output_share = std::make_shared<output_type>(output_share),
                leaf_share = std::make_shared<leaf_type>(),
                bytes_written= std::size_t(0),
                bytes_read= std::size_t(0),
                coro = ::asio::coroutine()
            ]
            (
                auto & self,
                const ::asio::error_code & error = {},
                std::size_t bytes_just_transmitted = 0
            )
            mutable
            {
                reenter (coro)
                {
                    yield async_post(work_executor, [&leaf, output_share]() mutable
                    {
                        *output_share = leaf.compute_and_get_blinded_output_share(*output_share);
                    }, std::move(self));

                    yield ::asio::async_write(peer_out,
                        ::asio::buffer(output_share.get(), sizeof(output_type)), std::move(self));
                    
                    bytes_written = bytes_just_transmitted;

                    if (error)
                    {
                        self.complete(error, *leaf_share, bytes_written, bytes_read);
                        break;
                    }

                    yield ::asio::async_read(peer_in,
                        ::asio::buffer(output_share.get(), sizeof(output_type)), std::move(self));
                    if constexpr(detail::has_lowest_layer_v<PeerT>)
                    {
                        if (do_quickack) peer_in.get_lowest_layer().set_option(quickack_toggle);
                    }

                    bytes_read = bytes_just_transmitted;

                    if (error)
                    {
                        self.complete(error, *leaf_share, bytes_written, bytes_read);
                        break;
                    }

                    yield async_post(work_executor, [&leaf, leaf_share, output_share]() mutable
                    {
                        *leaf_share = leaf.compute_and_get_leaf_share(*output_share);
                    }, std::move(self));

                    yield ::asio::async_write(peer_out,
                        ::asio::buffer(leaf_share.get(), sizeof(leaf_type)), std::move(self));
                    
                    bytes_written += bytes_just_transmitted;

                    if (error)
                    {
                        self.complete(error, *leaf_share, bytes_written, bytes_read);
                        break;
                    }

                    yield ::asio::async_read(peer_in,
                        ::asio::buffer(leaf_share.get(), sizeof(leaf_type)), std::move(self));
                    if constexpr(detail::has_lowest_layer_v<PeerT>)
                    {
                        if (do_quickack) peer_in.get_lowest_layer().set_option(quickack_toggle);
                    }

                    bytes_read += bytes_just_transmitted;

                    if (error)
                    {
                        self.complete(error, *leaf_share, bytes_written, bytes_read);
                        break;
                    }

                    yield async_post(work_executor, [&leaf, leaf_share]() mutable
                    {
                        *leaf_share = leaf.reconstruct_correction_word(*leaf_share);
                    }, std::move(self));

                    self.complete(error, *leaf_share, bytes_written, bytes_read);
                }
            },
        token, peer_in, peer_out, work_executor);
#include <asio/unyield.hpp>
}

template <std::size_t I = 0,
          typename PeerT,
          typename ExecutorT,
          typename DpfKey,
          typename OutputType,
          typename CompletionToken>
[[nodiscard]]
HEDLEY_ALWAYS_INLINE
auto async_assign_wildcard_output(PeerT & peer, ExecutorT work_executor,
    DpfKey & dpf, OutputType && output_share, CompletionToken && token)
{
    return async_assign_wildcard_output<I>(peer, peer, work_executor, dpf,
        std::forward<OutputType>(output_share),
        std::forward<CompletionToken>(token));
}

template <std::size_t I = 0,
          typename PeerT,
          typename DpfKey,
          typename OutputType,
          typename CompletionToken>
[[nodiscard]]
HEDLEY_ALWAYS_INLINE
auto async_assign_wildcard_output(PeerT & peer_in, PeerT & peer_out,
    DpfKey & dpf, OutputType && output_share, CompletionToken && token)
{
    auto work_executor = ::asio::system_executor();
    return async_assign_wildcard_output<I>(peer_in, peer_out, work_executor, dpf,
        std::forward<OutputType>(output_share),
        std::forward<CompletionToken>(token));
}

template <std::size_t I = 0,
          typename PeerT,
          typename DpfKey,
          typename OutputType,
          typename CompletionToken>
[[nodiscard]]
HEDLEY_ALWAYS_INLINE
auto async_assign_wildcard_output(PeerT & peer, DpfKey & dpf,
    OutputType && output_share, CompletionToken && token)
{
    auto work_executor = ::asio::system_executor();
    return async_assign_wildcard_output<I>(peer, peer, dpf,
        std::forward<OutputType>(output_share),
        std::forward<CompletionToken>(token));
}

//
// make_interior_correction_word
//

template <typename DpfKey,
          typename PeerT,
          typename InteriorNode>
auto make_interior_correction_word(PeerT & peer, const InteriorNode & left,
    const InteriorNode & right, dpf::bit dir, ::asio::error_code & error);

template <typename DpfKey,
          typename PeerT,
          typename InteriorNode>
HEDLEY_ALWAYS_INLINE
auto make_interior_correction_word(PeerT & peer, const InteriorNode & left,
    const InteriorNode & right, dpf::bit dir)
{
    ::asio::error_code error{};
    auto ret = dpf::asio::make_interior_correction_word(peer, left, right, dir, error);
    if (error) throw error;
    return ret;
}

//
// async_make_interior_correction_word
//

template <typename DpfKey,
          typename PeerT,
          typename ExecutorT,
          typename InteriorNode,
          typename CompletionToken>
[[nodiscard]]
auto async_make_interior_correction_word(PeerT & peer, ExecutorT work_executor, 
    const InteriorNode & left, const InteriorNode & right, dpf::bit dir,
    CompletionToken && token);

template <typename DpfKey,
          typename PeerT,
          typename ExecutorT,
          typename InteriorNode,
          typename CompletionToken>
[[nodiscard]]
HEDLEY_ALWAYS_INLINE
auto async_make_interior_correction_word(PeerT & peer, const InteriorNode & left,
    const InteriorNode & right, dpf::bit dir, CompletionToken && token)
{
    auto work_executor = ::asio::system_executor();
    return async_make_interior_correction_word(peer, left, right, dir,
        std::forward<CompletionToken>(token));
}

}  // namespace asio

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_ASIO_HPP__
