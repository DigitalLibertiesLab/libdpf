\mainpage notitle
<a id="mainpage"/>

[TOC]

![](libdpf-tagline.png)

# Introduction

## Motivation {#motivation}

## Features {#features}

## Credits {#credits}

## Disclaimer {#disclaimer}

- - -
<div style="float:left; color:#999;">&larr;&nbsp;Prev</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref basics)</div>



<!-- PAGE SEPARATOR -->



\page basics DPF Basics

[TOC]

<!-- # DPF Basics {#basics} -->

# Point functions {#point_functions}

# DPF Trees {#dpf_trees}

- - -
<div style="float:left; color:#999;">[&larr;&nbsp;Prev](mainpage)</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref getting_started)</div>



<!-- PAGE SEPARATOR -->



\page getting_started Getting Started

[TOC]

The getting started guide consists of the following subsections.

  - \subpage input_types
  - \subpage output_types
  - \subpage evaluation
  - \subpage iterables

- - -
<div style="float:left;">[&larr;&nbsp;Prev](@ref mainpage)</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref input_types)</div>



<!-- PAGE SEPARATOR -->



\page input_types Input Types

[TOC]

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

- - -
<div style="float:left;">[&larr;&nbsp;Prev](@ref getting_started)</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref output_types)</div>



<!-- PAGE SEPARATOR -->



\page output_types Output Types

[TOC]

<!-- # Output Types {#outut_types} -->

# Integer scalar types

# Extended-precision integer scalar types

# dpf::bit

# dpf::bitstring<Nbits>

# dpf::wildcard<T>

# dpf::xor_wrapper<T>

# Custom output type requirements {#custom_output_types}
- - -
<div style="float:left;">[&larr;&nbsp;Prev](@ref input_types)</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref evaluation)</div>



<!-- PAGE SEPARATOR -->



\page evaluation Evaluating DPFs

[TOC]

<!-- # Evaluating DPFs {#evalaution} -->

# dpf::eval_point

# dpf::eval_interval

# dpf::eval_full

# dpf::eval_sequence

# Output buffers

# Memoizers
- - -
<div style="float:left;">[&larr;&nbsp;Prev](@ref output_types)</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref iterables)</div>



<!-- PAGE SEPARATOR -->



\page iterables Iterables

[TOC]

<!-- # Iterables {#iterables} -->

# dpf::setbit_index_iterable

# dpf::subsequence_iterable

# dpf::subinterval_iterable

# dpf::zip_iterable

# dpf::parallel_bit_iterable

# dpf::advice_bit_iterable
- - -
<div style="float:left;">[&larr;&nbsp;Prev](@ref evaluation)</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref miscellany)</div>



<!-- PAGE SEPARATOR -->



\page miscellany Miscellany

[TOC]

The miscellany page consists of the following subpages.

  - \subpage bugs
  - \subpage changes
  - \subpage todo
  - \subpage submodules
  - \subpage authors
  - \subpage license

- - -
<div style="float:left;">[&larr;&nbsp;Prev](@ref evaluation)</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref bugs)</div>



<!-- PAGE SEPARATOR -->



\page bugs Bugs

[TOC]


- - -
<div style="float:left;">[&larr;&nbsp;Prev](@ref miscellany)</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref changes)</div>



<!-- PAGE SEPARATOR -->



\page changes CHANGELOG

[TOC]


- - -
<div style="float:left;">[&larr;&nbsp;Prev](@ref bugs)</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref todo)</div>



<!-- PAGE SEPARATOR -->



\page todo TODO list

[TOC]


- - -
<div style="float:left;">[&larr;&nbsp;Prev](@ref changes)</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref submodules)</div>



<!-- PAGE SEPARATOR -->



\page submodules Submodules

[TOC]


- - -
<div style="float:left;">[&larr;&nbsp;Prev](@ref todo)</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref authors)</div>



<!-- PAGE SEPARATOR -->



\page authors Authors

[TOC]

<!-- # Authors {#authors} -->

 - Christopher Jiang <christopher.jiang@ucalgary.ca>
 - Ryan Henry <ryan.henry@ucalgary.ca>
 - Kyle Storrier <kyle.storrier@ucalgary.ca>
 - Adithya Vadapalli <avadapal@uwaterloo.ca>

- - -
<div style="float:left;">[&larr;&nbsp;Prev](@ref submodules)</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref license)</div>



<!-- PAGE SEPARATOR -->



\page license License

[TOC]

