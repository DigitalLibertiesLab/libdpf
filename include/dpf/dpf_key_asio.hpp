    template <typename OutputType,
              typename PeerT,
              typename LeafT,
              typename CompletionToken>
    static auto async_exchange_and_reconstruct_leaf_shares(PeerT & peer,
        const LeafT & share, CompletionToken && token)
    {
        auto peer_share = std::make_unique<LeafT>();

#include <asio/yield.hpp>
        return asio::async_compose<
            CompletionToken, void(LeafT, asio::error_code)>(
                [
                    &peer,
                    my_share = share,
                    peer_share = std::move(peer_share),
                    coro = asio::coroutine()
                ]
                (
                    auto & self,
                    const asio::error_code & error = {},
                    std::size_t = 0
                )
                mutable
                {
                    reenter (coro)
                    {
                        yield asio::async_write(peer,
                            asio::buffer(&my_share, sizeof(LeafT)),
                            std::move(self));

                        yield asio::async_read(peer,
                            asio::buffer(peer_share.get(), sizeof(LeafT)),
                            std::move(self));

                        self.complete(
                            add_leaf<OutputType>(my_share, *peer_share),
                            error);
                    }
                },
            token, peer);
#include <asio/unyield.hpp>
    }

    template <std::size_t I,
              typename PeerT,
              typename OutputType,
              typename BeaverT,
              typename CompletionToken,
              std::enable_if_t<std::greater{}(outputs_per_leaf_v<OutputType, exterior_node>, 1), bool> = true>
    static auto async_compute_naked_leaf_share(PeerT & peer,
        OutputType output, const BeaverT & beaver, CompletionToken && token)
    {
        static_assert(std::is_same_v<OutputType, concrete_type_t<std::tuple_element_t<I, outputs_tuple>>>);

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
        using leaf_type = std::tuple_element_t<I, leaf_tuple>;
        auto my_output = std::make_unique<OutputType>(output);
        auto peer_output = std::make_unique<OutputType>();
HEDLEY_PRAGMA(GCC diagnostic pop)

#include <asio/yield.hpp>
        return asio::async_compose<
            CompletionToken, void(leaf_type, asio::error_code)>(
                [
                    &peer,
                    output,
                    &output_blind = beaver.output_blind,
                    &blinded_vector = beaver.blinded_vector,
                    &vector_blind = beaver.vector_blind,
                    my_output = std::move(my_output),
                    peer_output = std::move(peer_output),
                    coro = asio::coroutine()
                ]
                (
                    auto & self,
                    const asio::error_code & error = {},
                    std::size_t = 0
                )
                mutable
                {
                    reenter (coro)
                    {
                        *my_output += output_blind;
                        yield asio::async_write(peer,
                            asio::buffer(my_output.get(), sizeof(OutputType)),
                            std::move(self));

                        yield asio::async_read(peer,
                            asio::buffer(peer_output.get(), sizeof(OutputType)),
                            std::move(self));

                        self.complete(
                            subtract_leaf<OutputType>(
                                multiply_leaf(blinded_vector, output),
                                multiply_leaf(vector_blind, *peer_output)),
                            error);
                    }
                },
            token, peer);
#include <asio/unyield.hpp>
    }

    template <std::size_t I,
              typename PeerT,
              typename OutputType,
              typename BeaverT,
              typename CompletionToken,
              std::enable_if_t<std::equal_to(outputs_per_leaf_v<OutputType, exterior_node>, 1), bool> = true>
    static auto async_compute_naked_leaf_share(PeerT & peer,
        const OutputType & output, const BeaverT & beaver, CompletionToken && token)
    {
        static_assert(std::is_same_v<OutputType, std::tuple_element_t<I, outputs_tuple>>);

        using leaf_type = std::tuple_element_t<I, leaf_tuple>;

        return asio::async_compose<
            CompletionToken, void(leaf_type, asio::error_code)>(
                [
                    &output,
                    coro = asio::coroutine()
                ]
                (
                    auto & self,
                    const asio::error_code & error = {},
                    std::size_t = 0
                )
                mutable
                {
                    leaf_type out;
                    std::memcpy(&out, &output, sizeof(leaf_type));
                    self.complete(out, error);
                },
            token, peer);
    }

    template <std::size_t I = 0,
            typename OutputType,
            typename StreamT,
            typename CompletionToken>
    auto async_assign_leaf(StreamT & peer, const OutputType & output, CompletionToken && token)
    {
HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
        using leaf_type = std::tuple_element_t<I, leaf_tuple>;
        using output_type = concrete_type_t<std::tuple_element_t<I, outputs_tuple>>;
        static_assert(std::is_same_v<OutputType, output_type>);
HEDLEY_PRAGMA(GCC diagnostic pop)

#include <asio/yield.hpp>
        return asio::async_compose<
            CompletionToken, void(asio::error_code)>(
                [
                    &peer,
                    &wildcard_mask = this->mutable_wildcard_mask,
                    &output,
                    &leaf = std::get<I>(this->mutable_leaf_tuple),
                    &beaver = std::get<I>(this->mutable_beaver_tuple),
                    coro = asio::coroutine()
                ]
                (
                    auto & self,
                    leaf_type && leaf_buf = leaf_type{},
                    const asio::error_code & error = {}
                )
                mutable
                {
                    reenter (coro)
                    {
                        if (wildcard_mask.test(I) == false)
                        {
                            throw std::logic_error("not a wildcard");
                        }
                        if (beaver.is_locked->test_and_set())
                        {
                            // once locked, *never* locked
                            // (even in event of failure)
                            throw std::logic_error("already locked");
                        }

                        yield async_compute_naked_leaf_share<I>(peer, output, beaver, std::move(self));
                        leaf = add_leaf<OutputType>(leaf, leaf_buf);
                        yield async_exchange_and_reconstruct_leaf_shares<OutputType>(peer, leaf, std::move(self));
                        leaf = leaf_buf;
                        wildcard_mask[I] = false;

                        self.complete(error);
                    }
                },
            token, peer);
#include <asio/unyield.hpp>
    }