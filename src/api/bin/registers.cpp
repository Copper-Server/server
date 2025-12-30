/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/registers.hpp>
#include <src/base_objects/slot.hpp>

namespace copper_server::api::registers {
    std::unordered_map<std::string, armor_trim_material> armor_trim_materials;
    std::unordered_map<std::string, armor_trim_pattern> armor_trim_patterns;
    std::unordered_map<std::string, biome> biomes;
    std::unordered_map<std::string, chat_type> chat_types;
    std::unordered_map<std::string, damage_type> damage_types;
    std::unordered_map<std::string, dimension_type> dimension_types;
    std::unordered_map<std::string, wolf_sound_variant> wolf_sound_variants;
    std::unordered_map<std::string, wolf_variant> wolf_variants;
    std::unordered_map<std::string, entity_variant> cat_variants;
    std::unordered_map<std::string, entity_variant> chicken_variants;
    std::unordered_map<std::string, entity_variant> cow_variants;
    std::unordered_map<std::string, entity_variant> pig_variants;
    std::unordered_map<std::string, entity_variant> frog_variants;
    std::unordered_map<std::string, banner_pattern> bannerPatterns;
    std::unordered_map<std::string, painting_variant> painting_variants;
    std::unordered_map<std::string, instrument> instruments;
    std::unordered_map<std::string, int32_t> entity_pose;

    list_array<std::unordered_map<std::string, armor_trim_material>::iterator> armor_trim_materials_cache;
    list_array<std::unordered_map<std::string, armor_trim_pattern>::iterator> armor_trim_patterns_cache;
    list_array<std::unordered_map<std::string, biome>::iterator> biomes_cache;
    list_array<std::unordered_map<std::string, chat_type>::iterator> chat_types_cache;
    list_array<std::unordered_map<std::string, damage_type>::iterator> damage_types_cache;
    list_array<std::unordered_map<std::string, dimension_type>::iterator> dimension_types_cache;
    list_array<std::unordered_map<std::string, wolf_sound_variant>::iterator> wolf_sound_variants_cache;
    list_array<std::unordered_map<std::string, wolf_variant>::iterator> wolf_variants_cache;
    list_array<std::unordered_map<std::string, entity_variant>::iterator> cat_variants_cache;
    list_array<std::unordered_map<std::string, entity_variant>::iterator> chicken_variants_cache;
    list_array<std::unordered_map<std::string, entity_variant>::iterator> cow_variants_cache;
    list_array<std::unordered_map<std::string, entity_variant>::iterator> pig_variants_cache;
    list_array<std::unordered_map<std::string, entity_variant>::iterator> frog_variants_cache;
    list_array<std::unordered_map<std::string, banner_pattern>::iterator> bannerPatterns_cache;
    list_array<std::unordered_map<std::string, painting_variant>::iterator> painting_variants_cache;
    list_array<std::unordered_map<std::string, instrument>::iterator> instruments_cache;
    list_array<std::unordered_map<std::string, int32_t>::iterator> entity_pose_cache;

    //SERVER
    std::unordered_map<std::string, advancement> advancements;

    std::unordered_map<std::string, trial_spawner_config> trial_spawner_configs;
    list_array<std::unordered_map<std::string, trial_spawner_config>::iterator> trial_spawner_configs_cache;

    std::unordered_map<std::string, attribute> attributes;
    list_array<decltype(attributes)::iterator> attributes_cache;

    std::unordered_map<std::string, jukebox_song> jukebox_songs;
    list_array<decltype(jukebox_songs)::iterator> jukebox_songs_cache;


    util::nbt_compound current_protocol_registers;
    uint32_t current_protocol_id;

    std::string normalize_entry(const std::string& entry) {
        if (entry.find(':') != std::string::npos)
            if (entry.size())
                if (entry[0] != ':')
                    return entry;
        return "minecraft:" + entry;
    }

