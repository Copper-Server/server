/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_PACKETS_HELP
#define SRC_BASE_OBJECTS_PACKETS_HELP
#include <algorithm>
#include <cstdint>
#include <library/list_array.hpp>
#include <src/base_objects/id_registry.hpp>
#include <string>
#include <unordered_map>
#include <variant>

namespace copper_server::base_objects {
    namespace internal {
        template <template <class...> class Base, class... Ts>
        void test(Base<Ts...>&);
    }

    template <class type>
    concept is_packet = requires(type& d) {
        type::packet_id::value;
    };

    struct SharedClientData;
    template <auto value>
    using ic = std::integral_constant<decltype(value), value>;

    template <class T>
    concept enum_concept = std::is_enum_v<T>;

    template <template <class...> class, class, class = void>
    constexpr bool is_template_base_of = false;

    template <template <class...> class Base, class Derived>
    constexpr bool is_template_base_of<Base, Derived, std::void_t<decltype(internal::test<Base>(std::declval<Derived&>()))>> = true;

    template <class type>
    concept is_convertible_to_packet_form = requires(type& d) {
        type::from_packet(d.to_packet());
    };

    template <is_convertible_to_packet_form type>
    using convertible_to_packet_type = decltype(std::declval<type>().to_packet());

    template <class A>
    struct for_each_type {
        static constexpr auto each(auto&& fn) {
            fn.template operator()<A>();
        }
    };

    template <class... Args>
    struct for_each_type<std::variant<Args...>> {
        static constexpr void each(auto&& lambda) {
            (
                [&]() {
                    lambda.template operator()<Args>();
                }(),
                ...
            );
        }
    };

    template <class... Args>
    constexpr bool is_correct_variant() {
        for_each_type<std::variant<Args...>>::each([]<class T>() {
            static_assert(std::is_copy_constructible_v<T>);
            static_assert(std::is_move_constructible_v<T>);
            static_assert(std::is_copy_assignable_v<T>);
            static_assert(std::is_move_assignable_v<T>);
        });
        return true;
    }

    namespace switches_to {
        struct status {
            std::strong_ordering operator<=>(const status& other) const = default;
        };

        struct login {
            std::strong_ordering operator<=>(const login& other) const = default;
        };

        struct configuration {
            std::strong_ordering operator<=>(const configuration& other) const = default;
        };

        struct play {
            std::strong_ordering operator<=>(const play& other) const = default;
        };
    }

    struct disconnect_after {
        std::strong_ordering operator<=>(const disconnect_after& other) const = default;
    };

    struct compound_packet {
        std::strong_ordering operator<=>(const compound_packet& other) const = default;
    };

    template <int32_t id>
    struct packet {
        using packet_id = ic<id>;
        std::strong_ordering operator<=>(const packet& other) const = default;
    };

    template <int32_t value>
    struct enum_item {
        using item_id = ic<value>;
        std::strong_ordering operator<=>(const enum_item& other) const = default;
    };

    template <size_t value, size_t mask, ptrdiff_t order>
    struct flag_item {
        using flag_value = ic<value>;
        using flag_mask = ic<mask>;
        using flag_order = ic<order>;
        std::strong_ordering operator<=>(const flag_item& other) const = default;
    };

    template <class T, class R>
    static constexpr bool could_be_preprocessed = requires(T& v, R& it) { v.preprocess(it); };

    struct identifier {
        std::string value;

        constexpr identifier() {}

        constexpr identifier(std::string&& value) : value(std::move(value)) {}

        constexpr identifier(const std::string& value) : value(value) {}

        constexpr identifier(identifier&& value) : value(std::move(value.value)) {}

        constexpr identifier(const identifier& value) : value(value.value) {}

        constexpr identifier(std::string_view value) : value(value) {}

        template <size_t siz>
        constexpr identifier(const char (&value)[siz]) : value(value) {}

        constexpr identifier& operator=(identifier&& other) {
            value = std::move(other.value);
            return *this;
        }

        constexpr identifier& operator=(const identifier& other) {
            value = other.value;
            return *this;
        }

        constexpr operator std::string&() & {
            return value;
        }

        constexpr operator std::string&&() && {
            return std::move(value);
        }

        constexpr operator const std::string&() const& {
            return value;
        }

        auto operator<=>(const identifier& other) const = default;
    };

    template <size_t size>
    struct string_sized {
        std::string value;
        static constexpr inline size_t max_size = size;

        constexpr string_sized() {}

        constexpr string_sized(std::string&& value) : value(std::move(value)) {}

        constexpr string_sized(const std::string& value) : value(value) {}

        constexpr string_sized(string_sized<size>&& value) : value(std::move(value.value)) {}

        constexpr string_sized(const string_sized<size>& value) : value(value.value) {}

