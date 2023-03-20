## TODO list

# Input types:
  - (optional) support for `NTL::ZZ_p`
  - (optional) support for `NTL::zz_p`
  - (optional) support for `gmp::mpz`

# Post-processing
  - `dpf::advice_bit_iterator` for iterating over advice bits
  - "linear sketch" for verifying DPF well-formedness
  - Sabre 2- and 3-verifier SNIPs for verifying DPF well-formedness
  - bitmore-style splicing for `dpf::bit`s

# Gen and eval:
  - Doerner-shelat distributed generation (`eval_full` style)
  - Doerner-shelat distributed generation (`eval_sequence` style)
  - Sabre-like 2- and (2+1)-PC DPF generation
  - bitmore-style multiple DPF generation
  - `add_leaf` function to increase arity at the leaf layer
  - `make_dpf` that writes keys to sockets
  - `json`-based representation of DPF keys

# PRGs:
  - LowMC-based PRG
  - ChaCha20-based PRG

# Extensions:
  - non-interactive DCF
  - incremental DPF
  - programmable DPF

# Wrappers:
  - Python wrapper
  - Rust wrapper
  - GO wrapper
  - Java wrapper

# Portability:
  - full big-endian support
  - full ARM support
  - AVX512 support
  - msvcc support

# Meta
  - node and pair type "traits"

# Tag file
  - automagically fetch the tagfile from cppreference.org