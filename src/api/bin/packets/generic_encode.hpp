/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_BIN_PACKETS_GENERIC_ENCODE
#define SRC_API_BIN_PACKETS_GENERIC_ENCODE

#include <src/api/network/tcp.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <tuple>

#include <src/api/bin/packets/generic.hpp>

namespace copper_server::api::packets {

    template <class T, class... VisitedTypes>
    consteval bool need_preprocess_result() {
        if constexpr (contains_type_v<T, VisitedTypes...>) {
            return false;
        } else if constexpr (requires_check<T> || is_ordered_id<T>)
            return true;
        else if constexpr (
            is_std_array<T>
            || is_template_base_of<_list_array_impl::list_array, T>
            || is_template_base_of<base_objects::box, T>
            || is_template_base_of<std::optional, T>
            || is_template_base_of<value_optional, T>
            || is_template_base_of<sized_entry, T>
            || is_template_base_of<packet_compress, T>
        ) {
            return need_preprocess_result<typename T::value_type, T, VisitedTypes...>();
        } else if constexpr (is_flags_list_from<T> || is_template_base_of<flags_list, T>) {
            bool res = false;
            T::for_each_flag_in_order([&]<class IT>() {
                if (!res)
                    res = need_preprocess_result<IT, T, VisitedTypes...>();
            });
            return res;
        } else if constexpr (is_template_base_of<enum_switch, T> || is_template_base_of<partial_enum_switch, T>) {
            bool res = false;
            T::for_each([&]<class IT>() {
                if (!res)
                    res = need_preprocess_result<IT, T, VisitedTypes...>();
            });
            return res;
        } else if constexpr (is_template_base_of<or_, T>) {
            return need_preprocess_result<typename T::var_0, T, VisitedTypes...>() || need_preprocess_result<typename T::var_1, T, VisitedTypes...>();
        } else if constexpr (is_template_base_of<bool_or, T>) {
            return need_preprocess_result<typename T::var_0, T, VisitedTypes...>() || need_preprocess_result<typename T::var_1, T, VisitedTypes...>();
        } else if constexpr (std::is_same_v<identifier, T>) {
        } else if constexpr (std::is_same_v<json_text_component, T>) {
        } else if constexpr (std::is_same_v<var_int32, T>) {
        } else if constexpr (std::is_same_v<base_objects::velocity, T>) {
        } else if constexpr (std::is_same_v<var_int64, T>) {
        } else if constexpr (std::is_same_v<optional_var_int32, T>) {
        } else if constexpr (std::is_same_v<optional_var_int64, T>) {
        } else if constexpr (std::is_same_v<base_objects::position, T>) {
        } else if constexpr (std::is_arithmetic_v<T>) {
        } else if constexpr (std::is_same_v<std::string, T>) {
        } else if constexpr (std::is_same_v<enbt::raw_uuid, T>) {
        } else if constexpr (std::is_same_v<Chat, T>) {
        } else if constexpr (
            std::is_same_v<enbt::value, T>
            || std::is_same_v<enbt::compound, T>
            || std::is_same_v<enbt::dynamic_array, T>
            || std::is_same_v<enbt::fixed_array, T>
            || std::is_same_v<enbt::uuid, T>
            || std::is_same_v<enbt::simple_array_i8, T>
            || std::is_same_v<enbt::simple_array_i16, T>
            || std::is_same_v<enbt::simple_array_i32, T>
            || std::is_same_v<enbt::simple_array_i64, T>
            || std::is_same_v<enbt::simple_array_ui8, T>
            || std::is_same_v<enbt::simple_array_ui16, T>
            || std::is_same_v<enbt::simple_array_ui32, T>
            || std::is_same_v<enbt::simple_array_ui64, T>
            || std::is_same_v<T, client_bound::play::play_packet>
            || std::is_arithmetic_v<T>
        ) {
        } else if constexpr (std::is_base_of_v<base_objects::palette_container, T>) {
        } else if constexpr (std::is_same_v<base_objects::palette_data_height_map, T>) {
        } else if constexpr (is_template_base_of<ignored, T>) {
        } else if constexpr (is_template_base_of<enum_as, T> || is_template_base_of<enum_as_flag, T>) {
        } else if constexpr (is_template_base_of<any_of, T>) {
        } else if constexpr (is_template_base_of<id_set, T>) {
        } else if constexpr (std::is_same_v<bit_list_array<uint64_t>, T>) {
        } else if constexpr (is_convertible_to_packet_form<T>) {
        } else if constexpr (api::id::is_source<T>) {
        } else if constexpr (is_template_base_of<enum_set, T>) {
            using Tupple_T = std::decay_t<decltype(std::declval<T>().values)>;
            bool res = false;
            util::for_each_type<Tupple_T>::each([&]<class T_Elem>() {
                if constexpr (need_preprocess_result<typename T_Elem::value_type, T, VisitedTypes...>())
                    res = true;
            });
            return res;
        } else {
            bool res = false;
            reflect::for_each_type<T>([&]<class I>() {
                if (res)
                    return;
                if constexpr (could_be_preprocessed<I, T> || requires_check<T>)
                    res = true;
                else if constexpr (need_preprocess_result<I, T, VisitedTypes...>())
                    res = true;
            });
            return res;
        }
        return false;
    }