### GNU GENERAL PUBLIC LICENSE

Version 2, June 1991

    Copyright (C) 1989, 1991 Free Software Foundation, Inc.  
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

    Everyone is permitted to copy and distribute verbatim copies
    of this license document, but changing it is not allowed.

### Preamble

The licenses for most software are designed to take away your freedom
to share and change it. By contrast, the GNU General Public License is
intended to guarantee your freedom to share and change free
software--to make sure the software is free for all its users. This
General Public License applies to most of the Free Software
Foundation's software and to any other program whose authors commit to
using it. (Some other Free Software Foundation software is covered by
the GNU Lesser General Public License instead.) You can apply it to
your programs, too.

When we speak of free software, we are referring to freedom, not
price. Our General Public Licenses are designed to make sure that you
have the freedom to distribute copies of free software (and charge for
this service if you wish), that you receive source code or can get it
if you want it, that you can change the software or use pieces of it
in new free programs; and that you know you can do these things.

To protect your rights, we need to make restrictions that forbid
anyone to deny you these rights or to ask you to surrender the rights.
These restrictions translate to certain responsibilities for you if
you distribute copies of the software, or if you modify it.

For example, if you distribute copies of such a program, whether
gratis or for a fee, you must give the recipients all the rights that
you have. You must make sure that they, too, receive or can get the
source code. And you must show them these terms so they know their
rights.

We protect your rights with two steps: (1) copyright the software, and
(2) offer you this license which gives you legal permission to copy,
distribute and/or modify the software.

Also, for each author's protection and ours, we want to make certain
that everyone understands that there is no warranty for this free
software. If the software is modified by someone else and passed on,
we want its recipients to know that what they have is not the
original, so that any problems introduced by others will not reflect
on the original authors' reputations.

Finally, any free program is threatened constantly by software
patents. We wish to avoid the danger that redistributors of a free
program will individually obtain patent licenses, in effect making the
program proprietary. To prevent this, we have made it clear that any
patent must be licensed for everyone's free use or not licensed at
all.

The precise terms and conditions for copying, distribution and
modification follow.

### TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

**0.** This License applies to any program or other work which
contains a notice placed by the copyright holder saying it may be
distributed under the terms of this General Public License. The
"Program", below, refers to any such program or work, and a "work
based on the Program" means either the Program or any derivative work
under copyright law: that is to say, a work containing the Program or
a portion of it, either verbatim or with modifications and/or
translated into another language. (Hereinafter, translation is
included without limitation in the term "modification".) Each licensee
is addressed as "you".

Activities other than copying, distribution and modification are not
covered by this License; they are outside its scope. The act of
running the Program is not restricted, and the output from the Program
is covered only if its contents constitute a work based on the Program
(independent of having been made by running the Program). Whether that
is true depends on what the Program does.

**1.** You may copy and distribute verbatim copies of the Program's
source code as you receive it, in any medium, provided that you
conspicuously and appropriately publish on each copy an appropriate
copyright notice and disclaimer of warranty; keep intact all the
notices that refer to this License and to the absence of any warranty;
and give any other recipients of the Program a copy of this License
along with the Program.

You may charge a fee for the physical act of transferring a copy, and
you may at your option offer warranty protection in exchange for a
fee.

**2.** You may modify your copy or copies of the Program or any
portion of it, thus forming a work based on the Program, and copy and
distribute such modifications or work under the terms of Section 1
above, provided that you also meet all of these conditions:

  
**a)** You must cause the modified files to carry prominent notices
stating that you changed the files and the date of any change.

  
**b)** You must cause any work that you distribute or publish, that in
whole or in part contains or is derived from the Program or any part
thereof, to be licensed as a whole at no charge to all third parties
under the terms of this License.

  
**c)** If the modified program normally reads commands interactively
when run, you must cause it, when started running for such interactive
use in the most ordinary way, to print or display an announcement
including an appropriate copyright notice and a notice that there is
no warranty (or else, saying that you provide a warranty) and that
users may redistribute the program under these conditions, and telling
the user how to view a copy of this License. (Exception: if the
Program itself is interactive but does not normally print such an
announcement, your work based on the Program is not required to print
an announcement.)

These requirements apply to the modified work as a whole. If
identifiable sections of that work are not derived from the Program,
and can be reasonably considered independent and separate works in
themselves, then this License, and its terms, do not apply to those
sections when you distribute them as separate works. But when you
distribute the same sections as part of a whole which is a work based
on the Program, the distribution of the whole must be on the terms of
this License, whose permissions for other licensees extend to the
entire whole, and thus to each and every part regardless of who wrote
it.

