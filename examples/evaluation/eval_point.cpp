#include <iostream>

#include "dpf.hpp"

int main(int arc, char * argv[])
{
    uint16_t x = 42;
    auto [dpf0, dpf1] = dpf::make_dpf(x);

    auto res = dpf::eval_point(dpf0, x);

    std::cout << *dpf::eval_point(dpf0, 41) << " ^ " << *dpf::eval_point(dpf1, 41) << " = " << (*dpf::eval_point(dpf0, 41) ^ *dpf::eval_point(dpf1, 41)) << "\n"; // = 0
    std::cout << *dpf::eval_point(dpf0, x) << " ^ " << *dpf::eval_point(dpf1, x) << " = " << (*dpf::eval_point(dpf0, x) ^ *dpf::eval_point(dpf1, x)) << "\n"; // = 1
    std::cout << *dpf::eval_point(dpf0, 43) << " ^ " << *dpf::eval_point(dpf1, 43) << " = " << (*dpf::eval_point(dpf0, 43) ^ *dpf::eval_point(dpf1, 43)) << "\n"; // = 0

    return 0;
}