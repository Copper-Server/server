
#include "registers.hpp"

namespace crafted_craft {
    namespace registers {
        extern std::unordered_map<std::string, ArmorTrimMaterial> armorTrimMaterials;
        extern std::unordered_map<std::string, ArmorTrimPattern> armorTrimPatterns;
        extern std::unordered_map<std::string, Biome> biomes;
        extern std::unordered_map<std::string, ChatType> chatTypes;
        extern std::unordered_map<std::string, DamageType> damageTypes;
        extern std::unordered_map<std::string, DimensionType> dimensionTypes;
        extern std::unordered_map<std::string, WolfVariant> wolfVariants;
        extern std::unordered_map<std::string, BannerPattern> bannerPatterns;
        extern std::unordered_map<std::string, PaintingVariant> paintingVariants;


        //SERVER
        std::unordered_map<base_objects::block, uint16_t, base_objects::block_hash> blockPalette;
        std::unordered_map<uint16_t, EntityType*> entityList;
        std::unordered_map<int32_t, ItemType*> itemList;
        std::unordered_map<std::string, std::unordered_map<std::string, list_array<std::string>>> tags; //[namespace][tag][values]
        std::string default_tag_namespace = "minecraft";

        const list_array<std::string>& unfold_tag(const std::string& namespace_, const std::string& tag) {
            static list_array<std::string> empty;
            auto ns = tags.find(namespace_);
            if (ns == tags.end())
                return empty;
            auto t = ns->second.find(tag);
            if (t == ns->second.end())
                return empty;
            return t->second;
        }

        const list_array<std::string>& unfold_tag(const std::string& tag) {
            return unfold_tag(default_tag_namespace, tag);
        }