        template <size_t other_size>
        constexpr string_sized(string_sized<other_size>&& value) : value(std::move(value.value)) {}

        template <size_t other_size>
        constexpr string_sized(const string_sized<other_size>& value) : value(value.value) {}

        template <size_t siz>
        constexpr string_sized(const char (&value)[siz]) : value(value) {}

        constexpr string_sized& operator=(std::string&& other) {
            value = std::move(other);
            return *this;
        }

        constexpr string_sized& operator=(const std::string& other) {
            value = other;
            return *this;
        }

        constexpr string_sized& operator=(string_sized<size>&& other) {
            value = std::move(other.value);
            return *this;
        }

        constexpr string_sized& operator=(const string_sized<size>& other) {
            value = other.value;
            return *this;
        }

        template <size_t other_size>
        constexpr string_sized& operator=(string_sized<other_size>&& other) {
            other = std::move(value);
            return *this;
        }

        template <size_t other_size>
        constexpr string_sized& operator=(const string_sized<other_size>& other) {
            other = value;
            return *this;
        }

        constexpr operator std::string&() & {
            return value;
        }

        constexpr operator std::string&&() && {
            return std::move(value);
        }

        constexpr operator const std::string&() const& {
            return value;
        }

        auto operator<=>(const string_sized& other) const = default;
    };

    struct json_text_component {
        std::string value;

        auto operator<=>(const json_text_component& other) const = default;
    };

    template <class T, T min, T max>
    struct limited_num {
        static constexpr inline T check_min = min;
        static constexpr inline T check_max = max;
        T value;
        auto operator<=>(const limited_num& other) const = default;
    };

    struct var_int32 {
        using underlying_type = int32_t;
        using banner_pattern = id_source<var_int32, registry_source::banner_pattern>;
        using cat_variant = id_source<var_int32, registry_source::cat_variant>;
        using chat_type = id_source<var_int32, registry_source::chat_type>;
        using chicken_variant = id_source<var_int32, registry_source::chicken_variant>;
        using cow_variant = id_source<var_int32, registry_source::cow_variant>;
        using damage_type = id_source<var_int32, registry_source::damage_type>;
        using dialog = id_source<var_int32, registry_source::dialog>;
        using dimension_type = id_source<var_int32, registry_source::dimension_type>;
        using enchantment = id_source<var_int32, registry_source::enchantment>;
        using enchantment_provider = id_source<var_int32, registry_source::enchantment_provider>;
        using frog_variant = id_source<var_int32, registry_source::frog_variant>;
        using instrument = id_source<var_int32, registry_source::instrument>;
        using jukebox_song = id_source<var_int32, registry_source::jukebox_song>;
        using loot_table = id_source<var_int32, registry_source::loot_table>;
        using painting_variant = id_source<var_int32, registry_source::painting_variant>;
        using pig_variant = id_source<var_int32, registry_source::pig_variant>;
        using recipe = id_source<var_int32, registry_source::recipe>;
        using test_environment = id_source<var_int32, registry_source::test_environment>;
        using test_instance = id_source<var_int32, registry_source::test_instance>;
        using trim_material = id_source<var_int32, registry_source::trim_material>;
        using trim_pattern = id_source<var_int32, registry_source::trim_pattern>;
        using wolf_sound_variant = id_source<var_int32, registry_source::wolf_sound_variant>;
        using wolf_variant = id_source<var_int32, registry_source::wolf_variant>;
        using worldgen__biome = id_source<var_int32, registry_source::worldgen__biome>;
        using attribute = id_source<var_int32, registry_source::attribute>;
        using block_state = id_source<var_int32, registry_source::block_state>;
        using block_type = id_source<var_int32, registry_source::block_type>;
        using block_entity_type = id_source<var_int32, registry_source::block_entity_type>;
        using data_component_type = id_source<var_int32, registry_source::data_component_type>;
        using dimension = id_source<var_int32, registry_source::dimension>;
        using entity_type = id_source<var_int32, registry_source::entity_type>;
        using fluid = id_source<var_int32, registry_source::fluid>;
        using game_event = id_source<var_int32, registry_source::game_event>;
        using position_source_type = id_source<var_int32, registry_source::position_source_type>;
        using item = id_source<var_int32, registry_source::item>;
        using menu = id_source<var_int32, registry_source::menu>;
        using mob_effect = id_source<var_int32, registry_source::mob_effect>;
        using particle_type = id_source<var_int32, registry_source::particle_type>;
        using potion = id_source<var_int32, registry_source::potion>;
        using recipe_serializer = id_source<var_int32, registry_source::recipe_serializer>;
        using recipe_type = id_source<var_int32, registry_source::recipe_type>;
        using sound_event = id_source<var_int32, registry_source::sound_event>;
        using stat_type = id_source<var_int32, registry_source::stat_type>;
        using custom_stat = id_source<var_int32, registry_source::custom_stat>;
        using command_argument_type = id_source<var_int32, registry_source::command_argument_type>;
        using entity__activity = id_source<var_int32, registry_source::entity__activity>;
        using entity__memory_module_type = id_source<var_int32, registry_source::entity__memory_module_type>;
        using entity__schedule = id_source<var_int32, registry_source::entity__schedule>;
        using entity__sensor_type = id_source<var_int32, registry_source::entity__sensor_type>;
        using entity__motive = id_source<var_int32, registry_source::entity__motive>;
        using entity__villager_profession = id_source<var_int32, registry_source::entity__villager_profession>;
        using entity__villager_type = id_source<var_int32, registry_source::entity__villager_type>;
        using entity__poi_type = id_source<var_int32, registry_source::entity__poi_type>;
        using loot_table__loot_condition_type = id_source<var_int32, registry_source::loot_table__loot_condition_type>;
        using loot_table__loot_function_type = id_source<var_int32, registry_source::loot_table__loot_function_type>;
        using loot_table__loot_nbt_provider_type = id_source<var_int32, registry_source::loot_table__loot_nbt_provider_type>;
        using loot_table__loot_number_provider_type = id_source<var_int32, registry_source::loot_table__loot_number_provider_type>;
        using loot_table__loot_pool_entry_type = id_source<var_int32, registry_source::loot_table__loot_pool_entry_type>;
        using loot_table__loot_score_provider_type = id_source<var_int32, registry_source::loot_table__loot_score_provider_type>;
        using villager_variant = id_source<var_int32, registry_source::villager_variant>;
        using fox_variant = id_source<var_int32, registry_source::fox_variant>;
        using parrot_variant = id_source<var_int32, registry_source::parrot_variant>;
        using tropical_fish_pattern = id_source<var_int32, registry_source::tropical_fish_pattern>;
        using mooshroom_variant = id_source<var_int32, registry_source::mooshroom_variant>;
        using rabbit_variant = id_source<var_int32, registry_source::rabbit_variant>;
        using horse_variant = id_source<var_int32, registry_source::horse_variant>;
        using llama_variant = id_source<var_int32, registry_source::llama_variant>;
        using axolotl_variant = id_source<var_int32, registry_source::axolotl_variant>;

