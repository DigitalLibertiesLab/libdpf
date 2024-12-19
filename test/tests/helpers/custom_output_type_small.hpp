#include <cstdint>

class custom_output_type_small
{
  public:
    constexpr custom_output_type_small()
      : val_{0}
    { }

    constexpr custom_output_type_small(uint64_t val)
      : val_{val}
    { }

    custom_output_type_small operator+(const custom_output_type_small & rhs) const
    {
        return custom_output_type_small{this->val_ + rhs.val_};
    }

    custom_output_type_small operator-(const custom_output_type_small & rhs) const
    {
        return custom_output_type_small{this->val_ - rhs.val_};
    }

    custom_output_type_small operator^(const custom_output_type_small & rhs) const
    {
        return custom_output_type_small{this->val_ ^ rhs.val_};
    }

    bool operator==(const custom_output_type_small & rhs) const
    {
        return this->val_ == rhs.val_;
    }

  private:
    uint64_t val_;
};

namespace dpf::leaf_arithmetic
{

HEDLEY_PRAGMA(GCC diagnostic push)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wignored-attributes")
template <> struct add_t<custom_output_type_small, simde__m128i> final : public detail::add2x64_t {};
template <> struct subtract_t<custom_output_type_small, simde__m128i> final : public detail::sub2x64_t {};
template <> struct multiply_t<custom_output_type_small, simde__m128i> final : public detail::mul2x64_t {};
HEDLEY_PRAGMA(GCC diagnostic pop)

}