        //CLIENT
        //till 765
        list_array<uint8_t>& registryDataPacket() {
            static list_array<uint8_t> data;
            if (data.empty()) {
                data.push_back(0x05);
                enbt::compound nbt;
                { //minecraft:trim_material
                    enbt::dynamic_array _armorTrimMaterials;
                    for (auto& [name, it] : armorTrimMaterials) {
                        enbt::compound tmp;
                        tmp["name"] = name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            element["asset_name"] = it.asset_name;
                            element["ingredient"] = it.ingredient;
                            element["item_model_index"] = it.item_model_index;
                            {
                                enbt::compound override_armor_materials;
                                for (auto& [key, value] : it.override_armor_materials)
                                    override_armor_materials[key] = value;
                                element["override_armor_materials"] = std::move(override_armor_materials);
                            }
                            if (std::holds_alternative<std::string>(it.description))
                                element["description"] = std::get<std::string>(it.description);
                            else
                                element["description"] = std::get<Chat>(it.description).ToENBT();
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
                    for (auto& [name, it] : armorTrimPatterns) {
                        enbt::compound tmp;
                        tmp["name"] = name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            element["assert_id"] = it.assert_id;
                            element["template_item"] = it.template_item;
                            if (std::holds_alternative<std::string>(it.description))
                                element["description"] = std::get<std::string>(it.description);
                            else
                                element["description"] = std::get<Chat>(it.description).ToENBT();
                            element["decal"] = it.decal;
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
                    for (auto& [name, it] : biomes) {
                        enbt::compound tmp;
                        tmp["name"] = name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            element["has_precipitation"] = it.has_precipitation;
                            element["temperature"] = it.temperature;
                            element["temperature_modifier"] = it.temperature_modifier;
                            element["downfall"] = it.downfall;
                            { //effects
                                enbt::compound effects;
                                effects["fog_color"] = it.effects.fog_color;
                                effects["water_color"] = it.effects.water_color;
                                effects["water_fog_color"] = it.effects.water_fog_color;
                                effects["sky_color"] = it.effects.sky_color;
                                if (it.effects.foliage_color)
                                    effects["foliage_color"] = it.effects.foliage_color.value();
                                if (it.effects.grass_color)
                                    effects["grass_color"] = it.effects.grass_color.value();
                                if (it.effects.grass_color_modifier)
                                    effects["grass_color_modifier"] = it.effects.grass_color_modifier.value();
                                if (it.effects.particle) {
                                    enbt::compound particle;
                                    particle["type"] = it.effects.particle->options.type;
                                    particle["options"] = it.effects.particle->options.options;
                                    effects["particle"] = std::move(particle);
                                }
                                if (it.effects.ambient_sound) {
                                    if (std::holds_alternative<std::string>(*it.effects.ambient_sound))
                                        effects["ambient_sound"] = std::get<std::string>(*it.effects.ambient_sound);
                                    else if (std::holds_alternative<Biome::AmbientSound>(*it.effects.ambient_sound)) {
                                        enbt::compound ambient_sound;
                                        ambient_sound["sound"] = std::get<Biome::AmbientSound>(*it.effects.ambient_sound).sound;
                                        ambient_sound["range"] = std::get<Biome::AmbientSound>(*it.effects.ambient_sound).range;
                                        effects["ambient_sound"] = std::move(ambient_sound);
                                    }
                                }
                                if (it.effects.mood_sound) {
                                    enbt::compound mood_sound;
                                    mood_sound["sound"] = it.effects.mood_sound->sound;
                                    mood_sound["tick_delay"] = it.effects.mood_sound->tick_delay;
                                    mood_sound["block_search_extend"] = it.effects.mood_sound->block_search_extend;
                                    mood_sound["offset"] = it.effects.mood_sound->offset;
                                    effects["mood_sound"] = std::move(mood_sound);
                                }
                                if (it.effects.additions_sound) {
                                    enbt::compound additions_sound;
                                    additions_sound["sound"] = it.effects.additions_sound->sound;
                                    additions_sound["tick_chance"] = it.effects.additions_sound->tick_chance;
                                    effects["additions_sound"] = std::move(additions_sound);
                                }
                                if (it.effects.music) {
                                    enbt::compound music;
                                    music["sound"] = it.effects.music->sound;
                                    music["min_delay"] = it.effects.music->min_delay;
                                    music["max_delay"] = it.effects.music->max_delay;
                                    music["replace_current_music"] = it.effects.music->replace_current_music;
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
                    for (auto& [name, it] : chatTypes) {
                        enbt::compound tmp;
                        tmp["name"] = name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            if (it.chat) {
                                enbt::compound chat;
                                chat["translation_key"] = it.chat->translation_key;
                                {
                                    it.chat->style.GetExtra().clear();
                                    it.chat->style.SetText("");
                                    ENBT style = it.chat->style.ToENBT();
                                    style.remove("text");
                                    chat["style"] = std::move(style);
                                }
                                if (std::holds_alternative<std::string>(it.chat->parameters))
                                    chat["parameters"] = std::get<std::string>(it.chat->parameters);
                                else
                                    chat["parameters"] = std::get<std::vector<std::string>>(it.chat->parameters);

                                element["chat"] = std::move(chat);
                            }
                            if (it.narration) {
                                enbt::compound narration;
                                narration["translation_key"] = it.narration->translation_key;
                                {
                                    it.narration->style.GetExtra().clear();
                                    it.narration->style.SetText("");
                                    ENBT style = it.chat->style.ToENBT();
                                    style.remove("text");
                                    narration["style"] = std::move(style);
                                }
                                if (std::holds_alternative<std::string>(it.narration->parameters))
                                    narration["parameters"] = std::get<std::string>(it.narration->parameters);
                                else
                                    narration["parameters"] = std::get<std::vector<std::string>>(it.narration->parameters);
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
                    for (auto& [name, it] : damageTypes) {
                        enbt::compound tmp;
                        tmp["name"] = name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            element["message_id"] = it.message_id;
                            {
                                const char* scaling = nullptr;
                                switch (it.scaling) {
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
                            element["exhaustion"] = it.exhaustion;
                            if (it.effects) {
                                const char* effect = nullptr;
                                switch (*it.effects) {
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
                            if (it.death_message_type) {
                                const char* death_message_type = nullptr;
                                switch (*it.death_message_type) {
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
                    for (auto& [name, it] : dimensionTypes) {
                        enbt::compound tmp;
                        tmp["name"] = name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            if (std::holds_alternative<int32_t>(it.monster_spawn_light_level))
                                element["monster_spawn_light_level"] = std::get<int32_t>(it.monster_spawn_light_level);
                            else {
                                enbt::compound distribution;
                                auto& ddd = std::get<IntegerDistribution>(it.monster_spawn_light_level);
                                distribution["type"] = ddd.type;
                                distribution["value"] = ddd.value;
                                element["monster_spawn_light_level"] = std::move(distribution);
                            }
                            if (it.fixed_time)
                                element["fixed_time"] = it.fixed_time.value();
                            element["infiniburn"] = it.infiniburn;
                            element["effects"] = it.effects;
                            element["coordinate_scale"] = it.coordinate_scale;
                            element["ambient_light"] = it.ambient_light;
                            element["min_y"] = it.min_y;
                            element["height"] = it.height;
                            element["logical_height"] = it.logical_height;
                            element["monster_spawn_block_light_limit"] = it.monster_spawn_block_light_limit;
                            element["has_skylight"] = it.has_skylight;
                            element["has_ceiling"] = it.has_ceiling;
                            element["ultrawarm"] = it.ultrawarm;
                            element["natural"] = it.natural;
                            element["piglin_safe"] = it.piglin_safe;
                            element["has_raids"] = it.has_raids;
                            element["respawn_anchor_works"] = it.respawn_anchor_works;
                            element["bed_works"] = it.bed_works;
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