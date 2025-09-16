/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_ENCODING_ENBT_SERIALIZATION
#define SRC_UTIL_ENCODING_ENBT_SERIALIZATION
#include <library/enbt/io.hpp>
#include <src/util/encoding/common.hpp>

//include <src/util/reflect/*.hpp> headers to use reflection for serialization
namespace copper_server::util::encoding::enbt {
    template <class T>
    void serialize_entry(::enbt::value& res, const T& value) {
        using Type = std::decay_t<T>;
        if constexpr (
            std::is_arithmetic_v<Type>
            || std::is_same_v<std::string, Type>
            || std::is_same_v<::enbt::raw_uuid, Type>
            || std::is_same_v<::enbt::value, Type>
            || std::is_same_v<::enbt::compound, Type>
            || std::is_same_v<::enbt::dynamic_array, Type>
            || std::is_same_v<::enbt::fixed_array, Type>
            || std::is_same_v<::enbt::uuid, Type>
            || std::is_same_v<::enbt::simple_array_i8, Type>
            || std::is_same_v<::enbt::simple_array_i16, Type>
            || std::is_same_v<::enbt::simple_array_i32, Type>
            || std::is_same_v<::enbt::simple_array_i64, Type>
            || std::is_same_v<::enbt::simple_array_ui8, Type>
            || std::is_same_v<::enbt::simple_array_ui16, Type>
            || std::is_same_v<::enbt::simple_array_ui32, Type>
            || std::is_same_v<::enbt::simple_array_ui64, Type>
        ) {
            res = value;
        } else if constexpr (is_std_array<Type> || is_template_base_of<_list_array_impl::list_array, Type>) {
            ::enbt::fixed_array arr;
            arr.reserve(value.size());
            for (auto& it : value) {
                ::enbt::value in;
                serialize_entry(in, it);
                arr.push_back(in);
            }
            res = std::move(arr);
        } else if constexpr (
            std::is_same_v<base_objects::identifier, Type>
            || is_string_sized<Type>
            || std::is_same_v<base_objects::json_text_component, Type>
            || std::is_same_v<base_objects::var_int32, Type>
            || std::is_same_v<base_objects::var_int64, Type>
        )
            res = value.value;
        else if constexpr (std::is_same_v<Chat, Type>)
            res = value.ToENBT();
        else if constexpr (std::is_same_v<base_objects::optional_var_int32, Type>) {
            if (value)
                res = ::enbt::optional(value);
            else
                res = ::enbt::optional();
        } else if constexpr (std::is_same_v<base_objects::optional_var_int64, Type>) {
            if (value)
                res = ::enbt::optional(value);
            else
                res = ::enbt::optional();
        } else if constexpr (std::is_same_v<base_objects::position, Type>)
            res = ::enbt::compound{
                {"x", value.x},
                {"y", value.y},
                {"z", value.z}
            };
        else if constexpr (is_template_base_of<base_objects::ignored, Type>) {
        } else if constexpr (is_template_base_of<std::optional, Type>) {
            if (value) {
                ::enbt::value in;
                serialize_entry(in, *value);
                res = ::enbt::optional(std::move(in));
            } else
                res = ::enbt::optional();
        } else if constexpr (is_template_base_of<base_objects::enum_as, Type>) {
            res = reflect::get_enum_value(value.value);
        } else if constexpr (is_template_base_of<base_objects::enum_as_flag, Type>) {
            res = reflect::get_enum_flag_value(value.value);
        } else if constexpr (is_template_base_of<base_objects::or_, Type>) {
            std::visit(
                [&](auto& it) {
                    ::enbt::value tmp;
                    serialize_entry(tmp, it);
                    if constexpr (std::is_same_v<typename Type::var_0, std::decay_t<decltype(it)>>)
                        res = ::enbt::compound{{"var_0", std::move(tmp)}};
                    else
                        res = ::enbt::compound{{"var_1", std::move(tmp)}};
                },
                value
            );
        } else if constexpr (is_template_base_of<base_objects::enum_switch, Type>) {
            std::visit(
                [&](auto& it) {
                    using it_T = std::decay_t<decltype(it)>;
                    ::enbt::compound comp;
                    serialize_entry(comp["type"], typename Type::encode_type(it_T::item_id::value));
                    serialize_entry(comp["data"], it);
                    res = std::move(comp);
                },
                value
            );
        } else if constexpr (is_template_base_of<base_objects::box, Type>) {
            serialize_entry(res, *value);
        } else if constexpr (is_template_base_of<base_objects::any_of, Type>) {
            serialize_entry(res, value.value);
        } else if constexpr (is_template_base_of<base_objects::flags_list, Type>) {
            res = ::enbt::compound{{"flag", {}}, {"items", ::enbt::dynamic_array{}}};
            serialize_entry(res["flag"], value.flag);
            auto& items = res["items"];
            value.for_each_in_order([&](auto& it) {
                items.push({});
                serialize_entry(items.back(), it);
            });
        } else if constexpr (is_flags_list_from<Type>) {
            res = ::enbt::dynamic_array{};
            value.for_each_in_order([&](auto& it) {
                res.push({});
                serialize_entry(res.back(), it);
            });
        } else if constexpr (is_tvalue_template_base_of<base_objects::ordered_id, Type>) {
            serialize_entry(res, value.value);
        } else if constexpr (is_template_base_of<base_objects::value_optional, Type>) {
            if (value.rest && value.v){
                res = ::enbt::optional{
                    ::enbt::dynamic_array{::enbt::value{}, ::enbt::value{}}
                };
                serialize_entry(res.get_optional()->at(0), value.v);
                serialize_entry(res.get_optional()->at(1), *value.rest);
            }
            else res = ::enbt::optional{};
        } else if constexpr (is_template_base_of<base_objects::sized_entry, Type>) {
            serialize_entry(res, value.value);
            res = ::enbt::to_log_item(std::move(res));
        } else if constexpr (is_limited_num<Type>) {
            serialize_entry(res, value.value);
        } else if constexpr (is_convertible_to_packet_form<Type>) {
            serialize_entry(res, value.to_packet());
        } else if constexpr (api::id::is_source<Type>) {
            serialize_entry(res, value.to_string());
        } else {
            res = ::enbt::compound{};
            bool process_next = true;
            reflect::for_each_field_with_name(value, [&res, &process_next](auto& item, auto& name) {
                if (process_next)
                    serialize_entry(res[name], item);
                if constexpr (is_template_base_of<base_objects::depends_next, std::decay_t<decltype(item)>>)
                    process_next = (bool)item.value;
            });
        }
    }

