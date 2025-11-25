/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTIL_ENCODING_NBT_DESERIALIZATION
#define SRC_UTIL_ENCODING_NBT_DESERIALIZATION
#include <src/util/encoding/common.hpp>
#include <src/util/encoding/nbt/nbt_common.hpp>

namespace copper_server::util::encoding::nbt {

    template <class T, class T_prev>
    void deserialize_entry(T& res, util::nbt_read_stream& stream, T_prev& prev) {
        using Type = std::decay_t<T>;
        if constexpr (
            std::is_arithmetic_v<Type>
            || std::is_same_v<std::string, Type>
            || std::is_same_v<std::string_view, Type>
            || std::is_same_v<base_objects::uuid, Type>
            || std::is_same_v<base_objects::uuid_hex, Type>
            || std::is_same_v<base_objects::uuid_flat_hex, Type>
        )
            stream.read_as(res);
        else if constexpr (std::is_same_v<::enbt::value, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_enbt();
        } else if constexpr (std::is_same_v<::enbt::compound, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_enbt().as_compound();
        } else if constexpr (std::is_same_v<::enbt::dynamic_array, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_enbt().as_dyn_array();
        } else if constexpr (std::is_same_v<::enbt::fixed_array, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_enbt().as_fixed_array();
        } else if constexpr (std::is_same_v<::enbt::uuid, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_enbt().as_uuid();
        } else if constexpr (std::is_same_v<::enbt::simple_array_i8, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_enbt().as_i8_array();
        } else if constexpr (std::is_same_v<::enbt::simple_array_i16, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_enbt().as_i16_array();
        } else if constexpr (std::is_same_v<::enbt::simple_array_i32, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_enbt().as_i32_array();
        } else if constexpr (std::is_same_v<::enbt::simple_array_i64, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_enbt().as_i64_array();
        } else if constexpr (std::is_same_v<::enbt::simple_array_ui8, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_enbt().as_ui8_array();
        } else if constexpr (std::is_same_v<::enbt::simple_array_ui16, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_enbt().as_ui16_array();
        } else if constexpr (std::is_same_v<::enbt::simple_array_ui32, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_enbt().as_ui32_array();
        } else if constexpr (std::is_same_v<::enbt::simple_array_ui64, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_enbt().as_ui64_array();
        } else if constexpr (is_std_array<Type>) {
            stream.iterate(
                [&res](auto size) { if (res.size() != size)
                    throw std::runtime_error("Size mismatch, detected for std::array"); },
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
            std::is_same_v<api::packets::identifier, Type>
            || is_string_sized<Type>
            || std::is_same_v<api::packets::json_text_component, Type>
            || std::is_same_v<api::packets::var_int32, Type>
            || std::is_same_v<api::packets::var_int64, Type>
        )
            stream.read_as(res.value);
        else if constexpr (std::is_same_v<base_objects::velocity, Type>) {
            stream.iterate([&res](auto& name, auto& value) {
                if (name == "x")
                    value.read_into(res.x);
                else if (name == "y")
                    value.read_into(res.y);
                else if (name == "z")
                    value.read_into(res.z);
            });
        } else if constexpr (std::is_same_v<base_objects::chat, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
            res = base_objects::chat::from_enbt(ss.get_as_enbt());
        } else if constexpr (std::is_same_v<api::packets::optional_var_int32, Type>) {
            int32_t res_value = 0;
            stream.read_into(res_value);
            if (res_value != 0)
                res = res_value - 1;
        } else if constexpr (std::is_same_v<api::packets::optional_var_int64, Type>) {
            int64_t res_value = 0;
            stream.read_into(res_value);
            if (res_value != 0)
                res = res_value - 1;
        } else if constexpr (std::is_same_v<base_objects::position, Type>) {
            stream.iterate([&res](auto& name, auto& value) {
                if (name == "x")
                    value.read_into(res.x);
                else if (name == "y")
                    value.read_into(res.y);
                else if (name == "z")
                    value.read_into(res.z);
            });
        } else if constexpr (is_template_base_of<api::packets::ignored, Type>) {
            nbt_enbt_convert ss;
            stream.read_into(ss);
        } else if constexpr (is_template_base_of<std::optional, Type>) {
            stream.read_compound()
                .collect("opt", [&](util::nbt_read_stream& opt_stream) {
                    res.emplace();
                    deserialize_entry(*res, opt_stream, prev);
                })
                .make_collect();
        } else if constexpr (is_template_base_of<api::packets::enum_as, Type>) {
            std::string str;
            stream.read_into(str);
            res.value = reflect::get_enum_value<typename Type::enum_t>(str);
        } else if constexpr (is_template_base_of<api::packets::enum_as_flag, Type>) {
            std::string str;
            stream.read_into(str);
            res.value = reflect::get_enum_flag_value<typename Type::enum_t>(str);
        } else if constexpr (is_template_base_of<api::packets::or_, Type> || is_template_base_of<api::packets::bool_or, Type>) {
            if constexpr (get_nbt_type<typename Type::var_0>() != get_nbt_type<typename Type::var_1>()) {
                if (get_nbt_type<typename Type::var_0>() == stream.get_type()) {
                    typename Type::var_0 it{};
                    deserialize_entry(it, var_stream, prev);
                    res = std::move(it);
                } else {
                    typename Type::var_1 it{};
                    deserialize_entry(it, var_stream, prev);
                    res = std::move(it);
                }
            } else {
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
            }
        } else if constexpr (is_template_base_of<api::packets::enum_switch, Type>) {
            if constexpr (enum_switch_is_inline_eligible<Type>) {
                Type::for_each([]<class Ty>() {
                    if (get_nbt_type<Ty>() == type) {
                        Ty it{};
                        deserialize_entry(it, data_stream, prev);
                        res = std::move(it);
                    }
                });
            } else {
                std::string type;
                stream.double_pass_read(
                    [&type](util::nbt_read_stream& this_stream) {
                        this_stream
                            .read_compound()
                            .collect_into("type", type)
                            .make_collect();
                    },
                    [&res, &prev](util::nbt_read_stream& data_stream) {
                        Type::for_each([&data_stream, &res, &prev]<class Ty>() {
                            if (type == reflect::get_pretty_type_name<it_T>()) {
                                stream.iterate([&res, &prev](auto& name, auto& item_stream) {
                                    if (name == "type")
                                        return;
                                    reflect::visit_field(name, res, [&item_stream, &prev](auto& res_v) {
                                        deserialize_entry(res_v, item_stream, prev);
                                    });
                                });
                            }
                        });
                    }
                );
            }
        } else if constexpr (is_template_base_of<base_objects::box, Type>) {
            res.ptr = std::make_shared<typename Type::value_type>();
            deserialize_entry(*res, stream, prev);
        } else if constexpr (is_template_base_of<api::packets::any_of, Type> || is_ordered_id<Type> || is_template_base_of<api::packets::sized_entry, Type>) {
            deserialize_entry(res.value, stream, prev);
        } else if constexpr (is_template_base_of<api::packets::flags_list, Type>) {
            stream.iterate([&res, &prev](auto& name, auto& comp_stream) {
                if (name == "flag") {
                    deserialize_entry(res, comp_stream, prev);
                } else if (name == "items") {
                    auto arr_r = comp_stream.read_list();
                    res.for_each_set_flag_in_order([&arr_r, &res, &prev]<class Ty>() {
                        arr_r.read_one([&res, &prev](auto& flag_stream) {
                            flag_stream
                                .read_compound()
                                .collect("", [&](auto& i_flag_stream) {
                                    Ty tmp{};
                                    deserialize_entry(tmp, i_flag_stream, prev);
                                    res.values.emplace(Ty::flag_order::value, std::move(tmp));
                                })
                                .make_collect();
                        });
                    });
                }
            });
        } else if constexpr (is_flags_list_from<Type>) {
            auto arr_r = stream.read_list();
            auto& it = (*prev).*Type::preprocess_source_name::value;
            Type::for_each_set_flag_in_order(it, [&arr_r, &res]<class Ty>() {
                arr_r.read_one([&res](auto& flag_stream) {
                    flag_stream
                        .read_compound()
                        .collect("", [&](auto& i_flag_stream) {
                            Ty tmp{};
                            deserialize_entry(tmp, flag_stream);
                            res.set(std::move(tmp));
                        })
                        .force_all_collect();
                });
            });
        } else if constexpr (is_template_base_of<api::packets::value_optional, Type>) {
            stream.read_compound()
                .collect("opt", [&res, &prev](auto& opt_stream) {
                    opt_stream
                        .read_list()
                        .read_one([&res, &prev](auto& a_stream) {
                            a_stream
                                .read_compound()
                                .collect("", [&res, &prev](auto& ac_stream) {
                                    deserialize_entry(res.v, ac_stream, prev);
                                })
                                .force_all_collect();
                        })
                        .read_one([&res, &prev](auto& a_stream) {
                            a_stream
                                .read_compound()
                                .collect("", [&res, &prev](auto& ac_stream) {
                                    res.rest.emplace();
                                    deserialize_entry(*res.rest, ac_stream, prev);
                                })
                                .force_all_collect();
                        });
                })
                .make_collect();
        } else if constexpr (is_limited_num<Type>) {
            stream.read_into(res.value);
        } else if constexpr (is_convertible_to_nbt_form<Type>) {
            res = Type::from_nbt(stream);
        } else if constexpr (api::packets::is_convertible_to_packet_form<Type>) {
            api::packets::convertible_to_packet_type<Type> tmp{};
            deserialize_entry(tmp, stream, prev);
            res = Type::from_packet(std::move(tmp));
        } else if constexpr (api::id::is_source<Type>) {
            std::string tmp;
            stream.read_into(tmp);
            res = Type(tmp);
        } else if constexpr (is_template_base_of<base_objects::pool, Type>) {
            stream.iterate([&res, &prev](util::nbt_read_stream& pool_item) {
                int32_t weight = 0;
                typename Type::value_type data;

                pool_item
                    .read_compound()
                    .collect_as("weight", weight)
                    .collect("data", [&](auto& data_stream) {
                        deserialize_entry(data, data_stream, prev);
                    })
                    .force_all_collect();
                res.add(weight, std::move(data));
            });
        } else if constexpr (nbt_is_inline<Type> && reflect::fields_count<Type> == 1) {
            reflect::visit_field<Type>(0, [&]<class T>() {
                deserialize_entry(res, stream, prev);
            });
        } else {
            if (stream.get_type() == nbt_type::tag_compound)
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
                throw std::runtime_error("Invalid encoding");
        }
    }

    template <class T>
    void deserialize_entry(T& res, util::nbt_read_stream& stream) {
        deserialize_entry(res, stream, res);
    }
}

#endif /* SRC_UTIL_ENCODING_NBT_DESERIALIZATION */
