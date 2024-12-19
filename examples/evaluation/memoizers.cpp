#include <iostream>
#include <chrono>
#include "dpf.hpp"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

int main(int arc, char * argv[])
{
    // Making the DPF with an integer value
    uint16_t x = 42;
    using prg = dpf::prg::counter_wrapper<dpf::prg::dummy>;
    auto [dpf0, dpf1] = dpf::make_dpf<prg>(x);

    // Evaluating the DPF and counting how much it cost without memoizers
    auto t1 = high_resolution_clock::now();
    auto before = prg::count();
    for (int i = 0; i<1024*1024; i++)
    {
        dpf::eval_point(dpf0, i);

    }
    // Printing out the results
    auto after = prg::count();
    std::cout << "Without memoizers: " << "\n";
    std::cout << "PRG invocation: " << after-before << "\n";
    auto t2 = high_resolution_clock::now();
    duration<double, std::milli> ms_double = t2 - t1;
    std::cout << "Time of execution: " << ms_double.count() << "ms\n";

    // Evaluating the DPF and counting how much it cost with memoizers
    auto t3 = high_resolution_clock::now();
    before = prg::count();
    auto path = dpf::make_basic_path_memoizer(dpf0);
    for (int i = 0; i<1024*1024; i++)
    {
        dpf::eval_point(dpf0, i, path);   
    }
    // Printing out the results
    after = prg::count();
    std::cout << "With memoizers: " << "\n";
    std::cout << "PRG invocation: " << after-before << "\n";
    auto t4 = high_resolution_clock::now();
    duration<double, std::milli> ms_double2 = t4 - t3;
    std::cout << "Time of execution with the memoizers: " << ms_double2.count() << "ms\n";

    return 0;
}