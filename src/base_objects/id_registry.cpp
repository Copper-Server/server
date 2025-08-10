/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/world.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/packets_help.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/registers.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server::base_objects {
    std::string from_registry_source_value(registry_source source, int32_t value) {
        switch (source) {
        case registry_source::banner_pattern:
            return registers::bannerPatterns_cache.at(value)->first;
        case registry_source::cat_variant:
            return registers::catVariants_cache.at(value)->first;
        case registry_source::chat_type:
            return registers::chatTypes_cache.at(value)->first;
        case registry_source::chicken_variant:
            return registers::chickenVariants_cache.at(value)->first;
        case registry_source::cow_variant:
            return registers::cowVariants_cache.at(value)->first;
        case registry_source::damage_type:
            return registers::damageTypes_cache.at(value)->first;
        case registry_source::dialog:
            return registers::chickenVariants_cache.at(value)->first; //TODO
        case registry_source::dimension_type:
            return registers::dimensionTypes_cache.at(value)->first;
        case registry_source::enchantment:
            return registers::enchantments_cache.at(value)->first;
        case registry_source::enchantment_provider:
            return registers::enchantment_providers_cache.at(value)->first;
        case registry_source::frog_variant:
            return registers::frogVariants_cache.at(value)->first;
        case registry_source::instrument:
            return registers::instruments_cache.at(value)->first;
        case registry_source::jukebox_song:
            return registers::jukebox_songs_cache.at(value)->first;
        case registry_source::loot_table:
            return registers::loot_table_cache.at(value)->first;
        case registry_source::painting_variant:
            return registers::paintingVariants_cache.at(value)->first;
        case registry_source::pig_variant:
            return registers::pigVariants_cache.at(value)->first;
        case registry_source::recipe:
            return registers::recipe_table_cache.at(value)->first;
        case registry_source::test_environment:
            return registers::enchantment_providers_cache.at(value)->first; //TODO
        case registry_source::test_instance:
            return registers::enchantment_providers_cache.at(value)->first; //TODO
        case registry_source::trim_material:
            return registers::armorTrimMaterials_cache.at(value)->first;
        case registry_source::trim_pattern:
            return registers::armorTrimPatterns_cache.at(value)->first;
        case registry_source::wolf_sound_variant:
            return registers::wolfSoundVariants_cache.at(value)->first;
        case registry_source::wolf_variant:
            return registers::wolfVariants_cache.at(value)->first;
        case registry_source::worldgen__biome:
            return registers::biomes_cache.at(value)->first;


        case registry_source::attribute:
            return registers::view_reg_pro_name("minecraft:attribute", value);
        case registry_source::block_state:
            return block::get_block((block_id_t)value).name;
        case registry_source::block_type:
            return block::get_general_block((block_id_t)value).name;
        case registry_source::block_entity_type:
            return block::get_block_entity((block_id_t)value).name;
        case registry_source::data_component_type:
            return registers::view_reg_pro_name("minecraft:data_component_type", value);
        case registry_source::entity_type:
            return entity_data::get_entity((uint16_t)value).id;
        case registry_source::fluid:
            return registers::view_reg_pro_name("minecraft:fluid", value);
        case registry_source::game_event:
            return registers::view_reg_pro_name("minecraft:game_event", value);
        case registry_source::potion:
            return registers::potions_cache.at(value)->first;
        case registry_source::dimension: {
            std::string res;
            api::world::get(value, [&res](auto& w) {
                res = w.world_name;
            });
            return res;
        }
        case registry_source::menu:
            return registers::view_reg_pro_name("minecraft:menu", value);
        default:
            return ""; //TODO
            //position_source_type,
            //item,
            //menu,
            //mob_effect,
            //particle_type,
            //recipe_serializer,
            //recipe_type,
            //sound_event,
            //stat_type,
            //custom_stat,
            //command_argument_type,
            //entity__activity,
            //entity__memory_module_type,
            //entity__schedule,
            //entity__sensor_type,
            //entity__motive,
            //entity__villager_profession,
            //entity__villager_type,
            //entity__poi_type,
            //loot_table__loot_condition_type,
            //loot_table__loot_function_type,
            //loot_table__loot_nbt_provider_type,
            //loot_table__loot_number_provider_type,
            //loot_table__loot_pool_entry_type,
            //loot_table__loot_score_provider_type,
            //villager_variant,
            //fox_variant,
            //parrot_variant,
            //tropical_fish_pattern,
            //mooshroom_variant,
            //rabbit_variant,
            //horse_variant,
            //llama_variant,
            //axolotl_variant,
        }
    }

    int32_t to_registry_source_value(registry_source source, const std::string& value) {
        switch (source) {
        case registry_source::banner_pattern:
            return registers::bannerPatterns.at(value).id;
        case registry_source::cat_variant:
            return registers::catVariants.at(value).id;
        case registry_source::chat_type:
            return registers::chatTypes.at(value).id;
        case registry_source::chicken_variant:
            return registers::chickenVariants.at(value).id;
        case registry_source::cow_variant:
            return registers::cowVariants.at(value).id;
        case registry_source::damage_type:
            return registers::damageTypes.at(value).id;
        case registry_source::dialog:
            return registers::chickenVariants.at(value).id; //TODO
        case registry_source::dimension_type:
            return registers::dimensionTypes.at(value).id;
        case registry_source::enchantment:
            return registers::enchantments.at(value).id;
        case registry_source::enchantment_provider:
            return registers::enchantment_providers.at(value).id;
        case registry_source::frog_variant:
            return registers::frogVariants.at(value).id;
        case registry_source::instrument:
            return registers::instruments.at(value).id;
        case registry_source::jukebox_song:
            return registers::jukebox_songs.at(value).id;
        case registry_source::loot_table:
            return registers::loot_table.at(value).id;
        case registry_source::painting_variant:
            return registers::paintingVariants.at(value).id;
        case registry_source::pig_variant:
            return registers::pigVariants.at(value).id;
        case registry_source::recipe:
            return registers::recipe_table.at(value).id;
        case registry_source::test_environment:
            return registers::enchantment_providers.at(value).id; //TODO
        case registry_source::test_instance:
            return registers::enchantment_providers.at(value).id; //TODO
        case registry_source::trim_material:
            return registers::armorTrimMaterials.at(value).id;
        case registry_source::trim_pattern:
            return registers::armorTrimPatterns.at(value).id;
        case registry_source::wolf_sound_variant:
            return registers::wolfSoundVariants.at(value).id;
        case registry_source::wolf_variant:
            return registers::wolfVariants.at(value).id;
        case registry_source::worldgen__biome:
            return registers::biomes.at(value).id;


        case registry_source::attribute:
            return registers::view_reg_pro_id("minecraft:attribute", value);
        case registry_source::block_state:
            return block::get_block(value).current_state;
        case registry_source::block_type:
            return block::get_block(value).general_block_id;
        case registry_source::block_entity_type:
            return block::get_block(value).block_entity_id;
        case registry_source::data_component_type:
            return registers::view_reg_pro_id("minecraft:data_component_type", value);
        case registry_source::entity_type:
            return entity_data::get_entity(value).entity_id;
        case registry_source::fluid:
            return registers::view_reg_pro_id("minecraft:fluid", value);
        case registry_source::game_event:
            return registers::view_reg_pro_id("minecraft:game_event", value);
        case registry_source::potion:
            return registers::potions.at(value).id;
        case registry_source::dimension: 
            return api::world::resolve_id(value);
        case registry_source::menu:
            return registers::view_reg_pro_id("minecraft:menu", value);
        default:
            return 0; //TODO
            //position_source_type,
            //item,
            //menu,
            //mob_effect,
            //particle_type,
            //recipe_serializer,
            //recipe_type,
            //sound_event,
            //stat_type,
            //custom_stat,
            //command_argument_type,
            //entity__activity,
            //entity__memory_module_type,
            //entity__schedule,
            //entity__sensor_type,
            //entity__motive,
            //entity__villager_profession,
            //entity__villager_type,
            //entity__poi_type,
            //loot_table__loot_condition_type,
            //loot_table__loot_function_type,
            //loot_table__loot_nbt_provider_type,
            //loot_table__loot_number_provider_type,
            //loot_table__loot_pool_entry_type,
            //loot_table__loot_score_provider_type,
            //villager_variant,
            //fox_variant,
            //parrot_variant,
            //tropical_fish_pattern,
            //mooshroom_variant,
            //rabbit_variant,
            //horse_variant,
            //llama_variant,
            //axolotl_variant,
        }
    }
}