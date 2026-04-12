/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_ENCODING_PACKET_ENCODE
#define SRC_UTIL_ENCODING_PACKET_ENCODE

#include "src/util/encoding/nbt/serialization.hpp"
#include <src/api/network/tcp.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <tuple>

#include <src/util/encoding/common.hpp>

namespace copper_server::util::encoding::packet {

    template <class T, class... VisitedTypes>
    bool need_preprocess_result();

    template <class T>
    void serialize_entry(base_objects::network::response_item& res, base_objects::shared_client_data& context, T&& value);

    template <class T, class T_prev>
    void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev);

    namespace detail {
        template <class T, class... VisitedTypes>
        consteval bool need_preprocess_result(priority_tag<1>)
            requires contains_type_v<T, VisitedTypes...>
        {
            return false;
        }

        template <class T, class...>
        consteval bool need_preprocess_result(priority_tag<1>)
            requires requires_check<T> || is_ordered_id<T>
        {
            return true;
        }

        template <class T, class... VisitedTypes>
        consteval bool need_preprocess_result(priority_tag<1>)
            requires is_std_array<T>
                     || is_template_base_of<_list_array_impl::list_array, T>
                     || is_template_base_of<base_objects::box, T>
                     || is_template_base_of<std::optional, T>
                     || is_template_base_of<base_objects::value_optional, T>
                     || is_template_base_of<base_objects::sized_entry, T>
                     || is_template_base_of<base_objects::packet_compress, T>
        {
            return need_preprocess_result<typename T::value_type, T, VisitedTypes...>();
        }

        template <class T, class... VisitedTypes>
        consteval bool need_preprocess_result(priority_tag<1>)
            requires is_flags_list_from<T> || is_template_base_of<base_objects::flags_list, T>
        {
            bool res = false;
            T::for_each_flag_in_order([&]<class IT>() {
                if (!res)
                    res = need_preprocess_result<IT, T, VisitedTypes...>();
            });
            return res;
        }

        template <class T, class... VisitedTypes>
        consteval bool need_preprocess_result(priority_tag<1>)
            requires is_template_base_of<base_objects::enum_switch, T> || is_template_base_of<base_objects::partial_enum_switch, T>
        {
            bool res = false;
            T::for_each([&]<class IT>() {
                if (!res)
                    res = need_preprocess_result<IT, T, VisitedTypes...>();
            });
            return res;
        }

        template <class T, class... VisitedTypes>
        consteval bool need_preprocess_result(priority_tag<1>)
            requires is_template_base_of<base_objects::or_, T> || is_template_base_of<base_objects::bool_or, T>
        {
            return need_preprocess_result<typename T::var_0, T, VisitedTypes...>() || need_preprocess_result<typename T::var_1, T, VisitedTypes...>();
        }

        template <class T, class...>
        consteval bool need_preprocess_result(priority_tag<1>)
            requires std::is_same_v<base_objects::identifier, T>
                     || std::is_same_v<base_objects::json_text_component, T>
                     || std::is_same_v<base_objects::var_int32, T>
                     || std::is_same_v<base_objects::velocity, T>
                     || std::is_same_v<base_objects::var_int64, T>
                     || std::is_same_v<base_objects::optional_var_int32, T>
                     || std::is_same_v<base_objects::optional_var_int64, T>
                     || std::is_same_v<base_objects::position, T>
                     || std::is_arithmetic_v<T>
                     || std::is_same_v<std::string, T>
                     || std::is_same_v<base_objects::uuid, T>
                     || std::is_same_v<base_objects::chat, T>
                     || std::is_same_v<T, api::packets::client_bound::play::play_packet>
                     || std::is_arithmetic_v<T>
                     || std::is_base_of_v<base_objects::palette_container, T>
                     || std::is_same_v<base_objects::palette_data_height_map, T>
                     || is_template_base_of<base_objects::ignored, T>
                     || is_template_base_of<base_objects::enum_as, T> || is_template_base_of<base_objects::enum_as_flag, T>
                     || is_template_base_of<base_objects::any_of, T>
                     || is_template_base_of<base_objects::id_set, T>
                     || std::is_same_v<bit_list_array<uint64_t>, T>
                     || base_objects::is_convertible_to_packet_form<T>
                     || api::id::is_source<T>
        {
            return false;
        }

        template <class T, class... VisitedTypes>
        consteval bool need_preprocess_result(priority_tag<1>)
            requires is_template_base_of<base_objects::enum_set, T>
        {
            using Tupple_T = std::decay_t<decltype(std::declval<T>().values)>;
            bool res = false;
            util::for_each_type<Tupple_T>::each([&]<class T_Elem>() {
                if constexpr (need_preprocess_result<typename T_Elem::value_type, T, VisitedTypes...>())
                    res = true;
            });
            return res;
        }

        template <class T, class... VisitedTypes>
        consteval bool need_preprocess_result(priority_tag<0>) {
            bool res = false;
            reflect::for_each_type<T>([&]<class I>() {
                if (res)
                    return;
                if constexpr (base_objects::could_be_preprocessed<I, T> || requires_check<T>)
                    res = true;
                else if constexpr (need_preprocess_result<I, T, VisitedTypes...>())
                    res = true;
            });
            return res;
        }
        template <class T>
        concept need_preprocess_result_v = detail::need_preprocess_result<T>();

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires base_objects::is_convertible_to_packet_form<Type>
        {
            serialize_entry(res, context, value.to_packet());
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires std::is_same_v<base_objects::identifier, Type>
        {
            res.write_identifier(value.value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires is_constant_value<Type>
        {
            auto to_write = Type::value::value;
            serialize_entry(res, context, to_write);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires is_std_array<Type>
        {
            for (auto& it : value)
                serialize_entry(res, context, it);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires is_string_sized<Type>
        {
            res.write_string(value.value, Type::max_size);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires std::is_same_v<base_objects::json_text_component, Type>
        {
            res.write_json_component(value.value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires std::is_same_v<base_objects::var_int32, Type>
        {
            res.write_var32(value.value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires std::is_same_v<base_objects::var_int64, Type>
        {
            res.write_var64(value.value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires std::is_same_v<base_objects::velocity, Type>
        {
            res.write_value(value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires std::is_same_v<base_objects::optional_var_int32, Type>
        {
            if (value)
                res.write_var32_check(static_cast<int64_t>(*value) + 1);
            else
                res.write_var32(0);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires std::is_same_v<base_objects::position, Type>
        {
            res.write_value(value.get());
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires std::is_arithmetic_v<Type>
        {
            res.write_value(value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires std::is_same_v<std::string, Type>
        {
            res.write_string(value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires std::is_same_v<base_objects::uuid, Type>
        {
            res.write_value(value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires std::is_same_v<base_objects::chat, Type>
        {
            res.write_direct(util::nbt_convert::build(value.to_nbt()).get_as_network());
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires std::is_same_v<util::nbt, Type>
        {
            res.write_direct(util::nbt_convert::build(value).get_as_network());
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<2>)
            requires std::is_same_v<util::nbt_convert, Type>
        {
            res.write_direct(value.get_as_network());
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires std::is_same_v<base_objects::palette_container, Type>
        {
            std::visit(
                [&]<class IT>(const IT& it) {
                    if constexpr (std::is_same_v<base_objects::palette_container_indirect, IT>) {
                        res.write_value(it.bits_per_entry);
                        res.write_var32_check(it.palette.size());
                        for (auto& i : it.palette)
                            res.write_var32(i);
                        res.write_direct(it.data.get());
                    } else if constexpr (std::is_same_v<base_objects::palette_container_single, IT>) {
                        res.write_value(static_cast<uint8_t>(0));
                        res.write_var32(it.id_of_palette);
                    } else if constexpr (std::is_same_v<base_objects::palette_data, IT>) {
                        res.write_value(static_cast<uint8_t>(it.bits_per_entry));
                        res.write_direct(it.get());
                    }
                },
                value.compile()
            );
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires std::is_same_v<base_objects::palette_data_height_map, Type>
        {
            res.write_array(value.get());
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::list_array_depend, Type>
        {
            size_t siz = value.size();
            size_t i = 1;
            for (auto&& it : value) {
                it.has_next_item = bool(siz != i);
                serialize_entry(res, context, it);
                ++i;
            }
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<_list_array_impl::list_array, Type>
        {
            if constexpr (!is_no_size<Type> && !std::is_base_of_v<api::packets::size_from_packet, Type>)
                res.write_var32_check(value.size());
            if constexpr (std::is_same_v<uint8_t, typename Type::value_type> || std::is_same_v<int8_t, typename Type::value_type>)
                res.write_direct(value);
            else {
                for (auto&& it : value)
                    serialize_entry(res, context, it);
            }
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::ignored, Type>
        {
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<std::optional, Type>
        {
            res.write_value((bool)value);
            if (value)
                serialize_entry(res, context, *value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::enum_as, Type> || is_template_base_of<base_objects::enum_as_flag, Type>
        {
            serialize_entry(res, context, value.get());
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::or_, Type>
        {
            std::visit(
                [&]<typename it_T>(it_T& it) {
                    if constexpr (std::is_same_v<typename Type::var_0, std::decay_t<it_T>>) {
                        if constexpr (is_template_base_of<base_objects::enum_as, typename Type::var_0>)
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
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::bool_or, Type>
        {
            std::visit(
                [&](auto& it) {
                    res.write_value(std::is_same_v<typename Type::var_0, std::decay_t<decltype(it)>>);
                    serialize_entry(res, context, it);
                },
                value
            );
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::enum_switch, Type>
        {
            std::visit(
                [&]<typename it_T_>(it_T_& it) {
                    using it_T = std::decay_t<it_T_>;
                    serialize_entry(res, context, typename Type::encode_type(it_T::item_id::value));
                    serialize_entry(res, context, it);
                },
                value
            );
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::partial_enum_switch, Type>
        {
            std::visit(
                [&]<typename T_it>(T_it& it) {
                    using it_T = std::decay_t<T_it>;
                    if constexpr (std::is_same_v<it_T, typename Type::encode_type>) {
                        serialize_entry(res, context, it);
                    } else {
                        serialize_entry(res, context, typename Type::encode_type(it_T::item_id::value));
                        serialize_entry(res, context, it);
                    }
                },
                value
            );
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::box, Type>
        {
            serialize_entry(res, context, *value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::any_of, Type>
        {
            serialize_entry(res, context, value.value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::packet_compress, Type>
        {
            res.apply_compression = true;
            res.compression_threshold = value.value;
            serialize_entry(res, context, value.value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::flags_list, Type>
        {
            serialize_entry(res, context, value.flag);
            value.for_each_in_order([&](auto& it) {
                serialize_entry(res, context, it);
            });
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<std::unordered_map, Type> && std::is_same_v<typename Type::key_type, std::string>
        {
            res.write_var32_check(value.size());
            for (auto& [key, it] : value) {
                res.write_string(key);
                serialize_entry(res, context, it);
            }
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<std::unordered_map, Type> && is_map_compatible<Type>
        {
            res.write_var32_check(value.size());
            for (auto& [key, it] : value) {
                serialize_entry(key, context, it);
                serialize_entry(res, context, it);
            }
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_flags_list_from<Type>
        {
            value.for_each_in_order([&](auto& it) {
                serialize_entry(res, context, it);
            });
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::id_set, Type>
        {
            std::visit(
                [&]<typename it_T>(it_T& it) {
                    if constexpr (std::is_same_v<base_objects::identifier, std::decay_t<it_T>>) {
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
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::value_optional, Type>
        {
            if (value.rest && value.v) {
                serialize_entry(res, context, value.v);
                serialize_entry(res, context, *value.rest);
            } else {
                decltype(value.v) tmp{0};
                serialize_entry(res, context, tmp);
            }
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::sized_entry, Type>
        {
            base_objects::network::response_item inner;
            serialize_entry(inner, context, value.value);
            typename Type::size_type size;
            if constexpr (sizeof(typename Type::size_type) >= 8)
                size = inner.data.size();
            else if constexpr (sizeof(typename Type::size_type) >= 4)
                size = static_cast<int32_t>(inner.data.size());
            else
                size = static_cast<int16_t>(inner.data.size());
            serialize_entry(res, context, size);
            res.write_in(inner);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_limited_num<Type>
        {
            serialize_entry(res, context, value.value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_bitset_fixed<Type>
        {
            res.write_direct(value.value.data());
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires std::is_same_v<bit_list_array<uint64_t>, Type>
        {
            res.write_var32_check(value.data().size());
            res.write_direct(value.data());
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires api::id::is_source<Type> || is_template_base_of<base_objects::depends_next, Type>
        {
            serialize_entry(res, context, value.value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_ordered_id<Type>
        {
            serialize_entry(res, context, value.value);
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires is_template_base_of<base_objects::enum_set, Type>
        {
            using Tupple_T = std::decay_t<decltype(value.values)>;
            {
                bit_list_array<uint8_t> bit(std::tuple_size_v<Tupple_T> - 1); //except header
                size_t i = 0;
                util::for_each_type<Tupple_T>::each([&]<class T_Elem>() {
                    if constexpr (!std::is_same_v<typename T_Elem::value_type, typename Type::header_t>) {
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
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<1>)
            requires make_packet_as_nbt<Type>
        {
            std::stringstream ss;
            util::nbt_write_stream nbt_stream(ss);
            util::encoding::nbt::serialize_entry(nbt_stream, value);
            res.write_direct(
                util::nbt_convert::build(
                    list_array<uint8_t>(
                        (const uint8_t*)ss.view().data(),
                        ss.view().size()
                    )
                )
                    .get_as_network()
            );
        }

        template <class Type>
        void serialize_impl(base_objects::network::response_item& res, base_objects::shared_client_data& context, Type&& value, priority_tag<0>) {
            bool process_next = true;
            reflect::for_each_field(value, [&value, &res, &context, &process_next]<class IT>(IT& item) {
                if (process_next) {
                    if constexpr (base_objects::is_item_depend<IT>) {
                        typename IT::base_depend tmp = item;
                        if (value.*IT::body_depend::value)
                            tmp = tmp | IT::depend_value::value;
                        serialize_entry(res, context, tmp);
                    } else
                        serialize_entry(res, context, item);
                    if constexpr (is_template_base_of<base_objects::depends_next, IT>)
                        process_next = static_cast<bool>(item.value);
                }
            });
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_no_size<T>)
        {
            if constexpr (is_template_base_of<_list_array_impl::list_array, T>)
                if (value.size() != T::get_depended_size(context, prev))
                    throw std::overflow_error("The size of list_array did not equals to depended values.");
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_bitset_fixed<T>)
        {

            if (value.value.size() != T::max_size::value)
                throw std::overflow_error("The bitset size not equal required one.");
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_list_array_fixed<T>)
        {
            if (value.size() != T::required_size)
                throw std::overflow_error("The list_array size not equal required one.");
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                for (auto& it : value)
                    preprocess_structure(context, it, prev);
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_std_array<T> || is_template_base_of<_list_array_impl::list_array, T>)
        {
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                for (auto& it : value)
                    preprocess_structure(context, it, prev);
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_string_sized<T>)
        {
            if (value.value.size() > T::max_size)
                throw std::overflow_error("The string size is over the limit.");
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_list_array_sized<T>)
        {
            if (value.size() > T::max_size)
                throw std::overflow_error("The list_array size is over the limit.");
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                for (auto& it : value.value)
                    preprocess_structure(context, it, prev);
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_template_base_of<std::optional, T>)
        {
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                if (value)
                    preprocess_structure(context, *value, prev);
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_limited_num<T>)
        {
            if (value.value > T::check_max)
                throw std::overflow_error("The value is too big");
            if (value.value < T::check_min)
                throw std::underflow_error("The value is too low");
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_template_base_of<base_objects::value_optional, T>)
        {
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                if (value.rest)
                    preprocess_structure(context, *value.rest, prev);
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_flags_list_from<T> || is_template_base_of<base_objects::flags_list, T>)
        {
            value.for_each([&]<class IT>(IT& it) {
                if constexpr (need_preprocess_result_v<IT>)
                    preprocess_structure(context, it, prev);
            });
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_template_base_of<base_objects::enum_switch, T> || is_template_base_of<base_objects::partial_enum_switch, T>)
        {
            std::visit(
                [&]<class IT>(IT& it) {
                    if constexpr (need_preprocess_result_v<IT>)
                        preprocess_structure(context, it, prev);
                },
                value
            );
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_template_base_of<base_objects::box, T>)
        {
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                preprocess_structure(context, *value, prev);
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_template_base_of<base_objects::depends_next, T> || is_template_base_of<base_objects::sized_entry, T> || is_template_base_of<base_objects::packet_compress, T>)
        {
            if constexpr (need_preprocess_result_v<typename T::value_type>)
                preprocess_structure(context, value.value, prev);
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_template_base_of<base_objects::or_, T> || is_template_base_of<base_objects::bool_or, T>)
        {
            std::visit(
                [&]<class Y>(Y& it) {
                    if constexpr (need_preprocess_result_v<Y>)
                        preprocess_structure(context, it, prev);
                },
                value
            );
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(
                std::is_same_v<base_objects::identifier, T>
                || std::is_same_v<base_objects::json_text_component, T>
                || std::is_same_v<base_objects::var_int32, T>
                || std::is_same_v<base_objects::var_int64, T>
                || std::is_same_v<base_objects::optional_var_int32, T>
                || std::is_same_v<base_objects::optional_var_int64, T>
                || std::is_same_v<base_objects::position, T>
                || std::is_arithmetic_v<T>
                || std::is_same_v<std::string, T>
                || std::is_same_v<base_objects::uuid, T>
                || std::is_same_v<base_objects::chat, T>
                || std::is_same_v<T, api::packets::client_bound::play::play_packet>
                || std::is_base_of_v<base_objects::palette_container, T>
                || std::is_same_v<base_objects::palette_data_height_map, T>
                || is_template_base_of<base_objects::ignored, T>
                || is_template_base_of<base_objects::enum_as, T>
                || is_template_base_of<base_objects::enum_as_flag, T>
                || is_template_base_of<base_objects::any_of, T>
                || is_template_base_of<base_objects::id_set, T>
                || std::is_same_v<bit_list_array<uint64_t>, T>
                || base_objects::is_convertible_to_packet_form<T>
                || api::id::is_source<T>
            )
        {
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_ordered_id<T>)
        {
            value.value = context.packets_state.internal_data.set([](auto& data) {
                return ++data.id_tracker[T::id_source];
            });
            value.is_valid = true;
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<1>)
            requires(is_template_base_of<base_objects::enum_set, T>)
        {
            using Tupple_T = std::decay_t<decltype(value.values)>;
            util::for_each_type<Tupple_T>::each([&]<class T_Elem>() {
                if constexpr (need_preprocess_result_v<typename T_Elem::value_type>) {
                    if (value.template has<typename T_Elem::value_type>())
                        preprocess_structure(context, value.template get<typename T_Elem::value_type>(), value);
                }
            });
        }

        template <class T, class T_prev>
        void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev, priority_tag<0>) {
            bool process_next = true;
            reflect::for_each_field(value, [&value, &context, &process_next](auto& item) {
                if (!process_next)
                    return;
                using I = std::decay_t<decltype(item)>;
                if constexpr (need_preprocess_result_v<I>)
                    preprocess_structure(context, item, value);
                if constexpr (base_objects::could_be_preprocessed<I, T>)
                    item.preprocess(value);

                if constexpr (is_template_base_of<base_objects::depends_next, std::decay_t<decltype(item)>>)
                    process_next = (bool)item.value;
            });
        }
    }

    template <class T, class... VisitedTypes>
    bool need_preprocess_result() {
        return detail::need_preprocess_result<T, VisitedTypes...>();
    }

    template <class T>
    void serialize_entry(base_objects::network::response_item& res, base_objects::shared_client_data& context, T&& value) {
        detail::serialize_impl(res, context, std::forward<T>(value), priority_tag<3>{});
    }

    template <class T, class T_prev>
    void preprocess_structure(base_objects::shared_client_data& context, T& value, T_prev& prev) {
        detail::preprocess_structure(context, value, prev, priority_tag<3>{});
    }

    template <class T>
    void serialize_packet(base_objects::network::response& res, base_objects::shared_client_data& context, T& value) {
        using Type = std::decay_t<T>;
        if constexpr (std::is_base_of_v<api::packets::compound_packet, Type>) {
            reflect::for_each_field(value, [&res, &context](auto& item) {
                using I = std::decay_t<decltype(item)>;
                if constexpr (api::packets::is_packet<I>) {
                    serialize_packet(res, context, item);
                } else if constexpr (std::is_same_v<I, api::packets::client_bound::play::play_packet>) {
                    std::visit([&](auto& it) { serialize_packet(res, context, it); }, item);
                } else if (is_template_base_of<_list_array_impl::list_array, I>) {
                    for (auto& it : item)
                        serialize_packet(res, context, it);
                }
            });
            if constexpr (std::is_base_of_v<api::packets::disconnect_after, Type>)
                res.do_disconnect_after_send = true;
        } else if constexpr (std::is_same_v<Type, api::packets::client_bound::play::play_packet>) {
            std::visit([&](auto& it) { serialize_packet(res, context, it); }, value);
        } else if constexpr (api::packets::is_packet<Type>) {
            base_objects::network::response_item it;
            it.write_id(Type::packet_id::value);
            serialize_entry(it, context, value);
            res += std::move(it);
            if constexpr (std::is_base_of_v<api::packets::disconnect_after, Type>)
                res.do_disconnect_after_send = true;
        }
    }

    template <class Type>
    void make_preprocess(base_objects::shared_client_data& context, Type& value) {
        if constexpr (detail::need_preprocess_result_v<Type>) {
            preprocess_structure(context, value, value);
            if constexpr (base_objects::could_be_preprocessed<Type, Type>)
                value.preprocess(value);
        }
    }

    template <class Ops, class T>
    bool make_send(base_objects::shared_client_data& context, T&& value) {
        using Type = std::decay_t<T>;
        if (!context.is_active())
            return false;
        make_preprocess(context, value);
        if (Ops::send_viewer().notify(value, context))
            return false;
        base_objects::network::response res;
        serialize_packet(res, context, value);
        if constexpr (std::is_base_of_v<api::packets::disconnect_after, Type>)
            res.do_disconnect_after_send = true;
        context.sendPacket(std::move(res));

        if constexpr (std::is_base_of_v<api::packets::switches_to::status, Type>)
            context << api::packets::switches_to::status{};
        else if constexpr (std::is_base_of_v<api::packets::switches_to::login, Type>)
            context << api::packets::switches_to::login{};
        else if constexpr (std::is_base_of_v<api::packets::switches_to::config, Type>)
            context << api::packets::switches_to::config{};
        else if constexpr (std::is_base_of_v<api::packets::switches_to::play, Type>)
            context << api::packets::switches_to::play{};

        Ops::post_send_viewer().notify(value, context);
        return true;
    }

    template <class Ops, class Type>
    base_objects::network::response make_encode(base_objects::shared_client_data& context, Type&& value) {
        make_preprocess(context, value);
        if (Ops::send_viewer().notify(value, context))
            return {};
        base_objects::network::response res;
        serialize_packet(res, context, value);
        if constexpr (std::is_base_of_v<api::packets::disconnect_after, Type>)
            res.do_disconnect_after_send = true;
        return res;
    }
}

#endif /* SRC_UTIL_ENCODING_PACKET_ENCODE */
