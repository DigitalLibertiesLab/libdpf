#include <unistd.h>
#include <cstdlib>
#include <array>
#include <initializer_list>

#include <hedley/hedley.h>

namespace dpf
{

template <typename Buffer, std::size_t N>
struct buffer_sequence : public std::array<Buffer, N> { };

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

template <typename T, typename ...Ts>
auto make_mutable_buffer_sequence(T & t, Ts & ...ts)
{
    return dpf::buffer_sequence<dpf::mutable_buffer, 1+sizeof...(Ts)>{
        dpf::mutable_buffer{&t, sizeof(T)},
        dpf::mutable_buffer{&ts, sizeof(Ts)}...};
}

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

template <typename T, typename ...Ts>
auto make_const_buffer_sequence(T & t, Ts & ...ts)
{
    return dpf::buffer_sequence<dpf::const_buffer, 1+sizeof...(Ts)>{
        dpf::const_buffer{&t, sizeof(T)},
        dpf::const_buffer{&ts, sizeof(Ts)}...};
}

template <typename MutableBuffer>
// [[no_discard]]
HEDLEY_WARN_UNUSED_RESULT
inline auto read_some(int fd, MutableBuffer buffer)
{
    ssize_t bytes_read = ::read(fd, reinterpret_cast<char *>(buffer.data()),
        buffer.size());
    if (HEDLEY_UNLIKELY(bytes_read == -1))
    {
        // TODO(ryan) proper error handling
    }
    return bytes_read;
}

template <typename MutableBuffer>
// [[no_discard]]
HEDLEY_WARN_UNUSED_RESULT
inline auto read_some(FILE * fp, MutableBuffer buffer)
{
    ssize_t bytes_read = fread(buffer.data(), 1, buffer.size(), fp);
    // TODO(ryan) proper error checking
    return bytes_read;
}

template <typename MutableBuffer>
// [[no_discard]]
HEDLEY_WARN_UNUSED_RESULT
inline auto read_some(std::istream & is, MutableBuffer buffer)
{
    is.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
    std::streamsize just_read = std::cin.gcount();
    if (HEDLEY_UNLIKELY(is.fail()))
    {
        // TODO(ryan) proper error checking/handling
    }
    return just_read;
}

template <typename F, typename MutableBuffer>
// [[no_discard]]
HEDLEY_WARN_UNUSED_RESULT
auto read(F fd, MutableBuffer buffer)
{
    std::size_t total_bytes_read = 0;
    do
    {
        ssize_t just_read = read_some(fd, buffer);
        buffer += just_read;
        total_bytes_read += just_read;
    } while (buffer.size());

    return total_bytes_read;
}

template <typename F, typename MutableBuffer, std::size_t N>
// [[no_discard]]
HEDLEY_WARN_UNUSED_RESULT
auto read(F fd, buffer_sequence<MutableBuffer, N> && buffers)
{
    std::size_t bytes_read = 0;
    for (auto & buf : buffers)
    {
        bytes_read += read(fd, buf);
    }
    return bytes_read;
}

// #include asio_support.hpp for the following defintions
template <typename MutableBuffer, typename ReadHandler>
auto async_read(struct asio_fd fd, MutableBuffer buffer, ReadHandler handler);
// {
//     auto [io_context, fd_] = fd;
//     asio::post(io_context, &read, fp_, std::ref(buffers), handler);
// }

template <typename MutableBuffer, typename ReadHandler>
auto async_read(struct asio_fp fp, MutableBuffer buffer, ReadHandler handler);

template <typename MutableBuffer, typename ReadHandler>
auto async_read(struct asio_is is, MutableBuffer buffer, ReadHandler handler);


template <typename ConstBuffer>
// [[no_discard]]
auto write_some(int fd, ConstBuffer buffer)
{
    ssize_t bytes_written = ::write(fd, reinterpret_cast<const char *>(buffer.data()),
        buffer.size());
    if (HEDLEY_UNLIKELY(bytes_written == -1))
    {
        // TODO(ryan) proper error handling
    }
    return bytes_written;
}

template <typename MutableBuffer>
// [[no_discard]]
auto write_some(FILE * fp, MutableBuffer buffer)
{
    ssize_t bytes_written = fwrite(buffer.data(), 1, buffer.size(), fp);
    // TODO(ryan) proper error checking
    return bytes_written;
}

template <typename ConstBuffer>
// [[no_discard]]
auto write_some(std::ostream & os, ConstBuffer buffer)
{
    os.write(reinterpret_cast<char *>(buffer.data()), buffer.size());
    std::streamsize just_written = buffer.size();
    if (HEDLEY_UNLIKELY(os.fail()))
    {
        // TODO(ryan) proper error checking/handling
    }
    return just_written;
}

template <typename F, typename ConstBuffer>
// [[no_discard]]
auto write(F fd, ConstBuffer buffer)
{
    std::size_t total_bytes_written = 0;
    do
    {
        ssize_t just_written = write_some(fd, buffer);
        buffer += just_written;
        total_bytes_written += just_written;
    } while (buffer.size());

    return total_bytes_written;
}

template <typename F, typename ConstBuffer, std::size_t N>
// [[no_discard]]
auto write(F fd, buffer_sequence<ConstBuffer, N> && buffers)
{
    std::size_t bytes_written = 0;
    for (auto & buf : buffers)
    {
        bytes_written += write(fd, buf);
    }
    return bytes_written;
}

// #include asio_support.hpp for the following defintions
template <typename ConstBuffer, typename WriteHandler>
auto async_write(struct asio_fd fd, ConstBuffer buffer, WriteHandler handler);

template <typename ConstBuffer, typename WriteHandler>
auto async_write(struct asio_fp fp, ConstBuffer buffer, WriteHandler handler);

template <typename ConstBuffer, typename WriteHandler>
auto async_write(struct asio_os os, ConstBuffer buffer, WriteHandler handler);

}  // namespace dpf
