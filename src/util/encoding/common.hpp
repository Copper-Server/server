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
#include <src/base_objects/packets_help.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/position.hpp>
#include <src/base_objects/box.hpp>
#include <src/util/reflect.hpp>


namespace copper_server::util::encoding {
    template <template <auto...> class Base, auto... Ts>
    void value_test(Base<Ts...>&);

    template <template <class, auto...> class Base, class T, auto... Ts>
    void tvalue_test(Base<T, Ts...>&);

    template <template <class...> class Base, class... Ts>
    void value_test(Base<Ts...>&);

    template <template <class...> class, class, class = void>
    constexpr bool is_template_base_of = false;

    template <template <class...> class Base, class Derived>
    constexpr bool is_template_base_of<Base, Derived, std::void_t<decltype(value_test<Base>(std::declval<Derived&>()))>> = true;

    template <class type>
    concept is_convertible_to_packet_form = requires(type& d) {
        type::from_packet(d.to_packet());
    };

    template <is_convertible_to_packet_form type>
    using convertible_to_packet_type = decltype(std::declval<type>().to_packet());


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
    struct is_flags_list_from_helper<base_objects::flags_list_from<S, ST, M, Ts...>> : std::true_type {};

    template <class type>
    concept is_flags_list_from = is_flags_list_from_helper<std::decay_t<type>>::value;

    template <class type>
    concept is_string_sized = is_value_template_base_of<base_objects::string_sized, type>;
    template <class type>
    concept is_bitset_fixed = is_value_template_base_of<base_objects::bitset_fixed, type>;

    template <class type>
    concept is_list_array_sized = is_tvalue_template_base_of<base_objects::list_array_sized, type> || is_tvalue_template_base_of<base_objects::list_array_sized_siz_from_packet, type> || is_tvalue_template_base_of<base_objects::list_array_sized_no_size, type>;

    template <class type>
    concept is_list_array_fixed = is_tvalue_template_base_of<base_objects::list_array_fixed, type>;

    template <class type>
    concept is_std_array = is_tvalue_template_base_of<std::array, type>;

    template <class type>
    concept is_limited_num = is_tvalue_template_base_of<base_objects::limited_num, type>;


    template <class base, class derived>
    concept template_base_of = is_template_base_of<base, derived>;

    template <class base, class derived>
    concept value_template_base_of = is_value_template_base_of<base, derived>;

    template <class base, class derived>
    concept tvalue_template_base_of = is_tvalue_template_base_of<base, derived>;
}

#endif /* SRC_UTIL_ENCODING_COMMON */
