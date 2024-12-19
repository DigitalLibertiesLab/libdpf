/// @file dpf/emplace.hpp
/// @brief Defines various template structures for efficient emplacement of
///        `dpf::dpf_key` objects.
/// @details Provides specialized template structures for the in-place
///          construction ("emplacing") of `dpf::dpf_key` objects into different
///          types of pre-allocated storage including smart pointers
///          (`std::unique_ptr` and `std::shared_ptr`), `std::optional`,
///          `std::variant`, raw pointers, and `std::reference_wrapper`s, as
///          well as containers that support `emplace_back`. The goal is to
///          facilitate efficient construction and storage of `dpf::dpf_key`
///          objects received from a dealer over a socket.
///
///          The emplacement functionalities are specialized for different
///          storage types to handle their unique construction requirements.
/// @copyright Copyright (c) 2019-2024 Ryan Henry and [others](@ref authors)
/// @license Released under a GNU General Public v2.0 (GPLv2) license;
///          see [LICENSE.md](@ref license) for details.

#ifndef LIBDPF_INCLUDE_DPF_EMPLACE_HPP__
#define LIBDPF_INCLUDE_DPF_EMPLACE_HPP__

#include <variant>
#include <memory>
#include <optional>
#include <functional>

namespace dpf
{

namespace utils
{

/// @brief Emplaces a `dpf::dpf_key` object into the specified, pre-allocated memory.
/// @tparam DpfKey The concrete specialization of `dpf::dpf_key` to construct.
/// @param storage Reference to the container where the `dpf::dpf_key` object will be emplaced.
/// @param root The root node used by the `dpf::dpf_key`.
/// @param correction_words Correction words array for the `dpf::dpf_key`.
/// @param correction_advice Correction advice array for the `dpf::dpf_key`.
/// @param leaves Leaf-node tuple for the `dpf::dpf_key`.
/// @param beavers Beaver tuple for the `dpf::dpf_key`.
/// @param offset_share The offset share (default: `0`).

/// @defgroup EmplaceFunctions Emplace Functions
/// @brief Group of functions for emplacing `dpf::dpf_key` objects into
///        pre-allocated memory.
/// @{

template <typename DpfKey, typename T>
struct dpf_emplacer
{
    using dpf_key = DpfKey;
    using interior_node = typename DpfKey::interior_node;
    using correction_words_array = typename DpfKey::correction_words_array;
    using correction_advice_array = typename DpfKey::correction_advice_array;
    using leaf_tuple = typename DpfKey::leaf_tuple;
    using beaver_tuple = typename DpfKey::beaver_tuple;
    using input_type = typename DpfKey::input_type;

    /// @brief Generic version is intentionally left undefined.
    static auto emplace(T & storage,
        const interior_node & root,
        const correction_words_array & correction_words,
        const correction_advice_array & correction_advice,
        const leaf_tuple & leaves,
        const beaver_tuple & beavers,
        const input_type & offset_share);
};

/// @brief Specialization for `std::unique_ptr`.
template <typename DpfKey>
struct dpf_emplacer<DpfKey, std::unique_ptr<DpfKey>>
{
    using dpf_key = DpfKey;
    using interior_node = typename DpfKey::interior_node;
    using correction_words_array = typename DpfKey::correction_words_array;
    using correction_advice_array = typename DpfKey::correction_advice_array;
    using leaf_tuple = typename DpfKey::leaf_tuple;
    using beaver_tuple = typename DpfKey::beaver_tuple;
    using input_type = typename DpfKey::input_type;

    HEDLEY_ALWAYS_INLINE
    static auto emplace(
        std::unique_ptr<DpfKey> & storage,
        const interior_node & root,
        const correction_words_array & correction_words,
        const correction_advice_array & correction_advice,
        const leaf_tuple & leaves,
        const beaver_tuple & beavers,
        const input_type & offset_share)
    {
        if (!storage)
        {
            storage.reset(new DpfKey(root, correction_words, correction_advice, leaves, beavers, offset_share));
        }
        else
        {
            storage->~DpfKey();
            new (storage.get()) DpfKey(root, correction_words, correction_advice, leaves, beavers, offset_share);
        }
    }
};

template <typename DpfKey>
struct dpf_emplacer<DpfKey, std::shared_ptr<DpfKey>>
{
    using dpf_key = DpfKey;
    using interior_node = typename DpfKey::interior_node;
    using correction_words_array = typename DpfKey::correction_words_array;
    using correction_advice_array = typename DpfKey::correction_advice_array;
    using leaf_tuple = typename DpfKey::leaf_tuple;
    using beaver_tuple = typename DpfKey::beaver_tuple;
    using input_type = typename DpfKey::input_type;

