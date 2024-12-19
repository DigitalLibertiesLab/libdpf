#include <iostream>
#include <cassert>
#include "dpf.hpp"

int main(int argc, char * argv[])
{
    using keyword_type = dpf::keyword<3, dpf::alphabets::lowercase_alpha>;
    using value_type = int;

    std::vector<keyword_type> keys = {"cat", "dog", "bat", "pig"};
    std::vector<value_type> values = {12, 34, 56, 78};

    auto [dpf0, dpf1] = dpf::make_dpf(keyword_type{"bat"});
    // auto [dpf0, dpf1] = dpf::make_dpf(keyword_type{"rat"}); // key does not exist; result will be 0

    int i=0;
    value_type res0{};
    auto [buf0, iter0] = dpf::eval_sequence(dpf0, std::begin(keys), std::end(keys));
    for (auto b : iter0) { if (b) res0 ^= values[i]; i++; }

    i=0;
    value_type res1{};
    auto [buf1, iter1] = dpf::eval_sequence(dpf1, std::begin(keys), std::end(keys));
    for (auto b : iter1) { if (b) res1 ^= values[i]; i++; }

    std::cout << (res0 ^ res1) << "\n";
    std::cout << values[2] << "\n";
}