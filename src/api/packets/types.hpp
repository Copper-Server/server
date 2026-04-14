/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_API_PACKETS_TYPES
#define SRC_API_PACKETS_TYPES
#include <algorithm>
#include <cstdint>
#include <library/list_array.hpp>
#include <src/api/id.hpp>
#include <src/base_objects/events/sync_event.hpp>
#include <src/util/cts.hpp>
#include <string>
#include <unordered_map>
#include <variant>

namespace copper_server::base_objects {
    struct shared_client_data;
    class command_manager;
    struct recipe;
}

namespace copper_server::api::packets {
    namespace events {
        extern base_objects::events::sync_event_no_cancel<base_objects::shared_client_data&> client_state_changed;
    }
    //reflect_map skip_begin
    template <class T>
    static constexpr bool is_enum_item = requires { T::item_id::value; };

    template <class T>
    static constexpr bool is_default_enum_item = requires { T::item_id::value; typename T::flag_default; };

    template <class T>
    static constexpr bool is_flag_item = requires {
        T::flag_value::value;
        T::flag_mask::value;
        T::flag_order::value;
    };

    namespace internal {
        template <template <class...> class Base, class... Ts>
        void test(Base<Ts...>&);

        template <class... Ts>
        struct find_default_item;

        template <class T, class... Ts>
        struct find_default_item<T, Ts...> {
            using type = std::conditional_t<is_default_enum_item<T>, T, typename find_default_item<Ts...>::type>;
        };

        template <>
        struct find_default_item<> {
            using type = void;
        };

        template <class... Ts>
        struct count_default_items;

        template <class T, class... Ts>
        struct count_default_items<T, Ts...> {
            static constexpr int value = (is_default_enum_item<T> ? 1 : 0) + count_default_items<Ts...>::value;
        };

        template <>
        struct count_default_items<> {
            static constexpr int value = 0;
        };
    }


