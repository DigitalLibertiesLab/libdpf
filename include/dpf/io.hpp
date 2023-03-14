/// @file dpf/dpf_key.hpp
/// @author Ryan Henry <ryan.henry@ucalgary.ca>
/// @brief
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_ASIO_HPP__
#define LIBDPF_INCLUDE_DPF_ASIO_HPP__

#include <unistd.h>
#include <cstdio>

#include <hedley/hedley.h>

#ifdef LIBDPF_USE_ASIO
#include <asio.hpp>
#endif

namespace dpf
{

namespace asio
{

#ifndef LIBDPF_USE_ASIO
struct mutable_buffer
{
  public:
    mutable_buffer(void * data, std::size_t size)
      : data_{reinterpret_cast<char *>(data)}, size_{size} { }
    mutable_buffer & operator+=(std::size_t n)
    {
        size_ -= n;
        data_ += n;
        return *this;
    }
    void * data() const { return reinterpret_cast<void *>(data_); }
    std::size_t size() const { return size_; }
  private:
    char * data_;
    std::size_t size_;
};

struct const_buffer
{
  public:
    const_buffer(void * data, std::size_t size)
      : data_{reinterpret_cast<char *>(data)}, size_{size} { }
    const_buffer & operator+=(std::size_t n)
    {
        size_ -= n;
        data_ += n;
        return *this;
    }
    const void * data() const { return reinterpret_cast<const void *>(data_); }
    std::size_t size() const { return size_; }

