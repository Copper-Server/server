/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/network/tcp.hpp>
#include <src/api/packets.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/base_objects/slot.hpp>
#include <src/log.hpp>
#include <src/util/reflect.hpp>
#include <src/util/reflect/calculations.hpp>
#include <src/util/reflect/component.hpp>
#include <src/util/reflect/dye_color.hpp>
#include <src/util/reflect/packets.hpp>
#include <src/util/reflect/packets_help.hpp>
#include <src/util/reflect/parsers.hpp>
#include <tuple>

namespace copper_server::api::packets {
    using namespace base_objects;
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
    concept requires_check = is_limited_num<type> || is_string_sized<type> || is_list_array_sized<type> || is_list_array_fixed<type> || is_value_template_base_of<no_size, type> || is_bitset_fixed<type>;

    template <typename T, typename... Ts>
    struct contains_type : std::disjunction<std::is_same<T, Ts>...> {};

    template <typename T, typename... Ts>
    inline constexpr bool contains_type_v = contains_type<T, Ts...>::value;

    template <class T, class... VisitedTypes>
    consteval bool need_preprocess_result() {
        if constexpr (contains_type_v<T, VisitedTypes...>) {
            return false;
        } else if constexpr (requires_check<T> || is_tvalue_template_base_of<ordered_id, T>)
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
        } else if constexpr (std::is_same_v<var_int64, T>) {
        } else if constexpr (std::is_same_v<optional_var_int32, T>) {
        } else if constexpr (std::is_same_v<optional_var_int64, T>) {
        } else if constexpr (std::is_same_v<position, T>) {
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
            || std::is_same_v<T, client_bound::status_packet>
            || std::is_same_v<T, client_bound::login_packet>
            || std::is_same_v<T, client_bound::configuration_packet>
            || std::is_same_v<T, client_bound::play_packet>
            || std::is_same_v<T, server_bound::handshake_packet>
            || std::is_same_v<T, server_bound::status_packet>
            || std::is_same_v<T, server_bound::login_packet>
            || std::is_same_v<T, server_bound::configuration_packet>
            || std::is_same_v<T, server_bound::play_packet>
            || std::is_arithmetic_v<T>
        ) {
        } else if constexpr (std::is_base_of_v<base_objects::pallete_container, T>) {
        } else if constexpr (std::is_same_v<base_objects::pallete_data_height_map, T>) {
        } else if constexpr (is_template_base_of<ignored, T>) {
        } else if constexpr (is_template_base_of<enum_as, T> || is_template_base_of<enum_as_flag, T>) {
        } else if constexpr (is_template_base_of<any_of, T>) {
        } else if constexpr (is_template_base_of<id_set, T>) {
        } else if constexpr (std::is_same_v<bit_list_array<uint64_t>, T>) {
        } else if constexpr (is_convertible_to_packet_form<T>) {
        } else if constexpr (is_id_source<T>) {
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

    namespace __internal {
        bool visit_packet_viewer(client_bound_packet& packet, SharedClientData& context);
        bool visit_packet_viewer(server_bound_packet& packet, SharedClientData& context);
        void visit_packet_post_send_viewer(client_bound_packet& packet, base_objects::SharedClientData& context);
    }

    template <class T>
    void serialize_entry(base_objects::network::response::item& res, SharedClientData& context, T&& value) {
        using Type = std::decay_t<T>;
        if constexpr (is_convertible_to_packet_form<Type>) {
            serialize_entry(res, context, value.to_packet());
        } else if constexpr (std::is_same_v<identifier, Type>)
            res.write_identifier(value.value);
        else if constexpr (is_std_array<Type>)
            for (auto& it : value)
                serialize_entry(res, context, it);
        else if constexpr (is_string_sized<Type>)
            res.write_string(value.value, Type::max_size);
        else if constexpr (std::is_same_v<json_text_component, Type>)
            res.write_json_component(value.value);
        else if constexpr (std::is_same_v<var_int32, Type>)
            res.write_var32(value.value);
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
        } else if constexpr (std::is_same_v<position, Type>)
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
        else if constexpr (std::is_base_of_v<base_objects::pallete_container, Type>) {
            std::visit(
                [&]<class IT>(IT&& it) {
                    if constexpr (std::is_same_v<base_objects::pallete_container_indirect, IT>) {
                        res.write_value(it.bits_per_entry);
                        res.write_var32_check(it.palette.size());
                        for (auto& i : it.palette)
                            res.write_var32(i);
                        res.write_direct(it.data.get());
                    } else if constexpr (std::is_same_v<base_objects::pallete_container_single, IT>) {
                        res.write_value((uint8_t)0);
                        res.write_var32(it.id_of_palette);
                    } else if constexpr (std::is_same_v<base_objects::pallete_data, IT>) {
                        res.write_value((uint8_t)it.bits_per_entry);
                        res.write_direct(it.get());
                    }
                },
                value.compile()
            );
        } else if constexpr (std::is_same_v<base_objects::pallete_data_height_map, Type>) {
            res.write_array(value.get());
        } else if constexpr (is_template_base_of<list_array_depend, Type>) {
            size_t siz = value.size();
            size_t i = 1;
            for (auto&& it : value) {
                it.has_next_item = bool(siz != i);
                serialize_entry(res, context, it);
            }
        } else if constexpr (is_template_base_of<_list_array_impl::list_array, Type>) {
            if constexpr (!is_value_template_base_of<no_size, Type> && !std::is_base_of_v<size_from_packet, Type>)
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
        } else if constexpr (is_id_source<Type> || is_template_base_of<base_objects::depends_next, T>) {
            serialize_entry(res, context, value.value);
        } else if constexpr (is_tvalue_template_base_of<ordered_id, Type>) {
            serialize_entry(res, context, value.value);
        } else if constexpr (is_template_base_of<enum_set, Type>) {
            using Tupple_T = std::decay_t<decltype(value.values)>;
            {
                bit_list_array<uint8_t> bit(std::tuple_size_v<Tupple_T> - 1); //except header
                size_t i = 0;
                util::for_each_type<Tupple_T>::each([&]<class T_Elem>() {
                    if constexpr (!std::is_same_v<typename T_Elem::value_type, Type::header_t>) {
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
    void preprocess_structure(SharedClientData& context, T& value, T_prev& prev) {
        if constexpr (is_value_template_base_of<no_size, T>) {
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
        } else if constexpr (std::is_same_v<position, T>) {
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
            || std::is_same_v<T, client_bound::status_packet>
            || std::is_same_v<T, client_bound::login_packet>
            || std::is_same_v<T, client_bound::configuration_packet>
            || std::is_same_v<T, client_bound::play_packet>
            || std::is_same_v<T, server_bound::handshake_packet>
            || std::is_same_v<T, server_bound::status_packet>
            || std::is_same_v<T, server_bound::login_packet>
            || std::is_same_v<T, server_bound::configuration_packet>
            || std::is_same_v<T, server_bound::play_packet>
            || std::is_arithmetic_v<T>
        ) {
        } else if constexpr (std::is_base_of_v<base_objects::pallete_container, T>) {
        } else if constexpr (std::is_same_v<base_objects::pallete_data_height_map, T>) {
        } else if constexpr (is_template_base_of<ignored, T>) {
        } else if constexpr (is_template_base_of<enum_as, T> || is_template_base_of<enum_as_flag, T>) {
        } else if constexpr (is_template_base_of<any_of, T>) {
        } else if constexpr (is_template_base_of<id_set, T>) {
        } else if constexpr (std::is_same_v<bit_list_array<uint64_t>, T>) {
        } else if constexpr (is_convertible_to_packet_form<T>) {
        } else if constexpr (is_id_source<T>) {
        } else if constexpr (is_tvalue_template_base_of<ordered_id, T>) {
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
                    std::visit([&](auto& it) { serialize_packet(res, context, it); }, item);
                } else if (is_template_base_of<_list_array_impl::list_array, I>) {
                    for (auto& it : item)
                        serialize_packet(res, context, it);
                }
            });
        }
    }

    template <class T>
    void make_preprocess_packet_(base_objects::SharedClientData& context, T& value) {
        using Type = std::decay_t<T>;
        if constexpr (need_preprocess_result_v<Type>) {
            preprocess_structure(context, value, value);
            if constexpr (could_be_preprocessed<Type, Type>)
                value.preprocess(value);
        }
    }

    void make_preprocess_packet(base_objects::SharedClientData& context, client_bound_packet& packet) {
        std::visit(
            [&](auto& mode) {
                return std::visit(
                    [&](auto& it) {
                        make_preprocess_packet_(context, it);
                    },
                    mode
                );
            },
            packet
        );
    }

    void make_preprocess_packet(base_objects::SharedClientData& context, server_bound_packet& packet) {
        std::visit(
            [&](auto& mode) {
                return std::visit(
                    [&](auto& it) {
                        make_preprocess_packet_(context, it);
                    },
                    mode
                );
            },
            packet
        );
    }

    bool send(SharedClientData& client, client_bound_packet&& packet) {
        if (!client.is_active())
            return false;
        make_preprocess_packet(client, packet);
        if (__internal::visit_packet_viewer(packet, client))
            return false;
        bool sw_status = false;
        bool sw_login = false;
        bool sw_configuration = false;
        bool sw_play = false;
        client.sendPacket(
            std::visit(
                [&](auto& mode) -> base_objects::network::response {
                    return std::visit(
                        [&]<class P>(P& it) -> base_objects::network::response {
                            base_objects::network::response res;
                            serialize_packet(res, client, it);
                            if constexpr (std::is_base_of_v<disconnect_after, P>)
                                res.do_disconnect_after_send = true;
                            if constexpr (std::is_base_of_v<switches_to::status, P>)
                                sw_status = true;
                            else if constexpr (std::is_base_of_v<switches_to::login, P>)
                                sw_login = true;
                            else if constexpr (std::is_base_of_v<switches_to::configuration, P>)
                                sw_configuration = true;
                            else if constexpr (std::is_base_of_v<switches_to::play, P>)
                                sw_play = true;
                            return res;
                        },
                        mode
                    );
                },
                packet
            )
        );
        if (sw_status)
            client << switches_to::status{};
        if (sw_login)
            client << switches_to::login{};
        if (sw_configuration)
            client << switches_to::configuration{};
        if (sw_play)
            client << switches_to::play{};

        __internal::visit_packet_post_send_viewer(packet, client);
        return true;
    }

    base_objects::network::response internal_encode(SharedClientData& client, client_bound_packet&& packet) {
        make_preprocess_packet(client, packet);
        return std::visit(
            [&client](auto& mode) -> base_objects::network::response {
                return std::visit(
                    [&client](auto& it) -> base_objects::network::response {
                        base_objects::network::response res;
                        serialize_packet(res, client, it);
                        return res;
                    },
                    mode
                );
            },
            packet
        );
    }

    base_objects::network::response internal_encode(SharedClientData& client, server_bound_packet&& packet) {
        make_preprocess_packet(client, packet);
        return std::visit(
            [&client](auto& mode) -> base_objects::network::response {
                return std::visit(
                    [&client](auto& it) -> base_objects::network::response {
                        base_objects::network::response res;
                        serialize_packet(res, client, it);
                        return res;
                    },
                    mode
                );
            },
            packet
        );
    }

    base_objects::network::response encode(client_bound_packet&& packet) {
        SharedClientData client;
        return internal_encode(client, std::move(packet));
    }

    base_objects::network::response encode(server_bound_packet&& packet) {
        SharedClientData client;
        return internal_encode(client, std::move(packet));
    }
}