Thus, it is not the intent of this section to claim rights or contest
your rights to work written entirely by you; rather, the intent is to
exercise the right to control the distribution of derivative or
collective works based on the Program.

In addition, mere aggregation of another work not based on the Program
with the Program (or with a work based on the Program) on a volume of
a storage or distribution medium does not bring the other work under
the scope of this License.

**3.** You may copy and distribute the Program (or a work based on it,
under Section 2) in object code or executable form under the terms of
Sections 1 and 2 above provided that you also do one of the following:

  
**a)** Accompany it with the complete corresponding machine-readable
source code, which must be distributed under the terms of Sections 1
and 2 above on a medium customarily used for software interchange; or,

  
**b)** Accompany it with a written offer, valid for at least three
years, to give any third party, for a charge no more than your cost of
physically performing source distribution, a complete machine-readable
copy of the corresponding source code, to be distributed under the
terms of Sections 1 and 2 above on a medium customarily used for
software interchange; or,

  
**c)** Accompany it with the information you received as to the offer
to distribute corresponding source code. (This alternative is allowed
only for noncommercial distribution and only if you received the
program in object code or executable form with such an offer, in
accord with Subsection b above.)

The source code for a work means the preferred form of the work for
making modifications to it. For an executable work, complete source
code means all the source code for all modules it contains, plus any
associated interface definition files, plus the scripts used to
control compilation and installation of the executable. However, as a
special exception, the source code distributed need not include
anything that is normally distributed (in either source or binary
form) with the major components (compiler, kernel, and so on) of the
operating system on which the executable runs, unless that component
itself accompanies the executable.

If distribution of executable or object code is made by offering
access to copy from a designated place, then offering equivalent
access to copy the source code from the same place counts as
distribution of the source code, even though third parties are not
compelled to copy the source along with the object code.

**4.** You may not copy, modify, sublicense, or distribute the Program
except as expressly provided under this License. Any attempt otherwise
to copy, modify, sublicense or distribute the Program is void, and
will automatically terminate your rights under this License. However,
parties who have received copies, or rights, from you under this
License will not have their licenses terminated so long as such
parties remain in full compliance.

**5.** You are not required to accept this License, since you have not
signed it. However, nothing else grants you permission to modify or
distribute the Program or its derivative works. These actions are
prohibited by law if you do not accept this License. Therefore, by
modifying or distributing the Program (or any work based on the
Program), you indicate your acceptance of this License to do so, and
all its terms and conditions for copying, distributing or modifying
the Program or works based on it.

**6.** Each time you redistribute the Program (or any work based on
the Program), the recipient automatically receives a license from the
original licensor to copy, distribute or modify the Program subject to
these terms and conditions. You may not impose any further
restrictions on the recipients' exercise of the rights granted herein.
You are not responsible for enforcing compliance by third parties to
this License.

**7.** If, as a consequence of a court judgment or allegation of
patent infringement or for any other reason (not limited to patent
issues), conditions are imposed on you (whether by court order,
agreement or otherwise) that contradict the conditions of this
License, they do not excuse you from the conditions of this License.
If you cannot distribute so as to satisfy simultaneously your
obligations under this License and any other pertinent obligations,
then as a consequence you may not distribute the Program at all. For
example, if a patent license would not permit royalty-free
redistribution of the Program by all those who receive copies directly
or indirectly through you, then the only way you could satisfy both it
and this License would be to refrain entirely from distribution of the
Program.

If any portion of this section is held invalid or unenforceable under
any particular circumstance, the balance of the section is intended to
apply and the section as a whole is intended to apply in other
circumstances.

It is not the purpose of this section to induce you to infringe any
patents or other property right claims or to contest validity of any
such claims; this section has the sole purpose of protecting the
integrity of the free software distribution system, which is
implemented by public license practices. Many people have made
generous contributions to the wide range of software distributed
through that system in reliance on consistent application of that
system; it is up to the author/donor to decide if he or she is willing
to distribute software through any other system and a licensee cannot
impose that choice.

This section is intended to make thoroughly clear what is believed to
be a consequence of the rest of this License.

**8.** If the distribution and/or use of the Program is restricted in
certain countries either by patents or by copyrighted interfaces, the
original copyright holder who places the Program under this License
may add an explicit geographical distribution limitation excluding
those countries, so that distribution is permitted only in or among
countries not thus excluded. In such case, this License incorporates
the limitation as if written in the body of this License.