        int32_t value = 0;

        constexpr var_int32() {}

        template <enum_concept T>
        constexpr var_int32(T value) : value((int32_t)value) {}

        constexpr var_int32(int32_t value) : value(value) {}

        constexpr operator int32_t&() {
            return value;
        }

        constexpr operator const int32_t&() const {
            return value;
        }

        template <enum_concept T>
        constexpr operator T() const {
            return (T)value;
        }

        auto operator<=>(const var_int32& other) const = default;
    };

    struct var_int64 {
        using underlying_type = int64_t;
        int64_t value = 0;

        constexpr var_int64() {}

        template <enum_concept T>
        constexpr var_int64(T value) : value((int64_t)value) {}

        constexpr var_int64(int64_t value) : value(value) {}

        constexpr var_int64(const var_int64& value) : value(value.value) {}

        constexpr operator int64_t&() {
            return value;
        }

        constexpr operator const int64_t&() const {
            return value;
        }

        template <enum_concept T>
        constexpr operator T() const {
            return (T)value;
        }

        auto operator<=>(const var_int64& other) const = default;
    };

    struct optional_var_int32 : public std::optional<int32_t> { //encoded same as var_int32 but if set the value incremented and checked for overflow, if not set encoded as 0
        using std::optional<int32_t>::optional;
    };

    struct optional_var_int64 : public std::optional<int64_t> { //encoded same as var_int64 but if set the value incremented and checked for overflow, if not set encoded as 0
        using std::optional<int64_t>::optional;
    };

    template <class Value, class T>
    struct value_optional {
        using depend_value = Value;
        using value_type = T;
        Value v;
        std::optional<T> rest;
        auto operator<=>(const value_optional& other) const = default;
    };

    //if value would be zero, next fields ignored
    template <class Value>
    struct depends_next {
        using value_type = Value;
        Value value;

        constexpr depends_next() : value() {}

        template <class T>
        constexpr depends_next(T value)
            requires(std::is_convertible_v<T, Value>)
            : value((Value)value) {}

        constexpr depends_next(Value value) : value(value) {}

        constexpr depends_next(const depends_next& value) : value(value.value) {}

        constexpr depends_next(depends_next&& value) : value(std::move(value.value)) {}

        constexpr depends_next& operator=(const depends_next& other) {
            value = other.value;
            return *this;
        }

