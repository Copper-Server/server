#ifndef SRC_REGISTERS
#define SRC_REGISTERS
#include "base_objects/block.hpp"
#include "base_objects/chat.hpp"
#include "base_objects/position.hpp"
#include "library/enbt.hpp"
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace crafted_craft {

    struct registers {
#pragma region CLIENT/SERVER
        struct IntegerDistribution {
            std::string type;
            ENBT value;
        };

        struct ArmorTrimMaterial {
            std::string& name;
            int32_t id;

            struct {
                std::string asset_name;
                std::string ingredient;
                float item_model_index;
                std::variant<std::string, std::vector<std::string>> override_armor_materials; //leather, chainmail, iron, gold, diamond, turtle, netherite
                std::variant<std::string, Chat> description;
            } element;
        };

        struct ArmorTrimPattern {
            std::string name;
            uint32_t id;

            struct {
                std::string assert_id;
                std::string template_item;
                std::variant<std::string, Chat> description;
                bool decal;
            } element;
        };

        struct Biome {
            struct Particle {
                struct {
                    std::string type;
                    ENBT options;
                } options;

                float probability;
            };

            struct AmbientSound {
                std::string sound;
                float range;
            };

            struct MoodSound {
                std::string sound;
                int32_t tick_delay = 6000;
                int32_t block_search_extend = 8;
                double offset = 2.0;
            };

            struct AdditionsSound {
                std::string sound;
                double tick_chance;
            };

            struct Music {
                std::string sound;
                int32_t min_delay = 12000;
                int32_t max_delay = 24000;
                bool replace_current_music = true;
            };

            std::string name;
            uint32_t id;

            struct {
                bool has_precipitation;
                float temperature;
                float temperature_modifier;
                float downfall;

                struct {
                    int32_t fog_color;
                    int32_t water_color;
                    int32_t water_fog_color;
                    int32_t sky_color;
                    std::optional<int32_t> foliage_color;
                    std::optional<int32_t> grass_color;
                    std::optional<int32_t> grass_color_modifier;
                    std::optional<Particle> particle;
                    std::optional<std::variant<std::string, AmbientSound>> ambient_sound;
                    std::optional<MoodSound> mood_sound;
                    std::optional<AdditionsSound> additions_sound;
                    std::optional<Music> music;
                } effects;
            } element;
        };

        struct ChatType {
            struct Decoration {
                std::string translation_key;
                Chat style;                                                     //main text and extra chat will be ignored
                std::variant<std::string, std::vector<std::string>> parameters; // sender, target, content
            };

            std::string name;
            uint32_t id;

            struct {
                std::optional<Decoration> chat;
                std::optional<Decoration> narration;
            } element;
        };

        struct DamageType {
            enum ScalingType {
                never,
                when_caused_by_living_non_player,
                always
            };

            enum EffectsType {
                hurt,
                thorns,
                drowning,
                burning,
                poking,
                freezing
            };

            enum DeathMessageType {
                _default, //"default"
                fall_variants,
                intentional_game_design
            };

            std::string name;
            uint32_t id;

            struct {
                std::string message_id;
                ScalingType scaling; //as string
                float exhaustion;
                std::optional<EffectsType> effects;
                std::optional<DeathMessageType> death_message_type;
            } element;
        };

        struct DimensionType {
            std::string name;
            uint32_t id;

            struct {
                std::variant<int32_t, IntegerDistribution> monster_spawn_light_level;
                std::optional<uint64_t> fixed_time;
                std::string infiniburn;
                std::string effects;
                double coordinate_scale;
                float ambient_light;
                int32_t min_y;
                int32_t height;
                int32_t logical_height;
                int32_t monster_spawn_block_light_limit;
                bool has_skylight : 1;
                bool has_ceiling : 1;
                bool ultrawarm : 1;
                bool natural : 1;
                bool piglin_safe : 1;
                bool has_raids : 1;
                bool respawn_anchor_works : 1;
                bool bed_works : 1;
            } element;
        };

#pragma endregion
#pragma region server

        struct EntityType {
            std::string id;
            virtual std::string GetName() = 0;

            //bunch ai stuff and other
            //...
        };

        struct ItemType {
            std::string id;
            uint8_t max_count;
        };

        struct BlockPalette {
            uint16_t id : 15;


            std::string full_id;
        };


#pragma endregion
        //CLIENT/SERVER
        static list_array<ArmorTrimMaterial> armorTrimMaterials;
        static list_array<ArmorTrimPattern> armorTrimPatterns;
        static list_array<Biome> biomes;
        static list_array<ChatType> chatTypes;
        static list_array<DamageType> damageTypes;
        static list_array<DimensionType> dimensionTypes;


        //SERVER
        static std::unordered_map<base_objects::block, uint16_t, base_objects::block_hash> blockPalette;
        static std::unordered_map<uint16_t, EntityType*> entityList;
        static std::unordered_map<int32_t, ItemType*> itemList;

        //CLIENT
        static list_array<uint8_t>& registryDataPacket() {
            static list_array<uint8_t> data;
            if (data.empty()) {
                data.push_back(0x05);
                ENBT nbt = ENBT::compound();
                { //minecraft:trim_material
                    ENBT _armorTrimMaterials = ENBT::fixed_array();
                    for (auto& it : armorTrimMaterials) {
                        ENBT tmp = ENBT::compound();
                        tmp["name"] = it.name;
                        tmp["id"] = it.id;
                        { //element
                            ENBT element = ENBT::compound();
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
                        _armorTrimMaterials.push(std::move(tmp));
                    }
                    ENBT entry = ENBT::compound();
                    entry["type"] = "minecraft:trim_material";
                    entry["value"] = std::move(_armorTrimMaterials);
                    nbt["minecraft:trim_material"] = std::move(entry);
                }
                { //minecraft:trim_pattern
                    ENBT _armorTrimPatterns = ENBT::fixed_array();
                    for (auto& it : armorTrimPatterns) {
                        ENBT tmp = ENBT::compound();
                        tmp["name"] = it.name;
                        tmp["id"] = it.id;
                        { //element
                            ENBT element = ENBT::compound();
                            element["assert_id"] = it.element.assert_id;
                            element["template_item"] = it.element.template_item;
                            if (std::holds_alternative<std::string>(it.element.description))
                                element["description"] = std::get<std::string>(it.element.description);
                            else
                                element["description"] = std::get<Chat>(it.element.description).ToENBT();
                            element["decal"] = it.element.decal;
                            tmp["element"] = std::move(element);
                        }
                        _armorTrimPatterns.push(std::move(tmp));
                    }
                    ENBT entry = ENBT::compound();
                    entry["type"] = "minecraft:trim_pattern";
                    entry["value"] = std::move(_armorTrimPatterns);
                    nbt["minecraft:trim_pattern"] = std::move(entry);
                }
                { //minecraft:worldgen/biome
                    ENBT _biomes = ENBT::fixed_array();
                    for (auto& it : biomes) {
                        ENBT tmp = ENBT::compound();
                        tmp["name"] = it.name;
                        tmp["id"] = it.id;
                        { //element
                            ENBT element = ENBT::compound();
                            element["has_precipitation"] = it.element.has_precipitation;
                            element["temperature"] = it.element.temperature;
                            element["temperature_modifier"] = it.element.temperature_modifier;
                            element["downfall"] = it.element.downfall;
                            { //effects
                                ENBT effects = ENBT::compound();
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
                                    ENBT particle = ENBT::compound();
                                    particle["type"] = it.element.effects.particle->options.type;
                                    particle["options"] = it.element.effects.particle->options.options;
                                    effects["particle"] = std::move(particle);
                                }
                                if (it.element.effects.ambient_sound) {
                                    if (std::holds_alternative<std::string>(*it.element.effects.ambient_sound))
                                        effects["ambient_sound"] = std::get<std::string>(*it.element.effects.ambient_sound);
                                    else if (std::holds_alternative<Biome::AmbientSound>(*it.element.effects.ambient_sound)) {
                                        ENBT ambient_sound = ENBT::compound();
                                        ambient_sound["sound"] = std::get<Biome::AmbientSound>(*it.element.effects.ambient_sound).sound;
                                        ambient_sound["range"] = std::get<Biome::AmbientSound>(*it.element.effects.ambient_sound).range;
                                        effects["ambient_sound"] = std::move(ambient_sound);
                                    }
                                }
                                if (it.element.effects.mood_sound) {
                                    ENBT mood_sound = ENBT::compound();
                                    mood_sound["sound"] = it.element.effects.mood_sound->sound;
                                    mood_sound["tick_delay"] = it.element.effects.mood_sound->tick_delay;
                                    mood_sound["block_search_extend"] = it.element.effects.mood_sound->block_search_extend;
                                    mood_sound["offset"] = it.element.effects.mood_sound->offset;
                                    effects["mood_sound"] = std::move(mood_sound);
                                }
                                if (it.element.effects.additions_sound) {
                                    ENBT additions_sound = ENBT::compound();
                                    additions_sound["sound"] = it.element.effects.additions_sound->sound;
                                    additions_sound["tick_chance"] = it.element.effects.additions_sound->tick_chance;
                                    effects["additions_sound"] = std::move(additions_sound);
                                }
                                if (it.element.effects.music) {
                                    ENBT music = ENBT::compound();
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
                        _biomes.push(std::move(tmp));
                    }
                    ENBT entry = ENBT::compound();
                    entry["type"] = "minecraft:worldgen/biome";
                    entry["value"] = std::move(_biomes);
                    nbt["minecraft:worldgen/biome"] = std::move(entry);
                }
                { // minecraft:chat_type
                    ENBT _chatTypes = ENBT::fixed_array();
                    for (auto& it : chatTypes) {
                        ENBT tmp = ENBT::compound();
                        tmp["name"] = it.name;
                        tmp["id"] = it.id;
                        { //element
                            ENBT element = ENBT::compound();
                            if (it.element.chat) {
                                ENBT chat = ENBT::compound();
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
                                ENBT narration = ENBT::compound();
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
                        _chatTypes.push(std::move(tmp));
                    }
                    ENBT entry = ENBT::compound();
                    entry["type"] = "minecraft:chat_type";
                    entry["value"] = std::move(_chatTypes);
                    nbt["minecraft:chat_type"] = std::move(entry);
                }
                { // minecraft:damage_type
                    ENBT _damageTypes = ENBT::fixed_array();
                    for (auto& it : damageTypes) {
                        ENBT tmp = ENBT::compound();
                        tmp["name"] = it.name;
                        tmp["id"] = it.id;
                        { //element
                            ENBT element = ENBT::compound();
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
                        _damageTypes.push(std::move(tmp));
                    }
                    ENBT entry = ENBT::compound();
                    entry["type"] = "minecraft:damage_type";
                    entry["value"] = std::move(_damageTypes);
                    nbt["minecraft:damage_type"] = std::move(entry);
                }
                { // minecraft:dimension_type
                    ENBT _dimensionTypes = ENBT::fixed_array();
                    for (auto& it : dimensionTypes) {
                        ENBT tmp = ENBT::compound();
                        tmp["name"] = it.name;
                        tmp["id"] = it.id;
                        { //element
                            ENBT element = ENBT::compound();
                            if (std::holds_alternative<int32_t>(it.element.monster_spawn_light_level))
                                element["monster_spawn_light_level"] = std::get<int32_t>(it.element.monster_spawn_light_level);
                            else {
                                ENBT distribution = ENBT::compound();
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
                        _dimensionTypes.push(std::move(tmp));
                    }
                    ENBT entry = ENBT::compound();
                    entry["type"] = "minecraft:dimension_type";
                    entry["value"] = std::move(_dimensionTypes);
                    nbt["minecraft:dimension_type"] = std::move(entry);
                }
                data.push_back(NBT(nbt).get_as_network());
            }
            return data;
        }
    };

}

#endif /* SRC_REGISTERS */
