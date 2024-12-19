#include <chrono>
#include <iostream>
#include "dpf.hpp"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

int main(int argc, char * argv[])
{
    using input_type = uint8_t;
    using prg = dpf::prg::counter_wrapper<dpf::prg::dummy>;

    constexpr int N = 50;
    std::array<input_type, N> keys{};
    for(int i=0; i<N; i++) keys[i] = i; // Create an array of keys

    // eval_sequence with recipe
    input_type x = 42;
    auto [dpf0, dpf1] = dpf::make_dpf<prg>(x); // First DPF to be able to create the recipe
    auto t1 = high_resolution_clock::now(); // To measure the time of execution
    auto before = prg::count(); // To count the number of PRG invocations
    auto recipe0 = dpf::make_sequence_recipe(dpf0, std::begin(keys), std::end(keys)); // Create a recipe
    auto recipe1 = dpf::make_sequence_recipe(dpf1, std::begin(keys), std::end(keys)); // Create a recipe
    for (int i=0; i<N; i++) 
    {
        auto [dpf00, dpf11] = dpf::make_dpf<prg>(i); // Make 50 DPFs
        dpf::eval_sequence(dpf0, recipe0); // Evaluate the DPFs with the recipe
        dpf::eval_sequence(dpf1, recipe0); // Evaluate the DPFs with the recipe
    }
    auto after = prg::count(); // Count the number of PRG invocations
    std::cout << "dpf::eval_sequence with recipe " << (after-before) << "\n";

    // eval_sequence without the recipe
    auto t2 = high_resolution_clock::now();
    duration<double, std::milli> ms_double = t2 - t1;
    std::cout << "Time of execution: " << ms_double.count() << "ms\n";
    auto t3 = high_resolution_clock::now();
    before = prg::count();
    for (int i=0; i<N; i++) 
    {
        auto [dpf00, dpf11] = dpf::make_dpf<prg>(i);
        dpf::eval_sequence(dpf00, std::begin(keys), std::end(keys));
        dpf::eval_sequence(dpf11, std::begin(keys), std::end(keys));
    }
    after = prg::count();
    std::cout << "dpf::eval_sequence used " << (after-before) << "\n";
    auto t4 = high_resolution_clock::now();
    duration<double, std::milli> ms_double2 = t4 - t3;
    std::cout << "Time of execution with the memoizers: " << ms_double2.count() << "ms\n";

    return 0;
}