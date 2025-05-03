#ifndef SRC_REGISTERS
#define SRC_REGISTERS
#include <library/enbt/enbt.hpp>
#include <src/base_objects/block.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/float_provider.hpp>
#include <src/base_objects/number_provider.hpp>
#include <src/base_objects/particle_data.hpp>
#include <src/base_objects/position.hpp>
#include <src/base_objects/recipe.hpp>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace copper_server {
    namespace registers {
#pragma region CLIENT/SERVER

        struct IntegerDistribution {
            std::string type;
            enbt::value value;

            enbt::compound get_enbt() const {
                enbt::compound distribution;
                if (value.is_compound()) {
                    distribution = value;
                    distribution["type"] = type;
                } else {
                    distribution["type"] = type;
                    distribution["value"] = value;
                }
                return distribution;
            }
        };

        struct Advancement {
            struct Display {
                struct Icon {
                    base_objects::item_id_t item;
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
            bool send_via_network_body = true;
            bool sends_telemetry_event = false;
        };

        struct JukeboxSong {
            struct custom {
                std::string sound_id; //resource location
                std::optional<float> fixed_range;
            };

            std::variant<std::string, custom> sound_event;
            int32_t comparator_output;
            int32_t length_in_seconds;
            Chat description;
            bool send_via_network_body = true;
        };

        struct ArmorTrimMaterial {
            std::variant<std::string, Chat> description;
            std::string asset_name;
            uint32_t id;
            bool allow_override = false;
            bool send_via_network_body = true;
        };

        struct ArmorTrimPattern {
            std::string asset_id;
            base_objects::item_id_t template_item;
            std::variant<std::string, Chat> description;
            bool decal;
            bool allow_override = false;
            bool send_via_network_body = true;
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
                int32_t block_search_extent = 8;
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
                float music_weight = 1;
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
                std::vector<Music> music;
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
            bool send_via_network_body = true;
        };

        struct ChatType {
            struct Decoration {
                std::string translation_key;
                std::optional<Chat> style;                                      //main text and extra chat will be ignored
                std::variant<std::string, std::vector<std::string>> parameters; // sender, target, content
            };

            std::optional<Decoration> chat;
            std::optional<Decoration> narration;

            uint32_t id;
            bool allow_override = false;
            bool send_via_network_body = true;
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
            bool send_via_network_body = true;
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
            bool send_via_network_body : 1 = true;
            uint32_t id;
        };

        struct WolfVariant {
            enbt::compound assets;
            enbt::dynamic_array spawn_conditions;

            uint32_t id;
            bool allow_override = false;
            bool send_via_network_body = true;
        };

        struct EntityVariant {
            std::string asset_id;
            std::optional<std::string> model;
            enbt::dynamic_array spawn_conditions;

            uint32_t id;
            bool allow_override = false;
            bool send_via_network_body = true;
        };

        struct WolfSoundVariant {
            std::string ambient_sound;
            std::string death_sound;
            std::string growl_sound;
            std::string hurt_sound;
            std::string pant_sound;
            std::string whine_sound;

            uint32_t id;
            bool allow_override = false;
            bool send_via_network_body = true;
        };

        struct BannerPattern {
            std::string asset_id;
            std::string translation_key;

            uint32_t id;
            bool allow_override = false;
            bool send_via_network_body = true;
        };

        struct PaintingVariant {
            Chat title;
            Chat author;
            std::string asset_id;
            uint32_t height;
            uint32_t width;

            uint32_t id;
            bool allow_override = false;
            bool send_via_network_body = true;
        };

        struct Instrument {
            std::variant<std::string, base_objects::slot_component::inner::sound_extended> sound_event;
            float use_duration;
            float range;
            Chat description;

            uint32_t id;
            bool send_via_network_body = true;
        };

#pragma endregion
#pragma region server

        struct enchantment {
            Chat description;
            std::variant<std::string, std::vector<std::string>, std::nullptr_t> exclusive_set;
            std::variant<std::string, std::vector<std::string>> supported_items;
            std::variant<std::string, std::vector<std::string>> primary_items;
            std::vector<std::string> slots;
            std::unordered_map<std::string, enbt::value> effects; //TODO create api for custom effects

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
            bool send_via_network_body = true;
        };

        struct effect {
            std::string name;
            uint32_t id;

            std::unordered_map<int32_t, int32_t> protocol;                                              //protocol -> protocol effect id
            static std::unordered_map<int32_t, std::unordered_map<int32_t, uint32_t>> protocol_aliases; //protocol -> protocol effect id -> internal id
        };

        struct potion {
            std::string name;
            uint32_t id;
            std::vector<uint32_t> effects;

            std::unordered_map<int32_t, int32_t> protocol;                                              //protocol -> protocol effect id
            static std::unordered_map<int32_t, std::unordered_map<int32_t, uint32_t>> protocol_aliases; //protocol -> protocol effect id -> internal id
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

            bool send_via_network_body = true;
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
                    base_objects::block name; //in json there string
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
                    base_objects::block block;
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

        struct item_attribute {
            std::string name;
            uint32_t id;

            std::unordered_map<int32_t, int32_t> protocol;                                              //protocol -> protocol effect id, can be undefined
            static std::unordered_map<int32_t, std::unordered_map<int32_t, uint32_t>> protocol_aliases; //protocol -> protocol effect id -> internal id, can be undefined
        };

#pragma endregion
        //CLIENT/SERVER
        extern std::unordered_map<std::string, ArmorTrimMaterial> armorTrimMaterials;
        extern std::unordered_map<std::string, ArmorTrimPattern> armorTrimPatterns;
        extern std::unordered_map<std::string, Biome> biomes;
        extern std::unordered_map<std::string, ChatType> chatTypes;
        extern std::unordered_map<std::string, DamageType> damageTypes;
        extern std::unordered_map<std::string, DimensionType> dimensionTypes;
        extern std::unordered_map<std::string, WolfSoundVariant> wolfSoundVariants;
        extern std::unordered_map<std::string, WolfVariant> wolfVariants;
        extern std::unordered_map<std::string, EntityVariant> catVariants;
        extern std::unordered_map<std::string, EntityVariant> chickenVariants;
        extern std::unordered_map<std::string, EntityVariant> cowVariants;
        extern std::unordered_map<std::string, EntityVariant> pigVariants;
        extern std::unordered_map<std::string, EntityVariant> frogVariants;
        extern std::unordered_map<std::string, BannerPattern> bannerPatterns;
        extern std::unordered_map<std::string, PaintingVariant> paintingVariants;
        extern std::unordered_map<std::string, Instrument> instruments;

        extern list_array<std::unordered_map<std::string, ArmorTrimMaterial>::iterator> armorTrimMaterials_cache;
        extern list_array<std::unordered_map<std::string, ArmorTrimPattern>::iterator> armorTrimPatterns_cache;
        extern list_array<std::unordered_map<std::string, Biome>::iterator> biomes_cache;
        extern list_array<std::unordered_map<std::string, ChatType>::iterator> chatTypes_cache;
        extern list_array<std::unordered_map<std::string, DamageType>::iterator> damageTypes_cache;
        extern list_array<std::unordered_map<std::string, DimensionType>::iterator> dimensionTypes_cache;
        extern list_array<std::unordered_map<std::string, WolfSoundVariant>::iterator> wolfSoundVariants_cache;
        extern list_array<std::unordered_map<std::string, WolfVariant>::iterator> wolfVariants_cache;
        extern list_array<std::unordered_map<std::string, EntityVariant>::iterator> catVariants_cache;
        extern list_array<std::unordered_map<std::string, EntityVariant>::iterator> chickenVariants_cache;
        extern list_array<std::unordered_map<std::string, EntityVariant>::iterator> cowVariants_cache;
        extern list_array<std::unordered_map<std::string, EntityVariant>::iterator> pigVariants_cache;
        extern list_array<std::unordered_map<std::string, EntityVariant>::iterator> frogVariants_cache;
        extern list_array<std::unordered_map<std::string, BannerPattern>::iterator> bannerPatterns_cache;
        extern list_array<std::unordered_map<std::string, PaintingVariant>::iterator> paintingVariants_cache;
        extern list_array<std::unordered_map<std::string, Instrument>::iterator> instruments_cache;


        //SERVER
        extern std::unordered_map<std::string, Advancement> advancements;


        extern std::unordered_map<std::string, item_attribute> attributes;
        extern list_array<decltype(attributes)::iterator> attributes_cache;

        extern std::unordered_map<std::string, JukeboxSong> jukebox_songs;


        extern std::unordered_map<uint32_t, enbt::compound> individual_registers;
        extern uint32_t use_registry_latest;
        enbt::compound& default_registry();
        enbt::value& default_registry_entries(const std::string& registry);
        int32_t view_reg_pro_id(const std::string& registry, const std::string& item, int32_t protocol = -1);
        std::string view_reg_pro_name(const std::string& registry, int32_t id, int32_t protocol = -1);
        list_array<int32_t> convert_reg_pro_id(const std::string& registry, const list_array<std::string>& item, int32_t protocol = -1);
        list_array<int32_t> convert_reg_pro_id(const std::string& registry, const std::vector<std::string>& item, int32_t protocol = -1);
        list_array<std::string> convert_reg_pro_name(const std::string& registry, const list_array<int32_t>& item, int32_t protocol = -1);
        list_array<std::string> convert_reg_pro_name(const std::string& registry, const std::vector<int32_t>& item, int32_t protocol = -1);


        extern std::unordered_map<std::string, potion> potions;
        extern list_array<decltype(potions)::iterator> potions_cache;


        extern std::unordered_map<std::string, effect> effects;
        extern list_array<decltype(effects)::iterator> effects_cache;

        extern std::unordered_map<std::string, enchantment> enchantments;
        extern list_array<decltype(enchantments)::iterator> enchantments_cache;
        extern std::unordered_map<std::string, enbt::compound> enchantment_providers;

        extern std::unordered_map<std::string, loot_table_item> loot_table;
        extern list_array<decltype(loot_table)::iterator> loot_table_cache;

        extern std::unordered_map<std::string, base_objects::recipe> recipe_table;
        extern list_array<decltype(recipe_table)::iterator> recipe_table_cache;

        extern std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, list_array<std::string>>>> tags; //[type][namespace][tag][values]   values can't contain other tags, parsers must resolve them
        extern std::string default_namespace;                                                                                                   //minecraft


        const list_array<std::string>& unfold_tag(const std::string& type, const std::string& namespace_, const std::string& tag);
        const list_array<std::string>& unfold_tag(const std::string& type, const std::string& tag);
    }
}

#endif /* SRC_REGISTERS */
