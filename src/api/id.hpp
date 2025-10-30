/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_ID
#define SRC_API_ID
#include <cstdint>
#include <library/list_array.hpp>
#include <src/api/ecs.hpp>
#include <src/api/tags.hpp>
#include <string>
#include <variant>

namespace enbt {
    struct raw_uuid;
}

namespace copper_server::api::id {
    enum class registry_source {
        banner_pattern,
        cat_variant,
        chat_type,
        chicken_variant,
        cow_variant,
        damage_type,
        dialog,
        dimension_type,
        enchantment,
        enchantment_provider,
        frog_variant,
        instrument,
        jukebox_song,
        loot_table,
        painting_variant,
        pig_variant,
        recipe,
        trim_material,
        trim_pattern,
        wolf_sound_variant,
        wolf_variant,
        worldgen__biome,
        block_type,
        block_entity_type,
        dimension,
        entity_type,
        fluid,
        game_event,
        item,
        potion,
        villager_variant,
        fox_variant,
        parrot_variant,
        tropical_fish_pattern,
        mooshroom_variant,
        rabbit_variant,
        horse_variant,
        llama_variant,
        axolotl_variant,


        activity,
        attribute,
        block_predicate_type,
        chunk_status,
        command_argument_type,
        consume_effect_type,
        creative_mode_tab,
        custom_stat,
        data_component_predicate_type,
        data_component_type,
        debug_subscription,
        decorated_pot_pattern,
        dialog_action_type,
        dialog_body_type,
        dialog_type,
        enchantment_effect_component_type,
        enchantment_entity_effect_type,
        enchantment_level_based_value_type,
        enchantment_location_based_effect_type,
        enchantment_provider_type,
        enchantment_value_effect_type,
        entity_sub_predicate_type,
        float_provider_type,
        height_provider_type,
        incoming_rpc_methods,
        input_control_type,
        int_provider_type,
        loot_condition_type,
        loot_function_type,
        loot_nbt_provider_type,
        loot_number_provider_type,
        loot_pool_entry_type,
        loot_score_provider_type,
        map_decoration_type,
        memory_module_type,
        menu,
        mob_effect,
        number_format_type,
        outgoing_rpc_methods,
        particle_type,
        point_of_interest_type,
        pos_rule_test,
        position_source_type,
        recipe_book_category,
        recipe_display,
        recipe_serializer,
        recipe_type,
        rule_block_entity_modifier,
        rule_test,
        schedule,
        sensor_type,
        slot_display,
        sound_event,
        spawn_condition_type,
        stat_type,
        test_environment_definition_type,
        test_function,
        test_instance_type,
        ticket_type,
        trigger_type,
        villager_profession,
        villager_type,
        worldgen__biome_source,
        worldgen__block_state_provider_type,
        worldgen__carver,
        worldgen__chunk_generator,
        worldgen__density_function_type,
        worldgen__feature,
        worldgen__feature_size_type,
        worldgen__foliage_placer_type,
        worldgen__material_condition,
        worldgen__material_rule,
        worldgen__placement_modifier_type,
        worldgen__pool_alias_binding,
        worldgen__root_placer_type,
        worldgen__structure_piece,
        worldgen__structure_placement,
        worldgen__structure_pool_element,
        worldgen__structure_processor,
        worldgen__structure_type,
        worldgen__tree_decorator_type,
        worldgen__trunk_placer_type,

        
        block_state,
        motive,
        entity_pose,
        entity_id,
    };

    namespace detail {
        std::string from_registry_source_value(registry_source source, int32_t value);
        int32_t to_registry_source_value(registry_source source, const std::string& value);
        api::tags::tag_handle to_registry_source_handle(registry_source source, std::string_view value);

        std::optional<api::ecs::entity> from_registry_source_entity(int32_t value);
        int32_t to_registry_source_entity(api::ecs::entity value);
        int32_t to_registry_source_entity(const enbt::raw_uuid& value);
        uint8_t to_registry_source_entity_index(int32_t value);

        list_array<int32_t> all_registry_source_value(registry_source source);
    }

    template <class T>
    concept _source_has_underlying_type = requires { typename T::underlying_type; };

    template <class From, class To>
    concept source_allow_cast = (_source_has_underlying_type<From> && std::convertible_to<From, typename To::underlying_type>)
                                || (!_source_has_underlying_type<From> && std::convertible_to<From, To>);

