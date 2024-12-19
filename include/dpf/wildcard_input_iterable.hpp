#ifndef LIBDPF_INCLUDE_DPF_WILDCARD_INPUT_ITERABLE_HPP__
#define LIBDPF_INCLUDE_DPF_WILDCARD_INPUT_ITERABLE_HPP__

#include <optional>

#include "dpf/rotation_iterable.hpp"
#include "dpf/subinterval_iterable.hpp"

namespace dpf
{

template <typename DpfKey,
          typename IteratorT>
class wildcard_input_iterable
{
  public:
    using dpf_type = DpfKey;
    using iterator = IteratorT;
    using const_iterator = IteratorT;
    using size_type = std::size_t;

    explicit wildcard_input_iterable(dpf_type & dpf, iterator begin, iterator end, size_type buf_size, size_type from, size_type to, size_type preclip, size_type outputs_per_leaf)
      : dpf_{dpf}, begin_{begin}, end_{end}, buf_size_{buf_size}, from_{from}, to_{to}, preclip_{preclip}, outputs_{outputs_per_leaf}, rot_{std::nullopt}
    { }

    auto get()
    {
        assert_not_wildcard_input(dpf_);

        if (rot_.has_value() == false)
        {
            rot_ = rotation_iterable<iterator>(begin_, end_, to_integral_t(dpf_.offset_x(0)));
        }
        return std::move(subinterval_iterable(std::begin(rot_.value()), buf_size_, from_, to_, from_, outputs_));
    }

    auto begin()
    {
        assert_not_wildcard_input(dpf_);

        if (rot_.has_value() == false)
        {
            rot_ = rotation_iterable<iterator>(begin_, end_, to_integral_t(dpf_.offset_x(0)));
        }
        return std::begin(subinterval_iterable(std::begin(rot_.value()), buf_size_, from_, to_, from_, outputs_));
    }

    auto end()
    {
        assert_not_wildcard_input(dpf_);

        if (rot_.has_value() == false)
        {
            rot_ = rotation_iterable<iterator>(begin_, end_, to_integral_t(dpf_.offset_x(0)));
        }
        return std::end(subinterval_iterable(std::begin(rot_.value()), buf_size_, from_, to_, from_, outputs_));
    }

  private:
    using input_type = typename dpf_type::input_type;
    static constexpr auto to_integral_t = dpf::utils::to_integral_type<input_type>{};

    dpf_type & dpf_;
    iterator begin_;
    iterator end_;
    size_type buf_size_;
    size_type from_;
    size_type to_;
    size_type preclip_;
    size_type outputs_;
    std::optional<rotation_iterable<iterator>> rot_;

};  // class dpf::wildcard_input_iterable

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_WILDCARD_INPUT_ITERABLE_HPP__
