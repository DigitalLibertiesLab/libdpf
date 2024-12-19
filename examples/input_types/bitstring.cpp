#include <iostream>
#include "dpf.hpp"

int main() {
    // Define a bit array with 10 bits
    using BitArrayBase = dpf::bit_array_base<int>;
    using ConcreteBitArray = dpf::bit_array_base<BitArrayBase>;

    ConcreteBitArray arr(10);

    // Set some bits to true
    arr.set(1);
    arr.set(3);
    arr.set(5);

    // Print the bit array
    std::cout << "Bit Array: ";
    for (std::size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i];
    }
    std::cout << std::endl;

    // Check if specific bits are set
    std::cout << "Bit at index 1 is set: " << arr.test(1) << std::endl;
    std::cout << "Bit at index 2 is set: " << arr.test(2) << std::endl;

    // Count the number of bits set to true
    std::cout << "Number of bits set to true: " << arr.count() << std::endl;

    // Toggle some bits
    arr.flip(3);
    arr.flip(6);

    // Print the modified bit array
    std::cout << "Modified Bit Array: ";
    for (std::size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i];
    }
    std::cout << std::endl;

    return 0;
}
