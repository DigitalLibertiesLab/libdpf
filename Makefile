.PHONY: docs
docs:
	@mkdir -p build/docs
	@sed    -e '/%LIBDPF_INCLUDE_GETTING_STARTED_INTRODUCTION%/{r doc/pages/introduction.md' -e 'd}' doc/libdpf.md > doc/libdpf_full.md
	@sed -i -e '/%LIBDPF_INCLUDE_GETTING_DPF_BASICS%/{r doc/pages/basics.md' -e 'd}' doc/libdpf_full.md
	@sed -i -e '/%LIBDPF_INCLUDE_GETTING_STARTED_INPUT_TYPES%/{r doc/pages/input_types.md' -e 'd}' doc/libdpf_full.md
	@sed -i -e '/%LIBDPF_INCLUDE_GETTING_STARTED_OUTPUT_TYPES%/{r doc/pages/output_types.md' -e 'd}' doc/libdpf_full.md
	@sed -i -e '/%LIBDPF_INCLUDE_GETTING_STARTED_EVALUATION%/{r doc/pages/evaluation.md' -e 'd}' doc/libdpf_full.md
	@sed -i -e '/%LIBDPF_INCLUDE_GETTING_STARTED_ITERABLES%/{r doc/pages/iterables.md' -e 'd}' doc/libdpf_full.md
	@sed -i -e '/%LIBDPF_INCLUDE_PIR_PIR1%/{r doc/pages/pir1.md' -e 'd}' doc/libdpf_full.md
	@sed -i -e '/%LIBDPF_INCLUDE_MISC_BUGS%/{r BUGS.md' -e 'd}' doc/libdpf_full.md
	@sed -i -e '/%LIBDPF_INCLUDE_MISC_CHANGES%/{r CHANGES.md' -e 'd}' doc/libdpf_full.md
	@sed -i -e '/%LIBDPF_INCLUDE_MISC_TODO%/{r TODO.md' -e 'd}' doc/libdpf_full.md
	@sed -i -e '/%LIBDPF_INCLUDE_MISC_SUBMODULES%/{r doc/pages/submodules.md' -e 'd}' doc/libdpf_full.md
	@sed -i -e '/%LIBDPF_INCLUDE_MISC_AUTHORS%/{r AUTHORS.md' -e 'd}' doc/libdpf_full.md
	@sed -i -e '/%LIBDPF_INCLUDE_MISC_LICENSE%/{r LICENSE.md' -e 'd}' doc/libdpf_full.md
	@sed -i -e '/%LIBDPF_INCLUDE_CODE_LISTINGS%/{r doc/pages/listings.md' -e 'd}' doc/libdpf_full.md
	@doxygen doc/Doxyfile