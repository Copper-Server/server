
/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_ENCODING_PACKET_DECODE
#define SRC_UTIL_ENCODING_PACKET_DECODE
#include "src/util/encoding/nbt/deserialization.hpp"
#include "src/util/nbt_stream.hpp"
#include <src/api/network/tcp.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/util/readers.hpp>
#include <src/util/templates.hpp>

#include <src/util/encoding/common.hpp>

namespace copper_server::util::encoding::packet {
    struct processor_handle_data {
        uint8_t mode;
        size_t id;
    };


    template <class T, class Prev_T>
    void decode_entry(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev);

    namespace detail {
        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<2>)
            requires base_objects::is_convertible_to_packet_form<std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            base_objects::convertible_to_packet_type<Type> res{};
            decode_entry(context, stream, res, prev);
            value = Type::from_packet(std::move(res));
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_same_v<base_objects::identifier, std::decay_t<T>>
        {
            value.value = stream.read_identifier();
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T&, Prev_T* prev, priority_tag<2>)
            requires is_constant_value<std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            decltype(Type::value::value) check;
            decode_entry(context, stream, check, prev);
            if (check != Type::value::value)
                throw std::runtime_error("The value is not equal to excepted.");
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<2>)
            requires is_std_array<std::decay_t<T>>
        {
            for (auto& it : value)
                decode_entry(context, stream, it, prev);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires is_string_sized<std::decay_t<T>>
        {
            value.value = stream.read_string(T::max_size);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_same_v<base_objects::json_text_component, std::decay_t<T>>
        {
            value.value = stream.read_json_component();
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_same_v<base_objects::var_int32, std::decay_t<T>>
        {
            value.value = stream.read_var<int32_t>();
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_same_v<base_objects::var_int64, std::decay_t<T>>
        {
            value.value = stream.read_var<int64_t>();
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_same_v<base_objects::velocity, std::decay_t<T>>
        {
            value = stream.read_velocity();
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_same_v<base_objects::optional_var_int32, std::decay_t<T>>
        {
            if (auto res = stream.read_var<int32_t>())
                value = base_objects::optional_var_int32(res - 1);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_same_v<base_objects::optional_var_int64, std::decay_t<T>>
        {
            if (auto res = stream.read_var<int64_t>())
                value = base_objects::optional_var_int64(res - 1);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_same_v<base_objects::position, std::decay_t<T>>
        {
            value.set(stream.read_value<decltype(value.get())>());
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_arithmetic_v<std::decay_t<T>>
        {
            value = stream.read_value<std::decay_t<T>>();
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_same_v<std::string, std::decay_t<T>>
        {
            value = stream.read_string();
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_same_v<base_objects::uuid, std::decay_t<T>>
        {
            value = stream.read_uuid();
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_same_v<base_objects::chat, std::decay_t<T>>
        {
            value = base_objects::chat::from_nbt(ReadNetworkNBT_nbt(stream));
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_same_v<util::nbt, std::decay_t<T>>
        {
            value = ReadNetworkNBT_nbt(stream);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<2>)
            requires std::is_same_v<util::nbt_convert, std::decay_t<T>>
        {
            value = ReadNetworkNBT(stream);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<1>)
            requires std::is_base_of_v<base_objects::palette_container, std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            auto bits_per_entry = stream.read_value<uint8_t>();
            static constexpr auto max_indirect
                = std::is_same_v<base_objects::palette_container_biome, Type>
                      ? base_objects::palette_container::max_indirect_biomes
                      : base_objects::palette_container::max_indirect_blocks;
            static constexpr auto entries_count
                = std::is_same_v<base_objects::palette_container_biome, Type>
                      ? 64
                      : 4096;
            if (bits_per_entry == 0) {
                base_objects::palette_container_single res;
                res.id_of_palette = stream.read_var<int32_t>();
                value.decompile(std::move(res));
            } else if (bits_per_entry <= max_indirect) {
                base_objects::palette_container_indirect res(bits_per_entry, entries_count);
                uint32_t palette = stream.read_var<uint32_t>();
                res.palette.reserve(palette);
                for (uint32_t i = 0; i < palette; i++)
                    res.palette.push_back(stream.read_var<uint32_t>());
                auto size = bits_per_entry * entries_count;
                size += size % 8;
                size /= 8;
                auto range = stream.range_read(size);
                res.data.bits_per_entry = bits_per_entry;
                res.data.data.data() = list_array<uint64_t>((uint64_t*)range.data_read(), range.size_read() / 8);
                value.decompile(std::move(res));
            } else {
                base_objects::palette_data res(bits_per_entry, entries_count);
                auto size = bits_per_entry * entries_count;
                size += size % 8;
                auto range = stream.range_read(size);
                res.data.data() = list_array<uint64_t>((uint64_t*)range.data_read(), range.size_read() / 8);
                value.decompile(std::move(res));
            }
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T*, priority_tag<1>)
            requires std::is_same_v<base_objects::palette_data_height_map, std::decay_t<T>>
        {
            value.bits_per_entry = base_objects::palette_data::bits_for_max(api::packets::get_size_source_value(context, api::packets::size_source::get_world_blocks_height));
            auto size = value.bits_per_entry * 256;
            size += size % 8;
            value.data.data() = stream.read_array<uint64_t>(static_cast<int32_t>(size / 8));
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::list_array_depend, std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            bool has_next = false;
            do {
                typename Type::value_type next;
                decode_entry(context, stream, next, prev);
                has_next = (bool)next.has_next_item;
                value.push_back(std::move(next));
            } while (has_next);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<_list_array_impl::list_array, std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            if constexpr (!is_no_size<Type> && !std::is_base_of_v<api::packets::size_from_packet, Type>) {
                value.resize(stream.read_var<int32_t>(), typename Type::value_type{});
            } else if constexpr (is_no_size<Type>) {
                value.resize(Type::get_depended_size(context, *prev), typename Type::value_type{});
            } else
                value.resize(stream.size_read() / sizeof(typename Type::value_type), typename Type::value_type{});
            for (auto&& it : value)
                decode_entry(context, stream, it, prev);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream&, T&, Prev_T*, priority_tag<1>)
            requires is_template_base_of<base_objects::ignored, std::decay_t<T>>
        {
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<std::optional, std::decay_t<T>>
        {
            value = std::nullopt;
            if (stream.read_value<bool>()) {
                value.emplace();
                decode_entry(context, stream, *value, prev);
            }
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::enum_as, std::decay_t<T>> || is_template_base_of<base_objects::enum_as_flag, std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            typename Type::encode_t val;
            decode_entry(context, stream, val, prev);
            value.value = static_cast<std::decay_t<decltype(value.value)>>(val);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::or_, std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            if (const auto res = stream.read_var<int32_t>())
                value = typename Type::var_0(res - 1);
            else {
                typename Type::var_1 in_res;
                decode_entry(context, stream, in_res, prev);
                value = std::move(in_res);
            }
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::bool_or, std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            if (stream.read_value<bool>()) {
                typename Type::var_0 in_res;
                decode_entry(context, stream, in_res, prev);
                value = std::move(in_res);
            } else {
                typename Type::var_1 in_res;
                decode_entry(context, stream, in_res, prev);
                value = std::move(in_res);
            }
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::enum_switch, std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            typename Type::encode_type id_check;
            decode_entry(context, stream, id_check, prev);
            Type::get_enum(id_check, [&]<class enum_T>() {
                static_assert(std::is_copy_constructible_v<enum_T>);
                static_assert(std::is_move_constructible_v<enum_T>);
                static_assert(std::is_copy_assignable_v<enum_T>);
                static_assert(std::is_move_assignable_v<enum_T>);
                enum_T make_res{};
                decode_entry(context, stream, make_res, prev);
                value = std::move(make_res);
            });
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::partial_enum_switch, std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            typename Type::encode_type id_check;
            decode_entry(context, stream, id_check, prev);
            Type::get_enum(id_check, [&]<class enum_T>() {
                if constexpr (std::is_same_v<enum_T, typename Type::encode_type>) {
                    value = std::move(id_check);
                } else {
                    enum_T make_res;
                    decode_entry(context, stream, make_res, prev);
                    value = std::move(make_res);
                }
            });
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::box, std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            value = std::make_shared<typename Type::value_type>();
            decode_entry(context, stream, *value, prev);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::depends_next, std::decay_t<T>>
        {
            decode_entry(context, stream, value.value, prev);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::any_of, std::decay_t<T>> || is_template_base_of<base_objects::packet_compress, std::decay_t<T>>
        {
            decode_entry(context, stream, value.value, prev);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::flags_list, std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            decode_entry(context, stream, value.flag, prev);
            Type res;
            value.for_each_set_flag_in_order([&]<class flag_T>() {
                flag_T make_res;
                decode_entry(context, stream, make_res, prev);
                res.set(std::move(make_res));
            });
            value = std::move(res);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<std::unordered_map, std::decay_t<T>> && std::is_same_v<typename std::decay_t<T>::key_type, std::string>
        {
            using Type = std::decay_t<T>;
            std::unordered_map<std::string, typename Type::mapped_type> res;
            auto size = stream.read_var<int32_t>();
            for (int32_t i = 0; i < size; i++) {
                std::string key;
                decode_entry(context, stream, key, prev);
                typename Type::mapped_type val;
                decode_entry(context, stream, val, prev);
                res.emplace(std::move(key), std::move(val));
            }
            value = std::move(res);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<std::unordered_map, std::decay_t<T>> && is_map_compatible<std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            Type res;
            auto size = stream.read_var<int32_t>();
            for (int32_t i = 0; i < size; i++) {
                typename Type::key_type key;
                decode_entry(context, stream, key, prev);
                typename Type::mapped_type val;
                decode_entry(context, stream, val, prev);
                res.emplace(std::move(key), std::move(val));
            }
            value = std::move(res);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_flags_list_from<std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            Type res;
            auto& it = (*prev).*Type::preprocess_source_name::value;
            value.for_each_set_flag_in_order(it, [&]<class flag_T>() {
                flag_T make_res{};
                decode_entry(context, stream, make_res, prev);
                res.set(std::move(make_res));
            });
            value = std::move(res);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::id_set, std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            base_objects::var_int32 size = 0;
            decode_entry(context, stream, size, prev);
            if (!size)
                value = (base_objects::identifier)stream.read_identifier();
            else {
                int32_t arr_size = size - 1;
                list_array<typename Type::id_type> res;
                res.resize(arr_size);
                for (int32_t i = 0; i < arr_size; i++)
                    decode_entry(context, stream, res[i], prev);
                value = std::move(res);
            }
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::value_optional, std::decay_t<T>>
        {
            decode_entry(context, stream, value.v, prev);
            if (value.v) {
                std::decay_t<decltype(*value.rest)> tmp{};
                decode_entry(context, stream, tmp, prev);
                value.rest = std::move(tmp);
            }
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::sized_entry, std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            typename Type::size_type size;
            decode_entry(context, stream, size, prev);
            ArrayStream inner = stream.range_read(size);
            decode_entry(context, inner, value.value, prev);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_limited_num<std::decay_t<T>>
        {
            decode_entry(context, stream, value.value, prev);
            //TODO add check
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<1>)
            requires is_bitset_fixed<std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            bit_list_array<uint8_t> res;
            res.resize(Type::max_size::value);
            size_t r = res.data().size();
            for (size_t i = 0; i < r; i++)
                res.data()[i] = stream.read_value<uint8_t>();
            value.value = std::move(res);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T*, priority_tag<1>)
            requires std::is_same_v<bit_list_array<uint64_t>, std::decay_t<T>>
        {
            value.data() = stream.read_array<uint64_t>();
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires api::id::is_source<std::decay_t<T>>
        {
            decode_entry(context, stream, value.value, prev);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_ordered_id<std::decay_t<T>>
        {
            using Type = std::decay_t<T>;
            decode_entry(context, stream, value.value, prev);
            value.is_valid = context.packets_state.internal_data.get([&](auto& data) {
                auto it = data.id_tracker.find(Type::id_source);
                return it != data.id_tracker.end() ? static_cast<decltype(value.value)>(it->second) == value.value : false;
            });
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires is_template_base_of<base_objects::enum_set, std::decay_t<T>>
        {
            using Tupple_T = std::decay_t<decltype(value.values)>;
            bit_list_array<uint8_t> bit(std::tuple_size_v<Tupple_T> - 1); //except header
            for (size_t i = 0; i < bit.data().size(); i++)
                bit.data()[i] = stream.read_value<uint8_t>();
            static constexpr auto type_table = []<size_t... I>(std::index_sequence<I...>) {
                return std::array<void (*)(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev), sizeof...(I)>{
                    [](base_objects::shared_client_data& t_context, ArrayStream& t_stream, T& t_value, Prev_T* t_prev) {
                        using DT = std::tuple_element_t<I, Tupple_T>::value_type;
                        DT v{};
                        decode_entry(t_context, t_stream, v, t_prev);
                        t_value.push(std::move(v));
                    }...
                };
            }(std::make_index_sequence<std::tuple_size_v<Tupple_T>>());
            size_t siz = stream.read_var<int32_t>();
            type_table[0](context, stream, value, prev);
            for (size_t i = 0; i < siz; i++)
                for (size_t j = 0; j < std::tuple_size_v<Tupple_T>; j++)
                    if (bit.at(i))
                        type_table[i + 1](context, stream, value, prev);
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data&, ArrayStream& stream, T& value, Prev_T* prev, priority_tag<1>)
            requires make_packet_as_nbt<std::decay_t<T>>
        {
            std::stringstream ss(std::string(reinterpret_cast<const char*>(stream.data_read()), stream.size_read()));
            util::nbt_read_stream nbt_stream(ss);
            util::encoding::nbt::deserialize_entry(value, nbt_stream, *prev);
            stream.range_read(ss.tellg());
        }

        template <class T, class Prev_T>
        void decode_impl(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T*, priority_tag<0>) {
            bool process_next = true;
            reflect::for_each_field(value, [&]<typename IT_T>(IT_T& item) {
                if (process_next) {
                    if constexpr (base_objects::is_item_depend<T>) {
                        typename T::base_depend tmp = item;
                        decode_entry(context, stream, tmp, &value);
                        value.*T::body_depend::value = {static_cast<bool>(tmp & T::depend_value::value)};
                    } else
                        decode_entry(context, stream, item, &value);
                    if constexpr (is_template_base_of<base_objects::depends_next, std::decay_t<IT_T>>)
                        process_next = static_cast<bool>(item.value);
                }
            });
        }
    }

    template <class T, class Prev_T>
    void decode_entry(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev) {
        static_assert(std::is_copy_constructible_v<T>);
        static_assert(std::is_move_constructible_v<T>);
        static_assert(std::is_copy_assignable_v<T>);
        static_assert(std::is_move_assignable_v<T>);
        detail::decode_impl(context, stream, value, prev);
    }

    template <class Ops, class T>
    bool decoder_make_process(base_objects::shared_client_data& context, T&& value) {
        if (Ops::receive_viewer().notify(value, context))
            return false;

        using Type = std::decay_t<T>;
        if constexpr (std::is_base_of_v<api::packets::switches_to::status, Type>)
            context << api::packets::switches_to::status{};
        else if constexpr (std::is_base_of_v<api::packets::switches_to::login, Type>)
            context << api::packets::switches_to::login{};
        else if constexpr (std::is_base_of_v<api::packets::switches_to::config, Type>)
            context << api::packets::switches_to::config{};
        else if constexpr (std::is_base_of_v<api::packets::switches_to::play, Type>)
            context << api::packets::switches_to::play{};

        Ops::processor().notify(std::forward<T>(value), context);

        return true;
    }
}
#endif