        constexpr depends_next& operator=(depends_next&& other) {
            value = std::move(other.value);
            return *this;
        }

        constexpr operator Value&() {
            return value;
        }

        constexpr operator const Value&() const {
            return value;
        }

        template <class T>
        constexpr operator T() const
            requires(std::is_convertible_v<Value, T>)
        {
            return (T)value;
        }

        auto operator<=>(const depends_next& other) const = default;
    };


    enum class size_source {
        get_world_chunks_height,
        get_world_blocks_height,
    };

    size_t get_size_source_value(SharedClientData&, size_source);

    //this type provides way to get size of array while decoding, the values would also be checked to be equal to size of the container
    template <auto... DependedValues>
    struct no_size {
        template <class T>
        static size_t get_depended_size(SharedClientData& context, const T& val) {
            static auto get_value = [](SharedClientData& context, const T& val, auto&& it) -> size_t {
                if constexpr (std::is_same_v<std::decay_t<decltype(it)>, size_source>)
                    return get_size_source_value(context, it);
                else if constexpr (is_template_base_of<depends_next, std::decay_t<decltype(val.*it)>>) {
                    if constexpr (
                        std::is_same_v<std::decay_t<decltype((val.*it).value)>, var_int64>
                        || std::is_same_v<std::decay_t<decltype((val.*it).value)>, var_int32>
                    )
                        return (val.*it).value.value;
                    else
                        return (val.*it).value;
                } else if constexpr (
                    std::is_same_v<std::decay_t<decltype(val.*it)>, var_int64>
                    || std::is_same_v<std::decay_t<decltype(val.*it)>, var_int32>
                )
                    return (val.*it).value;
                else
                    return val.*it;
            };
            return (0 + ... + get_value(context, val, DependedValues));
        }
    };

    struct size_from_packet {};

    template <class T, size_t size>
    struct list_array_sized : public list_array<T> {
        using list_array<T>::list_array;
        static constexpr inline size_t max_size = size;

        list_array_sized(const list_array<T>& copy) : list_array<T>(copy) {}

        list_array_sized(list_array<T>&& mov) : list_array<T>(std::move(mov)) {}

        auto operator<=>(const list_array_sized& other) const = default;
    };

    template <class T, size_t size, auto... DependedValues>
    struct list_array_sized_no_size : public no_size<DependedValues...>, list_array_sized<T, size> {
        using list_array_sized<T, size>::list_array_sized;
        static constexpr inline size_t max_size = size;

        list_array_sized_no_size(const list_array<T>& copy) : list_array_sized<T, size>(copy) {}

        list_array_sized_no_size(list_array<T>&& mov) : list_array_sized<T, size>(std::move(mov)) {}

        auto operator<=>(const list_array_sized_no_size& other) const = default;
    };

    template <class T, auto... DependedValues>
    struct list_array_no_size : public no_size<DependedValues...>, list_array<T> {
        using list_array<T>::list_array;

        list_array_no_size(const list_array<T>& copy) : list_array<T>(copy) {}

        list_array_no_size(list_array<T>&& mov) : list_array<T>(std::move(mov)) {}

        auto operator<=>(const list_array_no_size& other) const = default;
    };

    template <class T, size_t size>
    struct list_array_sized_siz_from_packet : public size_from_packet, list_array_sized<T, size> {
        using list_array_sized<T, size>::list_array_sized;

        list_array_sized_siz_from_packet(const list_array<T>& copy) : list_array_sized<T, size>(copy) {}

        list_array_sized_siz_from_packet(list_array<T>&& mov) : list_array_sized<T, size>(std::move(mov)) {}

        auto operator<=>(const list_array_sized_siz_from_packet& other) const = default;
    };

    template <class T, class T_size>
    struct sized_entry {
        using size_type = T_size;
        using value_type = T;
        T value;
        auto operator<=>(const sized_entry& other) const = default;
    };

    template <class T, size_t size>
    struct list_array_fixed : public list_array<T> {
        static constexpr inline size_t required_size = size;
        using list_array<T>::list_array;
        auto operator<=>(const list_array_fixed& other) const = default;
    };

    template <class T>
    struct list_array_siz_from_packet : public size_from_packet, list_array<T> {
        using list_array<T>::list_array;
        auto operator<=>(const list_array_siz_from_packet& other) const = default;
    };

    template <size_t size>
    struct bitset_fixed {
        using max_size = ic<size>;
        bit_list_array<uint8_t> value;

        template <class R>
        void preprocess(R&) {
            value.resize(size);
        }

        auto operator<=>(const bitset_fixed& other) const = default;
    };

