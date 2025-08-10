/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/registers.hpp>

namespace copper_server {
    namespace registers {
        std::unordered_map<std::string, ArmorTrimMaterial> armorTrimMaterials;
        std::unordered_map<std::string, ArmorTrimPattern> armorTrimPatterns;
        std::unordered_map<std::string, Biome> biomes;
        std::unordered_map<std::string, ChatType> chatTypes;
        std::unordered_map<std::string, DamageType> damageTypes;
        std::unordered_map<std::string, DimensionType> dimensionTypes;
        std::unordered_map<std::string, WolfSoundVariant> wolfSoundVariants;
        std::unordered_map<std::string, WolfVariant> wolfVariants;
        std::unordered_map<std::string, EntityVariant> catVariants;
        std::unordered_map<std::string, EntityVariant> chickenVariants;
        std::unordered_map<std::string, EntityVariant> cowVariants;
        std::unordered_map<std::string, EntityVariant> pigVariants;
        std::unordered_map<std::string, EntityVariant> frogVariants;
        std::unordered_map<std::string, BannerPattern> bannerPatterns;
        std::unordered_map<std::string, PaintingVariant> paintingVariants;
        std::unordered_map<std::string, Instrument> instruments;

        list_array<std::unordered_map<std::string, ArmorTrimMaterial>::iterator> armorTrimMaterials_cache;
        list_array<std::unordered_map<std::string, ArmorTrimPattern>::iterator> armorTrimPatterns_cache;
        list_array<std::unordered_map<std::string, Biome>::iterator> biomes_cache;
        list_array<std::unordered_map<std::string, ChatType>::iterator> chatTypes_cache;
        list_array<std::unordered_map<std::string, DamageType>::iterator> damageTypes_cache;
        list_array<std::unordered_map<std::string, DimensionType>::iterator> dimensionTypes_cache;
        list_array<std::unordered_map<std::string, WolfSoundVariant>::iterator> wolfSoundVariants_cache;
        list_array<std::unordered_map<std::string, WolfVariant>::iterator> wolfVariants_cache;
        list_array<std::unordered_map<std::string, EntityVariant>::iterator> catVariants_cache;
        list_array<std::unordered_map<std::string, EntityVariant>::iterator> chickenVariants_cache;
        list_array<std::unordered_map<std::string, EntityVariant>::iterator> cowVariants_cache;
        list_array<std::unordered_map<std::string, EntityVariant>::iterator> pigVariants_cache;
        list_array<std::unordered_map<std::string, EntityVariant>::iterator> frogVariants_cache;
        list_array<std::unordered_map<std::string, BannerPattern>::iterator> bannerPatterns_cache;
        list_array<std::unordered_map<std::string, PaintingVariant>::iterator> paintingVariants_cache;
        list_array<std::unordered_map<std::string, Instrument>::iterator> instruments_cache;

        //SERVER
        std::unordered_map<std::string, Advancement> advancements;

        std::unordered_map<std::string, attribute> attributes;
        list_array<decltype(attributes)::iterator> attributes_cache;

        std::unordered_map<std::string, JukeboxSong> jukebox_songs;
        list_array<decltype(jukebox_songs)::iterator> jukebox_songs_cache;


        enbt::compound current_protocol_registers;
        uint32_t current_protocol_id;

        enbt::value& view_registry_entries(const std::string& registry) {
            return current_protocol_registers.at(registry).at("entries");
        }

        enbt::value& view_registry_proto_invert(const std::string& registry) {
            return current_protocol_registers.at(registry).at("proto_invert");
        }

        int32_t view_reg_pro_id(const std::string& registry, const std::string& item) {
            if (item.contains(":"))
                return view_registry_entries(registry).at(item).at("protocol_id");
            else
                return view_registry_entries(registry).at("minecraft:" + item).at("protocol_id");
        }

        std::string view_reg_pro_name(const std::string& registry, int32_t id) {
            return view_registry_proto_invert(registry).at(id);
        }

        list_array<int32_t> convert_reg_pro_id(const std::string& registry, const list_array<std::string>& item) {
            auto& entries = view_registry_entries(registry);
            return item.convert<int32_t>([&entries](const auto& item) {
                if (item.contains(":"))
                    return entries.at(item).at("protocol_id");
                else
                    return entries.at("minecraft:" + item).at("protocol_id");
            });
        }

        list_array<std::string> convert_reg_pro_name(const std::string& registry, const list_array<int32_t>& item) {
            auto& entries = view_registry_proto_invert(registry);
            return item.convert<std::string>([&entries](const auto& item) { return entries.at(item); });
        }

        list_array<int32_t> convert_reg_pro_id(const std::string& registry, const std::vector<std::string>& item) {
            auto& entries = view_registry_entries(registry);
            list_array<int32_t> result;
            result.reserve(item.size());
            for (const auto& i : item) {
                if (i.contains(":"))
                    result.push_back(entries.at(i).at("protocol_id"));
                else
                    result.push_back(entries.at("minecraft:" + i).at("protocol_id"));
            }
            return result;
        }

        list_array<std::string> convert_reg_pro_name(const std::string& registry, const std::vector<int32_t>& item) {
            auto& entries = view_registry_proto_invert(registry);
            list_array<std::string> result;
            result.reserve(item.size());
            for (const auto& i : item)
                result.push_back(entries.at(i));
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

        std::unordered_map<base_objects::block, uint16_t, base_objects::block_hash> blockPalette;
    }
}