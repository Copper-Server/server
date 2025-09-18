/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/entity_id_map.hpp>
#include <src/api/id.hpp>
#include <src/api/registers.hpp>
#include <src/api/tags.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/packets_help.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server::api::id::detail {
    std::string from_registry_source_value(registry_source source, int32_t value) {
        switch (source) {
        case registry_source::banner_pattern:
            return api::registers::bannerPatterns_cache.at(value)->first;
        case registry_source::cat_variant:
            return api::registers::catVariants_cache.at(value)->first;
        case registry_source::chat_type:
            return api::registers::chatTypes_cache.at(value)->first;
        case registry_source::chicken_variant:
            return api::registers::chickenVariants_cache.at(value)->first;
        case registry_source::cow_variant:
            return api::registers::cowVariants_cache.at(value)->first;
        case registry_source::damage_type:
            return api::registers::damageTypes_cache.at(value)->first;
        case registry_source::dialog:
            return api::registers::chickenVariants_cache.at(value)->first; //TODO
        case registry_source::dimension_type:
            return api::registers::dimensionTypes_cache.at(value)->first;
        case registry_source::enchantment:
            return api::registers::enchantments_cache.at(value)->first;
        case registry_source::enchantment_provider:
            return api::registers::enchantment_providers_cache.at(value)->first;
        case registry_source::frog_variant:
            return api::registers::frogVariants_cache.at(value)->first;
        case registry_source::instrument:
            return api::registers::instruments_cache.at(value)->first;
        case registry_source::jukebox_song:
            return api::registers::jukebox_songs_cache.at(value)->first;
        case registry_source::loot_table:
            return api::registers::loot_table_cache.at(value)->first;
        case registry_source::painting_variant:
            return api::registers::paintingVariants_cache.at(value)->first;
        case registry_source::pig_variant:
            return api::registers::pigVariants_cache.at(value)->first;
        case registry_source::recipe:
            return api::registers::recipe_table_cache.at(value)->first;
        case registry_source::test_environment:
            return api::registers::enchantment_providers_cache.at(value)->first; //TODO
        case registry_source::test_instance:
            return api::registers::enchantment_providers_cache.at(value)->first; //TODO
        case registry_source::trim_material:
            return api::registers::armorTrimMaterials_cache.at(value)->first;
        case registry_source::trim_pattern:
            return api::registers::armorTrimPatterns_cache.at(value)->first;
        case registry_source::wolf_sound_variant:
            return api::registers::wolfSoundVariants_cache.at(value)->first;
        case registry_source::wolf_variant:
            return api::registers::wolfVariants_cache.at(value)->first;
        case registry_source::worldgen__biome:
            return api::registers::biomes_cache.at(value)->first;
        case registry_source::entity_pose:
            return api::registers::entity_pose_cache.at(value)->first;


        case registry_source::attribute:
            return (std::string)api::registers::view_reg_pro_name("minecraft:attribute", value);
        case registry_source::particle_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:particle_type", value);
        case registry_source::sound_event:
            return (std::string)api::registers::view_reg_pro_name("minecraft:sound_event", value);
        case registry_source::stat_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:stat_type", value);
        case registry_source::recipe_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:recipe_type", value);
        case registry_source::custom_stat:
            return (std::string)api::registers::view_reg_pro_name("minecraft:custom_stat", value);
        case registry_source::recipe_serializer:
            return (std::string)api::registers::view_reg_pro_name("minecraft:recipe_serializer", value);
        case registry_source::mob_effect:
            return (std::string)api::registers::view_reg_pro_name("minecraft:mob_effect", value);
        case registry_source::command_argument_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:command_argument_type", value);
        case registry_source::menu:
            return (std::string)api::registers::view_reg_pro_name("minecraft:menu", value);
        case registry_source::item:
            return (std::string)api::registers::view_reg_pro_name("minecraft:item", value);
        case registry_source::activity:
            return (std::string)api::registers::view_reg_pro_name("minecraft:activity", value);
        case registry_source::memory_module_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:memory_module_type", value);
        case registry_source::schedule:
            return (std::string)api::registers::view_reg_pro_name("minecraft:schedule", value);
        case registry_source::motive:
            return (std::string)api::registers::view_reg_pro_name("minecraft:position_source_type", value); //TODO
        case registry_source::villager_profession:
            return (std::string)api::registers::view_reg_pro_name("minecraft:villager_profession", value);
        case registry_source::villager_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:villager_type", value);
        case registry_source::poi_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:point_of_interest_type", value);
        case registry_source::position_source_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:position_source_type", value);
        case registry_source::loot_condition_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:loot_condition_type", value);
        case registry_source::loot_function_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:loot_function_type", value);
        case registry_source::loot_nbt_provider_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:loot_nbt_provider_type", value);
        case registry_source::loot_number_provider_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:loot_number_provider_type", value);
        case registry_source::loot_pool_entry_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:loot_pool_entry_type", value);
        case registry_source::loot_score_provider_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:loot_score_provider_type", value);
        case registry_source::block_state:
            return base_objects::block::get_block((base_objects::block_id_t)value).name;
        case registry_source::block_type:
            return base_objects::block::get_general_block((base_objects::block_id_t)value).name;
        case registry_source::block_entity_type:
            return base_objects::block::get_block_entity((base_objects::block_id_t)value).name;
        case registry_source::data_component_type:
            return (std::string)api::registers::view_reg_pro_name("minecraft:data_component_type", value);
        case registry_source::entity_type:
            return base_objects::entity_data::get_entity((uint16_t)value).id;
        case registry_source::fluid:
            return (std::string)api::registers::view_reg_pro_name("minecraft:fluid", value);
        case registry_source::game_event:
            return (std::string)api::registers::view_reg_pro_name("minecraft:game_event", value);
        case registry_source::potion:
            return api::registers::potions_cache.at(value)->first;
        case registry_source::dimension: {
            std::string res;
            api::world::get(value, [&res](auto& w) {
                res = w.world_name;
            });
            return res;
        }

        case registry_source::entity_id:
            return api::entity_id_map::get_uuid(value).to_string();


        case registry_source::villager_variant:
        case registry_source::fox_variant:
        case registry_source::parrot_variant:
        case registry_source::tropical_fish_pattern:
        case registry_source::mooshroom_variant:
        case registry_source::rabbit_variant:
        case registry_source::horse_variant:
        case registry_source::llama_variant:
        case registry_source::axolotl_variant:
        default:
            return ""; //TODO
        }
    }

    int32_t to_registry_source_value(registry_source source, const std::string& value) {
        switch (source) {
        case registry_source::banner_pattern:
            return api::registers::bannerPatterns.at(value).id;
        case registry_source::cat_variant:
            return api::registers::catVariants.at(value).id;
        case registry_source::chat_type:
            return api::registers::chatTypes.at(value).id;
        case registry_source::chicken_variant:
            return api::registers::chickenVariants.at(value).id;
        case registry_source::cow_variant:
            return api::registers::cowVariants.at(value).id;
        case registry_source::damage_type:
            return api::registers::damageTypes.at(value).id;
        case registry_source::dialog:
            return api::registers::chickenVariants.at(value).id; //TODO
        case registry_source::dimension_type:
            return api::registers::dimensionTypes.at(value).id;
        case registry_source::enchantment:
            return api::registers::enchantments.at(value).id;
        case registry_source::enchantment_provider:
            return api::registers::enchantment_providers.at(value).id;
        case registry_source::frog_variant:
            return api::registers::frogVariants.at(value).id;
        case registry_source::instrument:
            return api::registers::instruments.at(value).id;
        case registry_source::jukebox_song:
            return api::registers::jukebox_songs.at(value).id;
        case registry_source::loot_table:
            return api::registers::loot_table.at(value).id;
        case registry_source::painting_variant:
            return api::registers::paintingVariants.at(value).id;
        case registry_source::pig_variant:
            return api::registers::pigVariants.at(value).id;
        case registry_source::recipe:
            return api::registers::recipe_table.at(value).id;
        case registry_source::test_environment:
            return api::registers::enchantment_providers.at(value).id; //TODO
        case registry_source::test_instance:
            return api::registers::enchantment_providers.at(value).id; //TODO
        case registry_source::trim_material:
            return api::registers::armorTrimMaterials.at(value).id;
        case registry_source::trim_pattern:
            return api::registers::armorTrimPatterns.at(value).id;
        case registry_source::wolf_sound_variant:
            return api::registers::wolfSoundVariants.at(value).id;
        case registry_source::wolf_variant:
            return api::registers::wolfVariants.at(value).id;
        case registry_source::worldgen__biome:
            return api::registers::biomes.at(value).id;
        case registry_source::entity_pose:
            return api::registers::entity_pose.at(value);


        case registry_source::attribute:
            return api::registers::view_reg_pro_id("minecraft:attribute", value);
        case registry_source::particle_type:
            return api::registers::view_reg_pro_id("minecraft:particle_type", value);
        case registry_source::sound_event:
            return api::registers::view_reg_pro_id("minecraft:sound_event", value);
        case registry_source::stat_type:
            return api::registers::view_reg_pro_id("minecraft:stat_type", value);
        case registry_source::recipe_type:
            return api::registers::view_reg_pro_id("minecraft:recipe_type", value);
        case registry_source::custom_stat:
            return api::registers::view_reg_pro_id("minecraft:custom_stat", value);
        case registry_source::recipe_serializer:
            return api::registers::view_reg_pro_id("minecraft:recipe_serializer", value);
        case registry_source::mob_effect:
            return api::registers::view_reg_pro_id("minecraft:mob_effect", value);
        case registry_source::command_argument_type:
            return api::registers::view_reg_pro_id("minecraft:command_argument_type", value);
        case registry_source::menu:
            return api::registers::view_reg_pro_id("minecraft:menu", value);
        case registry_source::item:
            return api::registers::view_reg_pro_id("minecraft:item", value);
        case registry_source::activity:
            return api::registers::view_reg_pro_id("minecraft:activity", value);
        case registry_source::memory_module_type:
            return api::registers::view_reg_pro_id("minecraft:memory_module_type", value);
        case registry_source::schedule:
            return api::registers::view_reg_pro_id("minecraft:schedule", value);
        case registry_source::motive:
            return api::registers::view_reg_pro_id("minecraft:position_source_type", value); //TODO
        case registry_source::villager_profession:
            return api::registers::view_reg_pro_id("minecraft:villager_profession", value);
        case registry_source::villager_type:
            return api::registers::view_reg_pro_id("minecraft:villager_type", value);
        case registry_source::poi_type:
            return api::registers::view_reg_pro_id("minecraft:point_of_interest_type", value);
        case registry_source::position_source_type:
            return api::registers::view_reg_pro_id("minecraft:position_source_type", value);
        case registry_source::loot_condition_type:
            return api::registers::view_reg_pro_id("minecraft:loot_condition_type", value);
        case registry_source::loot_function_type:
            return api::registers::view_reg_pro_id("minecraft:loot_function_type", value);
        case registry_source::loot_nbt_provider_type:
            return api::registers::view_reg_pro_id("minecraft:loot_nbt_provider_type", value);
        case registry_source::loot_number_provider_type:
            return api::registers::view_reg_pro_id("minecraft:loot_number_provider_type", value);
        case registry_source::loot_pool_entry_type:
            return api::registers::view_reg_pro_id("minecraft:loot_pool_entry_type", value);
        case registry_source::loot_score_provider_type:
            return api::registers::view_reg_pro_id("minecraft:loot_score_provider_type", value);
        case registry_source::block_state:
            return base_objects::block::get_block(value).current_state;
        case registry_source::block_type:
            return base_objects::block::get_block(value).general_block_id;
        case registry_source::block_entity_type:
            return base_objects::block::get_block(value).block_entity_id;
        case registry_source::data_component_type:
            return api::registers::view_reg_pro_id("minecraft:data_component_type", value);
        case registry_source::entity_type:
            return base_objects::entity_data::get_entity(value).entity_id;
        case registry_source::fluid:
            return api::registers::view_reg_pro_id("minecraft:fluid", value);
        case registry_source::game_event:
            return api::registers::view_reg_pro_id("minecraft:game_event", value);
        case registry_source::potion:
            return api::registers::potions.at(value).id;
        case registry_source::dimension:
            return api::world::resolve_id(value);


        case registry_source::entity_id:
            return api::entity_id_map::get_id(enbt::raw_uuid::from_string(value));

        case registry_source::villager_variant:
        case registry_source::fox_variant:
        case registry_source::parrot_variant:
        case registry_source::tropical_fish_pattern:
        case registry_source::mooshroom_variant:
        case registry_source::rabbit_variant:
        case registry_source::horse_variant:
        case registry_source::llama_variant:
        case registry_source::axolotl_variant:
        default:
            return 0; //TODO
        }
    }

    api::tags::tag_handle to_registry_source_handle(registry_source source, std::string_view value) {
        switch (source) {
        case registry_source::banner_pattern:
            return api::tags::get_tag_handle(api::tags::builtin_entry::banner_pattern, value);
        case registry_source::damage_type:
            return api::tags::get_tag_handle(api::tags::builtin_entry::damage_type, value);
        case registry_source::enchantment:
            return api::tags::get_tag_handle(api::tags::builtin_entry::enchantment, value);
        case registry_source::painting_variant:
            return api::tags::get_tag_handle(api::tags::builtin_entry::painting_variant, value);
        case registry_source::instrument:
            return api::tags::get_tag_handle(api::tags::builtin_entry::instrument, value);
        case registry_source::item:
            return api::tags::get_tag_handle(api::tags::builtin_entry::item, value);
        case registry_source::block_type:
            return api::tags::get_tag_handle(api::tags::builtin_entry::block, value);
        case registry_source::block_state:
            return api::tags::get_tag_handle(api::tags::builtin_entry::block_state, value);
        case registry_source::entity_type:
            return api::tags::get_tag_handle(api::tags::builtin_entry::entity_type, value);
        case registry_source::fluid:
            return api::tags::get_tag_handle(api::tags::builtin_entry::fluid, value);
        case registry_source::game_event:
            return api::tags::get_tag_handle(api::tags::builtin_entry::game_event, value);
        default:
            std::unreachable();
        }
    }

    list_array<int32_t> all_registry_source_value(registry_source source) {
        switch (source) {
        case registry_source::banner_pattern:
            return api::registers::bannerPatterns_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::cat_variant:
            return api::registers::catVariants_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::chat_type:
            return api::registers::chatTypes_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::chicken_variant:
            return api::registers::chickenVariants_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::cow_variant:
            return api::registers::cowVariants_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::damage_type:
            return api::registers::damageTypes_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::dialog:
            return api::registers::chickenVariants_cache.convert_fn([](auto& it) { return it->second.id; }); //TODO
        case registry_source::dimension_type:
            return api::registers::dimensionTypes_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::enchantment:
            return api::registers::enchantments_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::enchantment_provider:
            return api::registers::enchantment_providers_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::frog_variant:
            return api::registers::frogVariants_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::instrument:
            return api::registers::instruments_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::jukebox_song:
            return api::registers::jukebox_songs_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::loot_table:
            return api::registers::loot_table_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::painting_variant:
            return api::registers::paintingVariants_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::pig_variant:
            return api::registers::pigVariants_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::recipe:
            return api::registers::recipe_table_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::test_environment:
            return api::registers::enchantment_providers_cache.convert_fn([](auto& it) { return it->second.id; }); //TODO
        case registry_source::test_instance:
            return api::registers::enchantment_providers_cache.convert_fn([](auto& it) { return it->second.id; }); //TODO
        case registry_source::trim_material:
            return api::registers::armorTrimMaterials_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::trim_pattern:
            return api::registers::armorTrimPatterns_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::wolf_sound_variant:
            return api::registers::wolfSoundVariants_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::wolf_variant:
            return api::registers::wolfVariants_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::worldgen__biome:
            return api::registers::biomes_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::entity_pose:
            return api::registers::entity_pose_cache.convert_fn([](auto& it) { return it->second; });


        case registry_source::attribute:
            return api::registers::reg_ids("minecraft:attribute");
        case registry_source::particle_type:
            return api::registers::reg_ids("minecraft:particle_type");
        case registry_source::sound_event:
            return api::registers::reg_ids("minecraft:sound_event");
        case registry_source::stat_type:
            return api::registers::reg_ids("minecraft:stat_type");
        case registry_source::recipe_type:
            return api::registers::reg_ids("minecraft:recipe_type");
        case registry_source::custom_stat:
            return api::registers::reg_ids("minecraft:custom_stat");
        case registry_source::recipe_serializer:
            return api::registers::reg_ids("minecraft:recipe_serializer");
        case registry_source::mob_effect:
            return api::registers::reg_ids("minecraft:mob_effect");
        case registry_source::command_argument_type:
            return api::registers::reg_ids("minecraft:command_argument_type");
        case registry_source::menu:
            return api::registers::reg_ids("minecraft:menu");
        case registry_source::item:
            return api::registers::reg_ids("minecraft:item");
        case registry_source::activity:
            return api::registers::reg_ids("minecraft:activity");
        case registry_source::memory_module_type:
            return api::registers::reg_ids("minecraft:memory_module_type");
        case registry_source::schedule:
            return api::registers::reg_ids("minecraft:schedule");
        case registry_source::motive:
            return api::registers::reg_ids("minecraft:position_source_type"); //TODO
        case registry_source::villager_profession:
            return api::registers::reg_ids("minecraft:villager_profession");
        case registry_source::villager_type:
            return api::registers::reg_ids("minecraft:villager_type");
        case registry_source::poi_type:
            return api::registers::reg_ids("minecraft:point_of_interest_type");
        case registry_source::position_source_type:
            return api::registers::reg_ids("minecraft:position_source_type");
        case registry_source::loot_condition_type:
            return api::registers::reg_ids("minecraft:loot_condition_type");
        case registry_source::loot_function_type:
            return api::registers::reg_ids("minecraft:loot_function_type");
        case registry_source::loot_nbt_provider_type:
            return api::registers::reg_ids("minecraft:loot_nbt_provider_type");
        case registry_source::loot_number_provider_type:
            return api::registers::reg_ids("minecraft:loot_number_provider_type");
        case registry_source::loot_pool_entry_type:
            return api::registers::reg_ids("minecraft:loot_pool_entry_type");
        case registry_source::loot_score_provider_type:
            return api::registers::reg_ids("minecraft:loot_score_provider_type");
        case registry_source::block_state:
            return base_objects::block::get_block_states();
        case registry_source::block_type:
            return base_objects::block::get_block_generals();
        case registry_source::block_entity_type:
            return base_objects::block::get_block_entities();
        case registry_source::data_component_type:
            return api::registers::reg_ids("minecraft:data_component_type");
        case registry_source::entity_type:
            return base_objects::entity_data::get_entity_ids();
        case registry_source::fluid:
            return api::registers::reg_ids("minecraft:fluid");
        case registry_source::game_event:
            return api::registers::reg_ids("minecraft:game_event");
        case registry_source::potion:
            return api::registers::potions_cache.convert_fn([](auto& it) { return it->second.id; });
        case registry_source::dimension:
            return api::world::request_ids();


        case registry_source::entity_id:
            return api::entity_id_map::query_ids();

        case registry_source::villager_variant:
        case registry_source::fox_variant:
        case registry_source::parrot_variant:
        case registry_source::tropical_fish_pattern:
        case registry_source::mooshroom_variant:
        case registry_source::rabbit_variant:
        case registry_source::horse_variant:
        case registry_source::llama_variant:
        case registry_source::axolotl_variant:
        default:
            return 0; //TODO
        }
    }

    base_objects::entity_ref from_registry_source_entity(int32_t value) {
        return api::entity_id_map::get_entity(value);
    }

    uint8_t to_registry_source_entity_index(int32_t value) {
        return api::entity_id_map::id_index(value);
    }

    int32_t to_registry_source_entity(const base_objects::entity_ref& value) {
        return value->protocol_id;
    }

    int32_t to_registry_source_entity(const enbt::raw_uuid& value) {
        return api::entity_id_map::get_id(value);
    }
}
