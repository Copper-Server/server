/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <library/enbt/senbt.hpp>
#include <src/api/packets.hpp>
#include <src/base_objects/slot.hpp>
#include <src/util/reflect.hpp>
#include <src/util/reflect/calculations.hpp>
#include <src/util/reflect/component.hpp>
#include <src/util/reflect/dye_color.hpp>
#include <src/util/reflect/packets.hpp>
#include <src/util/reflect/packets_help.hpp>
#include <src/util/reflect/parsers.hpp>

namespace copper_server::api::packets {
    using namespace base_objects;
    template <class type>
    concept is_packet = requires(type& d) {
        type::packet_id::value;
    };


    template <template <auto...> class Base, auto... Ts>
    void value_test(Base<Ts...>&);

    template <template <class, auto...> class Base, class T, auto... Ts>
    void tvalue_test(Base<T, Ts...>&);


    template <template <auto...> class, class, class = void>
    constexpr bool is_value_template_base_of = false;

    template <template <auto...> class Base, class Derived>
    constexpr bool is_value_template_base_of<Base, Derived, std::void_t<decltype(value_test<Base>(std::declval<Derived&>()))>> = true;


    template <template <class, auto...> class, class, class = void>
    constexpr bool is_tvalue_template_base_of = false;

    template <template <class, auto...> class Base, class Derived>
    constexpr bool is_tvalue_template_base_of<Base, Derived, std::void_t<decltype(tvalue_test<Base>(std::declval<Derived&>()))>> = true;

    // Helper to detect instantiations of flags_list_from
    template <class T>
    struct is_flags_list_from_helper : std::false_type {};

    template <class S, class ST, ST S::* M, class... Ts>
    struct is_flags_list_from_helper<flags_list_from<S, ST, M, Ts...>> : std::true_type {};

    template <class type>
    concept is_flags_list_from = is_flags_list_from_helper<std::decay_t<type>>::value;

    template <class type>
    concept is_string_sized = is_value_template_base_of<string_sized, type>;
    template <class type>
    concept is_bitset_fixed = is_value_template_base_of<bitset_fixed, type>;

    template <class type>
    concept is_list_array_sized = is_tvalue_template_base_of<list_array_sized, type> || is_tvalue_template_base_of<list_array_sized_siz_from_packet, type> || is_tvalue_template_base_of<list_array_sized_no_size, type>;

    template <class type>
    concept is_list_array_fixed = is_tvalue_template_base_of<list_array_fixed, type>;

    template <class type>
    concept is_std_array = is_tvalue_template_base_of<std::array, type>;

    template <class type>
    concept is_limited_num = is_tvalue_template_base_of<limited_num, type>;

    template <class type>
    concept requires_check = is_limited_num<type> || is_string_sized<type> || is_list_array_sized<type> || is_list_array_fixed<type> || is_value_template_base_of<no_size, type>;

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