**9.** The Free Software Foundation may publish revised and/or new
versions of the General Public License from time to time. Such new
versions will be similar in spirit to the present version, but may
differ in detail to address new problems or concerns.

Each version is given a distinguishing version number. If the Program
specifies a version number of this License which applies to it and
"any later version", you have the option of following the terms and
conditions either of that version or of any later version published by
the Free Software Foundation. If the Program does not specify a
version number of this License, you may choose any version ever
published by the Free Software Foundation.

**10.** If you wish to incorporate parts of the Program into other
free programs whose distribution conditions are different, write to
the author to ask for permission. For software which is copyrighted by
the Free Software Foundation, write to the Free Software Foundation;
we sometimes make exceptions for this. Our decision will be guided by
the two goals of preserving the free status of all derivatives of our
free software and of promoting the sharing and reuse of software
generally.

**NO WARRANTY**

**11.** BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO
WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.
EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR
OTHER PARTIES PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE
PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME
THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

**12.** IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN
WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY
AND/OR REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU
FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE
PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING
RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A
FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF
SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
DAMAGES.

### END OF TERMS AND CONDITIONS

### How to Apply These Terms to Your New Programs

If you develop a new program, and you want it to be of the greatest
possible use to the public, the best way to achieve this is to make it
free software which everyone can redistribute and change under these
terms.

To do so, attach the following notices to the program. It is safest to
attach them to the start of each source file to most effectively
convey the exclusion of warranty; and each file should have at least
the "copyright" line and a pointer to where the full notice is found.

    one line to give the program's name and an idea of what it does.
    Copyright (C) yyyy  name of author

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

Also add information on how to contact you by electronic and paper
mail.

If the program is interactive, make it output a short notice like this
when it starts in an interactive mode:

    Gnomovision version 69, Copyright (C) year name of author
    Gnomovision comes with ABSOLUTELY NO WARRANTY; for details
    type `show w'.  This is free software, and you are welcome
    to redistribute it under certain conditions; type `show c' 
    for details.

The hypothetical commands \`show w' and \`show c' should show the
appropriate parts of the General Public License. Of course, the
commands you use may be called something other than \`show w' and
\`show c'; they could even be mouse-clicks or menu items--whatever
suits your program.

You should also get your employer (if you work as a programmer) or
your school, if any, to sign a "copyright disclaimer" for the program,
if necessary. Here is a sample; alter the names:

    Yoyodyne, Inc., hereby disclaims all copyright
    interest in the program `Gnomovision'
    (which makes passes at compilers) written 
    by James Hacker.

    signature of Ty Coon, 1 April 1989
    Ty Coon, President of Vice

