#include <iostream>

#include "dpf.hpp"

int main(int arc, char * argv[])
{
    uint16_t x = 42; // Input value
    using prg = dpf::prg::counter_wrapper<dpf::prg::dummy>; // This is just to count the number of PRG invocations
    auto before = prg::count(); // In order to show how much this program cost
    auto [dpf0, dpf1] = dpf::make_dpf<prg>(x);
    auto after = prg::count();
    std::cout << "dpf::make_dpf used " << (after-before) << "\n";

    before = prg::count();
    auto [buf0, iter0] = dpf::eval_full(dpf0); 
    after = prg::count();
    std::cout << "dpf::eval_full(dpf0) used " << (after-before) << "\n";
    before = prg::count();
    auto [buf1, iter1] = dpf::eval_full(dpf1);
    after = prg::count();
    std::cout << "dpf::eval_full(dpf1) used " << (after-before) << "\n";
    // Retrieve the original input by iterating over the two buffers
    for (size_t i = 0; i < buf0.size(); ++i) { 
        bool item1 = buf0[i];
        bool item2 = buf1[i];
        if (item1 ^ item2) std::cout << "The original input is: " << i << std::endl;
    }

    std::cout << "Total PRG invocation: " << prg::count() << "\n";
    return 0;
}