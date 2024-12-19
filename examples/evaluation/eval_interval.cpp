#include <iostream>
#include "dpf.hpp"

int main(int arc, char * argv[])
{
    uint16_t x = 42, y;
    using prg = dpf::prg::counter_wrapper<dpf::prg::dummy>; 

    // Make the DPF
    auto before = prg::count();
    auto [dpf0, dpf1] = dpf::make_dpf<prg>(x);
    auto after = prg::count();
    std::cout << "dpf::make_dpf prg invocation: " << (after-before) << "\n";

    // Evaluate the DPF by interval
    before = prg::count();
    int from = 0, to = 49;
    auto [buf0, iter0] = dpf::eval_interval(dpf0, from, to); 
    auto [buf1, iter1] = dpf::eval_interval(dpf1, from, to);
    after = prg::count();
    std::cout << "dpf::eval_interval prg invocation: " << (after-before) << "\n";

    // Retrieve the original input by iterating over the two buffers
    std::vector<bool> result;
    for (size_t i = from; i < to+1; ++i) {
        bool item1 = buf0[i];
        bool item2 = buf1[i];
        result.push_back(item1 ^ item2);
        if (item1 ^ item2) y=i;
    }
    // Print out the XOR interval
    for (const auto& item : result) {
        std::cout << static_cast<bool>(item);
    }
    std::cout << std::endl;
    if (y == x) std::cout << "The orginal value is: " << x << std::endl;
    else std::cout << "The evaluated inputs did not match the original value." << std::endl; 

    std::cout << "Total PRG invocation: " << prg::count() << std::endl;

    return 0;
}