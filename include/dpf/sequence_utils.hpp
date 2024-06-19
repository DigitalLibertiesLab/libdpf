#ifndef LIBDPF_INCLUDE_DPF_SEQUENCE_UTILS_HPP__
#define LIBDPF_INCLUDE_DPF_SEQUENCE_UTILS_HPP__

namespace dpf
{

struct return_type_tag_{};

struct return_entire_node_tag_ final : public return_type_tag_ {};
// static constexpr auto return_entire_node_tag = return_entire_node_tag_{};

struct return_output_only_tag_ final : public return_type_tag_ {};
// static constexpr auto return_output_only_tag = return_output_only_tag_{};

}  // namespace dpf

#endif  // LIBDPF_INCLUDE_DPF_SEQUENCE_UTILS_HPP__
