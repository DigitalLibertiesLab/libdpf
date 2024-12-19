#include <iostream>

#include "dpf.hpp"

int main(int argc, char * argv[])
{
    using value_type = int;
    using output = bool;

    auto [dpf0, dpf1] = dpf::make_dpf<dpf::bitstring>(12);

    std::cout << (res0 ^ res1) << "\n";
    std::cout << values[2] << "\n";
    return 0;
}