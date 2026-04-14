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
#include <src/util/encoding/nbt/nbt_common.hpp>

//include <src/util/reflect/*.hpp> headers to use reflection for serialization
namespace copper_server::util::encoding::nbt {

    template <class T>
    void serialize_flattened(util::nbt_write_compound_stream& comp, const T& value);

    template <class T>
    void serialize_entry(util::nbt_write_stream& res, const T& value);

    namespace detail {
        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires std::is_arithmetic_v<Type>
                     || std::is_same_v<std::string, Type>
                     || std::is_same_v<std::string_view, Type>
                     || std::is_same_v<base_objects::uuid, Type>
                     || std::is_same_v<base_objects::uuid_hex, Type>
                     || std::is_same_v<base_objects::uuid_flat_hex, Type>
        {
            res.write(value);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires is_template_base_of<base_objects::box, Type>
        {
            serialize_entry(res, *value);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires is_template_base_of<api::packets::any_of, Type>
        {
            serialize_entry(res, value.value);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires is_std_array<Type> || is_template_base_of<_list_array_impl::list_array, Type>
        {
            res.write_list().iterable(value, [](auto& item, auto& stream) {
                serialize_entry(stream, item);
            });
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires is_template_base_of<std::unordered_map, Type> && std::is_same_v<typename Type::key_type, std::string>
        {
            auto compound = res.write_compound();
            for (auto& [key, it] : value)
                compound.write(key, [&it](auto& stream) {
                    serialize_entry(stream, it);
                });
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires is_template_base_of<std::unordered_map, Type> && is_map_compatible<Type>
        {
            auto compound = res.write_compound();
            for (auto& [key, it] : value)
                compound.write(key.to_string(), [&it](auto& stream) {
                    serialize_entry(stream, it);
                });
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires std::is_same_v<api::packets::identifier, Type>
                     || is_string_sized<Type>
                     || std::is_same_v<api::packets::json_text_component, Type>
                     || std::is_same_v<api::packets::var_int32, Type>
                     || std::is_same_v<api::packets::var_int64, Type>
        {
            res.write(value.value);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires std::is_same_v<base_objects::velocity, Type>
        {
            res.write_compound()
                .write("x", value.x)
                .write("y", value.y)
                .write("z", value.z);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires std::is_same_v<base_objects::chat, Type>
        {
            res.write(value.to_nbt());
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires std::is_same_v<api::packets::optional_var_int32, Type>
        {
            int32_t res_value = 0;
            if (value) {
                int64_t tmp = *value;
                tmp += 1;
                if (tmp > INT32_MAX)
                    throw std::out_of_range("Value out of range");
                res_value = static_cast<int32_t>(tmp);
            }
            res.write(res_value);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires std::is_same_v<api::packets::optional_var_int64, Type>
        {
            int64_t res_value = 0;
            if (value) {
                res_value = *value;
                res_value += 1;
                if (res_value <= *value)
                    throw std::out_of_range("Value out of range");
            }
            res.write(res_value);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires std::is_same_v<base_objects::position, Type>
        {
            res.write_compound()
                .write("x", value.x)
                .write("y", value.y)
                .write("z", value.z);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires is_template_base_of<api::packets::ignored, Type>
        {
            res.write_compound();
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires is_template_base_of<std::optional, Type>
        {
            auto comp = res.write_compound();
            comp.write("opt", [&value](auto& stream) {
                serialize_entry(stream, *value);
            });
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires is_template_base_of<api::packets::enum_as, Type>
        {
            res.write(reflect::get_enum_value(value.value));
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires is_template_base_of<api::packets::enum_as_flag, Type>
        {
            res.write(reflect::get_enum_flag_value(value.value));
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<2>)
            requires is_template_base_of<api::packets::or_, Type> || is_template_base_of<api::packets::bool_or, Type>
        {
            std::visit(
                [&res](auto& it) {
                    serialize_entry(res, it);
                },
                value
            );
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires is_template_base_of<api::packets::or_, Type> || is_template_base_of<api::packets::bool_or, Type>
        {
            std::visit(
                [&](auto& it) {
                    using it_T = std::decay_t<decltype(it)>;
                    if constexpr (nbt_is_inline<it_T> && reflect::fields_count<it_T>() == 1 && enum_switch_is_inline_eligible<Type>()) {
                        reflect::visit_field<Type>(0, [&]<class T>() {
                            serialize_entry(res, it);
                        });
                    } else {
                        auto comp = res.write_compound();
                        comp.write("type", reflect::get_pretty_type_name<it_T>());
                        bool process_next = true;
                        reflect::for_each_field_with_name(it, [&process_next, &comp](auto& item, std::string_view name) {
                            if (process_next) {
                                if constexpr (is_template_base_of<std::optional, Type>) {
                                    if (!item){
                                        return;
                                    } else{
                                        comp.write(name, [&item](auto& stream) {
                                            serialize_entry(stream, *item);
                                        });
                                    }
                                } else if constexpr (is_template_base_of<api::packets::ignored, Type>) {
                                    return;
                                } else{
                                    comp.write(name, [&item](auto& stream) {
                                        serialize_entry(stream, item);
                                    });
                                }
                            }
                            
                            if constexpr (is_template_base_of<api::packets::depends_next, std::decay_t<decltype(item)>>)
                                process_next = (bool)item.value;
                        });
                    }
                },
                value
            );
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires is_template_base_of<api::packets::flags_list, Type>
        {
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
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires is_flags_list_from<Type>
        {
            auto arr_w = res.write_list();
            value.for_each_in_order([&arr_w](auto& it) {
                arr_w.write([&it](auto& stream) {
                    stream.write_compound().write("", [&it](auto& item_stream) {
                        serialize_entry(item_stream, it);
                    });
                });
            });
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires std::is_same_v<util::nbt, std::decay_t<Type>>
        {
            res.write(value);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires std::is_same_v<util::nbt_convert, std::decay_t<Type>>
        {
            res.write(value);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires std::is_same_v<util::nbt_compound, std::decay_t<Type>>
        {
            res.write(value);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires is_ordered_id<Type>
        {
            serialize_entry(res, value.value);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires is_template_base_of<api::packets::value_optional, Type>
        {
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
                                serialize_entry(ac_stream, *value.rest);
                            });
                        });
                });
            }
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires is_template_base_of<api::packets::sized_entry, Type>
        {
            serialize_entry(res, value.value);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires is_limited_num<Type>
        {
            serialize_entry(res, value.value);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires is_convertible_to_nbt_form<Type>
        {
            value.to_nbt(res);
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires api::packets::is_convertible_to_packet_form<Type>
        {
            serialize_entry(res, value.to_packet());
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires api::id::is_source<Type>
        {
            serialize_entry(res, value.to_string());
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires is_template_base_of<base_objects::pool, Type>
        {
            auto lis = res.write_list();
            value.iterate([&lis](int32_t weight, auto& val) {
                lis.write([&](util::nbt_write_stream& lis_stream) {
                    auto comp = lis_stream.write_compound();
                    comp.write("weight", weight);
                    comp.write("data", [&val](util::nbt_write_stream& comp_stream) {
                        serialize_entry(comp_stream, val);
                    });
                });
            });
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<1>)
            requires nbt_is_inline<Type> && (reflect::fields_count<Type> == 1)
        {
            reflect::visit_field<Type>(0, [&]<class T>() {
                serialize_entry(res, value);
            });
        }

        template <class Type>
        void serialize_impl(util::nbt_write_stream& res, const Type& value, priority_tag<0>) {
            bool process_next = true;
            auto comp = res.write_compound();
            reflect::for_each_field_with_name(value, [&process_next, &comp](auto& item, std::string_view name) {
                using ItemType = std::decay_t<decltype(item)>;
                if (process_next) {
                    if constexpr (is_template_base_of<std::optional, Type>) {
                        if (!item)
                            return;
                        else
                            comp.write(name, [&item](auto& stream) {
                                serialize_entry(stream, *item);
                            });
                    } else if constexpr (is_template_base_of<api::packets::ignored, Type>)
                        return;
                    else if constexpr (is_flattened_type_v<ItemType>) {
                        serialize_flattened(comp, item);
                    } else
                        comp.write(name, [&item](auto& stream) {
                            serialize_entry(stream, item);
                        });
                }
                if constexpr (is_template_base_of<api::packets::depends_next, std::decay_t<decltype(item)>>)
                    process_next = (bool)item.value;
            });
        }
    }

    template <class T>
    void serialize_entry(util::nbt_write_stream& res, const T& value) {
        detail::serialize_impl(res, value, priority_tag<2>{});
    }

    template <class T>
    void serialize_flattened(util::nbt_write_compound_stream& comp, const T& value) {
        using Type = std::decay_t<T>;

        if constexpr (is_template_base_of<api::packets::or_, Type> || is_template_base_of<api::packets::bool_or, Type>) {
            std::visit([&comp](auto& it) { serialize_flattened(comp, it); }, value);
        } else {
            reflect::for_each_field_with_name(value, [&comp](auto& item, std::string_view name) {
                using ItemType = std::decay_t<decltype(item)>;

                if constexpr (is_template_base_of<std::optional, ItemType>) {
                    if (item)
                        comp.write(name, [&](auto& s) { serialize_entry(s, *item); });
                } else if constexpr (is_flattened_type_v<ItemType>)
                    serialize_flattened(comp, item);
                else
                    comp.write(name, [&](auto& s) { serialize_entry(s, item); });
            });
        }
    }
}

#endif /* SRC_UTIL_ENCODING_NBT_SERIALIZATION */
