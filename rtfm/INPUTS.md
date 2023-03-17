## Input types

  - `T` such that `std::numeric_limits<T>::is_integer`

    Any of the standard integer types (both signed and unsigned) may be used
    as an input. In general, you should use the smallest type that suits your
    needs, as shorter bitlengths translate to small DPF keys and faster
    evaluations. (Don't miss `dpf::modint`.)

  - `__int128` and `unsigned __int128`

    The 128-bit integer types supported (as a compiler extension) by most
    major compilers.

  - `dpf::modint`

    A lightweight wrapper around a built-in integer type that lazily truncates
    results to a given bitlength (i.e. reduces modulo `2^n`). Selecting th
    smallest suitable `n` produces DPFs of the smallest possible size.

  - `dpf::bitstring`

    A class representing a binary string with that does not semantically stand
    for a numerical value.

  - `dpf::keyword`

    Fixed-length strings over a restricted alphabet. Taking advantage of the
    restrictions on the strings lets us encode each string as a (relatively)
    short integer. This makes it easy to implement DPFs that take keywords as
    input.

  - (maybe) `dpf::fixedpoint`

    This one is really meant as an output type, but it could be used as an
    input if you wanted.

  - `dpf::xor_wrapper`

    Any of the above may be wrapped in a `dpf::xor_wrapper`, a simple adapter
    class that makes `n`-bit integer arithmetic behave like component-wise
    `GF(2)^n` arithmetic (i.e., `+` and `-` becomes `^` while `*` becomes `&`)
