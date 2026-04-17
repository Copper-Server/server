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
    template <class T>
    consteval bool has_flattened_fields() {
        bool res = false;
        if constexpr (reflect::fields_count<T>() > 0) {
            reflect::for_each_type<T>([&]<class FieldT>() {
                if constexpr (is_flattened_type_v<FieldT>)
                    res = true;
            });
        }
        return res;
    }

    template <class T, class T_prev>
    void deserialize_entry(T& res, util::nbt_read_stream& stream, T_prev& prev);

    namespace detail {
        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<4>)
            requires std::is_arithmetic_v<Type>
                     || std::is_same_v<std::string, Type>
                     || std::is_same_v<std::string_view, Type>
                     || std::is_same_v<base_objects::uuid, Type>
                     || std::is_same_v<base_objects::uuid_hex, Type>
                     || std::is_same_v<base_objects::uuid_flat_hex, Type>
        {
            stream.read_as(res);
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<4>)
            requires is_template_base_of<base_objects::box, Type>
        {
            res.ptr = std::make_shared<typename Type::value_type>();
            deserialize_entry(*res, stream, prev);
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<4>)
            requires is_std_array<Type>
        {
            stream.iterate(
                [&res](auto size) { if (res.size() != size)
                    throw std::runtime_error("Size mismatch, detected for std::array"); },
                [&res, &prev](auto& item_stream) {
                    for (auto& item : res)
                        deserialize_entry(item, item_stream, prev);
                }
            );
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<4>)
            requires is_template_base_of<_list_array_impl::list_array, Type>
        {
            stream.iterate(
                [&res](auto size) { res.reserve(size); },
                [&res, &prev](auto& item_stream) {
                    res.emplace_back();
                    deserialize_entry(res.back(), item_stream, prev);
                }
            );
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<4>)
            requires is_template_base_of<std::unordered_map, Type> && std::is_same_v<typename Type::key_type, std::string>
        {
            stream.iterate(
                [&res, &prev](auto& key, auto& item_stream) {
                    deserialize_entry(res[key], item_stream, prev);
                }
            );
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<4>)
            requires is_template_base_of<std::unordered_map, Type> && is_map_compatible<Type>
        {
            stream.iterate(
                [&res, &prev](auto& key, auto& item_stream) {
                    deserialize_entry(res[key], item_stream, prev);
                }
            );
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<4>)
            requires std::is_same_v<api::packets::identifier, Type>
                     || is_string_sized<Type>
                     || std::is_same_v<api::packets::json_text_component, Type>
                     || std::is_same_v<api::packets::var_int32, Type>
                     || std::is_same_v<api::packets::var_int64, Type>
        {
            stream.read_as(res.value);
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<4>)
            requires std::is_same_v<base_objects::velocity, Type>
        {
            stream.iterate([&res](auto& name, auto& value) {
                if (name == "x")
                    value.read_into(res.x);
                else if (name == "y")
                    value.read_into(res.y);
                else if (name == "z")
                    value.read_into(res.z);
            });
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<4>)
            requires std::is_same_v<base_objects::chat, Type>
        {
            nbt_convert ss;
            stream.read_into(ss);
            res = base_objects::chat::from_nbt(ss.get_as_nbt());
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<4>)
            requires std::is_same_v<util::nbt, Type>
        {
            nbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_nbt();
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<4>)
            requires std::is_same_v<util::nbt_convert, Type>
        {
            stream.read_into(res);
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<4>)
            requires std::is_same_v<util::nbt_compound, Type>
        {
            nbt_convert ss;
            stream.read_into(ss);
            res = ss.get_as_nbt();
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<4>)
            requires std::is_same_v<api::packets::optional_var_int32, Type>
        {
            int32_t res_value = 0;
            stream.read_into(res_value);
            if (res_value != 0)
                res = res_value - 1;
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<4>)
            requires std::is_same_v<api::packets::optional_var_int64, Type>
        {
            int64_t res_value = 0;
            stream.read_into(res_value);
            if (res_value != 0)
                res = res_value - 1;
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<4>)
            requires std::is_same_v<base_objects::position, Type>
        {
            stream.iterate([&res](std::string_view name, auto& value) {
                int64_t temp = 0;
                if (name == "x") {
                    value.read_as(temp);
                    res.x = temp;
                } else if (name == "y") {
                    value.read_as(temp);
                    res.x = temp;
                } else if (name == "z") {
                    value.read_as(temp);
                    res.x = temp;
                }
            });
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<4>)
            requires is_template_base_of<api::packets::ignored, Type>
        {
            stream.skip();
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<4>)
            requires is_template_base_of<std::optional, Type>
        {
            stream.read_compound()
                .collect("opt", [&](util::nbt_read_stream& opt_stream) {
                    res.emplace();
                    deserialize_entry(*res, opt_stream, prev);
                })
                .make_collect();
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<4>)
            requires is_template_base_of<api::packets::enum_as, Type>
        {
            std::string str;
            stream.read_into(str);
            res.value = reflect::get_enum_value<typename Type::enum_t>(str);
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<4>)
            requires is_template_base_of<api::packets::enum_as_flag, Type>
        {
            std::string str;
            stream.read_into(str);
            res.value = reflect::get_enum_flag_value<typename Type::enum_t>(str);
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<3>)
            requires is_template_base_of<api::packets::or_, Type> || is_template_base_of<api::packets::bool_or, Type>
        {
            if constexpr (get_nbt_type<typename Type::var_0>() != get_nbt_type<typename Type::var_1>()) {
                if (get_nbt_type<typename Type::var_0>() == stream.get_type()) {
                    typename Type::var_0 it{};
                    deserialize_entry(it, stream, prev);
                    res = std::move(it);
                } else {
                    typename Type::var_1 it{};
                    deserialize_entry(it, stream, prev);
                    res = std::move(it);
                }
            } else {
                nbt_convert converter;
                stream.read_into(converter);
                auto buffered_data = converter.get_as_normal();

                auto try_decode = [&]<class VariantT>() -> std::optional<VariantT> {
                    try {
                        std::stringstream ss(std::string((const char*)buffered_data.data(), buffered_data.size()));
                        util::nbt_read_stream temp_stream(ss);

                        VariantT val{};
                        deserialize_entry(val, temp_stream, prev);
                        return val;
                    } catch (...) {
                        return std::nullopt;
                    }
                };

                if (auto v0 = try_decode.template operator()<typename Type::var_0>())
                    res = std::move(*v0);
                else if (auto v1 = try_decode.template operator()<typename Type::var_1>())
                    res = std::move(*v1);
                else
                    throw std::runtime_error("Failed to decode either variant of field");
            }
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<3>)
            requires is_template_base_of<api::packets::enum_switch, Type>
        {
            if constexpr (enum_switch_is_inline_eligible<Type>()) {
                Type::for_each([&res, &prev, &stream]<class Ty>() {
                    if (get_nbt_type<Ty>() == stream.get_type()) {
                        Ty it{};
                        deserialize_entry(it, stream, prev);
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
                    [&res, &prev, &type](util::nbt_read_stream& this_stream) {
                        Type::for_each([&this_stream, &res, &prev, &type]<class Ty>() {
                            if (type == reflect::get_pretty_type_name<Ty>()) {
                                this_stream.iterate([&res, &prev](auto& name, auto& item_stream) {
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
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<3>)
            requires is_template_base_of<api::packets::any_of, Type> || is_ordered_id<Type> || is_template_base_of<api::packets::sized_entry, Type>
        {
            deserialize_entry(res.value, stream, prev);
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<3>)
            requires is_template_base_of<api::packets::flags_list, Type>
        {
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
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<3>)
            requires is_flags_list_from<Type>
        {
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
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<3>)
            requires is_template_base_of<api::packets::value_optional, Type>
        {
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
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<3>)
            requires is_limited_num<Type>
        {
            stream.read_into(res.value);
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<3>)
            requires is_convertible_to_nbt_form<Type>
        {
            res = Type::from_nbt(stream);
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<3>)
            requires api::packets::is_convertible_to_packet_form<Type>
        {
            api::packets::convertible_to_packet_type<Type> tmp{};
            deserialize_entry(tmp, stream, prev);
            res = Type::from_packet(std::move(tmp));
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto&, priority_tag<3>)
            requires api::id::is_source<Type>
        {
            std::string tmp;
            stream.read_into(tmp);
            res = Type(tmp);
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<3>)
            requires nbt_is_inline<Type> && (reflect::fields_count<Type> == 1)
        {
            reflect::visit_field<Type>(0, [&]<class T>() {
                deserialize_entry(res, stream, prev);
            });
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<2>)
            requires(has_flattened_fields<Type>())
        {
            if (stream.get_type() != nbt_type::tag_compound)
                throw std::runtime_error("Expected Compound for struct");

            nbt_convert converter;
            stream.read_into(converter);
            auto buffer = converter.get_as_normal();

            reflect::for_each_field_with_name(res, [&](auto& field, std::string_view name) {
                using FieldType = std::decay_t<decltype(field)>;
                if constexpr (is_flattened_type_v<FieldType>) {
                    std::stringstream ss(std::string((char*)buffer.data(), buffer.size()));
                    util::nbt_read_stream replay_stream(ss);
                    deserialize_entry(field, replay_stream, prev);
                }
            });

            std::stringstream ss(std::string((char*)buffer.data(), buffer.size()));
            util::nbt_read_stream normal_stream(ss);

            normal_stream.iterate([&res, &prev](std::string_view name, util::nbt_read_stream& item_stream) {
                reflect::visit_field(name, res, [&](auto& field) {
                    using FieldType = std::decay_t<decltype(field)>;
                    if constexpr (!is_flattened_type_v<FieldType>)
                        deserialize_entry(field, item_stream, prev);
                    else
                        item_stream.skip();
                });
            });
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<1>)
            requires(reflect::fields_count<Type>() == 1)
        {
            if (stream.get_type() == nbt_type::tag_compound) {
                bool has_value = false;
                stream.iterate([&res, &prev, &has_value](auto& name, auto& item_stream) {
                    reflect::visit_field(name, res, [&item_stream, &prev](auto& res_v) {
                        deserialize_entry(res_v, item_stream, prev);
                    });
                    has_value = true;
                });
                if (!has_value)
                    throw std::runtime_error("Invalid encoding");
            } else {
                reflect::visit_field<Type>(0, [&res, &stream, &prev]<class T>() {
                    deserialize_entry(res, stream, prev);
                });
            }
        }

        template <class Type>
        void deserialize_impl(Type& res, util::nbt_read_stream& stream, auto& prev, priority_tag<0>) {
            if (stream.get_type() == nbt_type::tag_compound) {
                bool has_value = false;
                stream.iterate([&res, &prev, &has_value](auto& name, auto& item_stream) {
                    reflect::visit_field(name, res, [&item_stream, &prev](auto& res_v) {
                        deserialize_entry(res_v, item_stream, prev);
                    });
                    has_value = true;
                });
                if (!has_value) {
                    if constexpr (reflect::fields_count<Type>() != 0)
                        throw std::runtime_error("Invalid encoding");
                }
            } else {
                reflect::visit_field<Type>(0, [&res, &stream, &prev]<class T>() {
                    deserialize_entry(res, stream, prev);
                });
            }
        }
    }

    template <class T, class T_prev>
    void deserialize_entry(T& res, util::nbt_read_stream& stream, T_prev& prev) {
        detail::deserialize_impl(res, stream, prev, priority_tag<4>{});
    }

    template <class T>
    void deserialize_entry(T& res, util::nbt_read_stream& stream) {
        deserialize_entry(res, stream, res);
    }
}

#endif /* SRC_UTIL_ENCODING_NBT_DESERIALIZATION */
