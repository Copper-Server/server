/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_REGISTERS
#define SRC_API_REGISTERS
#include <library/enbt/enbt.hpp>
#include <src/api/id.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/number_provider.hpp>
#include <src/base_objects/recipe.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace copper_server::api::registers {
    struct advancement {
        struct display_t {
            struct {
                api::id::item item;
                std::string nbt;
            } icon;
            Chat title;
            std::string frame;
            std::string background;
            Chat description;
            bool show_toast;
            bool announce_to_chat;
            bool hidden;
        };

        struct rewards_t {
            std::vector<api::id::recipe> recipes;
            std::vector<api::id::loot_table> loot;
            int32_t experience = 0;
            std::string function;
        };

        std::optional<display_t> display;
        std::string parent;
        enbt::compound criteria;
        std::vector<std::vector<std::string>> requirements;
        rewards_t rewards;
        bool send_via_network_body = true;
        bool sends_telemetry_event = false;
    };

    struct jukebox_song {
        struct custom {
            api::id::sound_event sound_id;
            std::optional<float> fixed_range;
        };

        std::variant<api::id::sound_event, custom> sound_event;
        int32_t comparator_output = 0;
        int32_t length_in_seconds = 0;
        Chat description;

        uint32_t id = 0;
        bool send_via_network_body = true;
    };

    struct armor_trim_material {
        std::variant<std::string, Chat> description;
        std::string asset_name;
        uint32_t id;
        bool allow_override = false;
        bool send_via_network_body = true;
    };

    struct armor_trim_pattern {
        std::string asset_id;
        api::id::item template_item;
        std::variant<std::string, Chat> description;
        uint32_t id;
        bool decal;
        bool allow_override = false;
        bool send_via_network_body = true;
    };

    struct biome {
        struct particle {
            struct {
                api::id::particle_type type;
                enbt::value options;
            } options;

            float probability;
        };

        struct ambient_sound {
            api::id::sound_event sound;
            float range;
        };

        struct mood_sound {
            api::id::sound_event sound;
            int32_t tick_delay = 6000;
            int32_t block_search_extent = 8;
            double offset = 2.0;
        };

        struct additions_sound {
            api::id::sound_event sound;
            double tick_chance;
        };

        struct music {
            api::id::sound_event sound;
            int32_t min_delay = 12000;
            int32_t max_delay = 24000;
            bool replace_current_music = true;
            float music_weight = 1;
        };

        struct spawners_value {
            api::id::entity_type type;
            uint32_t max_count;
            uint32_t min_count;
            uint32_t weight;
        };

        struct spawn_costs_value {
            double energy_budget;
            double charge;
        };

        uint32_t id;
        bool allow_override = false;

        bool has_precipitation;
        float temperature;
        float downfall;
        std::optional<std::string> temperature_modifier;

        struct effects_t {
            int32_t fog_color;
            int32_t water_color;
            int32_t water_fog_color;
            int32_t sky_color;
            std::optional<int32_t> foliage_color;
            std::optional<int32_t> grass_color;
            std::optional<std::string> grass_color_modifier;
            std::optional<particle> particle;
            std::optional<std::variant<std::string, ambient_sound>> ambient_sound;
            std::optional<mood_sound> mood_sound;
            std::optional<additions_sound> additions_sound;
            std::vector<music> music;
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
        std::unordered_map<std::string, std::vector<spawners_value>> spawners;
        //mob_id>config
        std::unordered_map<std::string, spawn_costs_value> spawn_costs;

        double creature_spawn_probability = 0;
        bool send_via_network_body = true;
    };

    struct chat_type {
        struct decoration {
            std::string translation_key;
            std::optional<Chat> style;                                      //main text and extra chat will be ignored
            std::variant<std::string, std::vector<std::string>> parameters; // sender, target, content
        };

        std::optional<decoration> chat;
        std::optional<decoration> narration;

        uint32_t id;
        bool allow_override = false;
        bool send_via_network_body = true;
    };

    struct damage_type {
        enum class scaling_type {
            never,
            when_caused_by_living_non_player,
            always
        };

        enum class effects_type {
            hurt,
            thorns,
            drowning,
            burning,
            poking,
            freezing
        };

        enum class death_message_type {
            _default, //"default"
            fall_variants,
            intentional_game_design
        };


        std::string message_id;
        scaling_type scaling; //as string
        std::optional<effects_type> effects;
        std::optional<death_message_type> death_message_type;
        float exhaustion;
        uint32_t id;

        bool allow_override = false;
        bool send_via_network_body = true;
    };

    struct dimension_type {
        std::variant<int32_t, base_objects::number_provider> monster_spawn_light_level;
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

    struct wolf_variant {
        enbt::compound assets;
        enbt::dynamic_array spawn_conditions;

        uint32_t id;
        bool allow_override = false;
        bool send_via_network_body = true;
    };

    struct entity_variant {
        std::string asset_id;
        std::optional<std::string> model;
        enbt::dynamic_array spawn_conditions;

        uint32_t id;
        bool allow_override = false;
        bool send_via_network_body = true;
    };

    struct wolf_sound_variant {
        api::id::sound_event ambient_sound;
        api::id::sound_event death_sound;
        api::id::sound_event growl_sound;
        api::id::sound_event hurt_sound;
        api::id::sound_event pant_sound;
        api::id::sound_event whine_sound;

        uint32_t id;
        bool allow_override = false;
        bool send_via_network_body = true;
    };

    struct banner_pattern {
        std::string asset_id;
        std::string translation_key;

        uint32_t id = 0;
        bool allow_override = false;
        bool send_via_network_body = true;
    };

    struct painting_variant {
        Chat title;
        Chat author;
        std::string asset_id;
        uint32_t height = 0;
        uint32_t width = 0;

        uint32_t id = 0;
        bool allow_override = false;
        bool send_via_network_body = true;
    };

    struct instrument {
        struct custom {
            api::id::sound_event sound_name;
            std::optional<float> fixed_range;
        };

        std::variant<api::id::sound_event, custom> sound_event;
        float use_duration = 0.0f;
        float range = 0.0f;
        Chat description;

        uint32_t id = 0;
        bool send_via_network_body = true;
    };

    struct enchantment {
        Chat description;
        std::variant<std::string, std::vector<std::string>, std::nullptr_t> exclusive_set;
        std::variant<api::id::set::item, std::vector<api::id::item>> supported_items;
        std::variant<api::id::set::item, std::vector<api::id::item>> primary_items;
        std::vector<std::string> slots;
        std::unordered_map<std::string, enbt::value> effects; //TODO create api for custom effects

        struct {
            int32_t base = 0;
            int32_t per_level_above_first = 0;
        } min_cost;

        struct {
            int32_t base = 0;
            int32_t per_level_above_first = 0;
        } max_cost;

        int32_t anvil_cost = 0;
        int32_t weight = 0;
        uint8_t max_level = 0;

        uint32_t id = 0;
        bool send_via_network_body = true;
    };

    struct enchantment_provider {
        enbt::compound data;

        uint32_t id = 0;
        bool send_via_network_body = true;
    };

    struct effect {
        struct attribute_modifier {
            api::id::attribute attribute;
            enum class operation_e {
                add,
                add_multiplied_base,
                add_multiplied_total,
            } operation;
            double value;
        };

        std::unordered_set<std::string> required_features;
        list_array<attribute_modifier> attribute_modifiers;
        std::string name;
        std::string translation_key;
        std::string category;
        uint32_t id = 0;
        uint32_t fade_in_ticks = 0;
        uint32_t fade_out_ticks = 0;
        uint32_t fade_out_threshold_ticks = 0;
        uint32_t rgb = 0;
        bool is_instant = false;
        bool is_beneficial = false;
    };

    struct potion {
        struct effect_data {
            api::id::mob_effect id;
            int32_t duration;
            int32_t amplifier;
        };
        std::string name;
        uint32_t id = 0;
        std::vector<effect_data> effects;
        std::unordered_map<api::id::item, api::id::potion> recipe;
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

        uint32_t id = 0;
        bool send_via_network_body = true;
    };

    namespace world_gen {
        using biome = registers::biome;

        struct configured_carver {
            std::string type;

            struct {
                float probability = 0.0f;
                enbt::compound y; //number provider

                struct {
                    int32_t absolute = 0;
                    int32_t above_bottom = 0;
                    int32_t below_top = 0;
                } lava_level;

                std::variant<std::string, std::vector<std::string>> replaceable;

                struct Debug_settings {
                    bool debug = false;

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
            int32_t firstOctave = 0;
            std::vector<double> amplitudes;
        };

        struct noise_settings {
            int32_t sea_level = 0;
            bool disable_mob_generation = false;
            bool ore_veins_enabled = false;
            bool aquifers_enabled = false;
            bool legacy_random_source = false;

            struct state {
                api::id::block_type name; //in json there string
                std::unordered_map<std::string, std::string> properties;
            };

            state default_block;
            state default_fluid;

            struct spawn_target_v {
                struct temperature_value {
                    float min = 0.0f;
                    float max = 0.0f;
                };

                using variants = std::variant<temperature_value, std::vector<temperature_value>, float>;
                variants temperature;
                variants humidity;
                variants continentalness;
                variants erosion;
                variants weirdness;
                variants depth;
                float offset = 0.0f;
            };

            std::vector<spawn_target_v> spawn_target;

            struct {
                int32_t min_y = 0;
                int32_t height = 0;
                int32_t size_horizontal = 0;
                int32_t size_vertical = 0;
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
                    int32_t weight = 0;
                    int32_t min_count = 0;
                    int32_t max_count = 0;
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
                int32_t salt = 0;
                float frequency = 1.0;
                std::string frequency_reduction_method = "default";

                struct {
                    int32_t chunk_count = 0;
                    std::string other_set;
                } exclusion_zone;

                int32_t locale_offset[3] = {0, 0, 0};

                std::string type;
                enbt::compound custom_data; //virtual field, used in handlers
            } placement;
        };

        struct template_pool {
            struct element {
                int32_t weight = 0;

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
                api::id::block_state block;
                int32_t height = 0;
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

    struct attribute {
        std::string name;
        uint32_t id = 0;
        double default_value = 0.0;
    };

    //CLIENT/SERVER
    extern std::unordered_map<std::string, armor_trim_material> armorTrimMaterials;
    extern std::unordered_map<std::string, armor_trim_pattern> armorTrimPatterns;
    extern std::unordered_map<std::string, biome> biomes;
    extern std::unordered_map<std::string, chat_type> chatTypes;
    extern std::unordered_map<std::string, damage_type> damageTypes;
    extern std::unordered_map<std::string, dimension_type> dimensionTypes;
    extern std::unordered_map<std::string, wolf_sound_variant> wolfSoundVariants;
    extern std::unordered_map<std::string, wolf_variant> wolfVariants;
    extern std::unordered_map<std::string, entity_variant> catVariants;
    extern std::unordered_map<std::string, entity_variant> chickenVariants;
    extern std::unordered_map<std::string, entity_variant> cowVariants;
    extern std::unordered_map<std::string, entity_variant> pigVariants;
    extern std::unordered_map<std::string, entity_variant> frogVariants;
    extern std::unordered_map<std::string, banner_pattern> bannerPatterns;
    extern std::unordered_map<std::string, painting_variant> paintingVariants;
    extern std::unordered_map<std::string, instrument> instruments;
    extern std::unordered_map<std::string, int32_t> entity_pose;

    extern list_array<std::unordered_map<std::string, armor_trim_material>::iterator> armorTrimMaterials_cache;
    extern list_array<std::unordered_map<std::string, armor_trim_pattern>::iterator> armorTrimPatterns_cache;
    extern list_array<std::unordered_map<std::string, biome>::iterator> biomes_cache;
    extern list_array<std::unordered_map<std::string, chat_type>::iterator> chatTypes_cache;
    extern list_array<std::unordered_map<std::string, damage_type>::iterator> damageTypes_cache;
    extern list_array<std::unordered_map<std::string, dimension_type>::iterator> dimensionTypes_cache;
    extern list_array<std::unordered_map<std::string, wolf_sound_variant>::iterator> wolfSoundVariants_cache;
    extern list_array<std::unordered_map<std::string, wolf_variant>::iterator> wolfVariants_cache;
    extern list_array<std::unordered_map<std::string, entity_variant>::iterator> catVariants_cache;
    extern list_array<std::unordered_map<std::string, entity_variant>::iterator> chickenVariants_cache;
    extern list_array<std::unordered_map<std::string, entity_variant>::iterator> cowVariants_cache;
    extern list_array<std::unordered_map<std::string, entity_variant>::iterator> pigVariants_cache;
    extern list_array<std::unordered_map<std::string, entity_variant>::iterator> frogVariants_cache;
    extern list_array<std::unordered_map<std::string, banner_pattern>::iterator> bannerPatterns_cache;
    extern list_array<std::unordered_map<std::string, painting_variant>::iterator> paintingVariants_cache;
    extern list_array<std::unordered_map<std::string, instrument>::iterator> instruments_cache;
    extern list_array<std::unordered_map<std::string, int32_t>::iterator> entity_pose_cache;


    //SERVER
    extern std::unordered_map<std::string, advancement> advancements;


    extern std::unordered_map<std::string, attribute> attributes;
    extern list_array<decltype(attributes)::iterator> attributes_cache;

    extern std::unordered_map<std::string, jukebox_song> jukebox_songs;
    extern list_array<decltype(jukebox_songs)::iterator> jukebox_songs_cache;


    extern enbt::compound current_protocol_registers;
    extern uint32_t current_protocol_id;

    std::string normalize_entry(const std::string& str);
    std::string normalize_entry(std::string&& str);

    enbt::value& view_registry_entries(const std::string& registry);
    enbt::value& view_registry_proto_invert(const std::string& registry);
    list_array<int32_t> reg_ids(const std::string& registry);
    int32_t view_reg_pro_id(const std::string& registry, const std::string& item);
    std::string_view view_reg_pro_name(const std::string& registry, int32_t id);
    list_array<int32_t> convert_reg_pro_id(const std::string& registry, const list_array<std::string>& item);
    list_array<int32_t> convert_reg_pro_id(const std::string& registry, const std::vector<std::string>& item);
    list_array<std::string> convert_reg_pro_name(const std::string& registry, const list_array<int32_t>& item);
    list_array<std::string> convert_reg_pro_name(const std::string& registry, const std::vector<int32_t>& item);


    extern std::unordered_map<std::string, potion> potions;
    extern list_array<decltype(potions)::iterator> potions_cache;


    extern std::unordered_map<std::string, effect> effects;
    extern list_array<decltype(effects)::iterator> effects_cache;

    extern std::unordered_map<std::string, enchantment> enchantments;
    extern list_array<decltype(enchantments)::iterator> enchantments_cache;
    extern std::unordered_map<std::string, enchantment_provider> enchantment_providers;
    extern list_array<decltype(enchantment_providers)::iterator> enchantment_providers_cache;

    extern std::unordered_map<std::string, loot_table_item> loot_table;
    extern list_array<decltype(loot_table)::iterator> loot_table_cache;

    extern std::unordered_map<std::string, base_objects::recipe> recipe_table;
    extern list_array<decltype(recipe_table)::iterator> recipe_table_cache;
}


#endif /* SRC_REGISTERS */
