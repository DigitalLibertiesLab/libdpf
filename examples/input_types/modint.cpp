#include <iostream>
#include <cassert>
#include "dpf.hpp"

int main(int argc, char * argv[])
{
    dpf::modint<10> a(2048); // 2048 % 2^10 = 0
    dpf::modint<10> b(1026); // 1026 % 2^10 = 2
    std::cout << a + b << std::endl; // = 2
    
    dpf::modint<10> c(2051); // 2051 % 2^10 = 3
    dpf::modint<10> d(1026); // 1026 % 2^10 = 2
    std::cout << c - d << std::endl; // = 1

    dpf::modint<10> e(2050); // 2050 % 2^10 = 2
    dpf::modint<10> f(1026); // 1026 % 2^10 = 2
    std::cout << e * f << std::endl; // = 4

    dpf::modint<10> g(2052); // 2052 % 2^10 = 4
    dpf::modint<10> h(1026); // 1026 % 2^10 = 2
    std::cout << g / h << std::endl; // = 2

    dpf::modint<10> i(2048); // 2048 % 2^10 = 0
    dpf::modint<10> j(1024); // 1024 % 2^10 = 0
    std::cout << (i == j) << std::endl; // = 1
}