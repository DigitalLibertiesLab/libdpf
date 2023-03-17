/// @file dpf/parallel_bit_iterable.hpp
/// @author Christopher Jiang <christopher.jiang@ucalgary.ca>
/// @brief
/// @details
/// @copyright Copyright (c) 2019-2023 Ryan Henry and others
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref GPLv2) for details.

#ifndef LIBDPF_INCLUDE_DPF_PARALLEL_BIT_ITERABLE_HPP__
#define LIBDPF_INCLUDE_DPF_PARALLEL_BIT_ITERABLE_HPP__

#include <limits>
#include <algorithm>
#include <utility>

#include "dpf/bit_array.hpp"
#include "dpf/parallel_bit_iterable_helper.hpp"

namespace dpf
{

template <std::size_t BatchSize>
class parallel_const_bit_iterator;  // forward declaration

template <std::size_t BatchSize>
class parallel_bit_iterable
{
  public:
    using word_pointer = bit_array::word_pointer;
    static constexpr auto batch_size = BatchSize;
    using const_iterator = parallel_const_bit_iterator<batch_size>;

    template <typename Iter>
    explicit parallel_bit_iterable(Iter it)
      : begin_{init_array(it,
            [](Iter it){ return it->data(); })},
        end_{init_array(it,
            [](Iter it){ return it->data() + it->data_length(); })}
    { }