    template <class T, T flag, auto depend_prev_class>
    struct item_depend : public T {
        using depend_value = ic<flag>;
        using body_depend = ic<depend_prev_class>;
        using base_depend = T;
        using T::T;
        auto operator<=>(const item_depend& other) const = default;
    };

    template <class T>
    concept is_item_depend = requires {
        T::depend_value::value;
        T::body_depend::value;
        typename T::base_depend;
    };

    template <class T>
    concept struct_depends = requires(T& it) { it.has_next_item = {true}; };

    template <struct_depends T>
    struct list_array_depend : public list_array<T> {
        using list_array<T>::list_array;
        bool decoding_flag = false;

        list_array_depend(const list_array<T>& copy) : list_array<T>(copy) {}

        list_array_depend(list_array<T>&& mov) : list_array<T>(std::move(mov)) {}

        auto operator<=>(const list_array_depend& other) const = default;
    };

    template <class Variant0, class Variant1>
    struct or_ : public std::variant<Variant0, Variant1> {
        using var_0 = Variant0;
        using var_1 = Variant1;
        using base = std::variant<Variant0, Variant1>;

        or_() : base() {}

        or_(const base& v) : base(v) {}

        or_(base&& v) : base(std::move(v)) {}

        or_(const or_& v) : base((const base&)v) {}

        or_(or_&& v) : base(std::move((base&)v)) {}

        or_(var_0&& v) : base(std::move(v)) {}

        or_(var_1&& v) : base(std::move(v)) {}

        or_(const var_0& v) : base(v) {}

        or_(const var_1& v) : base(v) {}

        or_& operator=(const base& v) {
            (base&)* this = v;
            return *this;
        }

        or_& operator=(base&& v) {
            (base&)* this = std::move(v);
            return *this;
        }

        or_& operator=(const or_& v) {
            (base&)* this = (const base&)v;
            return *this;
        }

        or_& operator=(or_&& v) {
            (base&)* this = std::move((base&)v);
            return *this;
        }

        or_& operator=(var_0&& v) {
            (base&)* this = std::move(v);
            return *this;
        }

        or_& operator=(var_1&& v) {
            (base&)* this = std::move(v);
            return *this;
        }

        or_& operator=(const var_0& v) {
            (base&)* this = v;
            return *this;
        }

        or_& operator=(const var_1& v) {
            (base&)* this = v;
            return *this;
        }

        auto operator<=>(const or_& other) const {
            return std::visit(
                [this]<class T1>(const T1& v1) {
                    return std::visit(
                        [&v1]<class T0>(const T0& v0) {
                            if constexpr (std::is_same_v<T0, T1>)
                                return v0 == v1 ? std::strong_ordering::equal : std::strong_ordering::less;
                            else
                                return std::strong_ordering::less;
                        },
                        (const base&)*this
                    );
                },
                (const base&)other
            );
        }
    };

    template <class Variant0, class Variant1>
    struct bool_or : public std::variant<Variant0, Variant1> {
        using var_0 = Variant0;
        using var_1 = Variant1;
        using std::variant<Variant0, Variant1>::variant;

        auto operator<=>(const bool_or& other) const {
            return std::visit(
                [this]<class T1>(T1& v1) {
                    return std::visit(
                        [&v1]<class T0>(T0& v0) {
                            if constexpr (std::is_same_v<T0, T1>)
                                return v0 == v1 ? std::strong_ordering::equal : std::strong_ordering::less;
                            else
                                return std::strong_ordering::less;
                        },
                        *this
                    );
                },
                other
            );
        }
    };

    template <class T>
    struct packet_compress {
        using value_type = T;
        T value;
        auto operator<=>(const packet_compress& other) const = default;
    };

    template <class T>
    struct id_set : public std::variant<identifier, list_array<T>> {
        using base = std::variant<identifier, list_array<T>>;
        using std::variant<identifier, list_array<T>>::variant;
        using id_type = T;
    };

    struct Angle {
        uint8_t value;

        Angle(double val) : value(uint8_t((val * 3.14159265358979323846 * 2) / 360)) {}

        Angle() : value(0) {}

        Angle(const Angle& value) : value(value.value) {}

        Angle& operator=(const Angle& v) {
            value = v.value;
            return *this;
        }

        operator double() {
            return (value * 360) / (3.14159265358979323846 * 2);
        }

        auto operator<=>(const Angle& other) const = default;
    };

    template <class Enum, class T>
    struct enum_as {
        using encode_t = T;
        Enum value;

        constexpr enum_as() : value() {}

        constexpr enum_as(Enum e) : value(e) {}

        constexpr enum_as(T e) : value((Enum)e) {}

        constexpr T get() const {
            if constexpr (std::is_same_v<var_int32, T> || std::is_same_v<var_int64, T>)
                return (T)(typename T::underlying_type)value;
            else
                return (T)value;
        }

