#include <cstdint>

class output_type_large
{
  public:
    constexpr output_type_large()
      : a_{0}, b_{0}, c_{0}, d_{0}
    { }

    constexpr output_type_large(uint64_t val)
      : a_{val}, b_{val}, c_{val}, d_{val}
    { }

    constexpr output_type_large(uint64_t a, uint64_t b, uint64_t c, uint64_t d)
      : a_{a}, b_{b}, c_{c}, d_{d}
    { }

    output_type_large operator+(const output_type_large & rhs) const
    {
        return output_type_large{this->a_ + rhs.a_, this->b_ + rhs.b_, this->c_ + rhs.c_, this->d_ + rhs.d_};
    }

    output_type_large operator-(const output_type_large & rhs) const
    {
        return output_type_large{this->a_ - rhs.a_, this->b_ - rhs.b_, this->c_ - rhs.c_, this->d_ - rhs.d_};
    }

    output_type_large operator^(const output_type_large & rhs) const
    {
        return output_type_large{this->a_ ^ rhs.a_, this->b_ ^ rhs.b_, this->c_ ^ rhs.c_, this->d_ ^ rhs.d_};
    }

    bool operator==(const output_type_large & rhs) const
    {
        return this->a_ == rhs.a_ && this->b_ == rhs.b_ && this->c_ == rhs.c_ && this->d_ == rhs.d_;
    }

  private:
    uint64_t a_, b_, c_, d_;
};

namespace dpf::utils
{

template <>
struct make_from_integral_value<output_type_large>
{
    constexpr output_type_large operator()(uint64_t val) const noexcept
    {
        return output_type_large{val, val, val, val};
    }
};

}
