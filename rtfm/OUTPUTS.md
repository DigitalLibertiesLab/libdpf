## Output types {#output_types}

  - `T` such that `std::numeric_limits<T>::is_integer`

    Any of the standard integer types (both signed and unsigned) may be used
    as an input.

  - `__int128` and `unsigned __int128`

    The 128-bit integer types supported (as a compiler extension) by most
    major compilers.

  - `dpf::bit`

    This is a special class that represents a single bit. It is functionally
    equivalent to `bool` except, unlike with bool, if you use `dpf::bit` as an
    output type, you get access to some fancy [iterators](@ref iterables).

  - `dpf::fixedpoint`

  - `dpf::bitstring`

  - `dpf::wildcard_value`

  - `dpf::xor_wrapper`

  - `T` such that `std::is_trivially_copyable<T>::value`

    Really anything that can be copied is fair game.
