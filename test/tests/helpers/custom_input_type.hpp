#include <cstdint>

class custom_input_type
{
  public:
    constexpr custom_input_type()
      : val_{0}
    { }

    constexpr custom_input_type(uint16_t val)
      : val_{val}
    { }

    custom_input_type operator<<(std::size_t shift) const
    {
        return custom_input_type{static_cast<uint16_t>(val_ << shift)};
    }

    custom_input_type & operator<<=(std::size_t shift)
    {
        this->val_ <<= shift;
        return *this;
    }

    custom_input_type operator>>(std::size_t shift) const
    {
        return custom_input_type{static_cast<uint16_t>(val_ >> shift)};
    }

    custom_input_type & operator>>=(std::size_t shift)
    {
        this->val_ >>= shift;
        return *this;
    }

    custom_input_type operator&(const custom_input_type & rhs) const
    {
        return custom_input_type{static_cast<uint16_t>(this->val_ & rhs.val_)};
    }

    custom_input_type operator^(const custom_input_type & rhs) const
    {
        return custom_input_type{static_cast<uint16_t>(this->val_ ^ rhs.val_)};
    }

    custom_input_type operator~() const
    {
        return custom_input_type{static_cast<uint16_t>(~this->val_)};
    }

    custom_input_type & operator++()
    {
        ++this->val_;
        return *this;
    }

    custom_input_type operator++(int)
    {
        custom_input_type tmp = *this;
        this->operator++();
        return tmp;
    }

    custom_input_type & operator--()
    {
        --this->val_;
        return *this;
    }

    custom_input_type operator--(int)
    {
        custom_input_type tmp = *this;
        this->operator--();
        return tmp;
    }

    bool operator==(const custom_input_type & rhs) const
    {
        return this->val_ == rhs.val_;
    }

    bool operator>(const custom_input_type & rhs) const
    {
        return this->val_ > rhs.val_;
    }

    bool operator>=(const custom_input_type & rhs) const
    {
        return this->val_ >= rhs.val_;
    }

    bool operator<(const custom_input_type & rhs) const
    {
        return this->val_ < rhs.val_;
    }

    bool operator<=(const custom_input_type & rhs) const
    {
        return this->val_ <= rhs.val_;
    }

    operator bool() const
    {
        return static_cast<bool>(this->val_);
    }

    operator uint16_t() const
    {
        return val_;
    }

    operator uint64_t() const
    {
        return static_cast<uint64_t>(val_);
    }

  private:
    uint16_t val_;
};

namespace dpf::utils
{

template <>
struct msb_of<custom_input_type>
{
    static constexpr custom_input_type value{0x8000};
};

template <>
struct mod_pow_2<custom_input_type>
{
    std::size_t operator()(custom_input_type val, std::size_t n) const noexcept
    {
        return static_cast<std::size_t>(static_cast<uint16_t>(val) % (1ul << n));
    }
};

}

namespace std
{

template<>
class numeric_limits<custom_input_type>
  : public numeric_limits<uint16_t> {};

template<>
class numeric_limits<custom_input_type const>
  : public numeric_limits<custom_input_type> {};

template<>
class numeric_limits<custom_input_type volatile>
  : public numeric_limits<custom_input_type> {};

template<>
class numeric_limits<custom_input_type const volatile>
  : public numeric_limits<custom_input_type> {};

}
