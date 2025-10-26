/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_BIN_PACKETS_GENERIC_STRINGIZE
#define SRC_API_BIN_PACKETS_GENERIC_STRINGIZE
#include <library/enbt/senbt.hpp>

#include <src/api/bin/packets/generic.hpp>

namespace copper_server::api::packets {
    namespace sp {
        template <class T>
        void serialize_entry(std::string& res, size_t spacing, const T& value);

        template <class T>
        void serialize_array(std::string& res, size_t spacing, const T& value) {
            list_array<std::string> res_tmp;
            res_tmp.resize(value.size());
            size_t i = 0;
            for (auto&& it : value)
                serialize_entry(res_tmp[i++], spacing + 4, it);
            bool one_line = true;
            for (auto& it : res_tmp)
                if (it.size() > 10 || it.contains('\n'))
                    one_line = false;
            if (one_line) {
                res += "[";
                bool has_prev = false;
                for (auto& it : res_tmp) {
                    if (has_prev)
                        res += ',';
                    has_prev = true;
                    res += std::move(it);
                }
                res += "]";
            } else {
                std::string sp(spacing + 4, ' ');
                bool has_prev = false;
                res += "[\n";
                for (auto& it : res_tmp) {
                    if (has_prev)
                        res += ",\n";
                    has_prev = true;
                    res += sp + std::move(it);
                }
                res += "\n" + std::string(spacing, ' ') + "]";
            }
        }

#pragma warning(push)
#pragma warning(disable : 4702)

