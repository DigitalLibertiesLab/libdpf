#include <iostream>
#include <cassert>
#include "dpf.hpp"

int main(int argc, char * argv[])
{
    using value_type = int;

    std::vector<value_type> values = {12, 34, 56, 78};

    auto [dpf0, dpf1] = dpf::make_dpf(value_type{56});

    int i=0;
    value_type res0{};
    auto [buf0, iter0] = dpf::eval_sequence(dpf0, std::begin(values), std::end(values));
    for (auto b : iter0) { if (b) res0 ^= values[i]; i++; }

    i=0;
    value_type res1{};
    auto [buf1, iter1] = dpf::eval_sequence(dpf1, std::begin(values), std::end(values));
    for (auto b : iter1) { if (b) res1 ^= values[i]; i++; }

    std::cout << (res0 ^ res1) << "\n";
    std::cout << values[2] << "\n";
}