This General Public License does not permit incorporating your program
into proprietary programs. If your program is a subroutine library,
you may consider it more useful to permit linking proprietary
applications with the library. If this is what you want to do, use the
[GNU Lesser General Public
License](https://www.gnu.org/licenses/lgpl.html) instead of this
License.
Running

- - -
<div style="float:left;">[&larr;&nbsp;Prev](@ref authors)</div>
<div style="float:right;">[Next&nbsp;&rarr;](@ref listings)</a></div>



<!-- PAGE SEPARATOR -->



\page listings Code Listings

[TOC]

<!-- # Code Listings {#listings} -->

  - \subpage input_type_examples
  - \subpage output_type_examples
  - \subpage evaluation_examples
  - \subpage iteratable_examples

\page input_type_examples input_types

  - \subpage input_types_2integral_types_8cpp
  - \subpage input_types_2extended_types_8cpp
  - \subpage input_types_2modint_8cpp
  - \subpage input_types_2bitstring_8cpp
  - \subpage input_types_2keyword_8cpp
  - \subpage input_types_2xor_wrapper_8cpp
  - \subpage input_types_2custom_8cpp

  \page "input_types_2integral_types_8cpp" input_types/integral_types.cpp
  \include{cpp} input_types/integral_types.cpp

  \page "input_types_2extended_types_8cpp" input_types/extended_types.cpp
  \include{cpp} input_types/extended_types.cpp

  \page "input_types_2modint_8cpp" input_types/modint.cpp
  \include{cpp} input_types/modint.cpp

  \page "input_types_2bitstring_8cpp" input_types/bitstring.cpp
  \include{cpp} input_types/bitstring.cpp

  \page "input_types_2keyword_8cpp" input_types/keyword.cpp
  \include{cpp} input_types/keyword.cpp

  \page "input_types_2xor_wrapper_8cpp" input_types/xor_wrapper.cpp
  \include{cpp} input_types/xor_wrapper.cpp

  \page "input_types_2custom_8cpp" input_types/custom.cpp
  \include{cpp} input_types/custom.cpp

\page output_type_examples output_types

  - \subpage output_types_2integral_types_8cpp
  - \subpage output_types_2extended_types_8cpp
  - \subpage output_types_2bit_8cpp
  - \subpage output_types_2bitstring_8cpp
  - \subpage output_types_2wildcard_8cpp
  - \subpage output_types_2xor_wrapper_8cpp
  - \subpage output_types_2custom_8cpp

  \page "output_types_2integral_types_8cpp" output_types/integral_types.cpp
  \include{cpp} output_types/integral_types.cpp

  \page "output_types_2extended_types_8cpp" output_types/extended_types.cpp
  \include{cpp} output_types/extended_types.cpp

  \page "output_types_2bit_8cpp" output_types/bit.cpp
  \include{cpp} output_types/bit.cpp

  \page "output_types_2bitstring_8cpp" output_types/bitstring.cpp
  \include{cpp} output_types/bitstring.cpp

  \page "output_types_2wildcard_8cpp" output_types/wildcard.cpp
  \include{cpp} output_types/wildcard.cpp

  \page "output_types_2xor_wrapper_8cpp" output_types/xor_wrapper.cpp
  \include{cpp} output_types/xor_wrapper.cpp

  \page "output_types_2custom_8cpp" output_types/custom.cpp
  \include{cpp} output_types/custom.cpp

\page evaluation_examples evaluation

  - \subpage evaluation_2eval_point_8cpp
  - \subpage evaluation_2eval_interval_8cpp
  - \subpage evaluation_2eval_full_8cpp
  - \subpage evaluation_2eval_sequence_8cpp
  - \subpage evaluation_2memoizers_8cpp
  - \subpage evaluation_2output_buffers_8cpp

  \page "evaluation_2eval_point_8cpp" evaluation/eval_point.cpp
  \include{cpp} evaluation/eval_point.cpp

  \page "evaluation_2eval_interval_8cpp" evaluation/eval_interval.cpp
  \include{cpp} evaluation/eval_interval.cpp

  \page "evaluation_2eval_full_8cpp" evaluation/eval_full.cpp
  \include{cpp} evaluation/eval_full.cpp

  \page "evaluation_2eval_sequence_8cpp" evaluation/eval_sequence.cpp
  \include{cpp} evaluation/eval_sequence.cpp

  \page "evaluation_2memoizers_8cpp" evaluation/memoizers.cpp
  \include{cpp} evaluation/memoizers.cpp

  \page "evaluation_2output_buffers_8cpp" evaluation/output_buffers.cpp
  \include{cpp} evaluation/output_buffers.cpp

\page iteratable_examples iterables

  - \subpage iterables_2setbit_index_iterable_8cpp
  - \subpage iterables_2advice_bit_iterable_8cpp
  - \subpage iterables_2parallel_bit_iterable_8cpp
  - \subpage iterables_2subinterval_iterable_8cpp
  - \subpage iterables_2subsequence_iterable_8cpp
  - \subpage iterables_2zip_iterable_8cpp

  \page "iterables_2setbit_index_iterable_8cpp" iterables/setbit_index_iterable.cpp
  \include{cpp} iterables/setbit_index_iterable.cpp

  \page "iterables_2advice_bit_iterable_8cpp" iterables/advice_bit_iterable.cpp
  \include{cpp} iterables/advice_bit_iterable.cpp

  \page "iterables_2parallel_bit_iterable_8cpp" iterables/parallel_bit_iterable.cpp
  \include{cpp} iterables/parallel_bit_iterable.cpp

  \page "iterables_2subinterval_iterable_8cpp" iterables/subinterval_iterable.cpp
  \include{cpp} iterables/subinterval_iterable.cpp

  \page "iterables_2subsequence_iterable_8cpp" iterables/subsequence_iterable.cpp
  \include{cpp} iterables/subsequence_iterable.cpp

  \page "iterables_2zip_iterable_8cpp" iterables/zip_iterable.cpp
  \include{cpp} iterables/zip_iterable.cpp
- - -
<div style="float:left;">[&larr;&nbsp;Prev](@ref license)</div>
<div style="float:right; color:#999;">Next&nbsp;&rarr;</div>