    template <class T>
    concept need_preprocess_result_v = need_preprocess_result<T>();

    template <class T>
    void serialize_entry(base_objects::network::response::item& res, base_objects::SharedClientData& context, T&& value) {
        using Type = std::decay_t<T>;
        if constexpr (is_convertible_to_packet_form<Type>) {
            serialize_entry(res, context, value.to_packet());
        } else if constexpr (std::is_same_v<identifier, Type>)
            res.write_identifier(value.value);
        else if constexpr (is_constant_value<Type>) {
            auto to_write = Type::value::value;
            serialize_entry(res, context, to_write);
        } else if constexpr (is_std_array<Type>)
            for (auto& it : value)
                serialize_entry(res, context, it);
        else if constexpr (is_string_sized<Type>)
            res.write_string(value.value, Type::max_size);
        else if constexpr (std::is_same_v<json_text_component, Type>)
            res.write_json_component(value.value);
        else if constexpr (std::is_same_v<var_int32, Type>)
            res.write_var32(value.value);
        else if constexpr (std::is_same_v<base_objects::velocity, Type>)
            res.write_value(value);
        else if constexpr (std::is_same_v<var_int64, Type>)
            res.write_var64(value.value);
        else if constexpr (std::is_same_v<optional_var_int32, Type>) {
            if (value)
                res.write_var32_check(int64_t(*value) + 1);
            else
                res.write_var32(0);
        } else if constexpr (std::is_same_v<optional_var_int64, Type>) {
            if (value)
                res.write_var64(*value + 1);
            else
                res.write_var64(0);
        } else if constexpr (std::is_same_v<base_objects::position, Type>)
            res.write_value(value.get());
        else if constexpr (std::is_arithmetic_v<Type>)
            res.write_value(value);
        else if constexpr (std::is_same_v<std::string, Type>)
            res.write_string(value);
        else if constexpr (std::is_same_v<enbt::raw_uuid, Type>)
            res.write_value(value);
        else if constexpr (std::is_same_v<Chat, Type>)
            res.write_direct(util::NBT::build(value.ToENBT()).get_as_network());
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
            res.write_direct(util::NBT::build((const enbt::value&)value).get_as_network());
        else if constexpr (std::is_base_of_v<base_objects::palette_container, Type>) {
            std::visit(
                [&]<class IT>(IT&& it) {
                    if constexpr (std::is_same_v<base_objects::palette_container_indirect, IT>) {
                        res.write_value(it.bits_per_entry);
                        res.write_var32_check(it.palette.size());
                        for (auto& i : it.palette)
                            res.write_var32(i);
                        res.write_direct(it.data.get());
                    } else if constexpr (std::is_same_v<base_objects::palette_container_single, IT>) {
                        res.write_value((uint8_t)0);
                        res.write_var32(it.id_of_palette);
                    } else if constexpr (std::is_same_v<base_objects::palette_data, IT>) {
                        res.write_value((uint8_t)it.bits_per_entry);
                        res.write_direct(it.get());
                    }
                },
                value.compile()
            );
        } else if constexpr (std::is_same_v<base_objects::palette_data_height_map, Type>) {
            res.write_array(value.get());
        } else if constexpr (is_template_base_of<list_array_depend, Type>) {
            size_t siz = value.size();
            size_t i = 1;
            for (auto&& it : value) {
                it.has_next_item = bool(siz != i);
                serialize_entry(res, context, it);
            }
        } else if constexpr (is_template_base_of<_list_array_impl::list_array, Type>) {
            if constexpr (!is_no_size<Type> && !std::is_base_of_v<size_from_packet, Type>)
                res.write_var32_check(value.size());
            for (auto&& it : value)
                serialize_entry(res, context, it);
        } else if constexpr (is_template_base_of<ignored, Type>) {
        } else if constexpr (is_template_base_of<std::optional, Type>) {
            res.write_value(bool(value));
            if (value)
                serialize_entry(res, context, *value);
        } else if constexpr (is_template_base_of<enum_as, Type> || is_template_base_of<enum_as_flag, Type>) {
            serialize_entry(res, context, value.get());
        } else if constexpr (is_template_base_of<or_, Type>) {
            std::visit(
                [&](auto& it) {
                    if constexpr (std::is_same_v<typename Type::var_0, std::decay_t<decltype(it)>>) {
                        if constexpr (is_template_base_of<enum_as, typename Type::var_0>)
                            res.write_var32_check(int64_t(it.value) + 1);
                        else
                            res.write_var32_check(int64_t(it) + 1);
                    } else {
                        res.write_var32(0);
                        serialize_entry(res, context, it);
                    }
                },
                value
            );
        } else if constexpr (is_template_base_of<bool_or, Type>) {
            std::visit(
                [&](auto& it) {
                    res.write_value(std::is_same_v<typename Type::var_0, std::decay_t<decltype(it)>>);
                    serialize_entry(res, context, it);
                },
                value
            );
        } else if constexpr (is_template_base_of<enum_switch, Type>) {
            std::visit(
                [&](auto& it) {
                    using it_T = std::decay_t<decltype(it)>;
                    serialize_entry(res, context, typename Type::encode_type(it_T::item_id::value));
                    serialize_entry(res, context, it);
                },
                value
            );
        } else if constexpr (is_template_base_of<partial_enum_switch, T>) {
            std::visit(
                [&](auto& it) {
                    using it_T = std::decay_t<decltype(it)>;
                    if constexpr (std::is_same_v<it_T, typename Type::encode_type>) {
                        serialize_entry(res, context, it);
                    } else {
                        serialize_entry(res, context, typename Type::encode_type(it_T::item_id::value));
                        serialize_entry(res, context, it);
                    }
                },
                value
            );
        } else if constexpr (is_template_base_of<base_objects::box, Type>) {
            serialize_entry(res, context, *value);
        } else if constexpr (is_template_base_of<any_of, Type>) {
            serialize_entry(res, context, value.value);
        } else if constexpr (is_template_base_of<packet_compress, Type>) {
            res.apply_compression = true;
            res.compression_threshold = value.value;
            serialize_entry(res, context, value.value);
        } else if constexpr (is_template_base_of<flags_list, Type>) {
            serialize_entry(res, context, value.flag);
            value.for_each_in_order([&](auto& it) {
                serialize_entry(res, context, it);
            });
        } else if constexpr (is_flags_list_from<Type>) {
            value.for_each_in_order([&](auto& it) {
                serialize_entry(res, context, it);
            });
        } else if constexpr (is_template_base_of<id_set, Type>) {
            std::visit(
                [&](auto& it) {
                    if constexpr (std::is_same_v<identifier, std::decay_t<decltype(it)>>) {
                        res.write_var32(0);
                        res.write_identifier(it);
                    } else {
                        res.write_var32_check(it.size() + 1);
                        for (auto& elem : it)
                            serialize_entry(res, context, elem);
                    }
                },
                value
            );
        } else if constexpr (is_template_base_of<value_optional, Type>) {
            if (value.rest && value.v) {
                serialize_entry(res, context, value.v);
                serialize_entry(res, context, *value.rest);
            } else {
                decltype(value.v) tmp{0};
                serialize_entry(res, context, tmp);
            }
        } else if constexpr (is_template_base_of<sized_entry, Type>) {
            base_objects::network::response::item inner;
            serialize_entry(inner, context, value.value);
            typename Type::size_type size;
            if constexpr (sizeof(typename Type::size_type) >= 8)
                size = inner.data.size();
            else if constexpr (sizeof(typename Type::size_type) >= 4)
                size = (int32_t)inner.data.size();
            else
                size = (int16_t)inner.data.size();
            serialize_entry(res, context, size);
            res.write_in(inner);
        } else if constexpr (is_limited_num<Type>) {
            serialize_entry(res, context, value.value);
        } else if constexpr (is_bitset_fixed<Type>) {
            res.write_direct(value.value.data());
        } else if constexpr (std::is_same_v<bit_list_array<uint64_t>, Type>) {
            res.write_var32_check(value.data().size());
            res.write_direct(value.data());
        } else if constexpr (api::id::is_source<Type> || is_template_base_of<base_objects::depends_next, T>) {
            serialize_entry(res, context, value.value);
        } else if constexpr (is_ordered_id<Type>) {
            serialize_entry(res, context, value.value);
        } else if constexpr (is_template_base_of<enum_set, Type>) {
            using Tupple_T = std::decay_t<decltype(value.values)>;
            {
                bit_list_array<uint8_t> bit(std::tuple_size_v<Tupple_T> - 1); //except header
                size_t i = 0;
                util::for_each_type<Tupple_T>::each([&]<class T_Elem>() {
                    if constexpr (!std::is_same_v<typename T_Elem::value_type, typename Type::header_t>) {
                        if (value.template has<typename T_Elem::value_type>())
                            bit[i] = true;
                        i++;
                    }
                });
                res.write_direct(std::move(bit).data());
            }
            std::optional<size_t> check;
            util::for_each_type<Tupple_T>::each([&check, &value]<class T_Elem>() {
                auto siz = value.template get<typename T_Elem::value_type>().size();
                if (siz == 0) //skip unset
                    return;
                if (!check)
                    check = siz;
                else if (*check != siz)
                    throw std::runtime_error("enum_set supposed to have same count of elements");
            });
            if (check) {
                size_t siz = *check;
                if (siz)
                    if (!value.template has<typename Type::header_t>())
                        throw std::runtime_error("enum_set supposed to have headers");
                res.write_var32_check(siz);
                for (size_t i = 0; i < siz; i++) {
                    util::for_each_type<Tupple_T>::each([&]<class T_Elem>() {
                        if (value.template has<typename T_Elem::value_type>())
                            serialize_entry(res, context, value.template get<typename T_Elem::value_type>()[i]);
                    });
                }
            } else
                res.write_var32(0);
        } else {
            bool process_next = true;
            reflect::for_each_field(value, [&value, &res, &context, &process_next]<class IT>(IT& item) {
                if (process_next) {
                    if constexpr (is_item_depend<IT>) {
                        typename IT::base_depend tmp = item;
                        if (value.*IT::body_depend::value)
                            tmp = tmp | IT::depend_value::value;
                        serialize_entry(res, context, tmp);
                    } else
                        serialize_entry(res, context, item);
                    if constexpr (is_template_base_of<depends_next, IT>)
                        process_next = (bool)item.value;
                }
            });
        }
    }

