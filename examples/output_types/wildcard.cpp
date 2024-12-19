#include <iostream>

#include "dpf.hpp"

int main(int argc, char * argv[])
{
    using input_type = uint8_t;
    using output_type = dpf::wildcard_value<uint32_t>;
    using dpf_type = dpf::utils::dpf_type_t<dpf::prg::aes128, dpf::prg::aes128, input_type, output_type>;
    input_type x = 12;
    output_type y;
    auto [dpf0, dpf1] = dpf::make_dpf(x, y);


    std::array<input_type, 5> points{12, 34, 56, 78, 90};
    auto recipe0 = dpf::make_sequence_recipe(dpf0, std::begin(points), std::end(points));
    auto recipe1 = dpf::make_sequence_recipe(dpf1, std::begin(points), std::end(points));
}