        constexpr auto operator<=>(const enum_as& other) const = default;

        constexpr enum_as operator|(const enum_as& it) const {
            return get() | it.get();
        }

        constexpr enum_as operator&(const enum_as& it) const {
            return get() & it.get();
        }

        constexpr enum_as operator~() const {
            return ~get();
        }

        constexpr operator bool() const {
            return get();
        }
    };

    template <class T>
    static constexpr bool is_enum_item = requires { T::item_id::value; };

    template <class ValueType, class... Ty>
    struct enum_switch : public std::variant<Ty...> {
        static constexpr inline bool is_correct = is_correct_variant<Ty...>();

        using encode_type = ValueType;
        using base = std::variant<Ty...>;

        enum_switch() : base() {}

        enum_switch(const enum_switch& v) {
            *this = v;
        }

        enum_switch(enum_switch&& v) {
            *this = std::move(v);
        }

        enum_switch(std::variant<Ty...>&& v) {
            *this = std::move(v);
        }

        enum_switch(const std::variant<Ty...>& v) {
            *this = v;
        }

        enum_switch(std::convertible_to<std::variant<Ty...>> auto&& v) {
            *this = std::move(v);
        }

        enum_switch(const std::convertible_to<std::variant<Ty...>> auto& v) {
            *this = v;
        }

        enum_switch& operator=(const enum_switch& v);
        enum_switch& operator=(enum_switch&& v);
        enum_switch& operator=(std::variant<Ty...>&& v);
        enum_switch& operator=(const std::variant<Ty...>& v);
        enum_switch& operator=(std::convertible_to<std::variant<Ty...>> auto&& v);
        enum_switch& operator=(const std::convertible_to<std::variant<Ty...>> auto& v);

        template <class FN>
        constexpr static void get_enum(size_t id, FN&& fn) {
            for_each_type<base>::each(
                [&]<class T>() {
                    if (T::item_id::value == id)
                        fn.template operator()<T>();
                }
            );
        }

        template <class FN>
        constexpr static void for_each(FN&& fn) {
            for_each_type<base>::each(
                [&]<class T>() {
                    fn.template operator()<T>();
                }
            );
        }

        bool operator==(const enum_switch& other) const = default;
        auto operator<=>(const enum_switch& other) const = default;
    };

    template <class ValueType, class... Ty>
    enum_switch<ValueType, Ty...>& enum_switch<ValueType, Ty...>::operator=(const enum_switch<ValueType, Ty...>& v) {
        (base&)* this = (const base&)v;
        return *this;
    }

    template <class ValueType, class... Ty>
    enum_switch<ValueType, Ty...>& enum_switch<ValueType, Ty...>::operator=(enum_switch<ValueType, Ty...>&& v) {
        (base&)* this = std::move((base&)v);
        return *this;
    }

    template <class ValueType, class... Ty>
    enum_switch<ValueType, Ty...>& enum_switch<ValueType, Ty...>::operator=(std::variant<Ty...>&& v) {
        (base&)* this = std::move(v);
        return *this;
    }

    template <class ValueType, class... Ty>
    enum_switch<ValueType, Ty...>& enum_switch<ValueType, Ty...>::operator=(const std::variant<Ty...>& v) {
        (base&)* this = v;
        return *this;
    }

    template <class ValueType, class... Ty>
    enum_switch<ValueType, Ty...>& enum_switch<ValueType, Ty...>::operator=(std::convertible_to<std::variant<Ty...>> auto&& v) {
        (base&)* this = std::move(v);
        return *this;
    }

    template <class ValueType, class... Ty>
    enum_switch<ValueType, Ty...>& enum_switch<ValueType, Ty...>::operator=(const std::convertible_to<std::variant<Ty...>> auto& v) {
        (base&)* this = v;
        return *this;
    }

    template <class T>
    static constexpr bool is_flag_item = requires {
        T::flag_value::value;
        T::flag_mask::value;
        T::flag_order::value;
    };

    template <class flag_type, class... Ty>
    struct flags_list {
        using max_orders = ic<std::max<ptrdiff_t>({Ty::flag_order::value...})>;
        using base = std::variant<Ty...>;
        flag_type flag;
        std::unordered_map<ptrdiff_t, std::variant<Ty...>> values; //order->value

        flags_list() {}

        static flags_list make(std::initializer_list<base> flags) {
            flags_list res;
            for (auto& it : flags)
                res.set(std::move(it));
            return res;
        }

        template <class T>
        bool is_set() const {
            return (values.find(T::flag_order::value) != values.end());
        }

        template <class FN>
        void for_each(FN&& fn) {
            for (auto& [order, it] : values)
                std::visit(fn, it);
        }