  private:
    const char * data_;
    std::size_t size_;
};

template <typename X>
auto buffer_sequence_begin(X & x) { return std::begin(x); }

template <>
auto buffer_sequence_begin<mutable_buffer>(mutable_buffer & x) { return &x; }

template <>
auto buffer_sequence_begin<const_buffer>(const_buffer & x) { return &x; }

template <typename X>
auto buffer_sequence_end(X & x) { return std::end(x); }

template <>
auto buffer_sequence_end<mutable_buffer>(mutable_buffer & x) { return (&x)+1; }

template <>
auto buffer_sequence_end<const_buffer>(const_buffer & x) { return (&x)+1; }
#endif

template <typename MutableBufferSequence>
HEDLEY_WARN_UNUSED_RESULT
std::size_t read(int fd, const MutableBufferSequence & buffers)
{
    std::size_t total_bytes_read = 0;
    for (auto it = buffer_sequence_begin(buffers);
            it != buffer_sequence_end(buffers); ++it)
    {
        do
        {
            ssize_t just_read = read(fd, reinterpret_cast<char *>(it->data()),
                it->size());
            if (HEDLEY_UNLIKELY(just_read == -1))
            {
                throw std::runtime_error("");
            }
            it->operator+=(just_read);
            total_bytes_read += just_read;
        } while (it->size());
    }
    return total_bytes_read;
}

template <typename MutableBufferSequence>
HEDLEY_WARN_UNUSED_RESULT
std::size_t read(FILE * fp, const MutableBufferSequence & buffers)
{
    std::size_t total_bytes_read = 0;
    for (auto it = buffer_sequence_begin(buffers);
            it != buffer_sequence_end(buffers); ++it)
    {
        do
        {
            ssize_t just_read = std::fread(reinterpret_cast<char *>(it->data()), 1,
                it->size(), fp);
            if (HEDLEY_UNLIKELY(just_read == 0))
            {
                throw std::runtime_error("");
            }
            it->operator+=(just_read);
            total_bytes_read += just_read;
        } while (it->size());
    }
    return total_bytes_read;
}

template <typename MutableBufferSequence>
HEDLEY_WARN_UNUSED_RESULT
std::size_t read(std::istream & is, const MutableBufferSequence & buffers)
{
    std::size_t total_bytes_read = 0;
    for (auto it = buffer_sequence_begin(buffers);
            it != buffer_sequence_end(buffers); ++it)
    {
        do
        {
            std::streamsize just_read = is.read(reinterpret_cast<char *>(it->data()),
                it->size());
            if (HEDLEY_UNLIKELY(is.fail()))
            {
                throw std::runtime_error("");
            }
            it->operator+=(just_read);
            total_bytes_read += just_read;
        } while (it->size());
    }
    return total_bytes_read;
}

#ifdef LIBDPF_USE_ASIO
using asio_fd = std::tuple<asio::io_context &, int>;
using asio_fp = std::tuple<asio::io_context &, FILE *>;
using asio_is = std::tuple<asio::io_context &, std::istream &>;

template <typename MutableBufferSequence>
void async_read(asio_fd & fd, const MutableBufferSequence & buffers)
{
    auto [io_context, fd_] = fd;
    io_context.post(&read, fd_, std::ref(buffers));
}

template <typename MutableBufferSequence>
void async_read(asio_fp fp, const MutableBufferSequence & buffers)
{
    auto [io_context, fp_] = fp;
    io_context.post(&read, fp_, std::ref(buffers));
}

template <typename MutableBufferSequence>
void async_read(asio_is & is, const MutableBufferSequence & buffers)
{
    auto [io_context, is_] = is;
    io_context.post(&read, std::ref(is_), std::ref(buffers));
}

using asio::async_read;
using asio::read;
#endif

template <typename ConstBufferSequence>
HEDLEY_WARN_UNUSED_RESULT
std::size_t write(int fd, const ConstBufferSequence & buffers)
{
    std::size_t total_bytes_written = 0;
    for (auto it = buffer_sequence_begin(buffers);
            it != buffer_sequence_end(buffers); ++it)
    {
        do
        {
            ssize_t just_written = write(fd, reinterpret_cast<char *>(it->data()),
                it->size());
            if (HEDLEY_UNLIKELY(just_written == -1))
            {
                throw std::runtime_error("");
            }
            it->operator+=(just_written);
            total_bytes_written += just_written;
        } while (it->size());
    }
    return total_bytes_written;
}

template <typename ConstBufferSequence>
HEDLEY_WARN_UNUSED_RESULT
std::size_t write(FILE * fp, const ConstBufferSequence & buffers)
{
    std::size_t total_bytes_written = 0;
    for (auto it = buffer_sequence_begin(buffers);
            it != buffer_sequence_end(buffers); ++it)
    {
        do
        {
            ssize_t just_written = std::fwrite(reinterpret_cast<char *>(it->data()),
                1, it->size(), fp);
            if (HEDLEY_UNLIKELY(just_written == 0))
            {
                throw std::runtime_error("");
            }
            it->operator+=(just_written);
            total_bytes_written += just_written;
        } while (it->size());
    }
    return total_bytes_written;
}

template <typename ConstBufferSequence>
HEDLEY_WARN_UNUSED_RESULT
std::size_t write(std::ostream & os, const ConstBufferSequence & buffers)
{
    std::size_t total_bytes_written = 0;
    for (auto it = buffer_sequence_begin(buffers);
            it != buffer_sequence_end(buffers); ++it)
    {
        do
        {
            std::streamsize just_written = os.write(reinterpret_cast<char *>(it->data()),
                it->size());
            if (HEDLEY_UNLIKELY(os.fail()))
            {
                throw std::runtime_error("");
            }
            it->operator+=(just_written);
            total_bytes_written += just_written;
        } while (it->size());
    }
    return total_bytes_written;
}

#ifdef LIBDPF_USE_ASIO
template <typename ConstBufferSequence>
void async_write(asio_fd fd, const ConstBufferSequence & buffers)
{
    auto [io_context, fd_] = fd;
    io_context.post(&write, fd_, std::ref(buffers));
}

template <typename ConstBufferSequence>
void async_write(asio_fp fp, ConstBufferSequence & buffers)
{
    auto [io_context, fp_] = fp;
    io_context.post(&write, fp_, std::ref(buffers));
}

template <typename ConstBufferSequence>
void async_write(asio_is & is, ConstBufferSequence & buffers)
{
    auto [io_context, is_] = is;
    io_context.post(&write, std::ref(is_), std::ref(buffers));
}

using asio::async_write;
using asio::write;
#endif

}  // namespace asio

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_ASIO_HPP__