    template <class Value, registry_source sourc>
    struct source {
        using underlying_type = Value;
        using reg_source = std::integral_constant<registry_source, sourc>;
        Value value{};

        source() {}

#pragma warning(push)
#pragma warning(disable : 4244)

        template <source_allow_cast<Value> T>
        source(T value)
            : value(static_cast<Value>(value)) {}

#pragma warning(pop)

        source(Value value) : value(value) {}

        source(const source& value) : value(value.value) {}

        source(source&& value) noexcept
            : value(std::move(value.value)) {}

        source(std::string_view value) : value((Value)detail::to_registry_source_value(sourc, std::string(value))) {}

        source(const std::string& value) : value((Value)detail::to_registry_source_value(sourc, value)) {}

        template <size_t N>
        source(const char (&value)[N]) : value((Value)detail::to_registry_source_value(sourc, value)) {}

        source(const char* value) : value((Value)detail::to_registry_source_value(sourc, value)) {}

        source& operator=(const source& other) {
            value = other.value;
            return *this;
        }

        source& operator=(source&& other) noexcept {
            value = std::move(other.value);
            return *this;
        }

        operator Value&() {
            return value;
        }

        operator const Value&() const {
            return value;
        }

        std::string to_string() const {
            return detail::from_registry_source_value(sourc, value);
        }

        static list_array<source> look_all() {
            return detail::all_registry_source_value(sourc).convert_fn([](auto id) {
                return source(id);
            });
        }

        template <source_allow_cast<Value> T>
        operator T() const {
            if constexpr (requires { typename Value::underlying_type; })
                return (T)(typename Value::underlying_type)value;
            else
                return (T)value;
        }

        auto operator<=>(const source& other) const = default;
    };

    template <registry_source sourc>
    struct source_set {
        using value_type = std::variant<api::tags::tag_handle, int32_t>;
        using reg_source = std::integral_constant<registry_source, sourc>;
        value_type value;

        source_set() {}

        source_set(const source_set& value) : value(value.value) {}

        source_set(source_set&& value) noexcept
            : value(std::move(value.value)) {}

        source_set(std::string_view value) : value(value.starts_with('#') ? value_type(detail::to_registry_source_handle(sourc, value)) : value_type(detail::to_registry_source_value(sourc, std::string(value)))) {}

        source_set(const std::string& value) : value(value.starts_with('#') ? value_type(detail::to_registry_source_handle(sourc, value)) : value_type(detail::to_registry_source_value(sourc, value))) {}

        template <size_t N>
        source_set(const char (&value)[N]) : source_set(std::string_view(value, N)) {}

        source_set(const char* value) : source_set(std::string_view(value)) {}

        source_set& operator=(const source_set& other) {
            value = other.value;
            return *this;
        }

        source_set& operator=(source_set&& other) noexcept {
            value = std::move(other.value);
            return *this;
        }

        list_array<int32_t> view_ids() const {
            return std::visit(
                []<class T>(const T& it) -> list_array<int32_t> {
                    if constexpr (std::is_same_v<int32_t, T>)
                        return {it};
                    else
                        return api::tags::unfold_tag_ids(it);
                },
                value
            );
        }

        const list_array<int32_t>& view_ids_direct() const {
            return std::visit(
                []<class T>(const T& it) -> const list_array<int32_t>& {
                    if constexpr (std::is_same_v<int32_t, T>) {
                        static list_array<int32_t> empty;
                        return empty;
                    } else
                        return api::tags::unfold_tag_ids(it);
                },
                value
            );
        }

        bool contains(int32_t id) const {
            return std::visit(
                [id]<class T>(const T& it) {
                    if constexpr (std::is_same_v<int32_t, T>) {
                        return it == id;
                    } else
                        return api::tags::contains(it, id);
                },
                value
            );
        }

        bool contains() const {
            return std::visit(
                []<class T>(const T& it) {
                    if constexpr (std::is_same_v<int32_t, T>) {
                        return false;
                    } else
                        return api::tags::contains(it);
                },
                value
            );
        }

        auto operator<=>(const source_set& other) const = default;
    };

    template <class Value>
    struct source<Value, registry_source::entity_id> {
        using underlying_type = Value;
        using reg_source = std::integral_constant<registry_source, registry_source::entity_id>;
        Value value{};