    template <class T, class T_prev>
    void preprocess_structure(base_objects::SharedClientData& context, T& value, T_prev& prev) {
        if constexpr (is_no_size<T>) {
            if constexpr (is_template_base_of<_list_array_impl::list_array, T>)
                if (value.size() != T::get_depended_size(context, prev))
                    throw std::overflow_error("The size of list_array did not equals to depended values.");
        }
        if constexpr (is_bitset_fixed<T>) {
            if (value.value.size() != T::max_size::value)
                throw std::overflow_error("The bitset size not equal required one.");
        } else if constexpr (is_list_array_fixed<T>) {
            if (value.size() != T::required_size)
                throw std::overflow_error("The list_array size not equal required one.");
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                for (auto& it : value)
                    preprocess_structure(context, it, prev);
        } else if constexpr (is_std_array<T> || is_template_base_of<_list_array_impl::list_array, T>) {
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                for (auto& it : value)
                    preprocess_structure(context, it, prev);
        } else if constexpr (is_string_sized<T>) {
            if (value.value.size() > T::max_size)
                throw std::overflow_error("The string size is over the limit.");
        } else if constexpr (is_list_array_sized<T>) {
            if (value.size() > T::max_size)
                throw std::overflow_error("The list_array size is over the limit.");
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                for (auto& it : value.value)
                    preprocess_structure(context, it, prev);
        } else if constexpr (is_template_base_of<std::optional, T>) {
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                if (value)
                    preprocess_structure(context, *value, prev);
        } else if constexpr (is_limited_num<T>) {
            if (value.value > T::check_max)
                throw std::overflow_error("The value is too big");
            if (value.value < T::check_min)
                throw std::underflow_error("The value is too low");
        } else if constexpr (is_template_base_of<value_optional, T>) {
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                if (value.rest)
                    preprocess_structure(context, *value.rest, prev);
        } else if constexpr (is_flags_list_from<T> || is_template_base_of<flags_list, T>) {
            value.for_each([&]<class IT>(IT& it) {
                if constexpr (need_preprocess_result_v<IT>)
                    preprocess_structure(context, it, prev);
            });
        } else if constexpr (is_template_base_of<enum_switch, T> || is_template_base_of<partial_enum_switch, T>) {
            std::visit(
                [&]<class IT>(IT& it) {
                    if constexpr (need_preprocess_result_v<IT>)
                        preprocess_structure(context, it, prev);
                },
                value
            );
        } else if constexpr (is_template_base_of<base_objects::box, T>) {
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                preprocess_structure(context, *value, prev);
        } else if constexpr (is_template_base_of<base_objects::depends_next, T> || is_template_base_of<sized_entry, T> || is_template_base_of<packet_compress, T>) {
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                preprocess_structure(context, value.value, prev);
        }

        else if constexpr (is_template_base_of<or_, T> || is_template_base_of<bool_or, T>) {
            std::visit(
                [&]<class Y>(Y& it) {
                    if constexpr (need_preprocess_result_v<Y>)
                        preprocess_structure(context, it, prev);
                },
                value
            );
        } else if constexpr (std::is_same_v<identifier, T>) {
        } else if constexpr (std::is_same_v<json_text_component, T>) {
        } else if constexpr (std::is_same_v<var_int32, T>) {
        } else if constexpr (std::is_same_v<var_int64, T>) {
        } else if constexpr (std::is_same_v<optional_var_int32, T>) {
        } else if constexpr (std::is_same_v<optional_var_int64, T>) {
        } else if constexpr (std::is_same_v<base_objects::position, T>) {
        } else if constexpr (std::is_arithmetic_v<T>) {
        } else if constexpr (std::is_same_v<std::string, T>) {
        } else if constexpr (std::is_same_v<enbt::raw_uuid, T>) {
        } else if constexpr (std::is_same_v<Chat, T>) {
        } else if constexpr (
            std::is_same_v<enbt::value, T>
            || std::is_same_v<enbt::compound, T>
            || std::is_same_v<enbt::dynamic_array, T>
            || std::is_same_v<enbt::fixed_array, T>
            || std::is_same_v<enbt::uuid, T>
            || std::is_same_v<enbt::simple_array_i8, T>
            || std::is_same_v<enbt::simple_array_i16, T>
            || std::is_same_v<enbt::simple_array_i32, T>
            || std::is_same_v<enbt::simple_array_i64, T>
            || std::is_same_v<enbt::simple_array_ui8, T>
            || std::is_same_v<enbt::simple_array_ui16, T>
            || std::is_same_v<enbt::simple_array_ui32, T>
            || std::is_same_v<enbt::simple_array_ui64, T>
            || std::is_same_v<T, client_bound::play::play_packet>
            || std::is_arithmetic_v<T>
        ) {
        } else if constexpr (std::is_base_of_v<base_objects::palette_container, T>) {
        } else if constexpr (std::is_same_v<base_objects::palette_data_height_map, T>) {
        } else if constexpr (is_template_base_of<ignored, T>) {
        } else if constexpr (is_template_base_of<enum_as, T> || is_template_base_of<enum_as_flag, T>) {
        } else if constexpr (is_template_base_of<any_of, T>) {
        } else if constexpr (is_template_base_of<id_set, T>) {
        } else if constexpr (std::is_same_v<bit_list_array<uint64_t>, T>) {
        } else if constexpr (is_convertible_to_packet_form<T>) {
        } else if constexpr (api::id::is_source<T>) {
        } else if constexpr (is_ordered_id<T>) {
            value.value = context.packets_state.internal_data.set([](auto& data) {
                return ++data.id_tracker[T::id_source];
            });
            value.is_valid = true;
        } else if constexpr (is_template_base_of<enum_set, T>) {
            using Tupple_T = std::decay_t<decltype(value.values)>;
            util::for_each_type<Tupple_T>::each([&]<class T_Elem>() {
                if constexpr (need_preprocess_result_v<typename T_Elem::value_type>) {
                    if (value.template has<typename T_Elem::value_type>())
                        preprocess_structure(context, value.template get<typename T_Elem::value_type>(), value);
                }
            });
        } else {
            bool process_next = true;
            reflect::for_each_field(value, [&value, &context, &process_next](auto& item) {
                if (!process_next)
                    return;
                using I = std::decay_t<decltype(item)>;
                if constexpr (need_preprocess_result_v<I>)
                    preprocess_structure(context, item, value);
                if constexpr (could_be_preprocessed<I, T>)
                    item.preprocess(value);

                if constexpr (is_template_base_of<depends_next, std::decay_t<decltype(item)>>)
                    process_next = (bool)item.value;
            });
        }
    }

