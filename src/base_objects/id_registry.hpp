/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_ID_REGISTRY
#define SRC_BASE_OBJECTS_ID_REGISTRY
#include <cstdint>
#include <string>

namespace copper_server::base_objects {

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
        test_environment,
        test_instance,
        trim_material,
        trim_pattern,
        wolf_sound_variant,
        wolf_variant,
        worldgen__biome,

        attribute,
        block_state,
        block_type,
        block_entity_type,
        data_component_type,
        dimension,
        entity_type,
        fluid,
        game_event,
        position_source_type,
        item,
        menu,
        mob_effect,
        particle_type,
        potion,
        recipe_serializer,
        recipe_type,
        sound_event,
        stat_type,
        custom_stat,
        command_argument_type,
        entity__activity,
        entity__memory_module_type,
        entity__schedule,
        entity__sensor_type,
        entity__motive,
        entity__villager_profession,
        entity__villager_type,
        entity__poi_type,
        loot_table__loot_condition_type,
        loot_table__loot_function_type,
        loot_table__loot_nbt_provider_type,
        loot_table__loot_number_provider_type,
        loot_table__loot_pool_entry_type,
        loot_table__loot_score_provider_type,
        villager_variant,
        fox_variant,
        parrot_variant,
        tropical_fish_pattern,
        mooshroom_variant,
        rabbit_variant,
        horse_variant,
        llama_variant,
        axolotl_variant,
    };

    std::string from_registry_source_value(registry_source source, int32_t value);
    int32_t to_registry_source_value(registry_source source, const std::string& value);


    template <class T>
    concept _id_source_has_underlying_type = requires { typename T::underlying_type; };

    template <class From, class To>
    concept id_source_allow_cast = (_id_source_has_underlying_type<From> && std::convertible_to<From, typename To::underlying_type>)
                                   || (!_id_source_has_underlying_type<From> && std::convertible_to<From, To>);

    template <class Value, registry_source source>
    struct id_source {
        using underlying_type = Value;
        using reg_source = std::integral_constant<registry_source, source>;
        Value value;

        constexpr id_source() {}

        template <id_source_allow_cast<Value> T>
        constexpr id_source(T value)
            : value(static_cast<Value>(value)) {}

        constexpr id_source(Value value) : value(value) {}

        constexpr id_source(const id_source& value) : value(value.value) {}

        constexpr id_source(id_source&& value) : value(std::move(value.value)) {}

        constexpr id_source(std::string_view value) : value((Value)to_registry_source_value(source, std::string(value))) {}

        constexpr id_source(const std::string& value) : value((Value)to_registry_source_value(source, value)) {}

        template <size_t N>
        constexpr id_source(const char (&value)[N]) : value((Value)to_registry_source_value(source, value)) {}

        constexpr id_source(const char* value) : value((Value)to_registry_source_value(source, value)) {}

        constexpr id_source& operator=(const id_source& other) {
            value = other.value;
            return *this;
        }

        constexpr id_source& operator=(id_source&& other) {
            value = std::move(other.value);
            return *this;
        }

        constexpr operator Value&() {
            return value;
        }

        constexpr operator const Value&() const {
            return value;
        }

        std::string_view to_string() const {
            return from_registry_source_value(source, value);
        }

        template <id_source_allow_cast<Value> T>
        constexpr operator T() const {
            if constexpr (requires { typename Value::underlying_type; })
                return (T)(typename Value::underlying_type)value;
            else
                return (T)value;
        }

        auto operator<=>(const id_source& other) const = default;
    };

    template <class type>
    concept is_id_source = requires(type& d) {
        typename type::underlying_type;
        type::reg_source::value;
        d.value;
    };

    using id_banner_pattern = id_source<int32_t, registry_source::banner_pattern>;
    using id_cat_variant = id_source<int32_t, registry_source::cat_variant>;
    using id_chat_type = id_source<int32_t, registry_source::chat_type>;
    using id_chicken_variant = id_source<int32_t, registry_source::chicken_variant>;
    using id_cow_variant = id_source<int32_t, registry_source::cow_variant>;
    using id_damage_type = id_source<int32_t, registry_source::damage_type>;
    using id_dialog = id_source<int32_t, registry_source::dialog>;
    using id_dimension_type = id_source<int32_t, registry_source::dimension_type>;
    using id_enchantment = id_source<int32_t, registry_source::enchantment>;
    using id_enchantment_provider = id_source<int32_t, registry_source::enchantment_provider>;
    using id_frog_variant = id_source<int32_t, registry_source::frog_variant>;
    using id_instrument = id_source<int32_t, registry_source::instrument>;
    using id_jukebox_song = id_source<int32_t, registry_source::jukebox_song>;
    using id_loot_table = id_source<int32_t, registry_source::loot_table>;
    using id_painting_variant = id_source<int32_t, registry_source::painting_variant>;
    using id_pig_variant = id_source<int32_t, registry_source::pig_variant>;
    using id_recipe = id_source<int32_t, registry_source::recipe>;
    using id_test_environment = id_source<int32_t, registry_source::test_environment>;
    using id_test_instance = id_source<int32_t, registry_source::test_instance>;
    using id_trim_material = id_source<int32_t, registry_source::trim_material>;
    using id_trim_pattern = id_source<int32_t, registry_source::trim_pattern>;
    using id_wolf_sound_variant = id_source<int32_t, registry_source::wolf_sound_variant>;
    using id_wolf_variant = id_source<int32_t, registry_source::wolf_variant>;
    using id_worldgen__biome = id_source<int32_t, registry_source::worldgen__biome>;
    using id_attribute = id_source<int32_t, registry_source::attribute>;
    using id_block_state = id_source<int32_t, registry_source::block_state>;
    using id_block_type = id_source<int32_t, registry_source::block_type>;
    using id_block_entity_type = id_source<int32_t, registry_source::block_entity_type>;
    using id_data_component_type = id_source<int32_t, registry_source::data_component_type>;
    using id_dimension = id_source<int32_t, registry_source::dimension>;
    using id_entity_type = id_source<int32_t, registry_source::entity_type>;
    using id_fluid = id_source<int32_t, registry_source::fluid>;
    using id_game_event = id_source<int32_t, registry_source::game_event>;
    using id_position_source_type = id_source<int32_t, registry_source::position_source_type>;
    using id_item = id_source<int32_t, registry_source::item>;
    using id_menu = id_source<int32_t, registry_source::menu>;
    using id_mob_effect = id_source<int32_t, registry_source::mob_effect>;
    using id_particle_type = id_source<int32_t, registry_source::particle_type>;
    using id_potion = id_source<int32_t, registry_source::potion>;
    using id_recipe_serializer = id_source<int32_t, registry_source::recipe_serializer>;
    using id_recipe_type = id_source<int32_t, registry_source::recipe_type>;
    using id_sound_event = id_source<int32_t, registry_source::sound_event>;
    using id_stat_type = id_source<int32_t, registry_source::stat_type>;
    using id_custom_stat = id_source<int32_t, registry_source::custom_stat>;
    using id_command_argument_type = id_source<int32_t, registry_source::command_argument_type>;
    using id_entity__activity = id_source<int32_t, registry_source::entity__activity>;
    using id_entity__memory_module_type = id_source<int32_t, registry_source::entity__memory_module_type>;
    using id_entity__schedule = id_source<int32_t, registry_source::entity__schedule>;
    using id_entity__sensor_type = id_source<int32_t, registry_source::entity__sensor_type>;
    using id_entity__motive = id_source<int32_t, registry_source::entity__motive>;
    using id_entity__villager_profession = id_source<int32_t, registry_source::entity__villager_profession>;
    using id_entity__villager_type = id_source<int32_t, registry_source::entity__villager_type>;
    using id_entity__poi_type = id_source<int32_t, registry_source::entity__poi_type>;
    using id_loot_table__loot_condition_type = id_source<int32_t, registry_source::loot_table__loot_condition_type>;
    using id_loot_table__loot_function_type = id_source<int32_t, registry_source::loot_table__loot_function_type>;
    using id_loot_table__loot_nbt_provider_type = id_source<int32_t, registry_source::loot_table__loot_nbt_provider_type>;
    using id_loot_table__loot_number_provider_type = id_source<int32_t, registry_source::loot_table__loot_number_provider_type>;
    using id_loot_table__loot_pool_entry_type = id_source<int32_t, registry_source::loot_table__loot_pool_entry_type>;
    using id_loot_table__loot_score_provider_type = id_source<int32_t, registry_source::loot_table__loot_score_provider_type>;
    using id_villager_variant = id_source<int32_t, registry_source::villager_variant>;
    using id_fox_variant = id_source<int32_t, registry_source::fox_variant>;
    using id_parrot_variant = id_source<int32_t, registry_source::parrot_variant>;
    using id_tropical_fish_pattern = id_source<int32_t, registry_source::tropical_fish_pattern>;
    using id_mooshroom_variant = id_source<int32_t, registry_source::mooshroom_variant>;
    using id_rabbit_variant = id_source<int32_t, registry_source::rabbit_variant>;
    using id_horse_variant = id_source<int32_t, registry_source::horse_variant>;
    using id_llama_variant = id_source<int32_t, registry_source::llama_variant>;
    using id_axolotl_variant = id_source<int32_t, registry_source::axolotl_variant>;
}

namespace std {
    template <class T, copper_server::base_objects::registry_source source>
    struct hash<copper_server::base_objects::id_source<T, source>> {
        size_t operator()(const copper_server::base_objects::id_source<T, source>& value) const {
            return hash<T>()((T)value);
        }
    };
}

#endif /* SRC_BASE_OBJECTS_ID_REGISTRY */
