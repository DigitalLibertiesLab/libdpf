#include <iostream>
#include <cstdio>
#include <tuple>

#include <asio.hpp>

namespace dpf
{

struct asio_fd : public std::tuple<asio::io_context &, int>{};
struct asio_fp : public std::tuple<asio::io_context &, FILE *>{};
struct asio_is : public std::tuple<asio::io_context &, std::istream &>{};
struct asio_os : public std::tuple<asio::io_context &, std::ostream &>{};
struct asio_ios : public std::tuple<asio::io_context &, std::iostream &>{};

template <typename MutableBuffer, typename ReadHandler>
auto async_read(asio_fd fd, MutableBuffer buffer, ReadHandler handler)
{
}

template <typename MutableBuffer, typename ReadHandler>
auto async_read(asio_fp fp, MutableBuffer buffer, ReadHandler handler)
{
}

template <typename MutableBuffer, typename ReadHandler>
auto async_read(asio_is is, MutableBuffer buffer, ReadHandler handler)
{
}

using asio::async_read;
using asio::read;

template <typename ConstBuffer, typename WriteHandler>
auto async_write(asio_fd fd, ConstBuffer buffer, WriteHandler handler)
{
}

template <typename ConstBuffer, typename WriteHandler>
auto async_write(asio_fp fp, ConstBuffer buffer, WriteHandler handler)
{
}

template <typename ConstBuffer, typename WriteHandler>
auto async_write(struct asio_os os, ConstBuffer buffer, WriteHandler handler)
{
}

using asio::async_write;
using asio::write;

}