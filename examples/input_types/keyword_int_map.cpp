#include <iostream>
#include <cassert>
#include <map>
#include "dpf.hpp"

int main(int argc, char * argv[])
{
    static constexpr char lc[] = "abcdefghijklmnopqrstuvwxyz";
    using keyword_type = dpf::keyword<3, lc>;
    using value_type = int;

    std::map<keyword_type, value_type> database{{"cat", 12},
                                                {"bat", 34},
                                                {"dog", 56},
                                                {"pig", 78}};

    keyword_type x = "bat";
    std::cout << x << std::endl;
    auto [dpf0, dpf1] = dpf::make_dpf(x);

    value_type share0{}, share1{};
    auto path0 = dpf::make_basic_path_memoizer(dpf0);
    auto path1 = dpf::make_basic_path_memoizer(dpf1);
    for (auto & [key, value] : database)
    {
        if (dpf::eval_point(dpf0, key, path0)) {share0 ^= value; std::cout << value << std::endl;}
        if (dpf::eval_point(dpf1, key, path1)) share1 ^= value;
    }

    std::cout << share0 << share1 << "\n";
    std::cout << std::string(x) << "(" << x << ")->" << (share0 ^ share1) << "\n";
}