        template <class T>
        void serialize_entry(std::string& res, size_t spacing, const T& value) {
            using Type = std::decay_t<T>;
            if constexpr (is_convertible_to_packet_form<Type>) {
                serialize_entry(res, spacing, value.to_packet());
            } else if constexpr (std::is_same_v<identifier, Type>)
                res += "\"" + value.value + "\"";
            else if constexpr (is_constant_value<Type>) {
                serialize_entry(res, spacing, Type::value::value);
            } else if constexpr (is_string_sized<Type>)
                res += "\"" + value.value + "\"";
            else if constexpr (std::is_same_v<json_text_component, Type>)
                res += "\"" + value.value + "\"";
            else if constexpr (std::is_same_v<var_int32, Type>)
                res += std::to_string(value.value);
            else if constexpr (std::is_same_v<base_objects::velocity, Type>)
                res += "{ x = " + std::to_string(value.x) + ", y = " + std::to_string(value.y) + ", z = " + std::to_string(value.z) + " }";
            else if constexpr (is_template_base_of<base_objects::box, Type>)
                serialize_entry(res, spacing, value);
            else if constexpr (std::is_same_v<var_int64, Type>)
                res += std::to_string(value.value);
            else if constexpr (std::is_same_v<optional_var_int32, Type>) {
                if (value)
                    res += std::to_string(*value);
                else
                    res += "null";
            } else if constexpr (std::is_same_v<optional_var_int64, Type>) {
                if (value)
                    res += std::to_string(*value);
                else
                    res += "null";
            } else if constexpr (std::is_same_v<base_objects::position, Type>)
                res += "{.x = " + std::to_string(value.x) + ".z = " + std::to_string(value.z) + ".y = " + std::to_string(value.y) + "}";
            else if constexpr (std::is_same_v<bool, Type>)
                res += value ? "true" : "false";
            else if constexpr (std::is_arithmetic_v<Type>)
                res += std::to_string(value);
            else if constexpr (std::is_same_v<std::string, Type>)
                res += "\"" + value + "\"";
            else if constexpr (std::is_same_v<enbt::raw_uuid, Type>)
                res += "\"" + value.to_string() + "\"";
            else if constexpr (std::is_same_v<base_objects::chat, Type>) {
                std::string alignment(spacing + 1, ' ');
                alignment[0] = '\n';
                res += list_array<char>(senbt::serialize(value.to_enbt(), false, true))
                           .replace('\n', alignment.data(), alignment.size())
                           .to_container<std::string>();
            } else if constexpr (
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
            ) {
                std::string alignment(spacing + 1, ' ');
                alignment[0] = '\n';
                res += list_array<char>(senbt::serialize(value, false, true))
                           .replace('\n', alignment.data(), alignment.size())
                           .to_container<std::string>();
            } else if constexpr (std::is_base_of_v<base_objects::palette_container, Type>) {
                res += "palette_data";
            } else if constexpr (std::is_same_v<base_objects::palette_data_height_map, Type>) {
                res += "palette_data";
            } else if constexpr (is_template_base_of<_list_array_impl::list_array, Type> || is_std_array<Type>) {
                serialize_array(res, spacing, value);
            } else if constexpr (is_template_base_of<ignored, Type>) {
            } else if constexpr (is_template_base_of<std::optional, Type>) {
                if (value)
                    serialize_entry(res, spacing, *value);
                else
                    res += "null";
            } else if constexpr (is_template_base_of<enum_as, Type>) {
                res += reflect::get_enum_value(value.value);
            } else if constexpr (is_template_base_of<enum_as_flag, Type>) {
                res += reflect::get_enum_flag_value(value.value);
            } else if constexpr (is_template_base_of<or_, Type> || is_template_base_of<bool_or, Type>) {
                std::visit([&](auto& it) { serialize_entry(res, spacing, it); }, value);
            } else if constexpr (is_template_base_of<enum_switch, Type>) {
                std::visit(
                    [&](auto& it) {
                        using it_T = std::decay_t<decltype(it)>;
                        res += "{ ";
                        serialize_entry(res, spacing, typename Type::encode_type(it_T::item_id::value));
                        res += ": ";
                        serialize_entry(res, spacing, it);
                        res += "}";
                    },
                    value
                );
            } else if constexpr (is_template_base_of<partial_enum_switch, Type>) {
                std::visit(
                    [&](auto& it) {
                        using it_T = std::decay_t<decltype(it)>;
                        if constexpr (std::is_same_v<it_T, typename Type::encode_type>) {
                            serialize_entry(res, spacing, it);
                        } else {
                            res += "{ ";
                            serialize_entry(res, spacing, typename Type::encode_type(it_T::item_id::value));
                            res += ": ";
                            serialize_entry(res, spacing, it);
                            res += "}";
                        }
                    },
                    value
                );
            } else if constexpr (is_template_base_of<std::unique_ptr, Type>) {
                serialize_entry(res, spacing, *value);
            } else if constexpr (is_template_base_of<flags_list, Type>) {
                res += "{\n" + std::string(spacing + 4, ' ');
                serialize_entry(res, spacing + 4, value.flag);
                res += ": {";
                bool has_prev = false;
                value.for_each_in_order([&](auto& it) {
                    if (has_prev)
                        res += ',';
                    has_prev = true;
                    res += "\n" + std::string(spacing + 4, ' ');
                    serialize_entry(res, spacing + 4, it);
                });
                if (has_prev)
                    res += "\n" + std::string(spacing + 4, ' ') + "}\n" + std::string(spacing, ' ') + "}";
                else
                    res += "}\n" + std::string(spacing, ' ') + "}";
            } else if constexpr (is_flags_list_from<Type>) {
                res += "{";
                bool has_prev = false;
                value.for_each_in_order([&](auto& it) {
                    if (has_prev)
                        res += ',';
                    has_prev = true;
                    res += "\n" + std::string(spacing + 4, ' ');
                    serialize_entry(res, spacing + 4, it);
                });
                if (has_prev)
                    res += "\n" + std::string(spacing, ' ') + "}";
                else
                    res += "}";
            } else if constexpr (is_template_base_of<value_optional, Type>) {
                if (value.rest && value.v) {
                    res += "{\n" + std::string(spacing + 4, ' ');
                    serialize_entry(res, spacing + 4, value.v);
                    res += ":";
                    serialize_entry(res, spacing + 8, *value.rest);
                    res += "\n" + std::string(spacing, ' ') + "}";
                } else {
                    res += "{";
                    decltype(value.v) tmp{0};
                    serialize_entry(res, spacing + 4, tmp);
                    res += "}";
                }
            } else if constexpr (
                is_template_base_of<sized_entry, Type>
                || is_template_base_of<any_of, Type>
                || is_template_base_of<packet_compress, Type>
                || is_limited_num<Type>
            )
                serialize_entry(res, spacing, value.value);
            else if constexpr (is_bitset_fixed<Type>) {
                serialize_array(res, spacing, value.value.data());
            } else if constexpr (api::id::is_source<Type>) {
                res += value.to_string();
            } else if constexpr (std::is_same_v<bit_list_array<uint64_t>, Type>) {
                serialize_array(res, spacing, value.data());
            } else if constexpr (is_template_base_of<enum_set, Type>) {
                using Tupple_T = std::decay_t<decltype(value.values)>;
                bool processed = false;
                bool it_processed = false;
                res += "[";
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
                    for (size_t i = 0; i < siz; i++) {
                        if (it_processed)
                            res += ",";
                        res += "\n" + std::string(spacing + 4, ' ') + "{";
                        util::for_each_type<Tupple_T>::each([&]<class T_Elem>() {
                            if (!value.template has<typename T_Elem::value_type>())
                                return;
                            if (processed)
                                res += ",";
                            res += "\n" + std::string(spacing + 8, ' ');
                            serialize_entry(res, spacing + 8, value.template get<typename T_Elem::value_type>()[i]);
                            processed = true;
                        });
                        res += "\n" + std::string(spacing + 4, ' ') + "}";
                        it_processed = true;
                    }
                }
                if (processed)
                    res += "\n" + std::string(spacing, ' ') + "]";
                else
                    res += "]";
            } else {
                bool process_next = true;
                bool processed = false;
                if constexpr (is_packet<Type>)
                    res += std::string(reflect::get_pretty_type_name<Type>()) + "<" + std::to_string(Type::packet_id::value) + "> {";
                else
                    res += std::string(reflect::get_pretty_type_name<Type>()) + " {";
                reflect::for_each_field_with_name(value, [&res, spacing, &process_next, &processed](auto& item, auto name) {
                    if (process_next) {
                        if (processed)
                            res += ",";
                        res += "\n" + std::string(spacing + 4, ' ') + std::string(name) + ": ";
                        serialize_entry(res, spacing + 4, item);
                        processed = true;
                        if constexpr (is_template_base_of<depends_next, std::decay_t<decltype(item)>>)
                            process_next = (bool)item.value;
                    }
                });
                if (processed)
                    res += "\n" + std::string(spacing, ' ') + "}";
                else
                    res += "}";
            }
        }

#pragma warning(pop)