    template <class T>
    void serialize_packet(base_objects::network::response& res, base_objects::SharedClientData& context, T& value) {
        using Type = std::decay_t<T>;
        if constexpr (is_packet<Type>) {
            base_objects::network::response::item it;
            it.write_id(Type::packet_id::value);
            serialize_entry(it, context, value);
            res += it;
            if constexpr (std::is_base_of_v<compound_packet, disconnect_after>)
                res.do_disconnect_after_send = true;
        } else if constexpr (std::is_base_of_v<compound_packet, Type>) {
            reflect::for_each_field(value, [&res, &context](auto& item) {
                using I = std::decay_t<decltype(item)>;
                if constexpr (is_packet<I>) {
                    serialize_packet(res, context, item);
                } else if constexpr (std::is_same_v<I, client_bound::play::play_packet>) {
                    std::visit([&](auto& it) { serialize_packet(res, context, it); }, item);
                } else if (is_template_base_of<_list_array_impl::list_array, I>) {
                    for (auto& it : item)
                        serialize_packet(res, context, it);
                }
            });
        }
    }

    template <class Type>
    void make_preprocess(base_objects::SharedClientData& context, Type& value) {
        if constexpr (need_preprocess_result_v<Type>) {
            preprocess_structure(context, value, value);
            if constexpr (could_be_preprocessed<Type, Type>)
                value.preprocess(value);
        }
    }