    struct shared_client_data;
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
            static_assert(std::default_initializable<T>);
            static_assert(std::is_default_constructible_v<T>);
            static_assert(std::is_copy_constructible_v<T>);
            static_assert(std::is_move_constructible_v<T>);
            static_assert(std::is_copy_assignable_v<T>);
            static_assert(std::is_move_assignable_v<T>);
        });
        return true;
    }

    template <class... T>
    struct generic_variant : public std::variant<T...> {
        using base = std::variant<T...>;
        using base::variant;
        using base::operator=;
    };

    //reflect_map skip_end
    namespace switches_to {
        struct status {
            constexpr status() = default;
            std::strong_ordering operator<=>(const status& other) const = default;
        };

        struct login {
            constexpr login() = default;
            std::strong_ordering operator<=>(const login& other) const = default;
        };

        struct config {
            constexpr config() = default;
            std::strong_ordering operator<=>(const config& other) const = default;
        };

        struct play {
            constexpr play() = default;
            std::strong_ordering operator<=>(const play& other) const = default;
        };
    }

    struct disconnect_after {
        std::strong_ordering operator<=>(const disconnect_after& other) const = default;
    };

    struct compound_packet { //declares packet as compound the decoder doesn't work for this packet
        std::strong_ordering operator<=>(const compound_packet& other) const = default;
    };

    template <int32_t id>
    struct packet {
        using packet_id = ic<id>;
        std::strong_ordering operator<=>(const packet& other) const = default;
    };

    template <class type>
    concept is_packet = requires(type& d) {
        type::packet_id::value;
    };

    template <auto id>
    struct constant_value {
        using value = ic<id>;
        std::strong_ordering operator<=>(const constant_value& other) const = default;
    };

    template <int32_t value>
    struct enum_item {
        using item_id = ic<value>;
        std::strong_ordering operator<=>(const enum_item& other) const = default;
    };

    template <int32_t value>
    struct default_enum_item {
        using flag_default = void;
        using item_id = ic<value>;
        std::strong_ordering operator<=>(const default_enum_item& other) const = default;
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
        using underlying_type = std::string;
        std::string value;

        constexpr identifier() {}

        constexpr identifier(std::string&& value) noexcept : value(std::move(value)) {}

        constexpr identifier(const std::string& value) : value(value) {}

        constexpr identifier(identifier&& value) noexcept
            : value(std::move(value.value)) {}

        constexpr identifier(const identifier& value) : value(value.value) {}

        constexpr identifier(std::string_view value) : value(value) {}

        template <size_t siz>
        constexpr identifier(const char (&value)[siz]) : value(value) {}

        constexpr identifier& operator=(identifier&& other) noexcept {
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

    struct degrees {
        float value;

        int8_t to_packet() {
            return (int8_t)floor(value * 256.0f / 360.0f);
        }

        static float from_packet(int8_t packedDegrees) {
            return float(int16_t(packedDegrees) * 360) / 256.0f;
        }
    };

    template <size_t size>
    struct string_sized {
        using underlying_type = std::string;
        std::string value;
        static constexpr inline size_t max_size = size;

        constexpr string_sized() {}

        constexpr string_sized(std::string&& value) noexcept : value(std::move(value)) {}

        constexpr string_sized(const std::string& value) : value(value) {}

        constexpr string_sized(string_sized<size>&& value) noexcept : value(std::move(value.value)) {}

        constexpr string_sized(const string_sized<size>& value) : value(value.value) {}

        template <size_t other_size>
        constexpr string_sized(string_sized<other_size>&& value) noexcept : value(std::move(value.value)) {}

        template <size_t other_size>
        constexpr string_sized(const string_sized<other_size>& value) : value(value.value) {}

        template <size_t siz>
        constexpr string_sized(const char (&value)[siz]) : value(value) {}

        constexpr string_sized& operator=(std::string&& other) noexcept {
            value = std::move(other);
            return *this;
        }

        constexpr string_sized& operator=(const std::string& other) {
            value = other;
            return *this;
        }

        constexpr string_sized& operator=(string_sized<size>&& other) noexcept {
            value = std::move(other.value);
            return *this;
        }

        constexpr string_sized& operator=(const string_sized<size>& other) {
            value = other.value;
            return *this;
        }

        template <size_t other_size>
        constexpr string_sized& operator=(string_sized<other_size>&& other) noexcept {
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
        using underlying_type = std::string;
        std::string value;
        json_text_component() = default;

        constexpr json_text_component(std::string&& value) noexcept : value(std::move(value)) {}

        constexpr json_text_component(const std::string& value) : value(value) {}

        constexpr json_text_component(json_text_component&& value) noexcept : value(std::move(value.value)) {}

        constexpr json_text_component(const json_text_component& value) : value(value.value) {}

        template <size_t siz>
        constexpr json_text_component(const char (&value)[siz]) : value(value) {}

        constexpr json_text_component& operator=(std::string&& other) noexcept {
            value = std::move(other);
            return *this;
        }

        constexpr json_text_component& operator=(const std::string& other) {
            value = other;
            return *this;
        }

        constexpr json_text_component& operator=(json_text_component&& other) noexcept {
            value = std::move(other.value);
            return *this;
        }

        constexpr json_text_component& operator=(const json_text_component& other) {
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

        auto operator<=>(const json_text_component& other) const = default;
    };

    template <class T, T min, T max>
    struct limited_num {
        using underlying_type = T;
        static constexpr inline T check_min = min;
        static constexpr inline T check_max = max;
        T value = {};

        constexpr limited_num() = default;

        constexpr limited_num(T&& value) noexcept : value(std::move(value)) {}

        constexpr limited_num(const T& value) noexcept : value(value) {}

        constexpr limited_num(const limited_num& value) noexcept : value(value.value) {}

        constexpr limited_num(limited_num&& value) noexcept : value(value.value) {}

        constexpr limited_num& operator=(limited_num&& other) {
            value = std::move(other.value);
            return *this;
        }

        constexpr limited_num& operator=(const limited_num& other) {
            value = other.value;
            return *this;
        }

        auto operator<=>(const limited_num& other) const = default;

        operator T() const {
            return value;
        }

        operator T&() {
            return value;
        }

        template <class U>
        operator U() const {
            return static_cast<U>(value);
        }
    };

    struct var_int32 {
        using underlying_type = int32_t;
        using banner_pattern = api::id::source<var_int32, api::id::registry_source::banner_pattern>;
        using cat_variant = api::id::source<var_int32, api::id::registry_source::cat_variant>;
        using chat_type = api::id::source<var_int32, api::id::registry_source::chat_type>;
        using chicken_variant = api::id::source<var_int32, api::id::registry_source::chicken_variant>;
        using cow_variant = api::id::source<var_int32, api::id::registry_source::cow_variant>;
        using damage_type = api::id::source<var_int32, api::id::registry_source::damage_type>;
        using dialog = api::id::source<var_int32, api::id::registry_source::dialog>;
        using dimension_type = api::id::source<var_int32, api::id::registry_source::dimension_type>;
        using enchantment = api::id::source<var_int32, api::id::registry_source::enchantment>;
        using enchantment_provider = api::id::source<var_int32, api::id::registry_source::enchantment_provider>;
        using frog_variant = api::id::source<var_int32, api::id::registry_source::frog_variant>;
        using instrument = api::id::source<var_int32, api::id::registry_source::instrument>;
        using jukebox_song = api::id::source<var_int32, api::id::registry_source::jukebox_song>;
        using loot_table = api::id::source<var_int32, api::id::registry_source::loot_table>;
        using painting_variant = api::id::source<var_int32, api::id::registry_source::painting_variant>;
        using pig_variant = api::id::source<var_int32, api::id::registry_source::pig_variant>;
        using recipe = api::id::source<var_int32, api::id::registry_source::recipe>;
        using trim_material = api::id::source<var_int32, api::id::registry_source::trim_material>;
        using trim_pattern = api::id::source<var_int32, api::id::registry_source::trim_pattern>;
        using wolf_sound_variant = api::id::source<var_int32, api::id::registry_source::wolf_sound_variant>;
        using wolf_variant = api::id::source<var_int32, api::id::registry_source::wolf_variant>;
        using worldgen__biome = api::id::source<var_int32, api::id::registry_source::worldgen__biome>;
        using block_type = api::id::source<var_int32, api::id::registry_source::block_type>;
        using block_entity_type = api::id::source<var_int32, api::id::registry_source::block_entity_type>;
        using dimension = api::id::source<var_int32, api::id::registry_source::dimension>;
        using entity_type = api::id::source<var_int32, api::id::registry_source::entity_type>;
        using fluid = api::id::source<var_int32, api::id::registry_source::fluid>;
        using game_event = api::id::source<var_int32, api::id::registry_source::game_event>;
        using item = api::id::source<var_int32, api::id::registry_source::item>;
        using potion = api::id::source<var_int32, api::id::registry_source::potion>;
        using villager_variant = api::id::source<var_int32, api::id::registry_source::villager_variant>;
        using fox_variant = api::id::source<var_int32, api::id::registry_source::fox_variant>;
        using parrot_variant = api::id::source<var_int32, api::id::registry_source::parrot_variant>;
        using tropical_fish_pattern = api::id::source<var_int32, api::id::registry_source::tropical_fish_pattern>;
        using mooshroom_variant = api::id::source<var_int32, api::id::registry_source::mooshroom_variant>;
        using rabbit_variant = api::id::source<var_int32, api::id::registry_source::rabbit_variant>;
        using horse_variant = api::id::source<var_int32, api::id::registry_source::horse_variant>;
        using llama_variant = api::id::source<var_int32, api::id::registry_source::llama_variant>;
        using axolotl_variant = api::id::source<var_int32, api::id::registry_source::axolotl_variant>;
        using trial_spawner_config = api::id::source<var_int32, api::id::registry_source::trial_spawner_config>;

        using activity = api::id::source<var_int32, api::id::registry_source::activity>;
        using attribute = api::id::source<var_int32, api::id::registry_source::attribute>;
        using block_predicate_type = api::id::source<var_int32, api::id::registry_source::block_predicate_type>;
        using chunk_status = api::id::source<var_int32, api::id::registry_source::chunk_status>;
        using command_argument_type = api::id::source<var_int32, api::id::registry_source::command_argument_type>;
        using consume_effect_type = api::id::source<var_int32, api::id::registry_source::consume_effect_type>;
        using creative_mode_tab = api::id::source<var_int32, api::id::registry_source::creative_mode_tab>;
        using custom_stat = api::id::source<var_int32, api::id::registry_source::custom_stat>;
        using data_component_predicate_type = api::id::source<var_int32, api::id::registry_source::data_component_predicate_type>;
        using data_component_type = api::id::source<var_int32, api::id::registry_source::data_component_type>;
        using debug_subscription = api::id::source<var_int32, api::id::registry_source::debug_subscription>;
        using decorated_pot_pattern = api::id::source<var_int32, api::id::registry_source::decorated_pot_pattern>;
        using dialog_action_type = api::id::source<var_int32, api::id::registry_source::dialog_action_type>;
        using dialog_body_type = api::id::source<var_int32, api::id::registry_source::dialog_body_type>;
        using dialog_type = api::id::source<var_int32, api::id::registry_source::dialog_type>;
        using enchantment_effect_component_type = api::id::source<var_int32, api::id::registry_source::enchantment_effect_component_type>;
        using enchantment_entity_effect_type = api::id::source<var_int32, api::id::registry_source::enchantment_entity_effect_type>;
        using enchantment_level_based_value_type = api::id::source<var_int32, api::id::registry_source::enchantment_level_based_value_type>;
        using enchantment_location_based_effect_type = api::id::source<var_int32, api::id::registry_source::enchantment_location_based_effect_type>;
        using enchantment_provider_type = api::id::source<var_int32, api::id::registry_source::enchantment_provider_type>;
        using enchantment_value_effect_type = api::id::source<var_int32, api::id::registry_source::enchantment_value_effect_type>;
        using entity_sub_predicate_type = api::id::source<var_int32, api::id::registry_source::entity_sub_predicate_type>;
        using float_provider_type = api::id::source<var_int32, api::id::registry_source::float_provider_type>;
        using height_provider_type = api::id::source<var_int32, api::id::registry_source::height_provider_type>;
        using incoming_rpc_methods = api::id::source<var_int32, api::id::registry_source::incoming_rpc_methods>;
        using input_control_type = api::id::source<var_int32, api::id::registry_source::input_control_type>;
        using int_provider_type = api::id::source<var_int32, api::id::registry_source::int_provider_type>;
        using loot_condition_type = api::id::source<var_int32, api::id::registry_source::loot_condition_type>;
        using loot_function_type = api::id::source<var_int32, api::id::registry_source::loot_function_type>;
        using loot_nbt_provider_type = api::id::source<var_int32, api::id::registry_source::loot_nbt_provider_type>;
        using loot_number_provider_type = api::id::source<var_int32, api::id::registry_source::loot_number_provider_type>;
        using loot_pool_entry_type = api::id::source<var_int32, api::id::registry_source::loot_pool_entry_type>;
        using loot_score_provider_type = api::id::source<var_int32, api::id::registry_source::loot_score_provider_type>;
        using map_decoration_type = api::id::source<var_int32, api::id::registry_source::map_decoration_type>;
        using memory_module_type = api::id::source<var_int32, api::id::registry_source::memory_module_type>;
        using menu = api::id::source<var_int32, api::id::registry_source::menu>;
        using mob_effect = api::id::source<var_int32, api::id::registry_source::mob_effect>;
        using number_format_type = api::id::source<var_int32, api::id::registry_source::number_format_type>;
        using outgoing_rpc_methods = api::id::source<var_int32, api::id::registry_source::outgoing_rpc_methods>;
        using particle_type = api::id::source<var_int32, api::id::registry_source::particle_type>;
        using point_of_interest_type = api::id::source<var_int32, api::id::registry_source::point_of_interest_type>;
        using pos_rule_test = api::id::source<var_int32, api::id::registry_source::pos_rule_test>;
        using position_source_type = api::id::source<var_int32, api::id::registry_source::position_source_type>;
        using recipe_book_category = api::id::source<var_int32, api::id::registry_source::recipe_book_category>;
        using recipe_display = api::id::source<var_int32, api::id::registry_source::recipe_display>;
        using recipe_serializer = api::id::source<var_int32, api::id::registry_source::recipe_serializer>;
        using recipe_type = api::id::source<var_int32, api::id::registry_source::recipe_type>;
        using rule_block_entity_modifier = api::id::source<var_int32, api::id::registry_source::rule_block_entity_modifier>;
        using rule_test = api::id::source<var_int32, api::id::registry_source::rule_test>;
        using schedule = api::id::source<var_int32, api::id::registry_source::schedule>;
        using sensor_type = api::id::source<var_int32, api::id::registry_source::sensor_type>;
        using slot_display = api::id::source<var_int32, api::id::registry_source::slot_display>;
        using sound_event = api::id::source<var_int32, api::id::registry_source::sound_event>;
        using spawn_condition_type = api::id::source<var_int32, api::id::registry_source::spawn_condition_type>;
        using stat_type = api::id::source<var_int32, api::id::registry_source::stat_type>;
        using test_environment_definition_type = api::id::source<var_int32, api::id::registry_source::test_environment_definition_type>;
        using test_function = api::id::source<var_int32, api::id::registry_source::test_function>;
        using test_instance_type = api::id::source<var_int32, api::id::registry_source::test_instance_type>;
        using ticket_type = api::id::source<var_int32, api::id::registry_source::ticket_type>;
        using trigger_type = api::id::source<var_int32, api::id::registry_source::trigger_type>;
        using villager_profession = api::id::source<var_int32, api::id::registry_source::villager_profession>;
        using villager_type = api::id::source<var_int32, api::id::registry_source::villager_type>;
        using worldgen__biome_source = api::id::source<var_int32, api::id::registry_source::worldgen__biome_source>;
        using worldgen__block_state_provider_type = api::id::source<var_int32, api::id::registry_source::worldgen__block_state_provider_type>;
        using worldgen__carver = api::id::source<var_int32, api::id::registry_source::worldgen__carver>;
        using worldgen__chunk_generator = api::id::source<var_int32, api::id::registry_source::worldgen__chunk_generator>;
        using worldgen__density_function_type = api::id::source<var_int32, api::id::registry_source::worldgen__density_function_type>;
        using worldgen__feature = api::id::source<var_int32, api::id::registry_source::worldgen__feature>;
        using worldgen__feature_size_type = api::id::source<var_int32, api::id::registry_source::worldgen__feature_size_type>;
        using worldgen__foliage_placer_type = api::id::source<var_int32, api::id::registry_source::worldgen__foliage_placer_type>;
        using worldgen__material_condition = api::id::source<var_int32, api::id::registry_source::worldgen__material_condition>;
        using worldgen__material_rule = api::id::source<var_int32, api::id::registry_source::worldgen__material_rule>;
        using worldgen__placement_modifier_type = api::id::source<var_int32, api::id::registry_source::worldgen__placement_modifier_type>;
        using worldgen__pool_alias_binding = api::id::source<var_int32, api::id::registry_source::worldgen__pool_alias_binding>;
        using worldgen__root_placer_type = api::id::source<var_int32, api::id::registry_source::worldgen__root_placer_type>;
        using worldgen__structure_piece = api::id::source<var_int32, api::id::registry_source::worldgen__structure_piece>;
        using worldgen__structure_placement = api::id::source<var_int32, api::id::registry_source::worldgen__structure_placement>;
        using worldgen__structure_pool_element = api::id::source<var_int32, api::id::registry_source::worldgen__structure_pool_element>;
        using worldgen__structure_processor = api::id::source<var_int32, api::id::registry_source::worldgen__structure_processor>;
        using worldgen__structure_type = api::id::source<var_int32, api::id::registry_source::worldgen__structure_type>;
        using worldgen__tree_decorator_type = api::id::source<var_int32, api::id::registry_source::worldgen__tree_decorator_type>;
        using worldgen__trunk_placer_type = api::id::source<var_int32, api::id::registry_source::worldgen__trunk_placer_type>;

        using block_state = api::id::source<var_int32, api::id::registry_source::block_state>;
        using motive = api::id::source<var_int32, api::id::registry_source::motive>;
        using entity_pose = api::id::source<var_int32, api::id::registry_source::entity_pose>;
        using entity_id = api::id::source<var_int32, api::id::registry_source::entity_id>;


        int32_t value = 0;

        constexpr var_int32() {}

        template <enum_concept T>
        constexpr var_int32(T value) noexcept : value((int32_t)value) {}

        constexpr var_int32(int32_t value) noexcept : value(value) {}

        constexpr var_int32(var_int32&& value) noexcept : value(value.value) {}

        constexpr var_int32(const var_int32& value) noexcept : value(value.value) {}

        constexpr var_int32& operator=(var_int32&& other) noexcept {
            value = std::move(other.value);
            return *this;
        }

        constexpr var_int32& operator=(const var_int32& other) noexcept {
            value = other.value;
            return *this;
        }

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
        constexpr var_int64(T value) noexcept : value((int64_t)value) {}

        constexpr var_int64(int64_t value) noexcept : value(value) {}

        constexpr var_int64(var_int64&& value) noexcept : value(value.value) {}

        constexpr var_int64(const var_int64& value) noexcept : value(value.value) {}

        constexpr var_int64& operator=(var_int64&& other) noexcept {
            value = std::move(other.value);
            return *this;
        }

        constexpr var_int64& operator=(const var_int64& other) noexcept {
            value = other.value;
            return *this;
        }

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
        using underlying_type = int32_t;
        using banner_pattern = api::id::source<optional_var_int32, api::id::registry_source::banner_pattern>;
        using cat_variant = api::id::source<optional_var_int32, api::id::registry_source::cat_variant>;
        using chat_type = api::id::source<optional_var_int32, api::id::registry_source::chat_type>;
        using chicken_variant = api::id::source<optional_var_int32, api::id::registry_source::chicken_variant>;
        using cow_variant = api::id::source<optional_var_int32, api::id::registry_source::cow_variant>;
        using damage_type = api::id::source<optional_var_int32, api::id::registry_source::damage_type>;
        using dialog = api::id::source<optional_var_int32, api::id::registry_source::dialog>;
        using dimension_type = api::id::source<optional_var_int32, api::id::registry_source::dimension_type>;
        using enchantment = api::id::source<optional_var_int32, api::id::registry_source::enchantment>;
        using enchantment_provider = api::id::source<optional_var_int32, api::id::registry_source::enchantment_provider>;
        using frog_variant = api::id::source<optional_var_int32, api::id::registry_source::frog_variant>;
        using instrument = api::id::source<optional_var_int32, api::id::registry_source::instrument>;
        using jukebox_song = api::id::source<optional_var_int32, api::id::registry_source::jukebox_song>;
        using loot_table = api::id::source<optional_var_int32, api::id::registry_source::loot_table>;
        using painting_variant = api::id::source<optional_var_int32, api::id::registry_source::painting_variant>;
        using pig_variant = api::id::source<optional_var_int32, api::id::registry_source::pig_variant>;
        using recipe = api::id::source<optional_var_int32, api::id::registry_source::recipe>;
        using trim_material = api::id::source<optional_var_int32, api::id::registry_source::trim_material>;
        using trim_pattern = api::id::source<optional_var_int32, api::id::registry_source::trim_pattern>;
        using wolf_sound_variant = api::id::source<optional_var_int32, api::id::registry_source::wolf_sound_variant>;
        using wolf_variant = api::id::source<optional_var_int32, api::id::registry_source::wolf_variant>;
        using worldgen__biome = api::id::source<optional_var_int32, api::id::registry_source::worldgen__biome>;
        using block_type = api::id::source<optional_var_int32, api::id::registry_source::block_type>;
        using block_entity_type = api::id::source<optional_var_int32, api::id::registry_source::block_entity_type>;
        using dimension = api::id::source<optional_var_int32, api::id::registry_source::dimension>;
        using entity_type = api::id::source<optional_var_int32, api::id::registry_source::entity_type>;
        using fluid = api::id::source<optional_var_int32, api::id::registry_source::fluid>;
        using game_event = api::id::source<optional_var_int32, api::id::registry_source::game_event>;
        using item = api::id::source<optional_var_int32, api::id::registry_source::item>;
        using potion = api::id::source<optional_var_int32, api::id::registry_source::potion>;
        using villager_variant = api::id::source<optional_var_int32, api::id::registry_source::villager_variant>;
        using fox_variant = api::id::source<optional_var_int32, api::id::registry_source::fox_variant>;
        using parrot_variant = api::id::source<optional_var_int32, api::id::registry_source::parrot_variant>;
        using tropical_fish_pattern = api::id::source<optional_var_int32, api::id::registry_source::tropical_fish_pattern>;
        using mooshroom_variant = api::id::source<optional_var_int32, api::id::registry_source::mooshroom_variant>;
        using rabbit_variant = api::id::source<optional_var_int32, api::id::registry_source::rabbit_variant>;
        using horse_variant = api::id::source<optional_var_int32, api::id::registry_source::horse_variant>;
        using llama_variant = api::id::source<optional_var_int32, api::id::registry_source::llama_variant>;
        using axolotl_variant = api::id::source<optional_var_int32, api::id::registry_source::axolotl_variant>;
        using trial_spawner_config = api::id::source<optional_var_int32, api::id::registry_source::trial_spawner_config>;

        using activity = api::id::source<optional_var_int32, api::id::registry_source::activity>;
        using attribute = api::id::source<optional_var_int32, api::id::registry_source::attribute>;
        using block_predicate_type = api::id::source<optional_var_int32, api::id::registry_source::block_predicate_type>;
        using chunk_status = api::id::source<optional_var_int32, api::id::registry_source::chunk_status>;
        using command_argument_type = api::id::source<optional_var_int32, api::id::registry_source::command_argument_type>;
        using consume_effect_type = api::id::source<optional_var_int32, api::id::registry_source::consume_effect_type>;
        using creative_mode_tab = api::id::source<optional_var_int32, api::id::registry_source::creative_mode_tab>;
        using custom_stat = api::id::source<optional_var_int32, api::id::registry_source::custom_stat>;
        using data_component_predicate_type = api::id::source<optional_var_int32, api::id::registry_source::data_component_predicate_type>;
        using data_component_type = api::id::source<optional_var_int32, api::id::registry_source::data_component_type>;
        using debug_subscription = api::id::source<optional_var_int32, api::id::registry_source::debug_subscription>;
        using decorated_pot_pattern = api::id::source<optional_var_int32, api::id::registry_source::decorated_pot_pattern>;
        using dialog_action_type = api::id::source<optional_var_int32, api::id::registry_source::dialog_action_type>;
        using dialog_body_type = api::id::source<optional_var_int32, api::id::registry_source::dialog_body_type>;
        using dialog_type = api::id::source<optional_var_int32, api::id::registry_source::dialog_type>;
        using enchantment_effect_component_type = api::id::source<optional_var_int32, api::id::registry_source::enchantment_effect_component_type>;
        using enchantment_entity_effect_type = api::id::source<optional_var_int32, api::id::registry_source::enchantment_entity_effect_type>;
        using enchantment_level_based_value_type = api::id::source<optional_var_int32, api::id::registry_source::enchantment_level_based_value_type>;
        using enchantment_location_based_effect_type = api::id::source<optional_var_int32, api::id::registry_source::enchantment_location_based_effect_type>;
        using enchantment_provider_type = api::id::source<optional_var_int32, api::id::registry_source::enchantment_provider_type>;
        using enchantment_value_effect_type = api::id::source<optional_var_int32, api::id::registry_source::enchantment_value_effect_type>;
        using entity_sub_predicate_type = api::id::source<optional_var_int32, api::id::registry_source::entity_sub_predicate_type>;
        using float_provider_type = api::id::source<optional_var_int32, api::id::registry_source::float_provider_type>;
        using height_provider_type = api::id::source<optional_var_int32, api::id::registry_source::height_provider_type>;
        using incoming_rpc_methods = api::id::source<optional_var_int32, api::id::registry_source::incoming_rpc_methods>;
        using input_control_type = api::id::source<optional_var_int32, api::id::registry_source::input_control_type>;
        using int_provider_type = api::id::source<optional_var_int32, api::id::registry_source::int_provider_type>;
        using loot_condition_type = api::id::source<optional_var_int32, api::id::registry_source::loot_condition_type>;
        using loot_function_type = api::id::source<optional_var_int32, api::id::registry_source::loot_function_type>;
        using loot_nbt_provider_type = api::id::source<optional_var_int32, api::id::registry_source::loot_nbt_provider_type>;
        using loot_number_provider_type = api::id::source<optional_var_int32, api::id::registry_source::loot_number_provider_type>;
        using loot_pool_entry_type = api::id::source<optional_var_int32, api::id::registry_source::loot_pool_entry_type>;
        using loot_score_provider_type = api::id::source<optional_var_int32, api::id::registry_source::loot_score_provider_type>;
        using map_decoration_type = api::id::source<optional_var_int32, api::id::registry_source::map_decoration_type>;
        using memory_module_type = api::id::source<optional_var_int32, api::id::registry_source::memory_module_type>;
        using menu = api::id::source<optional_var_int32, api::id::registry_source::menu>;
        using mob_effect = api::id::source<optional_var_int32, api::id::registry_source::mob_effect>;
        using number_format_type = api::id::source<optional_var_int32, api::id::registry_source::number_format_type>;
        using outgoing_rpc_methods = api::id::source<optional_var_int32, api::id::registry_source::outgoing_rpc_methods>;
        using particle_type = api::id::source<optional_var_int32, api::id::registry_source::particle_type>;
        using point_of_interest_type = api::id::source<optional_var_int32, api::id::registry_source::point_of_interest_type>;
        using pos_rule_test = api::id::source<optional_var_int32, api::id::registry_source::pos_rule_test>;
        using position_source_type = api::id::source<optional_var_int32, api::id::registry_source::position_source_type>;
        using recipe_book_category = api::id::source<optional_var_int32, api::id::registry_source::recipe_book_category>;
        using recipe_display = api::id::source<optional_var_int32, api::id::registry_source::recipe_display>;
        using recipe_serializer = api::id::source<optional_var_int32, api::id::registry_source::recipe_serializer>;
        using recipe_type = api::id::source<optional_var_int32, api::id::registry_source::recipe_type>;
        using rule_block_entity_modifier = api::id::source<optional_var_int32, api::id::registry_source::rule_block_entity_modifier>;
        using rule_test = api::id::source<optional_var_int32, api::id::registry_source::rule_test>;
        using schedule = api::id::source<optional_var_int32, api::id::registry_source::schedule>;
        using sensor_type = api::id::source<optional_var_int32, api::id::registry_source::sensor_type>;
        using slot_display = api::id::source<optional_var_int32, api::id::registry_source::slot_display>;
        using sound_event = api::id::source<optional_var_int32, api::id::registry_source::sound_event>;
        using spawn_condition_type = api::id::source<optional_var_int32, api::id::registry_source::spawn_condition_type>;
        using stat_type = api::id::source<optional_var_int32, api::id::registry_source::stat_type>;
        using test_environment_definition_type = api::id::source<optional_var_int32, api::id::registry_source::test_environment_definition_type>;
        using test_function = api::id::source<optional_var_int32, api::id::registry_source::test_function>;
        using test_instance_type = api::id::source<optional_var_int32, api::id::registry_source::test_instance_type>;
        using ticket_type = api::id::source<optional_var_int32, api::id::registry_source::ticket_type>;
        using trigger_type = api::id::source<optional_var_int32, api::id::registry_source::trigger_type>;
        using villager_profession = api::id::source<optional_var_int32, api::id::registry_source::villager_profession>;
        using villager_type = api::id::source<optional_var_int32, api::id::registry_source::villager_type>;
        using worldgen__biome_source = api::id::source<optional_var_int32, api::id::registry_source::worldgen__biome_source>;
        using worldgen__block_state_provider_type = api::id::source<optional_var_int32, api::id::registry_source::worldgen__block_state_provider_type>;
        using worldgen__carver = api::id::source<optional_var_int32, api::id::registry_source::worldgen__carver>;
        using worldgen__chunk_generator = api::id::source<optional_var_int32, api::id::registry_source::worldgen__chunk_generator>;
        using worldgen__density_function_type = api::id::source<optional_var_int32, api::id::registry_source::worldgen__density_function_type>;
        using worldgen__feature = api::id::source<optional_var_int32, api::id::registry_source::worldgen__feature>;
        using worldgen__feature_size_type = api::id::source<optional_var_int32, api::id::registry_source::worldgen__feature_size_type>;
        using worldgen__foliage_placer_type = api::id::source<optional_var_int32, api::id::registry_source::worldgen__foliage_placer_type>;
        using worldgen__material_condition = api::id::source<optional_var_int32, api::id::registry_source::worldgen__material_condition>;
        using worldgen__material_rule = api::id::source<optional_var_int32, api::id::registry_source::worldgen__material_rule>;
        using worldgen__placement_modifier_type = api::id::source<optional_var_int32, api::id::registry_source::worldgen__placement_modifier_type>;
        using worldgen__pool_alias_binding = api::id::source<optional_var_int32, api::id::registry_source::worldgen__pool_alias_binding>;
        using worldgen__root_placer_type = api::id::source<optional_var_int32, api::id::registry_source::worldgen__root_placer_type>;
        using worldgen__structure_piece = api::id::source<optional_var_int32, api::id::registry_source::worldgen__structure_piece>;
        using worldgen__structure_placement = api::id::source<optional_var_int32, api::id::registry_source::worldgen__structure_placement>;
        using worldgen__structure_pool_element = api::id::source<optional_var_int32, api::id::registry_source::worldgen__structure_pool_element>;
        using worldgen__structure_processor = api::id::source<optional_var_int32, api::id::registry_source::worldgen__structure_processor>;
        using worldgen__structure_type = api::id::source<optional_var_int32, api::id::registry_source::worldgen__structure_type>;
        using worldgen__tree_decorator_type = api::id::source<optional_var_int32, api::id::registry_source::worldgen__tree_decorator_type>;
        using worldgen__trunk_placer_type = api::id::source<optional_var_int32, api::id::registry_source::worldgen__trunk_placer_type>;

        using block_state = api::id::source<optional_var_int32, api::id::registry_source::block_state>;
        using motive = api::id::source<optional_var_int32, api::id::registry_source::motive>;
        using entity_pose = api::id::source<optional_var_int32, api::id::registry_source::entity_pose>;
        using entity_id = api::id::source<optional_var_int32, api::id::registry_source::entity_id>;

        using std::optional<int32_t>::optional;
        using std::optional<int32_t>::operator=;

        operator int32_t() const {
            if (has_value())
                return value();
            else
                return 0;
        }
    };

    struct optional_var_int64 : public std::optional<int64_t> { //encoded same as var_int64 but if set the value incremented and checked for overflow, if not set encoded as 0
        using underlying_type = int32_t;
        using std::optional<int64_t>::optional;
        using std::optional<int64_t>::operator=;

        operator int64_t() const {
            if (has_value())
                return value();
            else
                return 0;
        }
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

        constexpr depends_next(depends_next&& value) noexcept : value(std::move(value.value)) {}

        constexpr depends_next& operator=(const depends_next& other) {
            value = other.value;
            return *this;
        }

        constexpr depends_next& operator=(depends_next&& other) noexcept {
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

    size_t get_size_source_value(base_objects::shared_client_data&, size_source);

    //this type provides way to get size of array while decoding, the values would also be checked to be equal to size of the container
    template <auto... DependedValues>
    struct no_size {
        template <class T>
        static size_t get_depended_size(base_objects::shared_client_data& context, const T& val) {
            static auto get_value = [](base_objects::shared_client_data& context, const T& val, auto&& it) -> size_t {
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
        using list_array<T>::operator=;

        list_array_sized() : list_array<T>() {}

        list_array_sized(list_array<T>&& mov) noexcept : list_array<T>(std::move(mov)) {}

        list_array_sized(const list_array<T>& copy) : list_array<T>(copy) {}

        static constexpr inline size_t max_size = size;

        auto operator<=>(const list_array_sized& other) const = default;
    };

    template <class T, size_t size, auto... DependedValues>
    struct list_array_sized_no_size : public no_size<DependedValues...>, list_array_sized<T, size> {
        using list_array_sized<T, size>::list_array_sized;
        using list_array_sized<T, size>::operator=;

        list_array_sized_no_size() : list_array_sized<T, size>() {}

        list_array_sized_no_size(list_array<T>&& mov) noexcept : list_array_sized<T, size>(std::move(mov)) {}

        list_array_sized_no_size(const list_array<T>& copy) : list_array_sized<T, size>(copy) {}

        static constexpr inline size_t max_size = size;

        auto operator<=>(const list_array_sized_no_size& other) const = default;
    };

    template <class T, auto... DependedValues>
    struct list_array_no_size : public no_size<DependedValues...>, list_array<T> {
        using list_array<T>::list_array;
        using list_array<T>::operator=;

        list_array_no_size() : list_array<T>() {}

        list_array_no_size(list_array<T>&& mov) noexcept : list_array<T>(std::move(mov)) {}

        list_array_no_size(const list_array<T>& copy) : list_array<T>(copy) {}

        auto operator<=>(const list_array_no_size& other) const = default;
    };

    template <class T, size_t size>
    struct list_array_sized_siz_from_packet : public size_from_packet, list_array_sized<T, size> {
        using list_array_sized<T, size>::list_array_sized;
        using list_array_sized<T, size>::operator=;
        list_array_sized_siz_from_packet(): list_array_sized<T, size>() {}

        list_array_sized_siz_from_packet(list_array<T>&& mov) noexcept : list_array_sized<T, size>(std::move(mov)) {}

        list_array_sized_siz_from_packet(const list_array<T>& copy) : list_array_sized<T, size>(copy) {}

        static constexpr inline size_t max_size = size;


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
        using list_array<T>::list_array;
        using list_array<T>::operator=;

        list_array_fixed(): list_array<T>() {}
        list_array_fixed(const list_array<T>& copy) : list_array<T>(copy) {}

        list_array_fixed(list_array<T>&& mov) noexcept : list_array<T>(std::move(mov)) {}

        static constexpr inline size_t required_size = size;
        auto operator<=>(const list_array_fixed& other) const = default;
    };

    template <class T>
    struct list_array_siz_from_packet : public size_from_packet, list_array<T> {
        using list_array<T>::list_array;
        using list_array<T>::operator=;

        list_array_siz_from_packet(): list_array<T>() {}
        list_array_siz_from_packet(const list_array<T>& copy) : list_array<T>(copy) {}

        list_array_siz_from_packet(list_array<T>&& mov) noexcept : list_array<T>(std::move(mov)) {}

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
        using list_array<T>::operator=;

        list_array_depend() : list_array<T>() {}

        list_array_depend(const list_array<T>& copy) : list_array<T>(copy) {}

        list_array_depend(list_array<T>&& mov) noexcept : list_array<T>(std::move(mov)) {}

        bool decoding_flag = false;


        auto operator<=>(const list_array_depend& other) const = default;
    };

    template <class Variant0, class Variant1>
    struct or_ : public std::variant<Variant0, Variant1> {
        using var_0 = Variant0;
        using var_1 = Variant1;
        using base = std::variant<Variant0, Variant1>;

        or_() : base() {}

        or_(const base& v) : base(v) {}

        or_(base&& v) noexcept : base(std::move(v)) {}

        or_(const or_& v) : base((const base&)v) {}

        or_(or_&& v) noexcept : base(std::move((base&)v)) {}

        or_(var_0&& v) : base(std::move(v)) {}

        or_(var_1&& v) : base(std::move(v)) {}

        or_(const var_0& v) : base(v) {}

        or_(const var_1& v) : base(v) {}

        or_& operator=(const base& v) {
            (base&)* this = v;
            return *this;
        }

        or_& operator=(base&& v) noexcept {
            (base&)* this = std::move(v);
            return *this;
        }

        or_& operator=(const or_& v) {
            (base&)* this = (const base&)v;
            return *this;
        }

        or_& operator=(or_&& v) noexcept {
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

        Angle(Angle&& value) noexcept = default;

        Angle& operator=(const Angle& v) {
            value = v.value;
            return *this;
        }

        Angle& operator=(Angle&& v) noexcept {
            value = v.value;
            return *this;
        }

        explicit operator double() {
            return (value * 360.0) / (3.14159265358979323846 * 2);
        }

        auto operator<=>(const Angle& other) const = default;
    };

    template <class Enum, class T = int>
    struct enum_as {
        using encode_t = T;
        using enum_t = Enum;
        Enum value;

        constexpr enum_as() : value() {}

        constexpr enum_as(enum_as&&) noexcept = default;
        constexpr enum_as(const enum_as&) noexcept = default;
        constexpr enum_as& operator=(enum_as&&) noexcept = default;
        constexpr enum_as& operator=(const enum_as&) noexcept = default;

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

    template <class ValueType, class... Ty>
    struct enum_switch : public std::variant<Ty...> {
        static_assert(internal::count_default_items<Ty...>::value <= 1, "enum_switch can have at most one default item");
        static constexpr inline bool is_correct = is_correct_variant<Ty...>();

        using default_item = typename internal::find_default_item<Ty...>::type;
        using encode_type = ValueType;
        using base = std::variant<Ty...>;

        enum_switch() : base() {}

        enum_switch(const enum_switch& v) {
            *this = v;
        }

        enum_switch(enum_switch&& v) noexcept {
            *this = std::move(v);
        }

        enum_switch(std::variant<Ty...>&& v) noexcept {
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
        enum_switch& operator=(enum_switch&& v) noexcept;
        enum_switch& operator=(std::variant<Ty...>&& v) noexcept;
        enum_switch& operator=(const std::variant<Ty...>& v);
        enum_switch& operator=(std::convertible_to<std::variant<Ty...>> auto&& v) noexcept;
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

        template <class FN>
        constexpr static void get_default(FN&& fn) {
            if constexpr (!std::is_same_v<default_item, void>)
                fn.template operator()<default_item>();
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
    enum_switch<ValueType, Ty...>& enum_switch<ValueType, Ty...>::operator=(enum_switch<ValueType, Ty...>&& v) noexcept {
        (base&)* this = std::move((base&)v);
        return *this;
    }

    template <class ValueType, class... Ty>
    enum_switch<ValueType, Ty...>& enum_switch<ValueType, Ty...>::operator=(std::variant<Ty...>&& v) noexcept {
        (base&)* this = std::move(v);
        return *this;
    }

    template <class ValueType, class... Ty>
    enum_switch<ValueType, Ty...>& enum_switch<ValueType, Ty...>::operator=(const std::variant<Ty...>& v) {
        (base&)* this = v;
        return *this;
    }

    template <class ValueType, class... Ty>
    enum_switch<ValueType, Ty...>& enum_switch<ValueType, Ty...>::operator=(std::convertible_to<std::variant<Ty...>> auto&& v) noexcept {
        (base&)* this = std::move(v);
        return *this;
    }

    template <class ValueType, class... Ty>
    enum_switch<ValueType, Ty...>& enum_switch<ValueType, Ty...>::operator=(const std::convertible_to<std::variant<Ty...>> auto& v) {
        (base&)* this = v;
        return *this;
    }

    template <class header, class... Ty>
    struct enum_set {
        std::tuple<list_array<header>, list_array<Ty>...> values;
        using header_t = header;

        template <class T>
        void push(T&& mov) {
            std::get<list_array<std::decay_t<T>>>(values).push_back(std::move(mov));
        }

        template <class T>
        void push(const T& mov) {
            std::get<list_array<std::decay_t<T>>>(values).push_back(mov);
        }

        template <class T>
        void push() {
            std::get<list_array<std::decay_t<T>>>(values).push_back(T{});
        }

        template <class T>
        bool has() const {
            return std::get<list_array<std::decay_t<T>>>(values).size();
        }

        template <class T>
        list_array<T>& get() & {
            return std::get<list_array<std::decay_t<T>>>(values);
        }

        template <class T>
        const list_array<T>& get() const& {
            return std::get<list_array<std::decay_t<T>>>(values);
        }

        template <class T>
        list_array<T> get() && {
            return std::move(std::get<list_array<std::decay_t<T>>>(values));
        }
    };

    template <class flag_type, class... Ty>
    struct flags_list {
        using max_orders = ic<std::max<ptrdiff_t>({Ty::flag_order::value...})>;
        using base = std::variant<Ty...>;
        static constexpr inline bool is_correct = is_correct_variant<Ty...>();
        flag_type flag;
        std::unordered_map<ptrdiff_t, std::variant<Ty...>> values; //order->value

        flags_list() {}

        flags_list(flags_list&&) noexcept = default;
        flags_list(const flags_list&) = default;
        flags_list& operator=(flags_list&&) noexcept = default;
        flags_list& operator=(const flags_list&) = default;

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
        static constexpr inline bool is_correct = is_correct_variant<Ty...>();
        std::unordered_map<ptrdiff_t, std::variant<Ty...>> values; //flag_order->value

        flags_list_from() {}

        flags_list_from(flags_list_from&&) noexcept = default;
        flags_list_from(const flags_list_from&) = default;
        flags_list_from& operator=(flags_list_from&&) noexcept = default;
        flags_list_from& operator=(const flags_list_from&) = default;

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

        source_type get_flags() const {
            source_type res{0};
            for (auto& [id, value] : values) {
                std::visit(
                    [&]<class T>(T& it) {
                        res |= (T::flag_value::value & T::flag_mask::value);
                    },
                    value
                );
            }
            return res;
        }

        void preprocess(Source& source) {
            source.*source_name = get_flags();
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

    template <class Enum, class T = int>
    struct enum_as_flag {
        using encode_t = T;
        using enum_t = Enum;
        Enum value;

        constexpr enum_as_flag() : value() {}

        constexpr enum_as_flag(enum_as_flag&&) noexcept = default;
        constexpr enum_as_flag(const enum_as_flag&) = default;

        constexpr enum_as_flag(Enum e) : value(e) {}

        constexpr enum_as_flag(T e) : value((Enum)e) {}

        constexpr T get() const {
            return (T)value;
        }

        constexpr enum_as_flag& operator=(enum_as_flag&&) noexcept = default;
        constexpr enum_as_flag& operator=(const enum_as_flag&) = default;

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

    template <class base_type_, class... Ty>
    struct any_of {
        using base_type = base_type_;
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
        static constexpr inline bool is_correct = is_correct_variant<value_type, Ty...>();
        using encode_type = value_type;
        using base = std::variant<value_type, Ty...>;

        partial_enum_switch() : base() {}

        partial_enum_switch(const base& v) : base(v) {}

        partial_enum_switch(base&& v) noexcept : base(std::move(v)) {}

        partial_enum_switch(const partial_enum_switch& v) : base((const base&)v) {}

        partial_enum_switch(partial_enum_switch&& v) noexcept : base(std::move((base&)v)) {}

        template <std::constructible_from<base> T>
        partial_enum_switch(T&& v) noexcept : base(std::move(v)) {}

        template <std::constructible_from<base> T>
        partial_enum_switch(const T& v) : base(v) {}

        partial_enum_switch& operator=(const base& v) {
            (base&)* this = v;
            return *this;
        }

        partial_enum_switch& operator=(base&& v) noexcept {
            (base&)* this = std::move(v);
            return *this;
        }

        partial_enum_switch& operator=(const partial_enum_switch& v) {
            (base&)* this = (const base&)v;
            return *this;
        }

        partial_enum_switch& operator=(partial_enum_switch&& v) noexcept {
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

        ignored(ignored&& it) noexcept : value(std::move(it.value)) {}

        ignored(const ignored& it) : value(it.value) {}

        ignored(T&& it) noexcept : value(std::move(it)) {}

        ignored(const T& it) : value(it) {}

        ignored& operator=(T&& v) noexcept {
            value = std::move(v);
            return *this;
        }

        ignored& operator=(const T& v) {
            value = v;
            return *this;
        }

        ignored& operator=(ignored&& v) noexcept {
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

    template <class T, util::CTS id>
    struct ordered_id {
        using value_type = T;
        static inline const std::string id_source{id.data};
        T value;
        bool is_valid = true;

        constexpr ordered_id() : value() {}

        constexpr ordered_id(T e) : value(e) {}

        constexpr ordered_id(const ordered_id& c) : value(c.value), is_valid(c.is_valid) {}

        constexpr ordered_id(ordered_id&& m) noexcept : value(std::move(m.value)), is_valid(m.is_valid) {}

        constexpr ordered_id& operator=(const ordered_id& c) {
            value = c.value;
            is_valid = c.is_valid;
            return *this;
        }

        constexpr ordered_id& operator=(ordered_id&& m) noexcept {
            value = m.value;
            is_valid = m.is_valid;
            return *this;
        }

        constexpr auto operator<=>(const ordered_id& other) const = default;

        constexpr operator bool() const {
            return value;
        }
    };
}

namespace copper_server::base_objects {

    template <auto value>
    using constant_value = api::packets::constant_value<value>;

    template <int32_t value>
    using enum_item = api::packets::enum_item<value>;

    template <int32_t value>
    using default_enum_item = api::packets::default_enum_item<value>;
    template <size_t value, size_t mask, ptrdiff_t order>
    using flag_item = api::packets::flag_item<value, mask, order>;

    template <class T, class R>
    static constexpr bool could_be_preprocessed = api::packets::could_be_preprocessed<T, R>;

    template <class type>
    concept is_convertible_to_packet_form = api::packets::is_convertible_to_packet_form<type>;

    template <is_convertible_to_packet_form type>
    using convertible_to_packet_type = api::packets::convertible_to_packet_type<type>;

    using identifier = api::packets::identifier;
    using degrees = api::packets::degrees;

    template <size_t size>
    using string_sized = api::packets::string_sized<size>;
    using json_text_component = api::packets::json_text_component;
    template <class T, T min, T max>
    using limited_num = api::packets::limited_num<T, min, max>;

    using var_int32 = api::packets::var_int32;
    using var_int64 = api::packets::var_int64;
    using optional_var_int32 = api::packets::optional_var_int32;
    using optional_var_int64 = api::packets::optional_var_int64;

    template <class Value, class T>
    using value_optional = api::packets::value_optional<Value, T>;
    //if value would be zero, next fields ignored
    template <class Value>
    using depends_next = api::packets::depends_next<Value>;

    using size_source = api::packets::size_source;

    template <auto... DependedValues>
    using no_size = api::packets::no_size<DependedValues...>;

    template <class T, size_t size>
    using list_array_sized = api::packets::list_array_sized<T, size>;

    template <class T, size_t size, auto... DependedValues>
    using list_array_sized_no_size = api::packets::list_array_sized_no_size<T, size, DependedValues...>;

    template <class T, auto... DependedValues>
    using list_array_no_size = api::packets::list_array_no_size<T, DependedValues...>;

    template <class T, size_t size>
    using list_array_sized_siz_from_packet = api::packets::list_array_sized_siz_from_packet<T, size>;

    template <class T, class T_size>
    using sized_entry = api::packets::sized_entry<T, T_size>;

    template <class T, size_t size>
    using list_array_fixed = api::packets::list_array_fixed<T, size>;
    template <class T>
    using list_array_siz_from_packet = api::packets::list_array_siz_from_packet<T>;

    template <size_t size>
    using bitset_fixed = api::packets::bitset_fixed<size>;

    template <class T, T flag, auto depend_prev_class>
    using item_depend = api::packets::item_depend<T, flag, depend_prev_class>;

    template <class T>
    concept is_item_depend = api::packets::is_item_depend<T>;

    template <class T>
    concept struct_depends = api::packets::struct_depends<T>;

    template <struct_depends T>
    using list_array_depend = api::packets::list_array_depend<T>;

    template <class Variant0, class Variant1>
    using or_ = api::packets::or_<Variant0, Variant1>;

    template <class Variant0, class Variant1>
    using bool_or = api::packets::bool_or<Variant0, Variant1>;

    template <class T>
    using packet_compress = api::packets::packet_compress<T>;

    template <class T>
    using id_set = api::packets::id_set<T>;

    using Angle = api::packets::Angle;

    template <class Enum, class T = int>
    using enum_as = api::packets::enum_as<Enum, T>;

    template <class ValueType, class... Ty>
    using enum_switch = api::packets::enum_switch<ValueType, Ty...>;

    template <class header, class... Ty>
    using enum_set = api::packets::enum_set<header, Ty...>;

    template <class flag_type, class... Ty>
    using flags_list = api::packets::flags_list<flag_type, Ty...>;

    template <class Source, class SourceType, SourceType Source::* source_name, class... Ty>
    using flags_list_from = api::packets::flags_list_from<Source, SourceType, source_name, Ty...>;

    template <class Enum, class T = int>
    using enum_as_flag = api::packets::enum_as_flag<Enum, T>;

    template <class base_type, class... Ty>
    using any_of = api::packets::any_of<base_type, Ty...>;

    template <class value_type, class... Ty>
    using partial_enum_switch = api::packets::partial_enum_switch<value_type, Ty...>;

    template <class T>
    using ignored = api::packets::ignored<T>;

    template <class T, util::CTS id>
    using ordered_id = api::packets::ordered_id<T, id>;
}

copper_server::base_objects::shared_client_data& operator<<(copper_server::base_objects::shared_client_data& client, copper_server::api::packets::switches_to::status);
copper_server::base_objects::shared_client_data& operator<<(copper_server::base_objects::shared_client_data& client, copper_server::api::packets::switches_to::login);
copper_server::base_objects::shared_client_data& operator<<(copper_server::base_objects::shared_client_data& client, copper_server::api::packets::switches_to::config);
copper_server::base_objects::shared_client_data& operator<<(copper_server::base_objects::shared_client_data& client, copper_server::api::packets::switches_to::play);

namespace std {
    template <>
    struct hash<copper_server::api::packets::var_int32> {
        size_t operator()(const copper_server::api::packets::var_int32& value) const noexcept {
            return hash<int32_t>()(value.value);
        }
    };

    template <>
    struct hash<copper_server::api::packets::var_int64> {
        size_t operator()(const copper_server::api::packets::var_int64& value) const noexcept {
            return hash<int64_t>()(value.value);
        }
    };
}
#endif /* SRC_API_PACKETS_TYPES */
