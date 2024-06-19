#include <iostream>
#include "dpf.hpp"

int main(int argc, char * argv[])
{
    using input_type = uint16_t;
    using output_type = dpf::bit;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type>;
    auto memo0 = dpf::make_basic_full_memoizer<dpf_type>();
    auto memo1 = dpf::make_basic_full_memoizer<dpf_type>();
    input_type x = 42;
    output_type y = dpf::bit::one;
    auto [dpf0, dpf1] = dpf::make_dpf(x, y);

    auto advice0 = dpf::advice_bits_of(memo0);
    auto advice1 = dpf::advice_bits_of(memo1);

    auto it0 = std::begin(advice0), it1 = std::begin(advice1);
    for (std::size_t i = 0; i < std::size_t(1)<<dpf_type::depth; ++i, ++it0, ++it1)
    {
        std::cout << "Advice bit " << i << " of dpf0: " << *it0 << "\n";
    }
}