    template <class T>
    void serialize_entry(::enbt::io_helper::value_write_stream& res, const T& value) {
        using Type = std::decay_t<T>;
        if constexpr (
            std::is_arithmetic_v<Type>
            || std::is_same_v<std::string, Type>
            || std::is_same_v<::enbt::raw_uuid, Type>
            || std::is_same_v<::enbt::value, Type>
            || std::is_same_v<::enbt::compound, Type>
            || std::is_same_v<::enbt::dynamic_array, Type>
            || std::is_same_v<::enbt::fixed_array, Type>
            || std::is_same_v<::enbt::uuid, Type>
            || std::is_same_v<::enbt::simple_array_i8, Type>
            || std::is_same_v<::enbt::simple_array_i16, Type>
            || std::is_same_v<::enbt::simple_array_i32, Type>
            || std::is_same_v<::enbt::simple_array_i64, Type>
            || std::is_same_v<::enbt::simple_array_ui8, Type>
            || std::is_same_v<::enbt::simple_array_ui16, Type>
            || std::is_same_v<::enbt::simple_array_ui32, Type>
            || std::is_same_v<::enbt::simple_array_ui64, Type>
        ) {
            res.write(value);
        } else if constexpr (is_std_array<Type> || is_template_base_of<_list_array_impl::list_array, Type>) {
            res.write_array(value.size()).iterable(value, [](auto& item, auto& stream) {
                serialize_entry(stream, item);
            });
        } else if constexpr (
            std::is_same_v<base_objects::identifier, Type>
            || is_string_sized<Type>
            || std::is_same_v<base_objects::json_text_component, Type>
            || std::is_same_v<base_objects::var_int32, Type>
            || std::is_same_v<base_objects::var_int64, Type>
        )
            res.write(value.value);
        else if constexpr (std::is_same_v<Chat, Type>)
            res.write(value.ToENBT());
        else if constexpr (std::is_same_v<base_objects::optional_var_int32, Type> || std::is_same_v<base_objects::optional_var_int64, Type>) {
            auto opt = res.write_optional();
            if (value)
                opt.write([&value](auto& stream) {
                    serialize_entry(stream, *value);
                });
        } else if constexpr (std::is_same_v<base_objects::position, Type>)
            res.write_compound(3).write("x", value.x).write("y", value.y).write("z", value.z);
        else if constexpr (is_template_base_of<base_objects::ignored, Type>) {
            res.write(::enbt::value());
        } else if constexpr (is_template_base_of<std::optional, Type>) {
            auto opt = res.write_optional();
            if (value)
                opt.write([&value](auto& stream) {
                    serialize_entry(stream, *value);
                });
        } else if constexpr (is_template_base_of<base_objects::enum_as, Type>) {
            res.write(reflect::get_enum_value(value.value));
        } else if constexpr (is_template_base_of<base_objects::enum_as_flag, Type>) {
            res.write(reflect::get_enum_flag_value(value.value));
        } else if constexpr (is_template_base_of<base_objects::or_, Type>) {
            std::visit(
                [&res](auto& it) {
                    if constexpr (std::is_same_v<typename Type::var_0, std::decay_t<decltype(it)>>)
                        res.write_compound(1).write("var_0", [&it](auto& stream) {
                            serialize_entry(stream, it);
                        });
                    else
                        res.write_compound(1).write("var_1", [&it](auto& stream) {
                            serialize_entry(stream, it);
                        });
                },
                value
            );
        } else if constexpr (is_template_base_of<base_objects::enum_switch, Type>) {
            std::visit(
                [&](auto& it) {
                    using it_T = std::decay_t<decltype(it)>;
                    res
                        .write_compound(2)
                        .write("type", [&res](auto& stream) {
                            serialize_entry(stream, typename Type::encode_type(it_T::item_id::value));
                        })
                        .write("data", [&it](auto& stream) {
                            serialize_entry(stream, it);
                        });
                },
                value
            );
        } else if constexpr (is_template_base_of<base_objects::box, Type>) {
            serialize_entry(res, *value);
        } else if constexpr (is_template_base_of<base_objects::any_of, Type>) {
            serialize_entry(res, value.value);
        } else if constexpr (is_template_base_of<base_objects::flags_list, Type>) {
            res
                .write_compound(2)
                .write("flag", [&value](auto& stream) {
                    serialize_entry(stream, value.flag);
                })
                .write("items", [&value](auto& stream) {
                    auto arr_w = stream.write_array();
                    value.for_each_in_order([&arr_w](auto& it) {
                        arr_w.write([&it](auto& item_stream) {
                            serialize_entry(item_stream, it);
                        });
                    });
                });
        } else if constexpr (is_flags_list_from<Type>) {
            auto arr_w = res.write_array();
            value.for_each_in_order([&arr_w](auto& it) {
                arr_w.write([&it](auto& stream) {
                    serialize_entry(stream, it);
                });
            });
        } else if constexpr (is_tvalue_template_base_of<base_objects::ordered_id, Type>) {
            serialize_entry(res, value.value);
        } else if constexpr (is_template_base_of<base_objects::value_optional, Type>) {
            auto opt = res.write_optional();
            if (value.rest && value.v) {
                opt.write([&value](auto& stream) {
                    stream
                        .write_darray()
                        .write([&value](auto& a_stream) {
                            serialize_entry(a_stream, value.v);
                        })
                        .write([&value](auto& a_stream) {
                            serialize_entry(a_stream, *value.rest);
                        });
                });
            }
        } else if constexpr (is_template_base_of<base_objects::sized_entry, Type>) {
            res.write_log_item(value.value);
        } else if constexpr (is_limited_num<Type>) {
            serialize_entry(res, value.value);
        } else if constexpr (is_convertible_to_packet_form<Type>) {
            serialize_entry(res, value.to_packet());
        } else if constexpr (api::id::is_source<Type>) {
            serialize_entry(res, value.to_string());
        } else {
            bool process_next = true;
            auto comp = res.write_compound(reflect::fields_count<Type>());
            reflect::for_each_field_with_name(value, [&res, &process_next, &comp](auto& item, std::string_view name) {
                if (process_next)
                    comp.write(name, [&item](auto& stream) {
                        serialize_entry(stream, item);
                    });
                if constexpr (is_template_base_of<base_objects::depends_next, std::decay_t<decltype(item)>>)
                    process_next = (bool)item.value;
            });
        }
    }
}

#endif /* SRC_UTIL_ENCODING_ENBT_SERIALIZATION */
