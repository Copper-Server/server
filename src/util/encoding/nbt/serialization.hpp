/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTIL_ENCODING_NBT_SERIALIZATION
#define SRC_UTIL_ENCODING_NBT_SERIALIZATION
#include <src/util/encoding/common.hpp>
#include <src/util/nbt_stream.hpp>

//include <src/util/reflect/*.hpp> headers to use reflection for serialization
namespace copper_server::util::encoding::nbt {

    template <class T>
    void serialize_entry(util::nbt_write_stream& res, const T& value) {
        using Type = std::decay_t<T>;
        if constexpr (
            std::is_arithmetic_v<Type>
            || std::is_same_v<std::string, Type>
            || std::is_same_v<std::string_view, Type>
            || std::is_same_v<base_objects::uuid, Type>
            || std::is_same_v<base_objects::uuid_hex, Type>
            || std::is_same_v<base_objects::uuid_flat_hex, Type>
        ) {
            res.write(value);
        } else if constexpr (std::is_same_v<::enbt::raw_uuid, Type>) {
            res.write((const int32_t*)value.data, 4);
        } else if constexpr (std::is_same_v<::enbt::value, Type>
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
                             || std::is_same_v<::enbt::simple_array_ui64, Type>) {
            res.write(nbt_enbt_convert::build((::enbt::value)value), false);
        } else if constexpr (is_std_array<Type> || is_template_base_of<_list_array_impl::list_array, Type>) {
            res.write_list().iterable(value, [](auto& item, auto& stream) {
                serialize_entry(stream, item);
            });
        } else if constexpr (
            std::is_same_v<api::packets::identifier, Type>
            || is_string_sized<Type>
            || std::is_same_v<api::packets::json_text_component, Type>
            || std::is_same_v<api::packets::var_int32, Type>
            || std::is_same_v<api::packets::var_int64, Type>
        )
            res.write(value.value);
        else if constexpr (std::is_same_v<base_objects::velocity, Type>)
            res.write_compound()
                .write("x", value.x)
                .write("y", value.y)
                .write("z", value.z);
        else if constexpr (std::is_same_v<base_objects::chat, Type>)
            res.write(nbt_enbt_convert::build(value.to_enbt()), false);
        else if constexpr (std::is_same_v<api::packets::optional_var_int32, Type>) {
            int32_t res_value = 0;
            if (value) {
                int64_t tmp = *value;
                tmp += 1;
                if (tmp > INT32_MAX)
                    throw std::out_of_range("Value out of range");
                res_value = (int32_t)tmp;
            }
            opt.write(res_value);
        } else if constexpr (std::is_same_v<api::packets::optional_var_int64, Type>) {
            int64_t res_value = 0;
            if (value) {
                res_value = *value;
                res_value += 1;
                if (res_value <= *value)
                    throw std::out_of_range("Value out of range");
            }
            opt.write(res_value);
        } else if constexpr (std::is_same_v<base_objects::position, Type>)
            res.write_compound()
                .write("x", value.x)
                .write("y", value.y)
                .write("z", value.z);
        else if constexpr (is_template_base_of<api::packets::ignored, Type>) {
            res.write_compound();
        } else if constexpr (is_template_base_of<std::optional, Type>) {
            auto comp = res.write_compound();
            comp.write("opt", [&value](auto& stream) {
                serialize_entry(stream, *value);
            });
        } else if constexpr (is_template_base_of<api::packets::enum_as, Type>) {
            res.write(reflect::get_enum_value(value.value));
        } else if constexpr (is_template_base_of<api::packets::enum_as_flag, Type>) {
            res.write(reflect::get_enum_flag_value(value.value));
        } else if constexpr (is_template_base_of<api::packets::or_, Type>) {
            std::visit(
                [&res](auto& it) {
                    if constexpr (std::is_same_v<typename Type::var_0, std::decay_t<decltype(it)>>)
                        res.write_compound().write("var_0", [&it](auto& stream) {
                            serialize_entry(stream, it);
                        });
                    else
                        res.write_compound().write("var_1", [&it](auto& stream) {
                            serialize_entry(stream, it);
                        });
                },
                value
            );
        } else if constexpr (is_template_base_of<api::packets::enum_switch, Type>) {
            std::visit(
                [&](auto& it) {
                    using it_T = std::decay_t<decltype(it)>;
                    res
                        .write_compound()
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
        } else if constexpr (is_template_base_of<api::packets::any_of, Type>) {
            serialize_entry(res, value.value);
        } else if constexpr (is_template_base_of<api::packets::flags_list, Type>) {
            res
                .write_compound()
                .write("flag", [&value](auto& stream) {
                    serialize_entry(stream, value.flag);
                })
                .write("items", [&value](auto& stream) {
                    auto arr_w = stream.write_list();
                    value.for_each_in_order([&arr_w](auto& it) {
                        arr_w.write([&it](auto& item_stream) {
                            item_stream.write_compound().write("", [&it](auto& item_stream) {
                                serialize_entry(item_stream, it);
                            });
                        });
                    });
                });
        } else if constexpr (is_flags_list_from<Type>) {
            auto arr_w = res.write_list();
            value.for_each_in_order([&arr_w](auto& it) {
                arr_w.write([&it](auto& stream) {
                    stream.write_compound().write("", [&it](auto& item_stream) {
                        serialize_entry(item_stream, it);
                    });
                });
            });
        } else if constexpr (is_ordered_id<Type>) {
            serialize_entry(res, value.value);
        } else if constexpr (is_template_base_of<api::packets::value_optional, Type>) {
            auto opt = res.write_compound();
            if (value.rest && value.v) {
                opt.write("opt", [&value](auto& stream) {
                    stream
                        .write_list()
                        .write([&value](auto& a_stream) {
                            a_stream.write_compound().write("", [&value](auto& ac_stream) {
                                serialize_entry(ac_stream, value.v);
                            });
                        })
                        .write([&value](auto& a_stream) {
                            a_stream.write_compound().write("", [&value](auto& ac_stream) {
                                serialize_entry(a_stream, *value.rest);
                            });
                        });
                });
            }
        } else if constexpr (is_template_base_of<api::packets::sized_entry, Type>) {
            serialize_entry(res, value.value);
        } else if constexpr (is_limited_num<Type>) {
            serialize_entry(res, value.value);
        } else if constexpr (is_convertible_to_nbt_form<Type>) {
            value.to_nbt(res);
        } else if constexpr (api::packets::is_convertible_to_packet_form<Type>) {
            serialize_entry(res, value.to_packet());
        } else if constexpr (api::id::is_source<Type>) {
            serialize_entry(res, value.to_string());
        } else if constexpr (is_template_base_of<base_objects::pool, Type>) {
            auto lis = res.write_list();
            value.iterate([&lis](int32_t weight, auto& val) {
                lis.write([&](util::nbt_write_stream& lis_stream) {
                    auto comp = lis_stream.write_compound();
                    comp.write("weight", weight);
                    comp.write("data", [](util::nbt_write_stream& comp_stream) {
                        serialize_entry(comp_stream, val);
                    });
                });
            });
        } else {
            bool process_next = true;
            auto comp = res.write_compound();
            reflect::for_each_field_with_name(value, [&res, &process_next, &comp](auto& item, std::string_view name) {
                if (process_next) {
                    if constexpr (is_template_base_of<std::optional, Type>)
                        if (!item)
                            return;
                    comp.write(name, [&item](auto& stream) {
                        serialize_entry(stream, item);
                    });
                }
                if constexpr (is_template_base_of<api::packets::depends_next, std::decay_t<decltype(item)>>)
                    process_next = (bool)item.value;
            });
        }
    }
}

#endif /* SRC_UTIL_ENCODING_NBT_SERIALIZATION */
