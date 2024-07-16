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
    namespace registers {
#pragma region CLIENT/SERVER
        struct IntegerDistribution {
            std::string type;
            ENBT value;
        };

        struct ArmorTrimMaterial {
            std::string name;
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

        struct WolfVariant {
            std::string wild_texture;
            std::string tame_texture;
            std::string angry_texture;
            list_array<std::string> biomes;


            std::string name;
            uint32_t id;
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
        extern list_array<ArmorTrimMaterial> armorTrimMaterials;
        extern list_array<ArmorTrimPattern> armorTrimPatterns;
        extern list_array<Biome> biomes;
        extern list_array<ChatType> chatTypes;
        extern list_array<DamageType> damageTypes;
        extern list_array<DimensionType> dimensionTypes;
        extern list_array<WolfVariant> wolfVariants;


        //SERVER
        extern std::unordered_map<base_objects::block, uint16_t, base_objects::block_hash> blockPalette;
        extern std::unordered_map<uint16_t, EntityType*> entityList;
        extern std::unordered_map<int32_t, ItemType*> itemList;

        //CLIENT
        //till 765
        list_array<uint8_t>& registryDataPacket();
    }
}

#endif /* SRC_REGISTERS */