        template <class FN>
        void for_each_in_order(FN&& fn) {
            for (ptrdiff_t order = 0; order <= max_orders::value; order++) {
                auto it = values.find(order);
                if (it != values.end())
                    std::visit(fn, it->second);
            }
        }

        template <class FN>
        void for_each(FN&& fn) const {
            for (auto& [order, it] : values)
                std::visit(fn, it);
        }

        template <class FN>
        void for_each_in_order(FN&& fn) const {
            for (ptrdiff_t order = 0; order <= max_orders::value; order++) {
                auto it = values.find(order);
                if (it != values.end())
                    std::visit(fn, it->second);
            }
        }

        template <class FN>
        constexpr static void for_each_flag_in_order(FN&& fn) {
            for (ptrdiff_t order = 0; order <= max_orders::value; order++) {
                for_each_type<base>::each(
                    [&]<class T>() {
                        if (T::flag_order::value == order)
                            fn.template operator()<T>();
                    }
                );
            }
        }

        template <class FN>
        void for_each_set_flag_in_order(FN&& fn) {
            for_each_flag_in_order([&]<class T>() {
                if ((flag & T::flag_mask::value) == T::flag_value::value)
                    fn.template operator()<T>();
            });
        }

        template <class FN>
        void for_each_set_flag_in_order(FN&& fn) const {
            for_each_flag_in_order([&]<class T>() {
                if ((flag & T::flag_mask::value) == T::flag_value::value)
                    fn.template operator()<T>();
            });
        }

        template <class T>
        void set(T&& item) {
            values[T::flag_order::value] = std::move(item);
            update_flag();
        }

        template <class T>
        void set(const T&& item) = delete;

        template <class T>
        void set() {
            values[T::flag_order::value] = T();
            update_flag();
        }

        auto operator<=>(const flags_list& other) const = default;

    private:
        void update_flag() {
            flag = 0;
            for (auto& [id, value] : values) {
                std::visit(
                    [this](auto& it) {
                        using T = std::decay_t<decltype(it)>;
                        flag |= (T::flag_value::value & T::flag_mask::value);
                    },
                    value
                );
            }
        }
    };

    template <class Source, class SourceType, SourceType Source::* source_name, class... Ty>
    struct flags_list_from {
        using max_orders = ic<std::max<ptrdiff_t>({Ty::flag_order::value...})>;
        using preprocess_source_name = ic<source_name>;
        using base = std::variant<Ty...>;
        using source_type = SourceType;
        std::unordered_map<ptrdiff_t, std::variant<Ty...>> values; //flag_order->value
        source_type pre_process_result;

        flags_list_from() {}

        static flags_list_from make(std::initializer_list<base> flags) {
            flags_list_from res;
            for (auto& it : flags)
                res.set(std::move(it));
            return res;
        }

        template <class T>
        bool is_set() const {
            return (values.find(T::flag_order::value) != values.end());
        }

        void preprocess(Source& source) {
            source.*source_name = 0;
            for (auto& [id, value] : values) {
                std::visit(
                    [&]<class T>(T& it) {
                        source.*source_name |= (T::flag_value::value & T::flag_mask::value);
                    },
                    value
                );
            }
            pre_process_result = source.*source_name;
        }

        template <class FN>
        void for_each(FN&& fn) {
            for (auto& [order, it] : values)
                std::visit(fn, it);
        }

        template <class FN>
        void for_each_in_order(FN&& fn) {
            for (ptrdiff_t order = 0; order <= max_orders::value; order++) {
                auto it = values.find(order);
                if (it != values.end())
                    std::visit(fn, it->second);
            }
        }

        template <class FN>
        void for_each(FN&& fn) const {
            for (auto& [order, it] : values)
                std::visit(fn, it);
        }

        template <class FN>
        void for_each_in_order(FN&& fn) const {
            for (ptrdiff_t order = 0; order <= max_orders::value; order++) {
                auto it = values.find(order);
                if (it != values.end())
                    std::visit(fn, it->second);
            }
        }

        template <class FN>
        constexpr static void for_each_flag_in_order(FN&& fn) {
            for (ptrdiff_t order = 0; order <= max_orders::value; order++) {
                for_each_type<base>::each(
                    [&]<class T>() {
                        if (T::flag_order::value == order)
                            fn.template operator()<T>();
                    }
                );
            }
        }

        template <class FN>
        constexpr static void for_each_set_flag_in_order(auto flag, FN&& fn) {
            for_each_flag_in_order([&]<class T>() {
                if ((flag & T::flag_mask::value) == T::flag_value::value)
                    fn.template operator()<T>();
            });
        }

        template <class T>
        void set(T&& item) {
            values[T::flag_order::value] = base(std::move(item));
        }

