#include <cstdint>

class custom_output_type_large_plus_minus
{
  public:
    constexpr custom_output_type_large_plus_minus()
      : a_{0}, b_{0}, c_{0}, d_{0}
    { }

    constexpr custom_output_type_large_plus_minus(uint64_t val)
      : a_{val}, b_{val}, c_{val}, d_{val}
    { }

    constexpr custom_output_type_large_plus_minus(uint64_t a, uint64_t b, uint64_t c, uint64_t d)
      : a_{a}, b_{b}, c_{c}, d_{d}
    { }

    custom_output_type_large_plus_minus operator+(const custom_output_type_large_plus_minus & rhs) const
    {
        return custom_output_type_large_plus_minus{this->a_ + rhs.a_, this->b_ + rhs.b_, this->c_ + rhs.c_, this->d_ + rhs.d_};
    }

    custom_output_type_large_plus_minus operator-(const custom_output_type_large_plus_minus & rhs) const
    {
        return custom_output_type_large_plus_minus{this->a_ - rhs.a_, this->b_ - rhs.b_, this->c_ - rhs.c_, this->d_ - rhs.d_};
    }

    custom_output_type_large_plus_minus operator^(const custom_output_type_large_plus_minus & rhs) const
    {
        return custom_output_type_large_plus_minus{this->a_ ^ rhs.a_, this->b_ ^ rhs.b_, this->c_ ^ rhs.c_, this->d_ ^ rhs.d_};
    }

    bool operator==(const custom_output_type_large_plus_minus & rhs) const
    {
        return this->a_ == rhs.a_ && this->b_ == rhs.b_ && this->c_ == rhs.c_ && this->d_ == rhs.d_;
    }

  private:
    uint64_t a_, b_, c_, d_;
};

namespace dpf::utils
{

template <>
struct make_from_integral_value<custom_output_type_large_plus_minus>
{
    constexpr custom_output_type_large_plus_minus operator()(uint64_t val) const noexcept
    {
        return custom_output_type_large_plus_minus{val, val, val, val};
    }
};

}