    std::string normalize_entry(std::string&& entry) {
        if (entry.find(':') != std::string::npos)
            if (entry.size())
                if (entry[0] != ':')
                    return std::move(entry);
        return "minecraft:" + entry;
    }

    util::nbt& view_registry_entries(const std::string& registry) {
        return current_protocol_registers.at(registry).at("entries");
    }

    util::nbt& view_registry_proto_invert(const std::string& registry) {
        return current_protocol_registers.at(registry).at("proto_invert");
    }

    list_array<int32_t> reg_ids(const std::string& registry) {
        list_array<int32_t> res;
        auto& reg = view_registry_entries(registry);
        res.reserve(reg.get_compound().size());
        for (auto&& [name, it] : reg.get_compound())
            res.push_back(it.at("protocol_id").as_int());
        return res;
    }

    int32_t view_reg_pro_id(const std::string& registry, const std::string& item) {
        if (item.contains(":") && !item.starts_with(':'))
            return view_registry_entries(registry).at(item).at("protocol_id").as_int();
        else if (item.starts_with(':'))
            return view_registry_entries(registry).at("minecraft" + item).at("protocol_id").as_int();
        else
            return view_registry_entries(registry).at("minecraft:" + item).at("protocol_id").as_int();
    }

    std::string_view view_reg_pro_name(const std::string& registry, int32_t id) {
        return (const std::string&)view_registry_proto_invert(registry).at(id);
    }

    list_array<int32_t> convert_reg_pro_id(const std::string& registry, const list_array<std::string>& items) {
        auto& entries = view_registry_entries(registry);
        return items.convert<int32_t>([&entries](const std::string& item) {
            if (item.contains(":") && !item.starts_with(':'))
                return entries.at(item).at("protocol_id").as_int();
            else if (item.starts_with(':'))
                return entries.at("minecraft" + item).at("protocol_id").as_int();
            else
                return entries.at("minecraft:" + item).at("protocol_id").as_int();
        });
    }

    list_array<std::string> convert_reg_pro_name(const std::string& registry, const list_array<int32_t>& items) {
        auto& entries = view_registry_proto_invert(registry);
        return items.convert<std::string>([&entries](const auto& item) { return entries.at(item).as_int(); });
    }

    list_array<int32_t> convert_reg_pro_id(const std::string& registry, const std::vector<std::string>& items) {
        auto& entries = view_registry_entries(registry);
        list_array<int32_t> result;
        result.reserve(items.size());
        for (const auto& item : items) {
            if (item.contains(":") && !item.starts_with(':'))
                result.push_back(entries.at(item).at("protocol_id").as_int());
            else if (item.starts_with(':'))
                result.push_back(entries.at("minecraft" + item).at("protocol_id").as_int());
            else
                result.push_back(entries.at("minecraft:" + item).at("protocol_id").as_int());
        }
        return result;
    }

    list_array<std::string> convert_reg_pro_name(const std::string& registry, const std::vector<int32_t>& item) {
        auto& entries = view_registry_proto_invert(registry);
        list_array<std::string> result;
        result.reserve(item.size());
        for (const auto& i : item)
            result.push_back(entries.at(i).get_string());
        return result;
    }

    std::unordered_map<std::string, potion> potions;
    list_array<decltype(potions)::iterator> potions_cache;

    std::unordered_map<std::string, effect> effects;
    list_array<decltype(effects)::iterator> effects_cache;

    std::unordered_map<std::string, enchantment> enchantments;
    list_array<decltype(enchantments)::iterator> enchantments_cache;
    std::unordered_map<std::string, enchantment_provider> enchantment_providers;
    list_array<decltype(enchantment_providers)::iterator> enchantment_providers_cache;

    std::unordered_map<std::string, loot_table_item> loot_table;
    list_array<decltype(loot_table)::iterator> loot_table_cache;

    std::unordered_map<std::string, base_objects::recipe> recipe_table;
    list_array<decltype(recipe_table)::iterator> recipe_table_cache;
}