    HEDLEY_ALWAYS_INLINE
    static auto emplace(
        std::shared_ptr<DpfKey> & storage,
        const interior_node & root,
        const correction_words_array & correction_words,
        const correction_advice_array & correction_advice,
        const leaf_tuple & leaves,
        const beaver_tuple & beavers,
        const input_type & offset_share)
    {
        if (!storage)
        {
            storage.reset(new DpfKey(root, correction_words, correction_advice, leaves, beavers, offset_share));
        }
        else
        {
            storage->~DpfKey(); 
            new (storage.get()) DpfKey(root, correction_words, correction_advice, leaves, beavers, offset_share);
        }
    }
};

template <typename DpfKey>
struct dpf_emplacer<DpfKey, std::optional<DpfKey>>
{
    using dpf_key = DpfKey;
    using interior_node = typename DpfKey::interior_node;
    using correction_words_array = typename DpfKey::correction_words_array;
    using correction_advice_array = typename DpfKey::correction_advice_array;
    using leaf_tuple = typename DpfKey::leaf_tuple;
    using beaver_tuple = typename DpfKey::beaver_tuple;
    using input_type = typename DpfKey::input_type;

    HEDLEY_ALWAYS_INLINE
    static auto emplace(
        std::optional<DpfKey> & storage,
        const interior_node & root,
        const correction_words_array & correction_words,
        const correction_advice_array & correction_advice,
        const leaf_tuple & leaves,
        const beaver_tuple & beavers,
        const input_type & offset_share)
    {
        storage.emplace(root, correction_words, correction_advice, leaves, beavers, offset_share);
    }
};

template <typename DpfKey, typename ...Ts>
struct dpf_emplacer<DpfKey, std::variant<Ts...>>
{
    using dpf_key = DpfKey;
    using interior_node = typename DpfKey::interior_node;
    using correction_words_array = typename DpfKey::correction_words_array;
    using correction_advice_array = typename DpfKey::correction_advice_array;
    using leaf_tuple = typename DpfKey::leaf_tuple;
    using beaver_tuple = typename DpfKey::beaver_tuple;
    using input_type = typename DpfKey::input_type;

    HEDLEY_ALWAYS_INLINE
    static auto emplace(
        std::variant<Ts...> & storage,
        const interior_node & root,
        const correction_words_array & correction_words,
        const correction_advice_array & correction_advice,
        const leaf_tuple & leaves,
        const beaver_tuple & beavers,
        const input_type & offset_share)
    {
        storage.template emplace<DpfKey>(root, correction_words, correction_advice, leaves, beavers, offset_share);
    }
};

template <typename DpfKey>
struct dpf_emplacer<DpfKey, DpfKey *>
{
    using dpf_key = DpfKey;
    using interior_node = typename DpfKey::interior_node;
    using correction_words_array = typename DpfKey::correction_words_array;
    using correction_advice_array = typename DpfKey::correction_advice_array;
    using leaf_tuple = typename DpfKey::leaf_tuple;
    using beaver_tuple = typename DpfKey::beaver_tuple;
    using input_type = typename DpfKey::input_type;

    HEDLEY_ALWAYS_INLINE
    static auto emplace(
        DpfKey * storage,
        const interior_node & root,
        const correction_words_array & correction_words,
        const correction_advice_array & correction_advice,
        const leaf_tuple & leaves,
        const beaver_tuple & beavers,
        const input_type & offset_share)
    {
        ::new (storage) DpfKey(root, correction_words, correction_advice, leaves, beavers, offset_share);
    }
};

template <typename DpfKey>
struct dpf_emplacer<DpfKey, std::reference_wrapper<DpfKey>>
{
    using dpf_key = DpfKey;
    using interior_node = typename DpfKey::interior_node;
    using correction_words_array = typename DpfKey::correction_words_array;
    using correction_advice_array = typename DpfKey::correction_advice_array;
    using leaf_tuple = typename DpfKey::leaf_tuple;
    using beaver_tuple = typename DpfKey::beaver_tuple;
    using input_type = typename DpfKey::input_type;

    HEDLEY_ALWAYS_INLINE
    static auto emplace(
        std::reference_wrapper<DpfKey> storage,
        const interior_node & root,
        const correction_words_array & correction_words,
        const correction_advice_array & correction_advice,
        const leaf_tuple & leaves,
        const beaver_tuple & beavers,
        const input_type & offset_share)
    {
        storage.get().~DpfKey(); 
        ::new (&storage.get()) DpfKey(root, correction_words, correction_advice, leaves, beavers, offset_share);
    }
};

/// @}

template <typename DpfKey, typename ContainerT>
struct dpf_back_emplacer
{
    using dpf_key = DpfKey;
    using interior_node = typename DpfKey::interior_node;
    using correction_words_array = typename DpfKey::correction_words_array;
    using correction_advice_array = typename DpfKey::correction_advice_array;
    using leaf_tuple = typename DpfKey::leaf_tuple;
    using beaver_tuple = typename DpfKey::beaver_tuple;
    using input_type = typename DpfKey::input_type;

    static auto emplace_back(ContainerT & storage,
        const interior_node & root,
        const correction_words_array & correction_words,
        const correction_advice_array & correction_advice,
        const leaf_tuple & leaves,
        const beaver_tuple & beavers,
        const input_type & offset_share)
    {
        storage.emplace_back(root, correction_words, correction_advice, leaves, beavers, offset_share);
    }
};

}  // namespace utils

}  // namespace std

#endif  // LIBDPF_INCLUDE_DPF_EMPLACE_HPP__
