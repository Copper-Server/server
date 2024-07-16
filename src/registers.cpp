
#include "registers.hpp"

namespace crafted_craft {
    namespace registers {
        list_array<ArmorTrimMaterial> armorTrimMaterials;
        list_array<ArmorTrimPattern> armorTrimPatterns;
        list_array<Biome> biomes;
        list_array<ChatType> chatTypes;
        list_array<DamageType> damageTypes;
        list_array<DimensionType> dimensionTypes;
        list_array<WolfVariant> wolfVariants;


        //SERVER
        std::unordered_map<base_objects::block, uint16_t, base_objects::block_hash> blockPalette;
        std::unordered_map<uint16_t, EntityType*> entityList;
        std::unordered_map<int32_t, ItemType*> itemList;

        //CLIENT
        //till 765
        list_array<uint8_t>& registryDataPacket() {
            static list_array<uint8_t> data;
            if (data.empty()) {
                data.push_back(0x05);
                enbt::compound nbt;
                { //minecraft:trim_material
                    enbt::dynamic_array _armorTrimMaterials;
                    for (auto& it : armorTrimMaterials) {
                        enbt::compound tmp;
                        tmp["name"] = it.name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            element["asset_name"] = it.element.asset_name;
                            element["ingredient"] = it.element.ingredient;
                            element["item_model_index"] = it.element.item_model_index;
                            if (std::holds_alternative<std::string>(it.element.override_armor_materials))
                                element["override_armor_materials"] = std::get<std::string>(it.element.override_armor_materials);
                            else
                                element["override_armor_materials"] = std::get<std::vector<std::string>>(it.element.override_armor_materials);
                            if (std::holds_alternative<std::string>(it.element.description))
                                element["description"] = std::get<std::string>(it.element.description);
                            else
                                element["description"] = std::get<Chat>(it.element.description).ToENBT();
                            tmp["element"] = std::move(element);
                        }
                        _armorTrimMaterials.push_back((ENBT&&)std::move(tmp));
                    }
                    enbt::compound entry;
                    entry["type"] = "minecraft:trim_material";
                    entry["value"] = std::move(_armorTrimMaterials);
                    nbt["minecraft:trim_material"] = std::move(entry);
                }
                { //minecraft:trim_pattern
                    enbt::dynamic_array _armorTrimPatterns;
                    for (auto& it : armorTrimPatterns) {
                        enbt::compound tmp;
                        tmp["name"] = it.name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            element["assert_id"] = it.element.assert_id;
                            element["template_item"] = it.element.template_item;
                            if (std::holds_alternative<std::string>(it.element.description))
                                element["description"] = std::get<std::string>(it.element.description);
                            else
                                element["description"] = std::get<Chat>(it.element.description).ToENBT();
                            element["decal"] = it.element.decal;
                            tmp["element"] = std::move(element);
                        }
                        _armorTrimPatterns.push_back((ENBT&&)std::move(tmp));
                    }
                    enbt::compound entry;
                    entry["type"] = "minecraft:trim_pattern";
                    entry["value"] = std::move(_armorTrimPatterns);
                    nbt["minecraft:trim_pattern"] = std::move(entry);
                }
                { //minecraft:worldgen/biome
                    enbt::dynamic_array _biomes;
                    for (auto& it : biomes) {
                        enbt::compound tmp;
                        tmp["name"] = it.name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            element["has_precipitation"] = it.element.has_precipitation;
                            element["temperature"] = it.element.temperature;
                            element["temperature_modifier"] = it.element.temperature_modifier;
                            element["downfall"] = it.element.downfall;
                            { //effects
                                enbt::compound effects;
                                effects["fog_color"] = it.element.effects.fog_color;
                                effects["water_color"] = it.element.effects.water_color;
                                effects["water_fog_color"] = it.element.effects.water_fog_color;
                                effects["sky_color"] = it.element.effects.sky_color;
                                if (it.element.effects.foliage_color)
                                    effects["foliage_color"] = it.element.effects.foliage_color.value();
                                if (it.element.effects.grass_color)
                                    effects["grass_color"] = it.element.effects.grass_color.value();
                                if (it.element.effects.grass_color_modifier)
                                    effects["grass_color_modifier"] = it.element.effects.grass_color_modifier.value();
                                if (it.element.effects.particle) {
                                    enbt::compound particle;
                                    particle["type"] = it.element.effects.particle->options.type;
                                    particle["options"] = it.element.effects.particle->options.options;
                                    effects["particle"] = std::move(particle);
                                }
                                if (it.element.effects.ambient_sound) {
                                    if (std::holds_alternative<std::string>(*it.element.effects.ambient_sound))
                                        effects["ambient_sound"] = std::get<std::string>(*it.element.effects.ambient_sound);
                                    else if (std::holds_alternative<Biome::AmbientSound>(*it.element.effects.ambient_sound)) {
                                        enbt::compound ambient_sound;
                                        ambient_sound["sound"] = std::get<Biome::AmbientSound>(*it.element.effects.ambient_sound).sound;
                                        ambient_sound["range"] = std::get<Biome::AmbientSound>(*it.element.effects.ambient_sound).range;
                                        effects["ambient_sound"] = std::move(ambient_sound);
                                    }
                                }
                                if (it.element.effects.mood_sound) {
                                    enbt::compound mood_sound;
                                    mood_sound["sound"] = it.element.effects.mood_sound->sound;
                                    mood_sound["tick_delay"] = it.element.effects.mood_sound->tick_delay;
                                    mood_sound["block_search_extend"] = it.element.effects.mood_sound->block_search_extend;
                                    mood_sound["offset"] = it.element.effects.mood_sound->offset;
                                    effects["mood_sound"] = std::move(mood_sound);
                                }
                                if (it.element.effects.additions_sound) {
                                    enbt::compound additions_sound;
                                    additions_sound["sound"] = it.element.effects.additions_sound->sound;
                                    additions_sound["tick_chance"] = it.element.effects.additions_sound->tick_chance;
                                    effects["additions_sound"] = std::move(additions_sound);
                                }
                                if (it.element.effects.music) {
                                    enbt::compound music;
                                    music["sound"] = it.element.effects.music->sound;
                                    music["min_delay"] = it.element.effects.music->min_delay;
                                    music["max_delay"] = it.element.effects.music->max_delay;
                                    music["replace_current_music"] = it.element.effects.music->replace_current_music;
                                    effects["music"] = std::move(music);
                                }
                                element["effects"] = std::move(effects);
                            }
                            tmp["element"] = std::move(element);
                        }
                        _biomes.push_back((ENBT&&)std::move(tmp));
                    }
                    enbt::compound entry;
                    entry["type"] = "minecraft:worldgen/biome";
                    entry["value"] = std::move(_biomes);
                    nbt["minecraft:worldgen/biome"] = std::move(entry);
                }
                { // minecraft:chat_type
                    enbt::dynamic_array _chatTypes;
                    for (auto& it : chatTypes) {
                        enbt::compound tmp;
                        tmp["name"] = it.name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            if (it.element.chat) {
                                enbt::compound chat;
                                chat["translation_key"] = it.element.chat->translation_key;
                                {
                                    it.element.chat->style.GetExtra().clear();
                                    it.element.chat->style.SetText("");
                                    ENBT style = it.element.chat->style.ToENBT();
                                    style.remove("text");
                                    chat["style"] = std::move(style);
                                }
                                if (std::holds_alternative<std::string>(it.element.chat->parameters))
                                    chat["parameters"] = std::get<std::string>(it.element.chat->parameters);
                                else
                                    chat["parameters"] = std::get<std::vector<std::string>>(it.element.chat->parameters);

                                element["chat"] = std::move(chat);
                            }
                            if (it.element.narration) {
                                enbt::compound narration;
                                narration["translation_key"] = it.element.narration->translation_key;
                                {
                                    it.element.narration->style.GetExtra().clear();
                                    it.element.narration->style.SetText("");
                                    ENBT style = it.element.chat->style.ToENBT();
                                    style.remove("text");
                                    narration["style"] = std::move(style);
                                }
                                if (std::holds_alternative<std::string>(it.element.narration->parameters))
                                    narration["parameters"] = std::get<std::string>(it.element.narration->parameters);
                                else
                                    narration["parameters"] = std::get<std::vector<std::string>>(it.element.narration->parameters);
                                element["narration"] = std::move(narration);
                            }
                            tmp["element"] = std::move(element);
                        }
                        _chatTypes.push_back((ENBT&&)std::move(tmp));
                    }
                    enbt::compound entry;
                    entry["type"] = "minecraft:chat_type";
                    entry["value"] = std::move(_chatTypes);
                    nbt["minecraft:chat_type"] = std::move(entry);
                }
                { // minecraft:damage_type
                    enbt::dynamic_array _damageTypes;
                    for (auto& it : damageTypes) {
                        enbt::compound tmp;
                        tmp["name"] = it.name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            element["message_id"] = it.element.message_id;
                            {
                                const char* scaling = nullptr;
                                switch (it.element.scaling) {
                                case DamageType::ScalingType::never:
                                    scaling = "never";
                                    break;
                                case DamageType::ScalingType::when_caused_by_living_non_player:
                                    scaling = "when_caused_by_living_non_player";
                                    break;
                                case DamageType::ScalingType::always:
                                    scaling = "always";
                                    break;
                                }
                                if (scaling)
                                    element["scaling"] = scaling;
                            }
                            element["exhaustion"] = it.element.exhaustion;
                            if (it.element.effects) {
                                const char* effect = nullptr;
                                switch (*it.element.effects) {
                                case DamageType::EffectsType::hurt:
                                    effect = "hurt";
                                    break;
                                case DamageType::EffectsType::thorns:
                                    effect = "thorns";
                                    break;
                                case DamageType::EffectsType::drowning:
                                    effect = "drowning";
                                    break;
                                case DamageType::EffectsType::burning:
                                    effect = "burning";
                                    break;
                                case DamageType::EffectsType::poking:
                                    effect = "poking";
                                    break;
                                case DamageType::EffectsType::freezing:
                                    effect = "freezing";
                                    break;
                                default:
                                    break;
                                }
                                if (effect)
                                    element["effects"] = effect;
                            }
                            if (it.element.death_message_type) {
                                const char* death_message_type = nullptr;
                                switch (*it.element.death_message_type) {
                                case DamageType::DeathMessageType::_default:
                                    death_message_type = "default";
                                    break;
                                case DamageType::DeathMessageType::fall_variants:
                                    death_message_type = "fall_variants";
                                    break;
                                case DamageType::DeathMessageType::intentional_game_design:
                                    death_message_type = "intentional_game_design";
                                    break;
                                default:
                                    break;
                                }
                                if (death_message_type)
                                    element["death_message_type"] = death_message_type;
                            }
                            tmp["element"] = std::move(element);
                        }
                        _damageTypes.push_back((ENBT&&)std::move(tmp));
                    }
                    enbt::compound entry;
                    entry["type"] = "minecraft:damage_type";
                    entry["value"] = std::move(_damageTypes);
                    nbt["minecraft:damage_type"] = std::move(entry);
                }
                { // minecraft:dimension_type
                    enbt::dynamic_array _dimensionTypes;
                    for (auto& it : dimensionTypes) {
                        enbt::compound tmp;
                        tmp["name"] = it.name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            if (std::holds_alternative<int32_t>(it.element.monster_spawn_light_level))
                                element["monster_spawn_light_level"] = std::get<int32_t>(it.element.monster_spawn_light_level);
                            else {
                                enbt::compound distribution;
                                auto& ddd = std::get<IntegerDistribution>(it.element.monster_spawn_light_level);
                                distribution["type"] = ddd.type;
                                distribution["value"] = ddd.value;
                                element["monster_spawn_light_level"] = std::move(distribution);
                            }
                            if (it.element.fixed_time)
                                element["fixed_time"] = it.element.fixed_time.value();
                            element["infiniburn"] = it.element.infiniburn;
                            element["effects"] = it.element.effects;
                            element["coordinate_scale"] = it.element.coordinate_scale;
                            element["ambient_light"] = it.element.ambient_light;
                            element["min_y"] = it.element.min_y;
                            element["height"] = it.element.height;
                            element["logical_height"] = it.element.logical_height;
                            element["monster_spawn_block_light_limit"] = it.element.monster_spawn_block_light_limit;
                            element["has_skylight"] = it.element.has_skylight;
                            element["has_ceiling"] = it.element.has_ceiling;
                            element["ultrawarm"] = it.element.ultrawarm;
                            element["natural"] = it.element.natural;
                            element["piglin_safe"] = it.element.piglin_safe;
                            element["has_raids"] = it.element.has_raids;
                            element["respawn_anchor_works"] = it.element.respawn_anchor_works;
                            element["bed_works"] = it.element.bed_works;
                            tmp["element"] = std::move(element);
                        }
                        _dimensionTypes.push_back((ENBT&&)std::move(tmp));
                    }
                    enbt::compound entry;
                    entry["type"] = "minecraft:dimension_type";
                    entry["value"] = std::move(_dimensionTypes);
                    nbt["minecraft:dimension_type"] = std::move(entry);
                }
                data.push_back(NBT::build((ENBT&)nbt).get_as_network());
            }
            return data;
        }
    }
}