        source() {}

        template <source_allow_cast<Value> T>
        source(T value)
            : value(static_cast<Value>(value)) {}

        source(Value value) : value(value) {}

        source(const source& value) : value(value.value) {}

        source(source&& value) noexcept
            : value(std::move(value.value)) {}

        source(std::string_view value) : value((Value)detail::to_registry_source_value(registry_source::entity_id, std::string(value))) {}

        source(const std::string& value) : value((Value)detail::to_registry_source_value(registry_source::entity_id, value)) {}

        template <size_t N>
        source(const char (&value)[N]) : value((Value)detail::to_registry_source_value(registry_source::entity_id, value)) {}

        source(const char* value) : value((Value)detail::to_registry_source_value(registry_source::entity_id, value)) {}

        source(api::ecs::entity value) : value((Value)detail::to_registry_source_entity(value)) {}

        source(const enbt::raw_uuid& value) : value((Value)detail::to_registry_source_entity(value)) {}

        source& operator=(const source& other) {
            value = other.value;
            return *this;
        }

        source& operator=(source&& other) noexcept {
            value = std::move(other.value);
            return *this;
        }

        operator Value&() {
            return value;
        }

        operator const Value&() const {
            return value;
        }

        std::string to_string() const {
            return detail::from_registry_source_value(registry_source::entity_id, value);
        }

        std::optional<api::ecs::entity> get_entity() const {
            return detail::from_registry_source_entity(value);
        }

        uint8_t id_index() const {
            return detail::to_registry_source_entity_index(value);
        }

        static list_array<source> look_all() {
            return detail::all_registry_source_value(registry_source::entity_id).convert_fn([](auto id) {
                return source(id);
            });
        }

        template <source_allow_cast<Value> T>
        operator T() const {
            if constexpr (requires { typename Value::underlying_type; })
                return (T)(typename Value::underlying_type)value;
            else
                return (T)value;
        }

        auto operator<=>(const source& other) const = default;
    };

    template <class type>
    concept is_source = requires(type& d) {
        typename type::underlying_type;
        type::reg_source::value;
        d.value;
    };

    using banner_pattern = source<int32_t, registry_source::banner_pattern>;
    using cat_variant = source<int32_t, registry_source::cat_variant>;
    using chat_type = source<int32_t, registry_source::chat_type>;
    using chicken_variant = source<int32_t, registry_source::chicken_variant>;
    using cow_variant = source<int32_t, registry_source::cow_variant>;
    using damage_type = source<int32_t, registry_source::damage_type>;
    using dialog = source<int32_t, registry_source::dialog>;
    using dimension_type = source<int32_t, registry_source::dimension_type>;
    using enchantment = source<int32_t, registry_source::enchantment>;
    using enchantment_provider = source<int32_t, registry_source::enchantment_provider>;
    using frog_variant = source<int32_t, registry_source::frog_variant>;
    using instrument = source<int32_t, registry_source::instrument>;
    using jukebox_song = source<int32_t, registry_source::jukebox_song>;
    using loot_table = source<int32_t, registry_source::loot_table>;
    using painting_variant = source<int32_t, registry_source::painting_variant>;
    using pig_variant = source<int32_t, registry_source::pig_variant>;
    using recipe = source<int32_t, registry_source::recipe>;
    using trim_material = source<int32_t, registry_source::trim_material>;
    using trim_pattern = source<int32_t, registry_source::trim_pattern>;
    using wolf_sound_variant = source<int32_t, registry_source::wolf_sound_variant>;
    using wolf_variant = source<int32_t, registry_source::wolf_variant>;
    using worldgen__biome = source<int32_t, registry_source::worldgen__biome>;
    using block_type = source<int32_t, registry_source::block_type>;
    using block_entity_type = source<int32_t, registry_source::block_entity_type>;
    using dimension = source<int32_t, registry_source::dimension>;
    using entity_type = source<int32_t, registry_source::entity_type>;
    using fluid = source<int32_t, registry_source::fluid>;
    using game_event = source<int32_t, registry_source::game_event>;
    using item = source<int32_t, registry_source::item>;
    using potion = source<int32_t, registry_source::potion>;
    using villager_variant = source<int32_t, registry_source::villager_variant>;
    using fox_variant = source<int32_t, registry_source::fox_variant>;
    using parrot_variant = source<int32_t, registry_source::parrot_variant>;
    using tropical_fish_pattern = source<int32_t, registry_source::tropical_fish_pattern>;
    using mooshroom_variant = source<int32_t, registry_source::mooshroom_variant>;
    using rabbit_variant = source<int32_t, registry_source::rabbit_variant>;
    using horse_variant = source<int32_t, registry_source::horse_variant>;
    using llama_variant = source<int32_t, registry_source::llama_variant>;
    using axolotl_variant = source<int32_t, registry_source::axolotl_variant>;


