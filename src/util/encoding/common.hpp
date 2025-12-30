/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_ENCODING_COMMON
#define SRC_UTIL_ENCODING_COMMON
#include <src/api/packets/types.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/pool.hpp>
#include <src/util/cts.hpp>
#include <src/util/reflect.hpp>

namespace copper_server::base_objects {
    struct chat;
}

namespace copper_server::base_objects {
    struct position;
    class palette_container;
    struct palette_data_height_map;
    struct palette_container_indirect;
    struct palette_container_block;
    struct palette_container_biome;
    struct palette_container_single;
    struct palette_data;

    template <class T>
    struct box;
}

namespace copper_server::api::packets::client_bound::play {
    struct play_packet;
}

namespace copper_server::util::encoding {
    template <unsigned N>
    struct priority_tag : priority_tag<N - 1> {};

    template <>
    struct priority_tag<0> {};


    template <class T>
    concept is_map_compatible = requires(typename T::key_type& key) {
        key = key.to_string();
    };

    template <class T>
    concept make_packet_as_nbt = requires {
        typename T::packet_as_nbt;
    };
    
    template <class type>
    concept is_convertible_to_nbt_form = requires(const type& d, util::nbt_write_stream& w_stream, util::nbt_read_stream& r_stream) {
        d.to_nbt(w_stream);
        type::from_packet(r_stream);
    };

    template <template <auto...> class Base, auto... Ts>
    static void value_test(Base<Ts...>&) {}

    template <template <auto...> class, class, class = void>
    constexpr bool is_value_template_base_of = false;
    template <template <auto...> class Base, class Derived>
    constexpr bool is_value_template_base_of<Base, Derived, std::void_t<decltype(value_test<Base>(std::declval<Derived&>()))>> = true;

    template <class T>
    struct type_selector : std::integral_constant<size_t, 0> {};

    template <class T, T min, T max>
    struct type_selector<base_objects::limited_num<T, min, max>> : std::integral_constant<size_t, 1> {};

    template <class S, class ST, ST S::* M, class... Ts>
    struct type_selector<base_objects::flags_list_from<S, ST, M, Ts...>> : std::integral_constant<size_t, 2> {};

    template <size_t size>
    struct type_selector<base_objects::string_sized<size>> : std::integral_constant<size_t, 3> {};
    
    template <size_t size>
    struct type_selector<base_objects::bitset_fixed<size>> : std::integral_constant<size_t, 4> {};

    template <class T, size_t size>
    struct type_selector<base_objects::list_array_sized<T, size>> : std::integral_constant<size_t, 5> {};

    template <class T, auto ... dep_v>
    struct type_selector<base_objects::list_array_no_size<T, dep_v...>> : std::integral_constant<size_t, 6> {};

    template <class T, size_t size, auto ... dep_v>
    struct type_selector<base_objects::list_array_sized_no_size<T, size, dep_v...>> : std::integral_constant<size_t, 7> {};

    template <class T, size_t size>
    struct type_selector<base_objects::list_array_sized_siz_from_packet<T, size>> : std::integral_constant<size_t, 8> {};

    template <class T, class  Ts>
    struct type_selector<base_objects::sized_entry<T, Ts>> : std::integral_constant<size_t, 9> {};

    template <class T, size_t size>
    struct type_selector<base_objects::list_array_fixed<T, size>> : std::integral_constant<size_t, 10> {};

    template <class T>
    struct type_selector<base_objects::list_array_siz_from_packet<T>> : std::integral_constant<size_t, 11> {};

    template <class T, util::CTS id>
    struct type_selector<base_objects::ordered_id<T, id>> : std::integral_constant<size_t, 12> {};

    template <class T, size_t size>
    struct type_selector<std::array<T, size>> : std::integral_constant<size_t, 13> {};

    template <auto V>
    struct type_selector<base_objects::constant_value<V>> : std::integral_constant<size_t, 14> {};

    template <class type>
    concept is_limited_num = type_selector<type>::value == 1;

    template <class type>
    concept is_flags_list_from = type_selector<type>::value == 2;

    template <class type>
    concept is_string_sized = type_selector<type>::value == 3;

    template <class type>
    concept is_bitset_fixed = type_selector<type>::value == 4;

    template <class type>
    concept is_list_array_sized = type_selector<type>::value == 5 || type_selector<type>::value == 7 ||  type_selector<type>::value == 8;

    template <class type>
    concept is_list_array_fixed = type_selector<type>::value == 10;

    template <class type>
    concept is_ordered_id = type_selector<type>::value == 12;

    template <class type>
    concept is_std_array = type_selector<type>::value == 13;

    template <class type>
    concept is_constant_value = type_selector<type>::value == 14;

    template <class type>
    concept is_no_size = is_value_template_base_of<base_objects::no_size, type>;

    template <class type>
    concept requires_check = is_limited_num<type> || is_string_sized<type> || is_list_array_sized<type> || is_list_array_fixed<type> || is_no_size<type> || is_bitset_fixed<type>;


    template <typename T, typename... Ts>
    struct contains_type : std::disjunction<std::is_same<T, Ts>...> {};

    template <typename T, typename... Ts>
    inline constexpr bool contains_type_v = contains_type<T, Ts...>::value;
}

#endif /* SRC_UTIL_ENCODING_COMMON */