    template <typename ...Ts>
    explicit parallel_bit_iterable(const bit_array & t, const Ts & ...ts)
      : begin_{t.data(), ts.data()...},
        end_{t.data()+t.data_length(), ts.data()+ts.data_length()...}
    { }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator begin() const noexcept
    {
        return const_iterator{begin_};
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator cbegin() const noexcept
    {
        return begin();
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator end() const noexcept
    {
        return const_iterator{end_};
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    const_iterator cend() const noexcept
    {
        return end();
    }

  private:
    using array_type = std::array<word_pointer, batch_size>;

    template <typename F, typename Iter, std::size_t... Is>
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    static constexpr array_type init_array_impl(Iter it,
        std::index_sequence<Is...>, const F & lambda) noexcept
    {
        return {{ ((void)Is, lambda(it++))... }};
    }

    template <typename F, typename Iter,
        class Indices = std::make_index_sequence<batch_size>>
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    static constexpr array_type init_array(Iter it,
        const F & lambda) noexcept
    {
        return init_array_impl(it, Indices{}, lambda);
    }

    const array_type begin_, end_;
};  // class dpf::parallel_bit_iterable

template <std::size_t N>
class parallel_const_bit_iterator
{
  private:
    using word_type = bit_array::word_type;
    using word_array = std::array<word_type, N>;
    using word_pointer = bit_array::word_pointer;
    using word_pointer_array = std::array<word_pointer, N>;
    static constexpr std::size_t lg_batch_size = (N <= 2)
                                     ? 2 : std::ceil(std::log2(N));
    using helper = dpf::parallel_bit_iterable_helper<lg_batch_size>;
    using element_type = typename helper::element_type;
    using simde_type = typename helper::type;
    static constexpr auto bits_per_word = bit_array::bits_per_word;
    static constexpr auto bits_per_element = helper::bits_per_element;
    static constexpr auto bytes_per_batch = N * (bits_per_element/8);
    static constexpr auto elements_per_word = helper::elements_per_word;
    using simde_array = typename helper::array;

  public:
    static constexpr auto batch_size = N;

    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::array<std::pair<std::ptrdiff_t, std::size_t>,
                                       batch_size>;
    using value_type = std::array<element_type, batch_size>;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = std::add_pointer_t<value_type>;

    HEDLEY_ALWAYS_INLINE
    constexpr
    parallel_const_bit_iterator(parallel_const_bit_iterator &&) noexcept = default;
    HEDLEY_ALWAYS_INLINE
    constexpr
    parallel_const_bit_iterator(const parallel_const_bit_iterator &) noexcept = default;

    HEDLEY_CONST
    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    value_type operator*() const noexcept
    {
        value_type ret;
        simde_type temp = helper::and(all_vecs_[element_cnt_], vec_mask_);
        std::memcpy(ret.data(), &temp, bytes_per_batch);
        return ret;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    DPF_UNROLL_LOOPS
    parallel_const_bit_iterator & operator++() noexcept
    {
        if (HEDLEY_UNLIKELY(!(word_mask_ <<= 1)))
        {
            word_mask_ = word_lsb;
            element_mask_ = element_lsb;
            vec_mask_ = helper::right_shift(vec_mask_, bits_per_element - 1);
            element_cnt_ = 0;

            std::transform(iter_.begin(), iter_.end(), cur_word_.begin(),
                [](auto & it)
                {
                    return *(++it);
                });
            all_vecs_ = helper::build_vecs(cur_word_.data());
        }
        else if (HEDLEY_UNLIKELY(!(element_mask_ <<= 1)))
        {
            element_mask_ = element_lsb;
            vec_mask_ = helper::right_shift(vec_mask_, bits_per_element - 1);
            ++element_cnt_;
        }
        else
        {
            vec_mask_ = helper::left_shift(vec_mask_, 1);
        }
        return *this;
    }

    HEDLEY_NO_THROW
    parallel_const_bit_iterator operator++(int) noexcept
    {
        auto tmp = *this;
        parallel_const_bit_iterator::operator++();
        return tmp;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    DPF_UNROLL_LOOPS
    parallel_const_bit_iterator & operator--() noexcept
    {
        if (HEDLEY_UNLIKELY(!(word_mask_ >>= 1)))
        {
            word_mask_ = word_msb;
            element_mask_ = element_msb;
            vec_mask_ = helper::left_shift(vec_mask_, bits_per_element - 1);
            element_cnt_ = elements_per_word - 1;

            std::transform(iter_.begin(), iter_.end(), cur_word_.begin(),
                [](auto & it)
                {
                    return *(--it);
                });
            all_vecs_ = helper::build_vecs(cur_word_.data());
        }
        else if (HEDLEY_UNLIKELY(!(element_mask_ >>= 1)))
        {
            element_mask_ = element_msb;
            vec_mask_ = helper::left_shift(vec_mask_, bits_per_element - 1);
            --element_cnt_;
        }
        else
        {
            vec_mask_ = helper::right_shift(vec_mask_, 1);
        }
        return *this;
    }

    HEDLEY_NO_THROW
    parallel_const_bit_iterator operator--(int) noexcept
    {
        auto tmp = *this;
        parallel_const_bit_iterator::operator--();
        return tmp;
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator==(const parallel_const_bit_iterator & rhs) const noexcept
    {
        return (word_mask_ == rhs.word_mask_)
            && (std::equal(iter_.begin(), iter_.end(), rhs.iter_.begin()));
    }

    HEDLEY_NO_THROW
    HEDLEY_ALWAYS_INLINE
    constexpr bool operator!=(const parallel_const_bit_iterator & rhs) const noexcept
    {
        return !(*this == rhs);
    }

  private:
    static constexpr word_type word_lsb = word_type(1);
    static constexpr word_type word_msb = word_lsb << (bits_per_word-1);
    static constexpr element_type element_lsb = element_type(1);
    static constexpr element_type element_msb = element_lsb << (bits_per_element-1);

    template<std::size_t... Is>
    word_array dereferencing_initializer_impl(
        const word_pointer_array & arr, std::index_sequence<Is...>)
    {
        return {{ *(arr[Is])... }};
    }

    word_array dereferencing_initializer(const word_pointer_array & arr)
    {
        return dereferencing_initializer_impl(arr,
            std::make_index_sequence<batch_size>());
    }

    explicit constexpr parallel_const_bit_iterator(
        const word_pointer_array & arr) noexcept
      : iter_{arr},
        word_mask_{word_lsb},
        element_mask_{element_lsb},
        element_cnt_{0},
        cur_word_{dereferencing_initializer(arr)},
        vec_mask_{helper::get_mask()},
        all_vecs_{helper::build_vecs(cur_word_.data())}
    { }

    word_pointer_array iter_;
    word_type word_mask_;
    element_type element_mask_;
    std::size_t element_cnt_;

    word_array cur_word_;
    simde_type vec_mask_;
    simde_array all_vecs_;

    friend parallel_const_bit_iterator parallel_bit_iterable<batch_size>::begin() const noexcept;
    friend parallel_const_bit_iterator parallel_bit_iterable<batch_size>::end() const noexcept;
};  // class dpf::parallel_const_bit_iterator

template <std::size_t N, typename Iter>
HEDLEY_PURE
HEDLEY_ALWAYS_INLINE
auto batch_of(Iter it) noexcept
{
    return dpf::parallel_bit_iterable<N>{it};
}

template <typename... Ts>
HEDLEY_PURE
HEDLEY_ALWAYS_INLINE
auto batch_of(const dpf::bit_array & t, const Ts & ... ts) noexcept
{
    return dpf::parallel_bit_iterable<1+sizeof...(Ts)>{t, ts...};
}

template <std::size_t N, typename Iter, class UnaryFunction>
HEDLEY_PURE
HEDLEY_ALWAYS_INLINE
void for_each_batch(Iter it, UnaryFunction f)
{
    for (auto i : batch_of<N>(it)) f(i);
}

template <class UnaryFunction, typename... Ts>
HEDLEY_PURE
HEDLEY_ALWAYS_INLINE
void for_each_batch(const dpf::bit_array & t, const Ts & ... ts, UnaryFunction f)
{
    for (auto i : batch_of<1+sizeof...(Ts)>(t, ts...)) f(i);
}

}  // namespace dpf

namespace std
{

template <std::size_t N>
struct iterator_traits<typename dpf::parallel_const_bit_iterator<N>>
{
  private:
    using type = dpf::parallel_const_bit_iterator<N>;
  public:
    using iterator_category = typename type::iterator_category;
    using difference_type = typename type::difference_type;
    using value_type = typename type::value_type;
    using reference = typename type::reference;
    using const_reference = typename type::const_reference;
    using pointer = typename type::pointer;
};

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_PARALLEL_BIT_ITERABLE_HPP__