    using activity = source<int32_t, registry_source::activity>;
    using attribute = source<int32_t, registry_source::attribute>;
    using block_predicate_type = source<int32_t, registry_source::block_predicate_type>;
    using chunk_status = source<int32_t, registry_source::chunk_status>;
    using command_argument_type = source<int32_t, registry_source::command_argument_type>;
    using consume_effect_type = source<int32_t, registry_source::consume_effect_type>;
    using creative_mode_tab = source<int32_t, registry_source::creative_mode_tab>;
    using custom_stat = source<int32_t, registry_source::custom_stat>;
    using data_component_predicate_type = source<int32_t, registry_source::data_component_predicate_type>;
    using data_component_type = source<int32_t, registry_source::data_component_type>;
    using debug_subscription = source<int32_t, registry_source::debug_subscription>;
    using decorated_pot_pattern = source<int32_t, registry_source::decorated_pot_pattern>;
    using dialog_action_type = source<int32_t, registry_source::dialog_action_type>;
    using dialog_body_type = source<int32_t, registry_source::dialog_body_type>;
    using dialog_type = source<int32_t, registry_source::dialog_type>;
    using enchantment_effect_component_type = source<int32_t, registry_source::enchantment_effect_component_type>;
    using enchantment_entity_effect_type = source<int32_t, registry_source::enchantment_entity_effect_type>;
    using enchantment_level_based_value_type = source<int32_t, registry_source::enchantment_level_based_value_type>;
    using enchantment_location_based_effect_type = source<int32_t, registry_source::enchantment_location_based_effect_type>;
    using enchantment_provider_type = source<int32_t, registry_source::enchantment_provider_type>;
    using enchantment_value_effect_type = source<int32_t, registry_source::enchantment_value_effect_type>;
    using entity_sub_predicate_type = source<int32_t, registry_source::entity_sub_predicate_type>;
    using float_provider_type = source<int32_t, registry_source::float_provider_type>;
    using height_provider_type = source<int32_t, registry_source::height_provider_type>;
    using incoming_rpc_methods = source<int32_t, registry_source::incoming_rpc_methods>;
    using input_control_type = source<int32_t, registry_source::input_control_type>;
    using int_provider_type = source<int32_t, registry_source::int_provider_type>;
    using loot_condition_type = source<int32_t, registry_source::loot_condition_type>;
    using loot_function_type = source<int32_t, registry_source::loot_function_type>;
    using loot_nbt_provider_type = source<int32_t, registry_source::loot_nbt_provider_type>;
    using loot_number_provider_type = source<int32_t, registry_source::loot_number_provider_type>;
    using loot_pool_entry_type = source<int32_t, registry_source::loot_pool_entry_type>;
    using loot_score_provider_type = source<int32_t, registry_source::loot_score_provider_type>;
    using map_decoration_type = source<int32_t, registry_source::map_decoration_type>;
    using memory_module_type = source<int32_t, registry_source::memory_module_type>;
    using menu = source<int32_t, registry_source::menu>;
    using mob_effect = source<int32_t, registry_source::mob_effect>;
    using number_format_type = source<int32_t, registry_source::number_format_type>;
    using outgoing_rpc_methods = source<int32_t, registry_source::outgoing_rpc_methods>;
    using particle_type = source<int32_t, registry_source::particle_type>;
    using point_of_interest_type = source<int32_t, registry_source::point_of_interest_type>;
    using pos_rule_test = source<int32_t, registry_source::pos_rule_test>;
    using position_source_type = source<int32_t, registry_source::position_source_type>;
    using recipe_book_category = source<int32_t, registry_source::recipe_book_category>;
    using recipe_display = source<int32_t, registry_source::recipe_display>;
    using recipe_serializer = source<int32_t, registry_source::recipe_serializer>;
    using recipe_type = source<int32_t, registry_source::recipe_type>;
    using rule_block_entity_modifier = source<int32_t, registry_source::rule_block_entity_modifier>;
    using rule_test = source<int32_t, registry_source::rule_test>;
    using schedule = source<int32_t, registry_source::schedule>;
    using sensor_type = source<int32_t, registry_source::sensor_type>;
    using slot_display = source<int32_t, registry_source::slot_display>;
    using sound_event = source<int32_t, registry_source::sound_event>;
    using spawn_condition_type = source<int32_t, registry_source::spawn_condition_type>;
    using stat_type = source<int32_t, registry_source::stat_type>;
    using test_environment_definition_type = source<int32_t, registry_source::test_environment_definition_type>;
    using test_function = source<int32_t, registry_source::test_function>;
    using test_instance_type = source<int32_t, registry_source::test_instance_type>;
    using ticket_type = source<int32_t, registry_source::ticket_type>;
    using trigger_type = source<int32_t, registry_source::trigger_type>;
    using villager_profession = source<int32_t, registry_source::villager_profession>;
    using villager_type = source<int32_t, registry_source::villager_type>;
    using worldgen__biome_source = source<int32_t, registry_source::worldgen__biome_source>;
    using worldgen__block_state_provider_type = source<int32_t, registry_source::worldgen__block_state_provider_type>;
    using worldgen__carver = source<int32_t, registry_source::worldgen__carver>;
    using worldgen__chunk_generator = source<int32_t, registry_source::worldgen__chunk_generator>;
    using worldgen__density_function_type = source<int32_t, registry_source::worldgen__density_function_type>;
    using worldgen__feature = source<int32_t, registry_source::worldgen__feature>;
    using worldgen__feature_size_type = source<int32_t, registry_source::worldgen__feature_size_type>;
    using worldgen__foliage_placer_type = source<int32_t, registry_source::worldgen__foliage_placer_type>;
    using worldgen__material_condition = source<int32_t, registry_source::worldgen__material_condition>;
    using worldgen__material_rule = source<int32_t, registry_source::worldgen__material_rule>;
    using worldgen__placement_modifier_type = source<int32_t, registry_source::worldgen__placement_modifier_type>;
    using worldgen__pool_alias_binding = source<int32_t, registry_source::worldgen__pool_alias_binding>;
    using worldgen__root_placer_type = source<int32_t, registry_source::worldgen__root_placer_type>;
    using worldgen__structure_piece = source<int32_t, registry_source::worldgen__structure_piece>;
    using worldgen__structure_placement = source<int32_t, registry_source::worldgen__structure_placement>;
    using worldgen__structure_pool_element = source<int32_t, registry_source::worldgen__structure_pool_element>;
    using worldgen__structure_processor = source<int32_t, registry_source::worldgen__structure_processor>;
    using worldgen__structure_type = source<int32_t, registry_source::worldgen__structure_type>;
    using worldgen__tree_decorator_type = source<int32_t, registry_source::worldgen__tree_decorator_type>;
    using worldgen__trunk_placer_type = source<int32_t, registry_source::worldgen__trunk_placer_type>;


    using block_state = source<int32_t, registry_source::block_state>;
    using motive = source<int32_t, registry_source::motive>;
    using entity_pose = source<int32_t, registry_source::entity_pose>;
    using entity_id = source<int32_t, registry_source::entity_id>;

    namespace set {
        using banner_pattern = source_set<registry_source::banner_pattern>;
        using damage_type = source_set<registry_source::damage_type>;
        using enchantment = source_set<registry_source::enchantment>;
        using painting_variant = source_set<registry_source::painting_variant>;
        using instrument = source_set<registry_source::instrument>;
        using item = source_set<registry_source::item>;
        using block_state = source_set<registry_source::block_state>; //virtual, uses block tag
        using block_type = source_set<registry_source::block_type>;
        using entity_type = source_set<registry_source::entity_type>;
        using fluid = source_set<registry_source::fluid>;
        using game_event = source_set<registry_source::game_event>;
    }
}

namespace std {
    template <class T, copper_server::api::id::registry_source sourc>
    struct hash<copper_server::api::id::source<T, sourc>> {
        size_t operator()(const copper_server::api::id::source<T, sourc>& value) const {
            return hash<T>()((T)value);
        }
    };
}

#endif /* SRC_API_ID */