    template <class Ops, class Type>
    bool make_send(base_objects::SharedClientData& context, Type&& value) {
        if (!context.is_active())
            return false;
        make_preprocess(context, value);
        if (Ops::send_viewer().notify(value, context))
            return false;
        base_objects::network::response res;
        serialize_packet(res, context, value);
        if constexpr (std::is_base_of_v<disconnect_after, Type>)
            res.do_disconnect_after_send = true;
        context.sendPacket(std::move(res));

        if constexpr (std::is_base_of_v<switches_to::status, Type>)
            context << switches_to::status{};
        else if constexpr (std::is_base_of_v<switches_to::login, Type>)
            context << switches_to::login{};
        else if constexpr (std::is_base_of_v<switches_to::config, Type>)
            context << switches_to::config{};
        else if constexpr (std::is_base_of_v<switches_to::play, Type>)
            context << switches_to::play{};

        Ops::post_send_viewer().notify(value, context);
        return true;
    }

    

    template <class Ops, class Type>
    base_objects::network::response make_encode(base_objects::SharedClientData& context, Type&& value) {
        make_preprocess(context, value);
        base_objects::network::response res;
        serialize_packet(res, context, value);
        if constexpr (std::is_base_of_v<disconnect_after, Type>)
            res.do_disconnect_after_send = true;
        return res;
    }
}

#endif /* SRC_API_BIN_PACKETS_GENERIC_ENCODE */
