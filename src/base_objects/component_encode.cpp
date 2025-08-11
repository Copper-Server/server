/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets.hpp> //for reflection //TODO change reflect_map to be more abstract;
#include <src/base_objects/component.hpp>
#include <src/base_objects/slot.hpp>
#include <src/util/calculations.hpp> //for reflection //TODO change reflect_map to be more abstract;
#include <src/util/reflect.hpp>

namespace copper_server::base_objects {

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

    template <class T>
    void serialize_entry(enbt::value& res, const T& value) {
        using Type = std::decay_t<T>;
        if constexpr (
            std::is_same_v<identifier, Type>
            || std::is_arithmetic_v<Type>
            || std::is_same_v<std::string, Type>
            || std::is_same_v<enbt::raw_uuid, Type>
            || std::is_same_v<enbt::value, Type>
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
            res = value;
        } else if constexpr (is_std_array<Type> || is_template_base_of<_list_array_impl::list_array, Type>) {
            enbt::fixed_array arr;
            arr.reserve(value.size());
            for (auto& it : value) {
                enbt::value in;
                serialize_entry(in, it);
                arr.push_back(in);
            }
            res = std::move(arr);
        } else if constexpr (
            is_string_sized<Type>
            || std::is_same_v<json_text_component, Type>
            || std::is_same_v<var_int32, Type>
            || std::is_same_v<var_int64, Type>
        )
            res = value.value;
        else if constexpr (std::is_same_v<Chat, Type>)
            res = value.ToENBT();
        else if constexpr (std::is_same_v<optional_var_int32, Type>) {
            if (value)
                res = enbt::optional(value);
            else
                res = enbt::optional();
        } else if constexpr (std::is_same_v<optional_var_int64, Type>) {
            if (value)
                res = enbt::optional(value);
            else
                res = enbt::optional();
        } else if constexpr (std::is_same_v<position, Type>)
            res = enbt::compound{
                {"x", value.x},
                {"y", value.y},
                {"z", value.z}
            };
        else if constexpr (is_template_base_of<ignored, Type>) {
        } else if constexpr (is_template_base_of<std::optional, Type>) {
            if (value) {
                enbt::value in;
                serialize_entry(in, *value);
                res = enbt::optional(std::move(in));
            } else
                res = enbt::optional();
        } else if constexpr (is_template_base_of<enum_as, Type>) {
            res = reflect::get_enum_value(value.value);
        } else if constexpr (is_template_base_of<enum_as_flag, Type>) {
            res = reflect::get_enum_flag_value(value.value);
        } else if constexpr (is_template_base_of<or_, Type>) {
            std::visit(
                [&](auto& it) {
                    enbt::value tmp;
                    serialize_entry(tmp, it);
                    if constexpr (std::is_same_v<typename Type::var_0, std::decay_t<decltype(it)>>)
                        res = enbt::compound{{"var_0", std::move(tmp)}};
                    else
                        res = enbt::compound{{"var_1", std::move(tmp)}};
                },
                value
            );
        } else if constexpr (is_template_base_of<enum_switch, Type>) {
            std::visit(
                [&](auto& it) {
                    using it_T = std::decay_t<decltype(it)>;
                    serialize_entry(res, typename Type::encode_type(it_T::item_id::value));
                    serialize_entry(res, it);
                },
                value
            );
        } else if constexpr (is_template_base_of<base_objects::box, Type>) {
            serialize_entry(res, *value);
        } else if constexpr (is_template_base_of<any_of, Type>) {
            serialize_entry(res, value.value);
        } else if constexpr (is_template_base_of<flags_list, Type>) {
            serialize_entry(res, value.flag);
            value.for_each_in_order([&](auto& it) {
                serialize_entry(res, it);
            });
        } else if constexpr (is_flags_list_from<Type>) {
            value.for_each_in_order([&](auto& it) {
                serialize_entry(res, it);
            });
            //} else if constexpr (is_template_base_of<unordered_id, Type>) {
            //    serialize_entry(res, value.value); //TODO
            //} else if constexpr (is_template_base_of<ordered_id, Type>) {
            //    serialize_entry(res, value.value); //TODO
        } else if constexpr (is_template_base_of<value_optional, Type>) {
            if (value.rest && value.v) {
                serialize_entry(res, value.v);
                serialize_entry(res, *value.rest);
            } else {
                decltype(value.v) tmp{0};
                serialize_entry(res, tmp);
            }
        } else if constexpr (is_template_base_of<sized_entry, Type>) {
            serialize_entry(res, value.value);
        } else if constexpr (is_limited_num<Type>) {
            serialize_entry(res, value.value);
        } else if constexpr (is_convertible_to_packet_form<Type>) {
            serialize_entry(res, value.to_packet());
        } else if constexpr (is_id_source<Type>) {
            serialize_entry(res, value.to_string());
        } else {
            bool process_next = true;
            reflect::for_each_field(value, [&res, &process_next](auto& item) {
                if (process_next)
                    serialize_entry(res, item);
                if constexpr (is_template_base_of<depends_next, std::decay_t<decltype(item)>>)
                    process_next = (bool)item.value;
            });
        }
    }

    std::pair<std::string, enbt::value> component::encode_component(const component& item) {
        return std::visit(
            [](auto& it) {
                using T = std::decay_t<decltype(it)>;
                enbt::value res;
                serialize_entry(res, it);
                std::string nam;
                if constexpr (requires { T::actual_name::value; })
                    nam = "minecraft:" + std::string(T::actual_name::value);
                else
                    nam = "minecraft:" + std::string(reflect::get_pretty_type_name<T>());


                return std::pair<std::string, enbt::value>{
                    std::move(nam),
                    std::move(res)
                };
            },
            item.type
        );
    }
}