        template <class T>
        void serialize_entry(std::string& res, size_t spacing, const T& value) {
            using Type = std::decay_t<T>;
            if constexpr (is_convertible_to_packet_form<Type>) {
                serialize_entry(res, spacing, value.to_packet());
            } else if constexpr (std::is_same_v<identifier, Type>)
                res += "\"" + value.value + "\"";
            else if constexpr (is_string_sized<Type>)
                res += "\"" + value.value + "\"";
            else if constexpr (std::is_same_v<json_text_component, Type>)
                res += "\"" + value.value + "\"";
            else if constexpr (std::is_same_v<var_int32, Type>)
                res += std::to_string(value.value);
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
            } else if constexpr (std::is_same_v<position, Type>)
                res += "{.x = " + std::to_string(value.x) + ".z = " + std::to_string(value.z) + ".y = " + std::to_string(value.y) + "}";
            else if constexpr (std::is_same_v<bool, Type>)
                res += value ? "true" : "false";
            else if constexpr (std::is_arithmetic_v<Type>)
                res += std::to_string(value);
            else if constexpr (std::is_same_v<std::string, Type>)
                res += "\"" + value + "\"";
            else if constexpr (std::is_same_v<enbt::raw_uuid, Type>)
                res += "\"" + value.to_string() + "\"";
            else if constexpr (std::is_same_v<Chat, Type>) {
                std::string alignment(spacing + 1, ' ');
                alignment[0] = '\n';
                res += list_array<char>(senbt::serialize(value.ToENBT(), false, true))
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
            } else if constexpr (std::is_base_of_v<base_objects::pallete_container, Type>) {
                res += "pallete_data";
            } else if constexpr (std::is_same_v<base_objects::pallete_data_height_map, Type>) {
                res += "pallete_data";
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
                || is_id_source<Type>
            )
                serialize_entry(res, spacing, value.value);
            else if constexpr (is_bitset_fixed<Type>) {
                serialize_array(res, spacing, value.value.data());
            } else if constexpr (std::is_same_v<bit_list_array<uint64_t>, Type>) {
                serialize_array(res, spacing, value.data());
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

        template <class T>
        void serialize_packet(std::string& res, size_t spacing, T& value) {
            using Type = std::decay_t<T>;
            if constexpr (is_packet<Type>) {
                serialize_entry(res, spacing, value);
            } else if constexpr (std::is_base_of_v<compound_packet, Type>) {
                res += std::string(reflect::get_pretty_type_name<Type>()) + "<compound> {";
                bool processed = false;
                reflect::for_each_field_with_name(value, [&res, spacing, &processed](auto& item, auto name) {
                    using I = std::decay_t<decltype(item)>;
                    if (processed)
                        res += ",";
                    res += "\n" + std::string(spacing + 4, ' ') + std::string(name) + ": ";
                    if constexpr (is_packet<I>) {
                        serialize_packet(res, spacing + 4, item);
                    } else if constexpr (
                        std::is_same_v<I, client_bound::status_packet>
                        || std::is_same_v<I, client_bound::login_packet>
                        || std::is_same_v<I, client_bound::configuration_packet>
                        || std::is_same_v<I, client_bound::play_packet>
                        || std::is_same_v<I, server_bound::handshake_packet>
                        || std::is_same_v<I, server_bound::status_packet>
                        || std::is_same_v<I, server_bound::login_packet>
                        || std::is_same_v<I, server_bound::configuration_packet>
                        || std::is_same_v<I, server_bound::play_packet>
                    ) {
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
            }
        }
    }

    std::string stringize_packet(const client_bound::status_packet& packet) {
        return std::visit(
            [](auto& it) -> std::string {
                std::string res;
                sp::serialize_packet(res, 0, it);
                return res;
            },
            packet
        );
    }

    std::string stringize_packet(const client_bound::login_packet& packet) {
        return std::visit(
            [](auto& it) -> std::string {
                std::string res;
                sp::serialize_packet(res, 0, it);
                return res;
            },
            packet
        );
    }

    std::string stringize_packet(const client_bound::configuration_packet& packet) {
        return std::visit(
            [](auto& it) -> std::string {
                std::string res;
                sp::serialize_packet(res, 0, it);
                return res;
            },
            packet
        );
    }

    std::string stringize_packet(const client_bound::play_packet& packet) {
        return std::visit(
            [](auto& it) -> std::string {
                std::string res;
                sp::serialize_packet(res, 0, it);
                return res;
            },
            packet
        );
    }

    std::string stringize_packet(const server_bound::handshake_packet& packet) {
        return std::visit(
            [](auto& it) -> std::string {
                std::string res;
                sp::serialize_packet(res, 0, it);
                return res;
            },
            packet
        );
    }

    std::string stringize_packet(const server_bound::status_packet& packet) {
        return std::visit(
            [](auto& it) -> std::string {
                std::string res;
                sp::serialize_packet(res, 0, it);
                return res;
            },
            packet
        );
    }

    std::string stringize_packet(const server_bound::login_packet& packet) {
        return std::visit(
            [](auto& it) -> std::string {
                std::string res;
                sp::serialize_packet(res, 0, it);
                return res;
            },
            packet
        );
    }

    std::string stringize_packet(const server_bound::configuration_packet& packet) {
        return std::visit(
            [](auto& it) -> std::string {
                std::string res;
                sp::serialize_packet(res, 0, it);
                return res;
            },
            packet
        );
    }

    std::string stringize_packet(const server_bound::play_packet& packet) {
        return std::visit(
            [](auto& it) -> std::string {
                std::string res;
                sp::serialize_packet(res, 0, it);
                return res;
            },
            packet
        );
    }

    std::string stringize_packet(const client_bound_packet& packet) {
        return std::visit([](auto& it) { return stringize_packet(it); }, packet);
    }

    std::string stringize_packet(const server_bound_packet& packet) {
        return std::visit([](auto& it) { return stringize_packet(it); }, packet);
    }
}