        template <class T>
        void set(const T&& item) = delete;

        template <class T>
        void set() {
            values[T::flag_order::value] = base(T());
        }

        auto operator<=>(const flags_list_from& other) const = default;
    };

    template <class Enum, class T>
    struct enum_as_flag {
        using encode_t = T;
        Enum value;

        constexpr enum_as_flag() : value() {}

        constexpr enum_as_flag(Enum e) : value(e) {}

        constexpr enum_as_flag(T e) : value((Enum)e) {}

        constexpr T get() const {
            return (T)value;
        }

        constexpr auto operator<=>(const enum_as_flag& other) const = default;

        constexpr enum_as_flag operator|(const enum_as_flag& it) const {
            return get() | it.get();
        }

        constexpr enum_as_flag operator&(const enum_as_flag& it) const {
            return get() & it.get();
        }

        constexpr enum_as_flag operator~() const {
            return ~get();
        }

        constexpr operator bool() const {
            return get();
        }
    };

    template <class base_type, class... Ty>
    struct any_of {
        base_type value;

        template <class T>
        T& cast() {
            return reinterpret_cast<T&>(value);
        }

        template <class T>
        const T& cast() const {
            return reinterpret_cast<const T&>(value);
        }

        template <class T>
        any_of& operator=(T&& val) {
            value = reinterpret_cast<base_type&&>(val);
            return *this;
        }

        template <class T>
        any_of& operator=(const T& val) {
            value = reinterpret_cast<const base_type&>(val);
            return *this;
        }

        auto operator<=>(const any_of& other) const = default;
    };

    template <class value_type, class... Ty>
    struct partial_enum_switch : public std::variant<value_type, Ty...> {
        using encode_type = value_type;
        using base = std::variant<value_type, Ty...>;

        partial_enum_switch() : base() {}

        partial_enum_switch(const base& v) : base(v) {}

        partial_enum_switch(base&& v) : base(std::move(v)) {}

        partial_enum_switch(const partial_enum_switch& v) : base((const base&)v) {}

        partial_enum_switch(partial_enum_switch&& v) : base(std::move((base&)v)) {}

        template <std::constructible_from<base> T>
        partial_enum_switch(T&& v) : base(std::move(v)) {}

        template <std::constructible_from<base> T>
        partial_enum_switch(const T& v) : base(v) {}

        partial_enum_switch& operator=(const base& v) {
            (base&)* this = v;
            return *this;
        }

        partial_enum_switch& operator=(base&& v) {
            (base&)* this = std::move(v);
            return *this;
        }

        partial_enum_switch& operator=(const partial_enum_switch& v) {
            (base&)* this = (const base&)v;
            return *this;
        }

        partial_enum_switch& operator=(partial_enum_switch&& v) {
            (base&)* this = std::move((base&)v);
            return *this;
        }

        template <std::constructible_from<base> T>
        partial_enum_switch& operator=(T&& v) {
            (base&)* this = std::move(v);
            return *this;
        }

        template <std::constructible_from<base> T>
        partial_enum_switch& operator=(const T& v) {
            (base&)* this = v;
            return *this;
        }

        template <class FN>
        constexpr static void get_enum(size_t id, FN&& fn) {
            bool found = false;
            for_each_type<base>::each(
                [&]<class T>() {
                    if constexpr (std::is_same_v<T, value_type>)
                        ;
                    else if (T::item_id::value == id) {
                        found = true;
                        fn.template operator()<T>();
                    }
                }
            );
            if (!found)
                fn.template operator()<encode_type>();
        }

        template <class FN>
        constexpr static void for_each(FN&& fn) {
            for_each_type<base>::each(
                [&]<class T>() {
                    fn.template operator()<T>();
                }
            );
            fn.template operator()<encode_type>();
        }

        auto operator<=>(const partial_enum_switch& other) const = default;
    };

    template <class T>
    struct ignored {
        T value;

        ignored() : value() {}

        ignored(ignored&& it) : value(std::move(it.value)) {}

        ignored(const ignored& it) : value(it.value) {}

        ignored(T&& it) : value(std::move(it)) {}

        ignored(const T& it) : value(it) {}

        ignored& operator=(T&& v) {
            value = std::move(v);
            return *this;
        }

        ignored& operator=(const T& v) {
            value = v;
            return *this;
        }

        ignored& operator=(ignored&& v) {
            value = std::move(v.value);
            return *this;
        }

        ignored& operator=(const ignored& v) {
            value = v.value;
            return *this;
        }

        operator T&() {
            return value;
        }

        operator const T&() const {
            return value;
        }

        auto operator<=>(const ignored& other) const = default;
    };
}

#endif /* SRC_BASE_OBJECTS_PACKETS_HELP */
