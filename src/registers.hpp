#ifndef SRC_REGISTERS
#define SRC_REGISTERS
#include "base_objects/block.hpp"
#include "base_objects/chat.hpp"
#include "base_objects/entity.hpp"
#include "base_objects/float_provider.hpp"
#include "base_objects/number_provider.hpp"
#include "base_objects/particle_data.hpp"
#include "base_objects/position.hpp"
#include "base_objects/slot.hpp"
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
            enbt::value value;
        };

        struct Advancement {
            struct Display {
                struct Icon {
                    std::string item;
                    std::string nbt;
                };

                Icon icon;
                std::string title;
                std::string frame;
                std::string background;
                std::string description;
                bool show_toast;
                bool announce_to_chat;
                bool hidden;
            };

            struct Rewards {
                std::vector<std::string> recipes;
                std::vector<std::string> loot;
                int32_t experience = 0;
                std::string function;
            };

            std::optional<Display> display;
            std::string parent;
            enbt::compound criteria;
            std::vector<std::vector<std::string>> requirements;
            Rewards rewards;
            bool sends_telemetry_event = false;
        };

        using JukeboxSong = base_objects::slot_component::jukebox_playable::jukebox_extended;

        struct ArmorTrimMaterial {
            std::string asset_name;
            std::string ingredient;
            std::unordered_map<std::string, std::string> override_armor_materials; //leather, chainmail, iron, gold, diamond, turtle, netherite
            std::variant<std::string, Chat> description;
            float item_model_index;
            int32_t id;
            bool allow_override = false;
        };

        struct ArmorTrimPattern {
            std::string assert_id;
            std::string template_item;
            std::variant<std::string, Chat> description;
            bool decal;
            bool allow_override = false;
            uint32_t id;
        };

        struct Biome {
            struct Particle {
                struct {
                    std::string type;
                    enbt::value options;
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

            struct SpawnersValue {
                std::string type;
                uint32_t max_count;
                uint32_t min_count;
                uint32_t weight;
            };

            struct SpawnCostsValue {
                double energy_budget;
                double charge;
            };

            uint32_t id;
            bool allow_override = false;

            bool has_precipitation;
            float temperature;
            float downfall;
            std::optional<std::string> temperature_modifier;

            struct Effects {
                int32_t fog_color;
                int32_t water_color;
                int32_t water_fog_color;
                int32_t sky_color;
                std::optional<int32_t> foliage_color;
                std::optional<int32_t> grass_color;
                std::optional<std::string> grass_color_modifier;
                std::optional<Particle> particle;
                std::optional<std::variant<std::string, AmbientSound>> ambient_sound;
                std::optional<MoodSound> mood_sound;
                std::optional<AdditionsSound> additions_sound;
                std::optional<Music> music;
            } effects;

            //server side:
            std::unordered_map<std::string, std::vector<std::string>> carvers; //air, liquid
            //features field divided by generation steps:
            //RAW_GENERATION
            //LAKES
            //LOCAL_MODIFICATIONS
            //UNDERGROUND_STRUCTURES
            //SURFACE_STRUCTURES
            //STRONGHOLDS
            //UNDERGROUND_ORES
            //UNDERGROUND_DECORATION
            //FLUID_SPRINGS
            //VEGETAL_DECORATION
            //TOP_LAYER_MODIFICATION
            std::vector<std::vector<std::string>> features;
            std::unordered_map<std::string, std::vector<SpawnersValue>> spawners;
            //mob_id>config
            std::unordered_map<std::string, SpawnCostsValue> spawn_costs;

            double creature_spawn_probability = 0;
        };

        struct ChatType {
            struct Decoration {
                std::string translation_key;
                Chat style;                                                     //main text and extra chat will be ignored
                std::variant<std::string, std::vector<std::string>> parameters; // sender, target, content
            };

            std::optional<Decoration> chat;
            std::optional<Decoration> narration;

            uint32_t id;
            bool allow_override = false;
        };

        struct DamageType {
            enum class ScalingType {
                never,
                when_caused_by_living_non_player,
                always
            };

            enum class EffectsType {
                hurt,
                thorns,
                drowning,
                burning,
                poking,
                freezing
            };

            enum class DeathMessageType {
                _default, //"default"
                fall_variants,
                intentional_game_design
            };


            std::string message_id;
            ScalingType scaling; //as string
            std::optional<EffectsType> effects;
            std::optional<DeathMessageType> death_message_type;
            float exhaustion;
            uint32_t id;

            bool allow_override = false;
        };

        struct DimensionType {

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


            bool allow_override : 1 = false;
            uint32_t id;
        };

        struct WolfVariant {
            std::string wild_texture;
            std::string tame_texture;
            std::string angry_texture;
            list_array<std::string> biomes;

            uint32_t id;
            bool allow_override = false;
        };

        struct BannerPattern {
            std::string asset_id;
            std::string translation_key;

            uint32_t id;
            bool allow_override = false;
        };

        struct PaintingVariant {
            std::string asset_id;
            uint32_t height;
            uint32_t width;

            uint32_t id;
            bool allow_override = false;
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

        struct enchantment {
            Chat description;
            std::variant<std::string, std::vector<std::string>> exclusive_set;
            std::variant<std::string, std::vector<std::string>> supported_items;
            std::variant<std::string, std::vector<std::string>> primary_items;
            std::vector<std::string> slots;
            std::unordered_map<std::string, std::vector<enbt::compound>> effects;

            struct {
                int32_t base;
                int32_t per_level_above_first;
            } min_cost;

            struct {
                int32_t base;
                int32_t per_level_above_first;
            } max_cost;

            int32_t anvil_cost;
            int32_t weight;
            uint8_t max_level;

            uint32_t id;
        };

        struct potion {
        };

        struct item_modifier {
        };

        struct loot_table_item {
            struct pool {
                base_objects::number_provider rolls;
                base_objects::number_provider bonus_rolls;
                std::vector<enbt::compound> entries;
                std::vector<enbt::compound> functions;
                std::vector<enbt::compound> conditions; //predicates, can be empty
            };

            std::vector<pool> pools;
            std::vector<enbt::compound> functions;
            std::string type; //default: generic // used to filter loot context
            std::optional<std::string> random_sequence;
        };

        namespace world_gen {
            using biome = Biome;

            struct configured_carver {
                std::string type;

                struct {
                    float probability;
                    enbt::compound y; //number provider

                    struct {
                        int32_t absolute;
                        int32_t above_bottom;
                        int32_t below_top;
                    } lava_level;

                    std::variant<std::string, std::vector<std::string>> replaceable;

                    struct Debug_settings {
                        bool debug;

                        struct state {
                            std::string name;
                            std::unordered_map<std::string, std::string> properties;
                        };

                        state air_state;
                        state water_state;
                        state lava_state;
                        state barrier_state;
                    };

                    std::optional<Debug_settings> debug_settings;

                    enbt::compound custom_data; //virtual field, used in handlers
                } config;
            };

            struct configured_feature {
                std::string type;
                enbt::compound config;
            };

            struct density_function {
                std::string type;
                enbt::compound custom_data; //virtual field, used in handlers
            };

            struct noise {
                int32_t firstOctave;
                std::vector<double> amplitudes;
            };

            struct noise_settings {
                int32_t sea_level;
                bool disable_mob_generation;
                bool ore_veins_enabled;
                bool aquifers_enabled;
                bool legacy_random_source;

                struct state {
                    std::string name;
                    std::unordered_map<std::string, std::string> properties;
                };

                state default_block;
                state default_fluid;

                struct spawn_target_v {
                    struct temperature_value {
                        float min;
                        float max;
                    };

                    using variants = std::variant<temperature_value, std::vector<temperature_value>, float>;
                    variants temperature;
                    variants humidity;
                    variants continentalness;
                    variants erosion;
                    variants weirdness;
                    variants depth;
                    float offset;
                };

                std::vector<spawn_target_v> spawn_target;

                struct {
                    int32_t min_y;
                    int32_t height;
                    int32_t size_horizontal;
                    int32_t size_vertical;
                } noise;

                enbt::compound noise_router;
                enbt::compound surface_rule;
            };

            struct placed_feature {
                std::variant<std::string, configured_feature> feature;
                std::vector<enbt::compound> placement;
            };

            struct processor_list {
                std::vector<enbt::compound> processors;
            };

            struct structure {
                struct spawn_override {
                    struct spawn {
                        std::string type;
                        int32_t weight;
                        int32_t min_count;
                        int32_t max_count;
                    };

                    std::string bounding_box;
                    std::vector<spawn> spawns;
                };

                std::string type;
                std::variant<std::string, std::vector<std::string>> biomes;
                std::string step;
                std::string terrain_adaptation;
                std::unordered_map<std::string, spawn_override> spawn_overrides;
                enbt::compound custom_data; //virtual field, used in handlers
            };

            struct structure_set {
                std::vector<std::variant<std::string, structure>> structures;

                struct {
                    int32_t salt;
                    float frequency = 1.0;
                    std::string frequency_reduction_method = "default";

                    struct {
                        int32_t chunk_count;
                        std::string other_set;
                    } exclusion_zone;

                    int32_t locale_offset[3] = {0, 0, 0};

                    std::string type;
                    enbt::compound custom_data; //virtual field, used in handlers
                } placement;
            };

            struct template_pool {
                struct element {
                    int32_t weight;

                    struct {
                        std::string element_type;
                        std::string projection;
                        enbt::compound custom_data; //virtual field, used in handlers
                    } element;
                };

                std::string fallback;
                std::vector<element> elements;
            };

            struct world_preset {
                struct dimension {
                    std::string type;
                    enbt::compound custom_data; //virtual field, used in handlers
                };

                std::unordered_map<std::string, dimension> dimensions;
            };

            struct flat_level_generator_preset {
                struct layer {
                    std::string block;
                    int32_t height;
                };

                std::string display;

                struct {
                    std::vector<layer> layers;
                    std::string biome;
                    bool lakes = false;
                    bool features = false;
                    std::vector<std::string> structure_overrides;
                } settings;
            };

            struct multi_noise_biome_source_parameter_list {
                std::string preset; //ref to hardcoded preset
            };
        }


#pragma endregion
        //CLIENT/SERVER
        extern std::unordered_map<std::string, ArmorTrimMaterial> armorTrimMaterials;
        extern std::unordered_map<std::string, ArmorTrimPattern> armorTrimPatterns;
        extern std::unordered_map<std::string, Biome> biomes;
        extern std::unordered_map<std::string, ChatType> chatTypes;
        extern std::unordered_map<std::string, DamageType> damageTypes;
        extern std::unordered_map<std::string, DimensionType> dimensionTypes;
        extern std::unordered_map<std::string, WolfVariant> wolfVariants;
        extern std::unordered_map<std::string, BannerPattern> bannerPatterns;
        extern std::unordered_map<std::string, PaintingVariant> paintingVariants;

        extern list_array<std::unordered_map<std::string, ArmorTrimMaterial>::iterator> armorTrimMaterials_cache;
        extern list_array<std::unordered_map<std::string, ArmorTrimPattern>::iterator> armorTrimPatterns_cache;
        extern list_array<std::unordered_map<std::string, Biome>::iterator> biomes_cache;
        extern list_array<std::unordered_map<std::string, ChatType>::iterator> chatTypes_cache;
        extern list_array<std::unordered_map<std::string, DamageType>::iterator> damageTypes_cache;
        extern list_array<std::unordered_map<std::string, DimensionType>::iterator> dimensionTypes_cache;
        extern list_array<std::unordered_map<std::string, WolfVariant>::iterator> wolfVariants_cache;
        extern list_array<std::unordered_map<std::string, BannerPattern>::iterator> bannerPatterns_cache;
        extern list_array<std::unordered_map<std::string, PaintingVariant>::iterator> paintingVariants_cache;


        //SERVER
        extern std::unordered_map<std::string, Advancement> advancements;
        extern std::unordered_map<std::string, JukeboxSong> jukebox_songs;


        extern std::unordered_map<uint32_t, enbt::compound> individual_registers;


        extern std::unordered_map<std::string, potion> potions;
        extern list_array<decltype(potions)::iterator> potions_cache;

        extern std::unordered_map<std::string, enchantment> enchantments;
        extern std::unordered_map<std::string, enbt::compound> enchantment_providers;
        extern list_array<decltype(enchantments)::iterator> enchantments_cache;

        extern std::unordered_map<std::string, loot_table_item> loot_table;
        extern list_array<decltype(loot_table)::iterator> loot_table_cache;

        extern std::unordered_map<int32_t, ItemType*> itemList;
        extern std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, list_array<std::string>>>> tags; //[type][namespace][tag][values]   values can't contain other tags, parsers must resolve them
        extern std::string default_namespace;                                                                                                   //minecraft


        const list_array<std::string>& unfold_tag(const std::string& type, const std::string& namespace_, const std::string& tag);
        const list_array<std::string>& unfold_tag(const std::string& type, const std::string& tag);
    }
}

#endif /* SRC_REGISTERS */
