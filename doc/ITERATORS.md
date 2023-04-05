## Iterables {#iterables}

Iterables are wrappers that provide access to some means of iteration over the
object the wrap.

### The `dpf::setbit_index_iterable` class

The `dpf::setbit_index_iterable` class facilitates efficient iteration over the *indexes*
of the bits which are *set* in a potentially long bitstring. It wraps classes
that inherit from `dpf::bit_array_base`, such as `dpf::static_bit_array`,
`dpf::dynamic_bit_array`, and `dpf::bitstring`. This iterable is heavily
optimized and will handily outperform bit-by-bit traversal through the string.

This functionality is useful in PIR-like applications, where the set bits
might indicate which records to include as operands to an exclusive-OR. It
can be instantiated and used in the expected way,

```cpp
dpf::static_bit_array<num_bits> my_bits;
// ...
setbit_index_iterable setbits(my_bits);
for (auto idx : setbits) { do_important_thing(idx); } ```

or with some syntactic sugar

```cpp
dpf::static_bit_array<num_bits> my_bits;
// ...
for (auto idx : indices_set_in(my_bits)) { do_important_thing(idx); }```

or, if you prefer,

```cpp
dpf::static_bit_array<num_bits> my_bits;
// ...
for_each_set_index(my_bits, [](std::size_t i){ do_important_thing_from_lambda(i); });```

### The `dpf::parallel_bit_iterable` class

The `dpf::parallel_bit_iterable` class facilitates efficient bit-by-bit iteration through the bits several bitstrings simultaneously. Dereferencing its iterators yields an array of `bool`s indicating which bitstrings have the current bit set and which do not. It wraps classes that inherit from `dpf::bit_array_base`, such as `dpf::static_bit_array`, `dpf::dynamic_bit_array`, and `dpf::bitstring`.

The number of bitstrings to process in parallel is given as a template parameter; i.e., it must be known at compile time. It can be as low as `2` and as high as `32`. (**Pro tip:** For optimal efficiency, stick to powers of `2`.) The iterators use SIMD operations to extract the bits in parallel at a notably lower cost than traversing each bitstring in serial.

A `dpf::parallel_bit_iterable`is well-suited to settings such as a PIR server seeking to amortize the cost of memory I/O by processing many requests in parallel. It can be instantiated and used in the expected way,

```cpp
dpf::static_bit_array<num_bits> my_bits0, my_bits1, /*...,*/ my_bitsN;
// ...
setbit_index_iterable setbits(my_bits);
for (auto idx : setbits) { do_important_thing(idx); }```

or with some syntactic sugar

```cpp
dpf::static_bit_array<num_bits> my_bits0, my_bits1, /*...,*/ my_bitsN;
// ...
for (auto b : batch_of(my_bits0, mybits_1, /*...,*/ my_bitsN))
{
    assert(std::is_same_v<decltype(b), std::array<bool, N>>);
    do_important_thing(b);
}```

or, if you prefer,
```cpp
dpf::static_bit_array<num_bits> my_bits0, my_bits1, /*...,*/ my_bitsN;
// ...
for_each_set_index(my_bits0, mybits_1, /*...,*/ my_bitsN,
    [](std::array<bool, N> b){ do_important_thing_from_lambda(b); });```

### The `dpf::subsequence_iterable` class

    Iterate over an arbitrary subsequence of an array-like object. Instances
    of this iterable are returned by `dpf::eval_sequence`.

### The `dpf::subinterval_iterable` class

    Iterate over an arbitrary subsequence of an array-like object. Instances
    of this iterable are returned by `dpf::eval_sequence`.

### The `dpf::advice_bit_iterable` class

