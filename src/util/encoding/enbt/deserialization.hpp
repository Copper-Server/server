/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_ENCODING_ENBT_DESERIALIZATION
#define SRC_UTIL_ENCODING_ENBT_DESERIALIZATION
#include <library/enbt/io.hpp>
#include <src/util/encoding/common.hpp>

namespace copper_server::util::encoding::enbt {
    template <class T, class T_prev>
    void deserialize_entry(T& res, const ::enbt::value& value, T_prev& prev) {
        using Type = std::decay_t<T>;
        if constexpr (
            std::is_arithmetic_v<Type>
            || std::is_same_v<std::string, Type>
            || std::is_same_v<::enbt::raw_uuid, Type>
            || std::is_same_v<::enbt::value, Type>
        )
            res = (Type)value;
        else if constexpr (std::is_same_v<::enbt::compound, Type>)
            res = value.as_compound();
        else if constexpr (std::is_same_v<::enbt::dynamic_array, Type>)
            res = value.as_dyn_array();
        else if constexpr (std::is_same_v<::enbt::fixed_array, Type>)
            res = value.as_fixed_array();
        else if constexpr (std::is_same_v<::enbt::uuid, Type>)
            res = value.as_uuid();
        else if constexpr (std::is_same_v<::enbt::simple_array_i8, Type>)
            res = value.as_i8_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_i16, Type>)
            res = value.as_i16_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_i32, Type>)
            res = value.as_i32_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_i64, Type>)
            res = value.as_i64_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_ui8, Type>)
            res = value.as_ui8_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_ui16, Type>)
            res = value.as_ui16_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_ui32, Type>)
            res = value.as_ui32_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_ui64, Type>)
            res = value.as_ui64_array();
        else if constexpr (is_std_array<Type>) {
            if (value.size() != res.size())
                throw ::enbt::exception("Size mismatch, detected for std::array");
            size_t i = 0;
            for (auto& it : value.as_array())
                deserialize_entry(res[i++], it, prev);
        } else if constexpr (is_template_base_of<_list_array_impl::list_array, Type>) {
            res.reserve(value.size());
            for (auto& it : value.as_array()) {
                res.emplace_back();
                deserialize_entry(res.back(), it, prev);
            }
        } else if constexpr (
            std::is_same_v<base_objects::identifier, Type>
            || is_string_sized<Type>
            || std::is_same_v<base_objects::json_text_component, Type>
            || std::is_same_v<base_objects::var_int32, Type>
            || std::is_same_v<base_objects::var_int64, Type>
        )
            res.value = (typename Type::underlying_type)value;
        else if constexpr (std::is_same_v<Chat, Type>)
            res = Chat::fromEnbt(value);
        else if constexpr (std::is_same_v<base_objects::optional_var_int32, Type> || std::is_same_v<base_objects::optional_var_int64, Type>) {
            if (value.contains() && value.type_equal(::enbt::type::optional))
                res = *value.get_optional();
        } else if constexpr (std::is_same_v<base_objects::position, Type>)
            res = {value["x"], value["y"], value["z"]};
        else if constexpr (is_template_base_of<base_objects::ignored, Type>) {
        } else if constexpr (is_template_base_of<std::optional, Type>) {
            if (value.contains() && value.type_equal(::enbt::type::optional)) {
                res.emplace();
                deserialize_entry(res, *value.get_optional(), prev);
            }
        } else if constexpr (is_template_base_of<base_objects::enum_as, Type>) {
            res = reflect::get_enum_value<typename Type::enum_t>((std::string)value);
        } else if constexpr (is_template_base_of<base_objects::enum_as_flag, Type>) {
            res = reflect::get_enum_flag_value<typename Type::enum_t>((std::string)value);
        } else if constexpr (is_template_base_of<base_objects::or_, Type>) {
            if (value.is_compound()) {
                if (value.contains("var_0")) {
                    typename Type::var_0 tmp{};
                    deserialize_entry(tmp, value["var_0"], prev);
                    res = std::move(tmp);
                } else if (value.contains("var_1")) {
                    typename Type::var_1 tmp{};
                    deserialize_entry(tmp, value["var_1"], prev);
                    res = std::move(tmp);
                } else if constexpr (reflect::fields_count<typename Type::var_0>() > reflect::fields_count<typename Type::var_1>()) {
                    typename Type::var_0 tmp{};
                    deserialize_entry(tmp, value, prev);
                    res = std::move(tmp);
                } else if constexpr (reflect::fields_count<typename Type::var_1>() > reflect::fields_count<typename Type::var_0>()) {
                    typename Type::var_1 tmp{};
                    deserialize_entry(tmp, value, prev);
                    res = std::move(tmp);
                } else
                    throw ::enbt::exception("Invalid format");
            } else if (value.is_string()) {
                if constexpr (std::is_convertible_v<std::string, typename Type::var_0>)
                    res = typename Type::var_0{(std::string)value};
                else if constexpr (std::is_convertible_v<std::string, typename Type::var_1>)
                    res = typename Type::var_1{(std::string)value};
                else
                    throw ::enbt::exception("Invalid format, failed to convert string");
            } else if (value.is_numeric()) {
                switch (value.get_type()) {
                case ::enbt::type::integer:
                    switch (value.get_type_len()) {
                    case ::enbt::type_len::Tiny:
                        if (value.get_type_sign()) {
                            if constexpr (std::is_convertible_v<int8_t, typename Type::var_0>)
                                res = typename Type::var_0{(int8_t)value};
                            else if constexpr (std::is_convertible_v<int8_t, typename Type::var_1>)
                                res = typename Type::var_1{(int8_t)value};
                            else
                                throw ::enbt::exception("Invalid format, failed to convert i8");
                        } else if constexpr (std::is_convertible_v<uint8_t, typename Type::var_0>)
                            res = typename Type::var_0{(uint8_t)value};
                        else if constexpr (std::is_convertible_v<uint8_t, typename Type::var_1>)
                            res = typename Type::var_1{(uint8_t)value};
                        else
                            throw ::enbt::exception("Invalid format, failed to convert ui8");
                        break;
                    case ::enbt::type_len::Short:
                        if (value.get_type_sign()) {
                            if constexpr (std::is_convertible_v<int16_t, typename Type::var_0>)
                                res = typename Type::var_0{(int16_t)value};
                            else if constexpr (std::is_convertible_v<int16_t, typename Type::var_1>)
                                res = typename Type::var_1{(int16_t)value};
                            else
                                throw ::enbt::exception("Invalid format, failed to convert i16");
                        } else if constexpr (std::is_convertible_v<uint16_t, typename Type::var_0>)
                            res = typename Type::var_0{(uint16_t)value};
                        else if constexpr (std::is_convertible_v<uint16_t, typename Type::var_1>)
                            res = typename Type::var_1{(uint16_t)value};
                        else
                            throw ::enbt::exception("Invalid format, failed to convert ui16");
                        break;
                    case ::enbt::type_len::Default:
                        if (value.get_type_sign()) {
                            if constexpr (std::is_convertible_v<int32_t, typename Type::var_0>)
                                res = typename Type::var_0{(int32_t)value};
                            else if constexpr (std::is_convertible_v<int32_t, typename Type::var_1>)
                                res = typename Type::var_1{(int32_t)value};
                            else
                                throw ::enbt::exception("Invalid format, failed to convert i32");
                        } else if constexpr (std::is_convertible_v<uint32_t, typename Type::var_0>)
                            res = typename Type::var_0{(uint32_t)value};
                        else if constexpr (std::is_convertible_v<uint32_t, typename Type::var_1>)
                            res = typename Type::var_1{(uint32_t)value};
                        else
                            throw ::enbt::exception("Invalid format, failed to convert ui32");
                        break;
                    case ::enbt::type_len::Long:
                        if (value.get_type_sign()) {
                            if constexpr (std::is_convertible_v<int64_t, typename Type::var_0>)
                                res = typename Type::var_0{(int64_t)value};
                            else if constexpr (std::is_convertible_v<int64_t, typename Type::var_1>)
                                res = typename Type::var_1{(int64_t)value};
                            else
                                throw ::enbt::exception("Invalid format, failed to convert i32");
                        } else if constexpr (std::is_convertible_v<uint64_t, typename Type::var_0>)
                            res = typename Type::var_0{(uint64_t)value};
                        else if constexpr (std::is_convertible_v<uint64_t, typename Type::var_1>)
                            res = typename Type::var_1{(uint64_t)value};
                        else
                            throw ::enbt::exception("Invalid format, failed to convert ui64");
                        break;
                    default:
                        throw ::enbt::exception("Invalid format");
                    }
                    break;
                case ::enbt::type::var_integer:
                    switch (value.get_type_len()) {
                    case ::enbt::type_len::Default:
                        if (value.get_type_sign()) {
                            if constexpr (std::is_convertible_v<int32_t, typename Type::var_0>)
                                res = typename Type::var_0{(int32_t)value};
                            else if constexpr (std::is_convertible_v<int32_t, typename Type::var_1>)
                                res = typename Type::var_1{(int32_t)value};
                            else
                                throw ::enbt::exception("Invalid format, failed to convert var_i32");
                        } else if constexpr (std::is_convertible_v<uint32_t, typename Type::var_0>)
                            res = typename Type::var_0{(uint32_t)value};
                        else if constexpr (std::is_convertible_v<uint32_t, typename Type::var_1>)
                            res = typename Type::var_1{(uint32_t)value};
                        else
                            throw ::enbt::exception("Invalid format, failed to convert var_ui32");
                        break;
                    case ::enbt::type_len::Long:
                        if (value.get_type_sign()) {
                            if constexpr (std::is_convertible_v<int64_t, typename Type::var_0>)
                                res = typename Type::var_0{(int64_t)value};
                            else if constexpr (std::is_convertible_v<int64_t, typename Type::var_1>)
                                res = typename Type::var_1{(int64_t)value};
                            else
                                throw ::enbt::exception("Invalid format, failed to convert var_i32");
                        } else if constexpr (std::is_convertible_v<uint64_t, typename Type::var_0>)
                            res = typename Type::var_0{(uint64_t)value};
                        else if constexpr (std::is_convertible_v<uint64_t, typename Type::var_1>)
                            res = typename Type::var_1{(uint64_t)value};
                        else
                            throw ::enbt::exception("Invalid format, failed to convert var_ui64");
                        break;
                    default:
                        throw ::enbt::exception("Invalid format");
                    }
                    break;
                case ::enbt::type::comp_integer:
                    if constexpr (std::is_convertible_v<uint64_t, typename Type::var_0>)
                        res = typename Type::var_0{(uint64_t)value};
                    else if constexpr (std::is_convertible_v<uint64_t, typename Type::var_1>)
                        res = typename Type::var_1{(uint64_t)value};
                    else
                        throw ::enbt::exception("Invalid format, failed to convert var_ui64");
                    break;
                case ::enbt::type::floating:
                    switch (value.get_type_len()) {
                    case ::enbt::type_len::Default:
                        if constexpr (std::is_convertible_v<float, typename Type::var_0>)
                            res = typename Type::var_0{(float)value};
                        else if constexpr (std::is_convertible_v<float, typename Type::var_1>)
                            res = typename Type::var_1{(float)value};
                        else
                            throw ::enbt::exception("Invalid format, failed to convert float");
                        break;
                    case ::enbt::type_len::Long:
                        if constexpr (std::is_convertible_v<double, typename Type::var_0>)
                            res = typename Type::var_0{(double)value};
                        else if constexpr (std::is_convertible_v<double, typename Type::var_1>)
                            res = typename Type::var_1{(double)value};
                        else
                            throw ::enbt::exception("Invalid format, failed to convert double");
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    throw ::enbt::exception("Invalid format");
                }
            } else
                throw ::enbt::exception("Invalid format");
        } else if constexpr (is_template_base_of<base_objects::enum_switch, Type>) {
            if(value.is_compound()){
                if (value.contains("type") && value.contains("data")){
                    Type::get_enum(value["type"], [&]<class Ty>() {
                        Ty tmp{};
                        deserialize_entry(tmp, value["data"], prev);
                        res = std::move(tmp);
                    });
                    return;
                }
            }
            if constexpr (!std::is_same_v<typename Type::default_item, void>) {
                typename Type::default_item tmp{};
                deserialize_entry(tmp, value, prev);
                res = std::move(tmp);
            }
        } else if constexpr (is_template_base_of<base_objects::box, Type>) {
            res = std::make_shared<typename Type::value_type>();
            deserialize_entry(*res, value, prev);
        } else if constexpr (is_template_base_of<base_objects::any_of, Type>) {
            deserialize_entry(*res.value, value, prev);
        } else if constexpr (is_template_base_of<base_objects::flags_list, Type>) {
            deserialize_entry(res, value.at("flag"), prev);
            auto& items = value.at("items");
            size_t i = 0;
            res.for_each_set_flag_in_order([&res, &prev, &i, &items]<class Ty>() {
                Ty tmp{};
                deserialize_entry(tmp, items[i++], prev);
                res.values.emplace(Ty::flag_order::value, std::move(tmp));
            });
        } else if constexpr (is_flags_list_from<Type>) {
            auto& it = (*prev).*Type::preprocess_source_name::value;
            size_t i = 0;
            Type::for_each_set_flag_in_order(it, [&res, &prev, &value, &i]<class Ty>() {
                Ty tmp{};
                deserialize_entry(tmp, value.at(i++), prev);
                res.set(std::move(tmp));
            });
        } else if constexpr (is_ordered_id<Type>) {
            deserialize_entry(res.value, value, prev);
        } else if constexpr (is_template_base_of<base_objects::value_optional, Type>) {
            if (value.contains() && value.type_equal(::enbt::type::optional)) {
                res.rest.emplace();
                deserialize_entry(res.v, value.get_optional()->at(0), prev);
                deserialize_entry(*res.rest, value.get_optional()->at(1), prev);
            }
        } else if constexpr (is_template_base_of<base_objects::sized_entry, Type>) {
            deserialize_entry(res, value.get_log_value(), prev);
        } else if constexpr (is_limited_num<Type>) {
            deserialize_entry(res.value, value, prev);
        } else if constexpr (base_objects::is_convertible_to_packet_form<Type>) {
            base_objects::convertible_to_packet_type<Type> tmp{};
            deserialize_entry(tmp, value, prev);
            res = Type::from_packet(std::move(tmp));
        } else if constexpr (api::id::is_source<Type>) {
            res = (std::string)value;
        } else {
            if (value.is_compound())
                for (auto& [name, item_value] : value.as_compound())
                    reflect::visit_field(name, res, [&item_value, &prev](auto& res_v) {
                        deserialize_entry(res_v, item_value, prev);
                    });
            else if constexpr (reflect::fields_count<Type>() == 1)
                reflect::visit_field(0, res, [&value, &prev](auto& res_v) {
                    deserialize_entry(res_v, value, prev);
                });
            else if constexpr (reflect::fields_count<Type>() != 0)
                throw ::enbt::exception("Invalid encoding");
        }
    }

    template <class T, class T_prev>
    void deserialize_entry(T& res, ::enbt::io_helper::value_read_stream& stream, T_prev& prev) {
        using Type = std::decay_t<T>;
        if constexpr (
            std::is_arithmetic_v<Type>
            || std::is_same_v<std::string, Type>
            || std::is_same_v<::enbt::raw_uuid, Type>
            || std::is_same_v<::enbt::value, Type>
        )
            res = (Type)stream.read();
        else if constexpr (std::is_same_v<::enbt::compound, Type>)
            res = stream.read().as_compound();
        else if constexpr (std::is_same_v<::enbt::dynamic_array, Type>)
            res = stream.read().as_dyn_array();
        else if constexpr (std::is_same_v<::enbt::fixed_array, Type>)
            res = stream.read().as_fixed_array();
        else if constexpr (std::is_same_v<::enbt::uuid, Type>)
            res = stream.read().as_uuid();
        else if constexpr (std::is_same_v<::enbt::simple_array_i8, Type>)
            res = stream.read().as_i8_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_i16, Type>)
            res = stream.read().as_i16_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_i32, Type>)
            res = stream.read().as_i32_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_i64, Type>)
            res = stream.read().as_i64_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_ui8, Type>)
            res = stream.read().as_ui8_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_ui16, Type>)
            res = stream.read().as_ui16_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_ui32, Type>)
            res = stream.read().as_ui32_array();
        else if constexpr (std::is_same_v<::enbt::simple_array_ui64, Type>)
            res = stream.read().as_ui64_array();
        else if constexpr (is_std_array<Type>) {
            stream.iterate(
                [&res](auto size) { if (res.size() != size)
                    throw ::enbt::exception("Size mismatch, detected for std::array"); },
                [&res](auto& item_stream) {
                    for (auto& item : res)
                        deserialize_entry(item, item_stream, res);
                }
            );
        } else if constexpr (is_template_base_of<_list_array_impl::list_array, Type>) {
            stream.iterate(
                [&res](auto size) { res.reserve(size); },
                [&res, &prev](auto& item_stream) {
                    res.emplace_back();
                    deserialize_entry(res.back(), item_stream, prev);
                }
            );
        } else if constexpr (
            std::is_same_v<base_objects::identifier, Type>
            || is_string_sized<Type>
            || std::is_same_v<base_objects::json_text_component, Type>
            || std::is_same_v<base_objects::var_int32, Type>
            || std::is_same_v<base_objects::var_int64, Type>
        )
            res.value = (typename Type::underlying_type)stream.read();
        else if constexpr (std::is_same_v<Chat, Type>)
            res = Chat::fromEnbt(stream.read());
        else if constexpr (std::is_same_v<base_objects::optional_var_int32, Type> || std::is_same_v<base_objects::optional_var_int64, Type>) {
            stream.read_optional([&res](auto& has_stream) {
                deserialize_entry(res, has_stream, res);
            });
        } else if constexpr (std::is_same_v<base_objects::position, Type>) {
            stream.iterate([&res](auto& name, auto& value) {
                if (name == "x")
                    res.x = value.read();
                else if (name == "y")
                    res.y = value.read();
                else if (name == "z")
                    res.z = value.read();
            });
        } else if constexpr (is_template_base_of<base_objects::ignored, Type>) {
            stream.read(); //check if stream allows seek and peek ope
        } else if constexpr (is_template_base_of<std::optional, Type>) {
            stream.read_optional([&res, &prev](auto& has_stream) {
                res.emplace();
                deserialize_entry(*res, has_stream, prev);
            });
        } else if constexpr (is_template_base_of<base_objects::enum_as, Type>) {
            res.value = reflect::get_enum_value<typename Type::enum_t>((std::string&)stream.read());
        } else if constexpr (is_template_base_of<base_objects::enum_as_flag, Type>) {
            res.value = reflect::get_enum_flag_value<typename Type::enum_t>((std::string&)stream.read());
        } else if constexpr (is_template_base_of<base_objects::or_, Type>) {
            stream.iterate([&res, &prev](std::string_view view, auto& var_stream) {
                if (view == "var_0") {
                    typename Type::var_0 it{};
                    deserialize_entry(it, var_stream, prev);
                    res = std::move(it);
                } else if (view == "var_1") {
                    typename Type::var_1 it{};
                    deserialize_entry(it, var_stream, prev);
                    res = std::move(it);
                }
            });
        } else if constexpr (is_template_base_of<base_objects::enum_switch, Type>) {
            typename Type::encode_type selected_type{};
            stream
                .read_compound(true)
                .collect_as("type", selected_type)
                .collect("data", [&res, &prev, &selected_type](auto& data_stream) {
                    Type::get_enum(selected_type, [&res, &prev, &data_stream]<class Ty>() {
                        Ty it{};
                        deserialize_entry(it, data_stream, prev);
                        res = std::move(it);
                    });
                })
                .force_all_collect();
        } else if constexpr (is_template_base_of<base_objects::box, Type>) {
            res.ptr = std::make_shared<typename Type::value_type>();
            deserialize_entry(*res, stream, prev);
        } else if constexpr (is_template_base_of<base_objects::any_of, Type>) {
            deserialize_entry(res.value, stream, prev);
        } else if constexpr (is_template_base_of<base_objects::flags_list, Type>) {
            stream.iterate([&res, &prev](auto& name, auto& comp_stream) {
                if (name == "flag") {
                    deserialize_entry(res, comp_stream, prev);
                } else if (name == "items") {
                    auto arr_r = comp_stream.read_array();
                    res.for_each_set_flag_in_order([&arr_r, &res, &prev]<class Ty>() {
                        arr_r.read_one([&res, &prev](auto& flag_stream) {
                            Ty tmp{};
                            deserialize_entry(tmp, flag_stream, prev);
                            res.values.emplace(Ty::flag_order::value, std::move(tmp));
                        });
                    });
                }
            });
        } else if constexpr (is_flags_list_from<Type>) {
            auto arr_r = stream.read_array();
            auto& it = (*prev).*Type::preprocess_source_name::value;
            Type::for_each_set_flag_in_order(it, [&arr_r, &res]<class Ty>() {
                arr_r.read_one([&res](auto& flag_stream) {
                    Ty tmp{};
                    deserialize_entry(tmp, flag_stream);
                    res.set(std::move(tmp));
                });
            });
        } else if constexpr (is_ordered_id<Type>) {
            deserialize_entry(res.value, stream, prev);
        } else if constexpr (is_template_base_of<base_objects::value_optional, Type>) {
            stream.read_optional(
                [&res, &prev](auto& vopt_stream) {
                    vopt_stream
                        .read_darray()
                        .read_one([&res, &prev](auto& opt_stream) {
                            deserialize_entry(res.v, opt_stream, prev);
                        })
                        .read_one([&res, &prev](auto& opt_stream) {
                            res.rest.emplace();
                            deserialize_entry(*res.rest, opt_stream, prev);
                        });
                },
                [&res]() {
                    res.v = {};
                    res.rest = std::nullopt;
                }
            );
        } else if constexpr (is_template_base_of<base_objects::sized_entry, Type>) {
            stream.join_log_item([&res, &prev](auto& log_stream) {
                deserialize_entry(res, log_stream, prev);
            });
        } else if constexpr (is_limited_num<Type>) {
            res.value = stream.read();
        } else if constexpr (base_objects::is_convertible_to_packet_form<Type>) {
            base_objects::convertible_to_packet_type<Type> tmp{};
            deserialize_entry(tmp, stream, prev);
            res = Type::from_packet(std::move(tmp));
        } else if constexpr (api::id::is_source<Type>) {
            res = (std::string)stream.read();
        } else {
            if (stream.get_type_id().type == ::enbt::type::compound)
                stream.iterate([&res, &prev](auto& name, auto& item_stream) {
                    reflect::visit_field(name, res, [&item_stream, &prev](auto& res_v) {
                        deserialize_entry(res_v, item_stream, prev);
                    });
                });
            else if constexpr (reflect::fields_count<Type>() == 1)
                reflect::visit_field(0, res, [&stream, &prev](auto& res_v) {
                    deserialize_entry(res_v, stream, prev);
                });
            else if constexpr (reflect::fields_count<Type>() != 0)
                throw ::enbt::exception("Invalid encoding");
        }
    }

    template <class T>
    void deserialize_entry(T& res, const ::enbt::value& value) {
        deserialize_entry(res, value, res);
    }

    template <class T>
    void deserialize_entry(T& res, ::enbt::io_helper::value_read_stream& stream) {
        deserialize_entry(res, stream, res);
    }
}

#endif /* SRC_UTIL_ENCODING_ENBT_DESERIALIZATION */
