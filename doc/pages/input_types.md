<!-- # Input Types {#input_types} -->

An *input type* is the type used as the domain for the `x`-coordinate of a
DPF. `libdpf++` ships with native support for a number of convenient input
types, which are enumerated below. See also the [formal requirements](@ref custom_input_types)
for a type not listed below to be used as an input type.

Within the `libdpf++` source, the (typically deduced) template parameter
`typename InputT` indicates the input type of the DPF under consideration.
Moreover, the `dpf::dpf_key` class (and some others) publicly expose the
clause
```
    using input_type = InputT;
```
providing an easy way to programmatically determine the input type.

# Integer scalar types
Any integer scalar type&mdash;that is, any type `T` such that
`std::numeric_limits<T>::is_integer == true`&mdash;may be used as an input
type. In general, you should always opt for the "shortest" such type that
suits your needs, as shorter bitlengths translate to smaller DPF keys and
faster evaluations thereof.

**Pro tip**\n
Prefer the use of [fixed width integer types](https://en.cppreference.com/w/cpp/types/integer)
over [fundamental integer types](https://en.cppreference.com/w/cpp/language/types)
for specifying input types. For example, use `uint16_t` in place of
`unsigned short`, or `int32_t` in place of `int`. Doing so improves
portability and makes it easier to keep track of the resulting DPF depth.

**See also**\n
The `dpf::modint` class template for custom-bitlength integer types that
allow tighter control over size of DPF keys.

**Code samples**\n
<div class="tabbed">

  - <b class="tab-title">integral_types.cpp</b>  \include{cpp} input_types/integral_types.cpp

</div>

# Extended-precision integer scalar types

The extended-precision (`128`-bit) integer scalar types provided as
compiler extensions by most major C++ compilers (e.g.,  `__int128` and `unsigned __int128`), including `g++` and
`clang++` when compiling for `64`-bit targets. (As these types are not
defined in the C++17 standard, `std::numeric_limits` is not specialized 
for them, so that `std::numeric_limits<__int128>::is_integer` returns
`false`.)

**Pro tip**\n
Use `simde_uint128` (provided courtesy of (SIMD Everywhere)[https://github.com/simd-everywhere/simde])
to declare such 128-bit integers in compiler-independent way.

**Code samples**\n
<div class="tabbed">

  - <b class="tab-title">extended_types.cpp</b> \include{cpp} input_types/extended_types.cpp

</div>

# dpf::modint<Nbits>

Arbitrary-, yet fixed-bitlength unsigned integer types. `dpf::modint` is a
lightweight class template that adapts one of the above-mentioned integer
types for arithmetic modulo `2^Nbits`, where `std::size_t Nbits` is the
template parameter. The specialization chooses an appropriate type for
the underlying integer and uses bitmasking to lazily reduce that integer
when the value is read (by anything other than a like-sized `modint`).
This ensures that arithmetic on `modint`s is just as fast as arithmetic on
the underlying integer type. Currently, only specializations having
`Nbits` between `1` and `128` inclusive are supported.

**Pro tip**\n
Choose the smallest `Nbits` possible to get DPFs of the shortest length --
and with the fastest evaluations -- possible.

**Defined in**\n
@ref dpf/modint.hpp

**Code samples**\n
<div class="tabbed">

  - <b class="tab-title">modint.cpp</b>  \include{cpp} input_types/modint.cpp

</div>

# dpf::bitstring<Nbits>

Arbitrary-, yet fixed- bitlength binary strings types. `dpf::bitstring` is
a class template that represents a binary string of any given length.
Compared with `dpf::modint`, a `dpf::bitstring` is well suited to cases
where inputs do not semantically stand for numerical values. For example,
the input may be a pseudorandom identifier or a cryptographic key. There
is no fixed limit on the acceptable bitlength for a `dpf::bitstring`.

In contrast with `dpf::modint`, which uses a (possibly extended-precision)
integer type for its internal representation, the `dpf::bitstring` class
template derives from `dpf::static_bit_array` and, therefore, provides a
wealth of methods and helpers for interacting with its individual bits.

**Pro tip**\n
As always, choose the smallest `Nbits` possible to get DPFs of the
shortest length -- and with the fastest evaluations -- possible.

**Defined in**\n
@ref dpf/bitstring.hpp

**See also**\n
`dpf::bit` and `dpf::static_bit_array`

**Code samples**\n
<div class="tabbed">

  - <b class="tab-title">bitstring.cpp</b>  \include{cpp} input_types/bitstring.cpp

</div>

# dpf::keyword<Alphabet, N>

Fixed-length strings over restricted alphabets. `dpf::keyword` is an alias
for the class template `dpf::basic_fixed_length_string`, which represents
a string of length `N` consisting solely of letters from
`alphabet`, where `std::size_t N` and `static const char alphabet[]` are
template parameters. To a first approximation, the `dpf::keyword` class
template views each eligible string as an integer expressed in
radix-`std::strlen(alphabet)` and then stores the associated binary number as its
internal representation. This produces representations that are
*significantly* shorter than that of the associated C-string, especially
when `alphabet` comprises few elements.

For example
\code{cpp}
char cstr = "7fffae02";
std::cout << sizeof(cstr)*CHAR_BIT << "\n"; // prints 64

using keyword = dpf::keyword<8, dpf::alphabets::hex>;
keyword str = "7fffae02";
std::cout << dpf::bitlength_of(str) << "\n"; // prints 32

using keyword2 = dpf::keyword<8, dpf::alphabets::alphanumeric>;
keyword2 str2 = "7fffae02";
std::cout << dpf::bitlength_of(str2) << "\n"; // prints 48

static const char * my_alphabet = "7fae02";
using keyword3 = dpf::keyword<8, my_alphabet>;
keyword3 str3 = "7fffae02";
std::cout << dpf::bitlength_of(str2) << "\n"; // prints 21

using keyword = dpf::keyword<8, dpf::alphabets::lowercase_alpha>;
keyword str = "7fffae02";                     // error (disallowed chars)
\endcode

**Pro tip**\n
Strings implicitly padded to length `N` with "zeros"; i.e., with the first
letter in `alphabet`. To allow for strings of length *less than* `N`,
simply set ``alphabet[0]='\0'``.

**Defined in**\n
@ref dpf/keyword.hpp

**See also**\n
The `dpf::alphabets` namespace for a catalog of predefined alphabets.

**Code samples**\n
<div class="tabbed">

  - <b class="tab-title">keyword.cpp</b>  \include{cpp} input_types/keyword.cpp

</div>

# dpf::xor_wrapper<T>

An element of `GF(2)^N` for `N=8*sizeof(T)`. `xor_wrapper` is a
lightweight class template that adapts "integer-like" types so that
arithmetic behaves like component-wise `GF(2)^N` arithmetic; that is,
(binary `+` and `-` both become `^`; binary `*` becomes `&`; unary `-`
becomes a nop). This is useful for cases where instances of the input type
are to be XOR-shared among two parties. `xor_wrapper` may be specialized
with any of the above-mentioned input types.

**Defined in**\n
@ref dpf/xor_wrapper.hpp

**Code samples**\n
<div class="tabbed">

  - <b class="tab-title">xor_wrapper</b>  \include{cpp} input_types/xor_wrapper.cpp

</div>

- - -

# Custom input type requirements {#custom_input_types}

If you need to use an input type not natively supported by `libdpf++`, there
is a straightforward path to doing so.

Suppose we wish to use as an input type the struct
\code{cpp}
struct input_type { int32_t i; };
\endcode

We will walk through the process of making `input_type` a useable input type.
First, let's try to compile the line
\code{cpp}
auto [dpf0, dpf1] = dpf::make_dpf(input_type{1});
\endcode

The first error we encounter involves the default specialization of
`dpf::utils::msb_of<T>`, which assumes `T` will be an integral type\code.
Essentially, we need to tell `dpf_key` how to get a "*bitmask*" that points
at the most-significant bit of an `input_type`. (The scare quotes is because
the mask may not be a mask at all; e.g., `dpf::bitstring::bit_mask`.)

\code{cpp}
// inside namespace dpf::utils
template <> struct msb_of<input_type>
  : public std::integral_constant<int32_t, int32_t(1 << bitlength_of_v<int32_t>-1)> { };
\endcode

The next error we get is this expression

\code{gcc}
error: no match for ‘operator&’ (operand types are ‘int’ and ‘foo’)
                  bool bit = !!(mask & x);
\endcode

This is the major requirement for `msb_of`: the value it returns must be useable
in the expression `bool bit = !!(mask & x)` to determine whether to go left or
right at the masked bit. (The other requirement, which we would see here, is
that `operator>>=` must be overloaded so that `mask >>= 1` reduces the
significance of the masked bit by `1`. We won't encounter that in this
example.)

\code{cpp}
auto operator&(int x, input_type y) { return x & y.i; }
\endcode

Now we see that `dpf::offset_within_block` is trying to invoke `operator%`
via the `dpf::mod_pow_2` function.

\code{cpp}
auto operator%(foo x, long unsigned int y) { return x.i % y; }
\endcode

Now everything compiles without issues. 

Now suppose we wish to perform a `dpf::eval_interval` or `dpf::eval_full`. The
first error we see is in `interval_memoizer.hpp`:
\code{gcc}
error: no match for ‘operator>’ (operand types are ‘input_type’ and ‘input_type’)
       if (from > to)
\end{gcc}

For this, we must overload `operator>`:
\code{cpp}
auto operator>(input_type lhs, input_type rhs) { return lhs.value % rhs.value; }
\endcode
