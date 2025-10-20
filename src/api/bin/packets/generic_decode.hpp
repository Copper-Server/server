
/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_BIN_PACKETS_GENERIC_DECODE
#define SRC_API_BIN_PACKETS_GENERIC_DECODE
#include <src/api/network/tcp.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/util/readers.hpp>
#include <src/util/templates.hpp>

#include <src/api/bin/packets/generic.hpp>
namespace copper_server::api::packets {
    struct processor_handle_data {
        uint8_t mode;
        size_t id;
    };

    template <class T, class Prev_T>
    void decode_entry(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev) {
        static_assert(std::is_copy_constructible_v<T>);
        static_assert(std::is_move_constructible_v<T>);
        static_assert(std::is_copy_assignable_v<T>);
        static_assert(std::is_move_assignable_v<T>);
        using Type = std::decay_t<T>;
        if constexpr (is_convertible_to_packet_form<Type>) {
            convertible_to_packet_type<Type> res{};
            decode_entry(context, stream, res, prev);
            value = Type::from_packet(std::move(res));
        } else if constexpr (std::is_same_v<identifier, Type>)
            value.value = stream.read_identifier();
        else if constexpr (is_constant_value<Type>) {
            decltype(Type::value::value) check;
            decode_entry(context, stream, check, prev);
            if (check != Type::value::value)
                throw std::runtime_error("The value is not equal to excepted.");
        } else if constexpr (is_std_array<Type>)
            for (auto& it : value)
                decode_entry(context, stream, it, prev);
        else if constexpr (is_string_sized<Type>)
            value.value = stream.read_string(Type::max_size);
        else if constexpr (std::is_same_v<json_text_component, Type>)
            value.value = stream.read_json_component();
        else if constexpr (std::is_same_v<var_int32, Type>)
            value.value = stream.read_var<int32_t>();
        else if constexpr (std::is_same_v<var_int64, Type>)
            value.value = stream.read_var<int64_t>();
        else if constexpr (std::is_same_v<base_objects::velocity, Type>)
            value = stream.read_velocity();
        else if constexpr (std::is_same_v<optional_var_int32, Type>) {
            auto res = stream.read_var<int32_t>();
            if (res)
                value = optional_var_int32(res - 1);
        } else if constexpr (std::is_same_v<optional_var_int64, Type>) {
            auto res = stream.read_var<int64_t>();
            if (res)
                value = optional_var_int64(res - 1);
        } else if constexpr (std::is_same_v<base_objects::position, Type>)
            value.set(stream.read_value<decltype(value.get())>());
        else if constexpr (std::is_arithmetic_v<Type>)
            value = stream.read_value<Type>();
        else if constexpr (std::is_same_v<std::string, Type>)
            value = stream.read_string();
        else if constexpr (std::is_same_v<enbt::raw_uuid, Type>)
            value = stream.read_uuid();
        else if constexpr (std::is_same_v<Chat, Type>)
            value = Chat::fromEnbt(ReadNetworkNBT_enbt(stream));
        else if constexpr (
            std::is_same_v<enbt::value, Type>
            || std::is_same_v<enbt::compound, Type>
            || std::is_same_v<enbt::dynamic_array, Type>
            || std::is_same_v<enbt::fixed_array, Type>
            || std::is_same_v<enbt::uuid, Type>
            || std::is_same_v<enbt::simple_array_i8, Type>
            || std::is_same_v<enbt::simple_array_i16, Type>
            || std::is_same_v<enbt::simple_array_i32, Type>
            || std::is_same_v<enbt::simple_array_i64, Type>
            || std::is_same_v<enbt::simple_array_ui8, Type>
            || std::is_same_v<enbt::simple_array_ui16, Type>
            || std::is_same_v<enbt::simple_array_ui32, Type>
            || std::is_same_v<enbt::simple_array_ui64, Type>
        )
            value = ReadNetworkNBT_enbt(stream);
        else if constexpr (std::is_base_of_v<base_objects::palette_container, Type>) {
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
                stream.read_value<uint8_t>(); //always zero
                res.id_of_palette = stream.read_var<int32_t>();
                value.decompile(std::move(res));
            } else if (bits_per_entry <= max_indirect) {
                base_objects::palette_container_indirect res(bits_per_entry);
                uint32_t palette = stream.read_var<uint32_t>();
                res.palette.reserve(palette);
                for (uint32_t i = 0; i < palette; i++)
                    res.palette.push_back(stream.read_var<uint32_t>());
                auto size = bits_per_entry * entries_count;
                size += size % 8;
                auto range = stream.range_read(size);
                res.data.bits_per_entry = bits_per_entry;
                res.data.data.data() = list_array<uint8_t>(range.data_read(), range.size_read());
                value.decompile(std::move(res));
            } else {
                base_objects::palette_data res(bits_per_entry);
                auto size = bits_per_entry * entries_count;
                size += size % 8;
                auto range = stream.range_read(size);
                res.data.data() = list_array<uint8_t>(range.data_read(), range.size_read());
                value.decompile(std::move(res));
            }
        } else if constexpr (std::is_same_v<base_objects::palette_data_height_map, Type>) {
            value.bits_per_entry = base_objects::palette_data::bits_for_max(get_size_source_value(context, size_source::get_world_blocks_height));
            auto size = value.bits_per_entry * 256;
            size += size % 8;
            value.data.data() = stream.read_array<uint64_t>(int32_t(size / 8));
        } else if constexpr (is_template_base_of<list_array_depend, Type>) {
            bool has_next = false;
            do {
                typename Type::value_type next;
                decode_entry(context, stream, next, prev);
                has_next = (bool)next.has_next_item;
                value.push_back(std::move(next));
            } while (has_next);
        } else if constexpr (is_template_base_of<_list_array_impl::list_array, Type>) {
            if constexpr (!is_no_size<Type> && !std::is_base_of_v<size_from_packet, Type>) {
                value.resize(stream.read_var<int32_t>(), typename Type::value_type{});
            } else if constexpr (is_no_size<Type>) {
                value.resize(Type::get_depended_size(context, *prev), typename Type::value_type{});
            } else
                value.resize(stream.size_read() / sizeof(typename Type::value_type), typename Type::value_type{});
            for (auto&& it : value)
                decode_entry(context, stream, it, prev);
        } else if constexpr (is_template_base_of<ignored, Type>) {
        } else if constexpr (is_template_base_of<std::optional, Type>) {
            value = std::nullopt;
            if (stream.read_value<bool>()) {
                value.emplace();
                decode_entry(context, stream, *value, prev);
            }
        } else if constexpr (is_template_base_of<enum_as, Type> || is_template_base_of<enum_as_flag, Type>) {
            typename Type::encode_t val;
            decode_entry(context, stream, val, prev);
            value.value = (std::decay_t<decltype(value.value)>)val;
        } else if constexpr (is_template_base_of<or_, Type>) {
            auto res = stream.read_var<int32_t>();
            if (res)
                value = typename Type::var_0(res - 1);
            else {
                typename Type::var_1 in_res;
                decode_entry(context, stream, in_res, prev);
                value = std::move(in_res);
            }
        } else if constexpr (is_template_base_of<bool_or, Type>) {
            auto res = stream.read_value<bool>();
            if (res) {
                typename Type::var_0 in_res;
                decode_entry(context, stream, in_res, prev);
                value = std::move(in_res);
            } else {
                typename Type::var_1 in_res;
                decode_entry(context, stream, in_res, prev);
                value = std::move(in_res);
            }
        } else if constexpr (is_template_base_of<enum_switch, Type>) {
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
        } else if constexpr (is_template_base_of<partial_enum_switch, Type>) {
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
        } else if constexpr (is_template_base_of<base_objects::box, Type>) {
            value = std::make_shared<typename Type::value_type>();
            decode_entry(context, stream, *value, prev);
        } else if constexpr (is_template_base_of<base_objects::depends_next, Type>) {
            decode_entry(context, stream, value.value, prev);
        } else if constexpr (is_template_base_of<any_of, Type> || is_template_base_of<packet_compress, Type>) {
            decode_entry(context, stream, value.value, prev);
        } else if constexpr (is_template_base_of<flags_list, Type>) {
            decode_entry(context, stream, value.flag, prev);
            Type res;
            value.for_each_set_flag_in_order([&]<class flag_T>() {
                flag_T make_res;
                decode_entry(context, stream, make_res, prev);
                res.set(std::move(make_res));
            });
            value = std::move(res);
        } else if constexpr (is_flags_list_from<Type>) {
            Type res;
            auto& it = (*prev).*Type::preprocess_source_name::value;
            value.for_each_set_flag_in_order(it, [&]<class flag_T>() {
                flag_T make_res{};
                decode_entry(context, stream, make_res, prev);
                res.set(std::move(make_res));
            });
            value = std::move(res);
        } else if constexpr (is_template_base_of<id_set, Type>) {
            var_int32 size = 0;
            decode_entry(context, stream, size, prev);
            if (!size)
                value = (identifier)stream.read_identifier();
            else {
                int32_t arr_size = size - 1;
                list_array<typename Type::id_type> res;
                res.resize(arr_size);
                for (int32_t i = 0; i < arr_size; i++)
                    decode_entry(context, stream, res[i], prev);
                value = std::move(res);
            }
        } else if constexpr (is_template_base_of<value_optional, Type>) {
            decode_entry(context, stream, value.v, prev);
            if (value.v) {
                std::decay_t<decltype(*value.rest)> tmp{};
                decode_entry(context, stream, tmp, prev);
                value.rest = std::move(tmp);
            }
        } else if constexpr (is_template_base_of<sized_entry, Type>) {
            typename Type::size_type size;
            decode_entry(context, stream, size, prev);
            ArrayStream inner = stream.range_read(size);
            decode_entry(context, inner, value.value, prev);
        } else if constexpr (is_limited_num<Type>) {
            decode_entry(context, stream, value.value, prev);
        } else if constexpr (is_bitset_fixed<Type>) {
            bit_list_array<uint8_t> res;
            res.resize(Type::max_size::value);
            size_t r = res.data().size();
            for (size_t i = 0; i < r; i++)
                res.data()[i] = stream.read_value<uint8_t>();
            value.value = std::move(res);
        } else if constexpr (std::is_same_v<bit_list_array<uint64_t>, Type>) {
            value.data() = stream.read_array<uint64_t>();
        } else if constexpr (api::id::is_source<Type>) {
            decode_entry(context, stream, value.value, prev);
        } else if constexpr (is_ordered_id<Type>) {
            decode_entry(context, stream, value.value, prev);
            value.is_valid = context.packets_state.internal_data.get([&](auto& data) {
                auto it = data.id_tracker.find(Type::id_source);
                return it != data.id_tracker.end() ? ((decltype(value.value))it->second) == value.value : false;
            });
        } else if constexpr (is_template_base_of<enum_set, Type>) {
            using Tupple_T = std::decay_t<decltype(value.values)>;
            bit_list_array<uint8_t> bit(std::tuple_size_v<Tupple_T>);
            for (size_t i = 0; i < bit.size(); i++)
                bit.data()[i] = stream.read_value<uint8_t>();
            static constexpr auto type_table = []<size_t... I>(std::index_sequence<I...>) {
                return std::array<void (*)(base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev), sizeof...(I)>{
                    [](base_objects::shared_client_data& context, ArrayStream& stream, T& value, Prev_T* prev) {
                        using DT = std::tuple_element_t<I, Tupple_T>::value_type;
                        DT v{};
                        decode_entry(context, stream, v, prev);
                        value.push(std::move(v));
                    }...
                };
            }(std::make_index_sequence<std::tuple_size_v<Tupple_T>>());
            size_t siz = stream.read_var<int32_t>();
            for (size_t i = 0; i < siz; i++)
                for (size_t j = 0; j < std::tuple_size_v<Tupple_T>; j++)
                    if (bit.at(i) || j == 0) //j==0 is for header
                        type_table[i + 1](context, stream, value, prev);
        } else {
            bool process_next = true;
            reflect::for_each_field(value, [&](auto& item) {
                if (process_next) {
                    if constexpr (is_item_depend<T>) {
                        typename T::base_depend tmp = item;
                        decode_entry(context, stream, tmp, &value);
                        value.*T::body_depend::value = {bool(tmp & T::depend_value::value)};
                        tmp = tmp | ~T::depend_value::value;
                    } else
                        decode_entry(context, stream, item, &value);
                    if constexpr (is_template_base_of<depends_next, std::decay_t<decltype(item)>>)
                        process_next = (bool)item.value;
                }
            });
        }
    }

    template <class Ops, class T>
    bool decoder_make_process(base_objects::shared_client_data& context, T&& value) {
        if (Ops::receive_viewer().notify(value, context))
            return false;

        using Type = std::decay_t<T>;
        if constexpr (std::is_base_of_v<switches_to::status, Type>)
            context << switches_to::status{};
        else if constexpr (std::is_base_of_v<switches_to::login, Type>)
            context << switches_to::login{};
        else if constexpr (std::is_base_of_v<switches_to::config, Type>)
            context << switches_to::config{};
        else if constexpr (std::is_base_of_v<switches_to::play, Type>)
            context << switches_to::play{};

        Ops::processor().notify(std::move(value), context);

        return true;
    }
}
#endif