        template <class T>
        void serialize_packet(std::string& res, size_t spacing, const T& value) {
            using Type = std::decay_t<T>;
            if constexpr (std::is_base_of_v<compound_packet, Type>) {
                res += std::string(reflect::get_pretty_type_name<Type>()) + "<compound> {";
                bool processed = false;
                reflect::for_each_field_with_name(value, [&res, spacing, &processed](auto& item, auto name) {
                    using I = std::decay_t<decltype(item)>;
                    if (processed)
                        res += ",";
                    res += "\n" + std::string(spacing + 4, ' ') + std::string(name) + ": ";
                    if constexpr (is_packet<I>) {
                        serialize_packet(res, spacing + 4, item);
                    } else if constexpr (std::is_same_v<I, client_bound::play::play_packet>) {
                        std::visit([&](auto& it) { serialize_packet(res, spacing + 4, it); }, item);
                    } else if (is_template_base_of<_list_array_impl::list_array, Type>) {
                        res += "[";
                        bool arr_processed = false;
                        for (auto&& it : item) {
                            if (arr_processed)
                                res += ",\n";
                            else
                                res += "\n";
                            res += std::string(spacing, ' ');
                            serialize_packet(res, spacing + 4, it);
                            arr_processed = true;
                        }
                        if (arr_processed)
                            res += "\n";
                        res += std::string(spacing, ' ') + "]";
                    }
                    processed = true;
                });
                if (processed)
                    res += "\n" + std::string(spacing, ' ');

                res += "}";
            } else if constexpr (is_packet<Type>)
                serialize_entry(res, spacing, value);
        }
    }
}


#endif /* SRC_API_BIN_PACKETS_GENERIC_STRINGIZE */
