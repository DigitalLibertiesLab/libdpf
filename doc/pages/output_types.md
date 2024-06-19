<!-- # Output Types {#outut_types} -->

# Integer scalar types

# Extended-precision integer scalar types

# dpf::bit

# dpf::bitstring<Nbits>

# dpf::wildcard<T>
A `dpf::wildcard` is a struct template with a single parameter 
`T`, which must be a trivially copyable type (as indicated by 
`std::is_trivially_copyable<T>`). It is used as a placeholder 
for an instance of type `T`, which can be assigned later. Its 
intended to wrap an output types of a DPF.


# dpf::xor_wrapper<T>

# Custom output type requirements {#custom_output_types}