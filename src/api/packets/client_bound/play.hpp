/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_CLIENT_BOUND_PLAY
#define SRC_API_PACKETS_CLIENT_BOUND_PLAY
#include <library/enbt/enbt.hpp>
#include <optional>
#include <src/api/packets/chat_type.hpp>
#include <src/api/packets/debug_sub_scription_type.hpp>
#include <src/api/packets/difficulty.hpp>
#include <src/api/packets/gamemode.hpp>
#include <src/api/packets/ops.hpp>
#include <src/api/packets/slot.hpp>
#include <src/api/packets/teleport_flags.hpp>
#include <src/api/packets/types.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/entity/metadata.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/palette_container.hpp>
#include <src/base_objects/parsers.hpp>
#include <src/base_objects/particle_data.hpp>
#include <src/base_objects/position.hpp>
#include <src/util/calculations.hpp>

namespace copper_server::storage {
    class world_data;
}

namespace copper_server {
    namespace storage {
        class world_data;
    }

    namespace base_objects::world {
        struct chunk_data;
    }
}

namespace copper_server::api::packets::client_bound::play {
    struct play_packet;

    struct bundle_delimiter : public compound_packet {
        packet<0> begin;
        list_array<play_packet> packets; //do not include here packet that switches to other state, it would not be called
        packet<0> end;

        bundle_delimiter();
        bundle_delimiter(bundle_delimiter&&);
        bundle_delimiter(const bundle_delimiter&);
        bundle_delimiter(list_array<play_packet>&& mov);
        bundle_delimiter(const list_array<play_packet>&& copy);

        bundle_delimiter& operator=(bundle_delimiter&&);
        bundle_delimiter& operator=(const bundle_delimiter&);
    };

    struct add_entity : public packet<0x01> {
        var_int32::entity_id id;
        base_objects::uuid uuid;
        var_int32::entity_type type;
        double x;
        double y;
        double z;
        Angle pitch;
        Angle yaw;
        Angle head_yaw;
        var_int32 data;
        base_objects::velocity velocity;
    };

    struct animate : public packet<0x02> {
        enum class animation_e : uint8_t {
            swing_main_arm = 0,
            unrecognized = 1,
            leave_bed = 2,
            swing_offhand = 3,
            critical_hit = 4,
            enchanted_hit = 5,
        };
        using enum animation_e;

        var_int32::entity_id id;
        enum_as<animation_e, uint8_t> animation;
    };

    struct award_stats : public packet<0x03> {
        struct statistic {
            var_int32 category_id;
            var_int32 statistic_id;
            var_int32 value;
        };

        list_array<statistic> statistics;
    };

    struct block_changed_ack : public packet<0x04> {
        var_int32 block_sequence_id;
    };

    struct block_destruction : public packet<0x05> {
        var_int32::entity_id id;
        base_objects::position location;
        uint8_t destroy_stage;
    };

    struct block_entity_data : public packet<0x06> {
        base_objects::position location;
        var_int32::block_entity_type type;
        enbt::value data;
    };

    struct block_event : public packet<0x07> {
        base_objects::position location;
        uint8_t action_id;
        uint8_t action_param;
        var_int32::block_type block;
    };

    struct block_update : public packet<0x08> {
        base_objects::position location;
        var_int32::block_state block;
    };

    struct boss_event : public packet<0x09> {
        base_objects::uuid uuid;

        struct add : public enum_item<0> {
            base_objects::chat title;
            float health;
            var_int32 color;
            var_int32 division;
            uint8_t flags;
        };

        struct remove : public enum_item<1> {};

        struct update_health : public enum_item<2> {
            float health;
        };

        struct update_title : public enum_item<3> {
            base_objects::chat title;
        };

        struct update_style : public enum_item<4> {
            var_int32 color;
            var_int32 division;
        };

        struct update_flags : public enum_item<5> {
            uint8_t flags;
        };

        enum_switch<
            var_int32,
            add,
            remove,
            update_health,
            update_title,
            update_style,
            update_flags>
            action;
    };

    struct change_difficulty : public packet<0x0A> {
        enum_as<difficulty_e, uint8_t> difficulty;
        bool is_locked;
    };

    struct chunk_batch_finished : public packet<0x0B> {
        var_int32 batch_size;
    };

    struct chunk_batch_start : public packet<0x0C> {};

    struct chunks_biomes : public packet<0x0D> {
        int32_t z;
        int32_t x;
        sized_entry<list_array_no_size<base_objects::palette_container_biome, size_source::get_world_chunks_height>, var_int32> sections_of_biomes;

        static chunks_biomes create(const base_objects::world::chunk_data&);
    };

    struct clear_titles : public packet<0x0E> {
        bool reset;
    };

    struct command_suggestions : public packet<0x0F> {
        struct match {
            string_sized<32767> set;
            std::optional<base_objects::chat> tooltip = std::nullopt;
        };

        var_int32 suggestion_transaction_id;
        var_int32 start;
        var_int32 length;
        list_array<match> matches;
    };

    struct commands : public packet<0x10> {
        struct node {
            struct root_node : public flag_item<0, 0x3, 1> {};

            struct literal_node : public flag_item<1, 0x3, 1> {
                string_sized<32767> name;
            };

            struct argument_node : public flag_item<2, 0x3, 1> {
                string_sized<32767> name;
                base_objects::command_parser type;
            };

            struct is_executable : public flag_item<0x04, 0x04, -1> {};

            struct redirect_node : public flag_item<8, 0x8, 0> {
                var_int32 node;
            };

            struct suggestions_type : public flag_item<0x10, 0x10, 2> {
                identifier name;
            };

            struct is_restricted : public flag_item<0x20, 0x20, -2> {};

            int8_t flags;
            list_array<var_int32> children;
            flags_list_from<
                node,
                int8_t,
                &node::flags,
                literal_node,
                root_node,
                argument_node,
                is_executable,
                redirect_node,
                suggestions_type,
                is_restricted>
                flags_values;
        };

        list_array<node> nodes;
        var_int32 root_index;

        static commands create(const base_objects::command_manager& manager);
    };

    struct container_close : public packet<0x11> {
        var_int32 windows_id;
    };

    struct container_set_content : public packet<0x12> {
        var_int32 windows_id;
        var_int32 state_id;
        list_array<slot> inventory_data;
        slot carried_item;
    };

    struct container_set_data : public packet<0x13> {
        var_int32 windows_id;

        struct furnace {
            enum class property_e : uint8_t {
                fuel_left = 0,
                max_fuel = 1,
                progress = 2,
                max_progress = 3,
            };
            using enum property_e;
            enum_as<property_e, short> property;
            short value;
        };

        struct enchantment_table {
            enum class property_e : uint8_t {
                level_requirement_top = 0,
                level_requirement_middle = 1,
                level_requirement_bottom = 2,
                enchantment_seed = 3,
                enchantment_id_top = 4,
                enchantment_id_middle = 5,
                enchantment_id_bottom = 6,
                enchantment_lvl_top = 7,
                enchantment_lvl_middle = 8,
                enchantment_lvl_bottom = 9,
            };
            using enum property_e;
            enum_as<property_e, short> property;
            short value;
        };

        struct beacon {
            enum class property_e : uint8_t {
                power_level = 0,
                first_potion = 1,
                second_potion = 2,
            };
            using enum property_e;
            enum_as<property_e, short> property;
            short value;
        };

        struct anvil {
            enum class property_e : uint8_t {
                repair_cost = 0,
            };
            using enum property_e;
            enum_as<property_e, short> property;
            short value;
        };

        struct brewing_stand {
            enum class property_e : uint8_t {
                brew_time = 0, //400-0
                fuel_left = 1, //0-20
            };
            using enum property_e;
            enum_as<property_e, short> property;
            short value;
        };

        struct stonecutter {
            enum class property_e : uint8_t {
                selected_recipe = 0, //-1 = none
            };
            using enum property_e;
            enum_as<property_e, short> property;
            short value;
        };

        struct loom {
            enum class property_e : uint8_t {
                selected_pattern = 0, //0 = base
            };
            using enum property_e;
            enum_as<property_e, short> property;
            short value;
        };

        struct lectern {
            enum class property_e : uint8_t {
                page_number = 0,
            };
            using enum property_e;
            enum_as<property_e, short> property;
            short value;
        };

        struct smithing_table {
            enum class property_e : uint8_t {
                has_recipe_error = 0, // 0>= == false, 0< == true
            };
            using enum property_e;
            enum_as<property_e, short> property;
            short value;
        };

        struct other {
            short property;
            short value;
        };

        any_of<
            other,
            anvil,
            beacon,
            brewing_stand,
            furnace,
            enchantment_table,
            stonecutter,
            loom,
            lectern,
            smithing_table>
            value;
    };

    struct container_set_slot : public packet<0x14> {
        var_int32 windows_id;
        var_int32 state_id;
        short slot;
        struct slot item;
    };

    struct cookie_request : public packet<0x15> {
        identifier key;
    };

    struct cooldown : public packet<0x16> {
        identifier group;
        var_int32 ticks;
    };

    struct custom_chat_completions : public packet<0x17> {
        enum class suggestion_e : uint8_t {
            add = 0,
            remove = 1,
            set = 2,
        };
        using enum suggestion_e;
        enum_as<suggestion_e, var_int32> suggestion;
        list_array<string_sized<32767>> entries;
    };

    struct custom_payload : public packet<0x18> {
        identifier channel;
        list_array_sized_siz_from_packet<uint8_t, 1048576> payload;
    };

    struct damage_event : public packet<0x19> {
        var_int32::entity_id id;
        optional_var_int32::damage_type source_damage_type_id = std::nullopt;
        optional_var_int32::entity_id source_id = std::nullopt;
        optional_var_int32::entity_id source_direct_id = std::nullopt;
        std::optional<util::vector> source_pos = std::nullopt;
    };

    struct debug__block_value : public packet<0x1A> {
        base_objects::position pos;
        debug_sub_scription_type::optional data;
    };

    struct debug__chunk_value : public packet<0x1B> {
        int32_t z;
        int32_t x;
        debug_sub_scription_type::optional data;
    };

    struct debug__entity_value : public packet<0x1C> {
        var_int32::entity_id id;
        debug_sub_scription_type::optional data;
    };

    struct debug__event : public packet<0x1D> {
        debug_sub_scription_type::value data;
    };

    struct debug_sample : public packet<0x1E> {
        list_array<int64_t> sample;
        var_int32 sample_type;
    };

    struct delete_chat : public packet<0x21> {
        var_int32 message_id;
        std::optional<std::array<uint8_t, 256>> signature = std::nullopt;
    };

    struct disconnect : public packet<0x20>, disconnect_after {
        base_objects::chat reason;
    };

    struct disguised_chat : public packet<0x21> {
        base_objects::chat message;
        or_<var_int32::chat_type, chat_type> type;
        base_objects::chat sender;
        std::optional<base_objects::chat> target_name = std::nullopt;
    };

    struct entity_event : public packet<0x22> {
        var_int32::entity_id id;
        int8_t status;
    };

    struct entity_position_sync : public packet<0x23> {
        var_int32::entity_id id;
        double x;
        double y;
        double z;
        double velocity_x;
        double velocity_y;
        double velocity_z;
        float yaw;
        float pitch;
        bool on_ground;
    };

    struct explode : public packet<0x24> {
        struct player_delta_velocity_t {
            double x;
            double y;
            double z;
        };

        struct block_particle {
            base_objects::particle_data particle;
            float scaling;
            float speed;
        };

        double x;
        double y;
        double z;
        float radius;
        int32_t count;
        std::optional<player_delta_velocity_t> player_delta_velocity = std::nullopt;
        base_objects::particle_data particle;
        or_<var_int32::sound_event, base_objects::sound_event> sound;
        list_array<block_particle> block_particles;
    };

    struct forget_level_chunk : public packet<0x25> {
        int32_t z;
        int32_t x;
    };

    struct game_event : public packet<0x26> {
        struct no_respawn_block_available : public enum_item<0> {
            float _ignored = 0.0f;

            no_respawn_block_available() {}

            no_respawn_block_available(no_respawn_block_available&&) {}

            no_respawn_block_available(const no_respawn_block_available&) {}

            no_respawn_block_available& operator=(no_respawn_block_available&&) {
                return *this;
            }

            no_respawn_block_available& operator=(const no_respawn_block_available&) {
                return *this;
            }
        };

        struct raining_begin : public enum_item<1> {
            float _ignored = 0.0f;
        };

        struct raining_end : public enum_item<2> {
            float _ignored = 0.0f;
        };

        struct gamemode_change : public enum_item<3> {
            enum_as<gamemode_e, float> gamemode;
        };

        struct win_game : public enum_item<4> {
            float roll_credits; //true/false 0/1
        };

        struct demo_event : public enum_item<5> {
            enum class event_e : uint8_t {
                welcome = 0,
                movement_controls = 101,
                jump_controls = 102,
                inventory_controls = 103,
                demo_over = 104,
            };
            using enum event_e;
            enum_as<event_e, float> event;
        };

        struct arrow_hit_player : public enum_item<6> {
            float _ignored = 0.0f;
        };

        struct rain_level_change : public enum_item<7> {
            float level;
        };

        struct thunder_level_change : public enum_item<8> {
            float level;
        };

        struct puffer_fish_sting_sound : public enum_item<9> {
            float _ignored = 0.0f;
        };

        struct guardian_appear_animation : public enum_item<10> {
            float _ignored = 0.0f;
        };

        struct respawn_screen_mode : public enum_item<11> {
            float enabled; //true/false
        };

        struct limited_crafting_mode : public enum_item<12> {
            float enabled; //true/false
        };

        struct wait_for_level_chunks : public enum_item<13> {
            float _ignored = 0.0f;
        };

        enum_switch<
            var_int32,
            no_respawn_block_available,
            raining_begin,
            raining_end,
            gamemode_change,
            win_game,
            demo_event,
            arrow_hit_player,
            rain_level_change,
            thunder_level_change,
            puffer_fish_sting_sound,
            guardian_appear_animation,
            respawn_screen_mode,
            limited_crafting_mode,
            wait_for_level_chunks>
            event;
    };

    struct game_test_highlight_pos : public packet<0x27> {
        base_objects::position absolute_pos;
        base_objects::position relative_pos;
    };

    struct horse_screen_open : public packet<0x28> {
        var_int32 window_id;
        var_int32 columns_count;
        api::id::entity_id id;
    };

    struct hurt_animation : public packet<0x29> {
        var_int32::entity_id id;
        float yaw;
    };

    struct initialize_border : public packet<0x2A> {
        double x;
        double z;
        double old_diameter;
        double new_diameter;
        var_int64 speed_ms;
        var_int32 portal_teleport_boundary;
        var_int32 warning_blocks;
        var_int32 warning_time;
    };

    struct keep_alive : public packet<0x2B> {
        uint64_t keep_alive_id;
    };

    struct level_chunk_with_light : public packet<0x2C> {
        struct height_map {
            enum class type_e : uint8_t {
                world_surface = 1,
                ocean_floor = 3,
                motion_blocking = 4,
                motion_blocking_no_leaves = 5,
            };
            using enum type_e;
            enum_as<type_e, var_int32> type;
            base_objects::palette_data_height_map palette_data;
        };

        struct section {
            uint16_t block_count;
            base_objects::palette_container_block block_states;
            base_objects::palette_container_biome biomes;
        };

        struct block_entity {
            uint8_t xz;
            short y;
            var_int32::block_entity_type type;
            enbt::value data;
        };

        int32_t x;
        int32_t z;
        list_array<height_map> height_maps;
        sized_entry<list_array_no_size<section, size_source::get_world_chunks_height>, var_int32> sections;
        list_array<block_entity> block_entities;

        list_array<uint64_t> sky_light_mask;
        list_array<uint64_t> block_light_mask;
        list_array<uint64_t> empty_sky_light_mask;
        list_array<uint64_t> empty_block_light_mask;
        list_array<list_array_fixed<uint8_t, 2048>> sky_light;
        list_array<list_array_fixed<uint8_t, 2048>> block_light;

        static level_chunk_with_light create(const base_objects::world::chunk_data&, const storage::world_data&);
    };

    struct level_event : public packet<0x2D> {
        enum class event_id : uint16_t {
            dispenser_dispenses = 1000,
            dispenser_dispense_fail = 1001,
            dispenser_shoots = 1002,
            firework_shot = 1004,
            fire_extinguished = 1009,
            play_record = 1010,
            stop_record = 1011,
            ghast_warn = 1015,
            ghast_shoots = 1016,
            ender_dragon_shoots = 1017,
            blaze_shoots = 1018,
            zombie_attacks_wooden_door = 1019,
            zombie_attacks_iron_door = 1020,
            zombie_breaks_wooden_door = 1021,
            wither_breaks_block = 1022,
            wither_spawned = 1023,
            wither_shoots = 1024,
            bat_takes_of = 1025,
            zombie_infects = 1026,
            zombie_villager_converted = 1027,
            ender_dragon_dies = 1028,
            anvil_destroyed = 1029,
            anvil_used = 1030,
            anvil_lands = 1031,
            portal_travel = 1032,
            chorus_flower_grows = 1033,
            chorus_flower_dies = 1034,
            brewing_stand_brews = 1035,
            end_portal_created = 1038,
            phantom_bites = 1039,
            zombie_converts_to_drowned = 1040,
            husk_converts_to_zombie = 1041,
            grindstone_used = 1042,
            book_page_turned = 1043,
            smithing_table_used = 1044,
            pointed_dripstone_landing = 1045,
            lava_dripping_on_cauldron_from_dripstone = 1046,
            water_dripping_on_cauldron_from_dripstone = 1047,
            skeleton_converts_to_stray = 1048,
            crafter_successfully_crafts_item = 1049,
            crafter_fails_to_craft_item = 1050,

            composter_composts = 1500,
            lava_converts_block = 1501,
            redstone_torch_burns_out = 1502,
            ender_eye_placed_in_end_portal_frame = 1503,
            fluid_drips_from_dripstone = 1504,
            bone_meal_particles_and_sound = 1505,
            dispenser_activation_smoke = 2000,
            block_break_and_sound = 2001,
            splash_potion_particle_effect = 2002,
            eye_of_ender_entity_break_animation = 2003,
            spawner_spawns_mob = 2004,
            dragon_breath = 2006,
            instant_splash_potion = 2007,
            ender_dragon_destroys_block = 2008,
            wet_sponge_vaporizes = 2009,
            crafter_activation_smoke = 2010,
            bee_fertilizes_plant = 2011,
            turtle_egg_placed = 2012,
            smash_attack_mace = 2013,
            end_gateway_spawns = 3000,
            ender_dragon_resurrected = 3001,
            electric_spark = 3002,
            copper_apply_wax = 3003,
            copper_remove_wax = 3004,
            copper_scrape_oxidation = 3005,
            sculk_charge = 3006,
            sculk_shrieker_shriek = 3007,
            block_finished_brushing = 3008,
            sniffer_egg_cracks = 3009,
            trial_spawner_spawns_mob_at_spawner = 3011,
            trial_spawner_spawns_mob_at_spawn_location = 3012,
            trial_spawner_detects_player = 3013,
            trial_spawner_ejects_item = 3014,
            vault_activates = 3015,
            vault_deactivates = 3016,
            vault_ejects_item = 3017,
            cobweb_weaved = 3018,
            ominous_trial_spawner_detects_player = 3019,
            trial_spawner_turns_ominous = 3020,
            ominous_item_spawner_spawns_item = 3021,
        };
        enum_as<event_id, int32_t> event;
        base_objects::position location;
        int32_t data;
        bool disable_volume;
    };

    struct level_particles : public packet<0x2E> {
        bool long_distance;
        bool always_visible;
        double x;
        double y;
        double z;
        float offset_x;
        float offset_y;
        float offset_z;
        float max_speed;
        int32_t particle_count;
        base_objects::particle_data particle;
    };

    struct light_update : public packet<0x2F> {
        int32_t x;
        int32_t z;
        list_array<uint64_t> sky_light_mask;
        list_array<uint64_t> block_light_mask;
        list_array<uint64_t> empty_sky_light_mask;
        list_array<uint64_t> empty_block_light_mask;
        list_array<list_array_fixed<uint8_t, 2048>> sky_light;
        list_array<list_array_fixed<uint8_t, 2048>> block_light;


        static light_update create(const base_objects::world::chunk_data&);
    };

    struct login : public packet<0x30> {
        struct death_location_t {
            identifier world;
            base_objects::position location;
        };

        api::id::entity_id id;
        bool is_hardcore;
        list_array<identifier> dimension_names;
        var_int32 max_players;
        var_int32 view_distance;
        var_int32 simulation_distance;
        bool reduced_debug_info;
        bool respawn_screen;
        bool limited_crafting_enabled;
        var_int32::dimension_type dimension_type;
        identifier dimension_name;
        int64_t seed_hashed;
        enum_as<gamemode_e, uint8_t> gamemode;
        enum_as<optional_gamemode_e, int8_t> prev_gamemode;
        bool world_is_debug;
        bool world_is_flat;
        std::optional<death_location_t> death_location = std::nullopt;
        var_int32 portal_cooldown;
        var_int32 sea_level;
        bool enforce_secure_chat;
    };

    struct map_item_data : public packet<0x31> {
        struct icon {
            enum class type_e : uint8_t {
                white_arrow = 0,
                green_arrow = 1,
                red_arrow = 2,
                blue_arrow = 3,
                white_cross = 4,
                red_pointer = 5,
                white_circle = 6,
                small_white_circle = 7,
                mansion = 8,
                monument = 9,
                white_banner = 10,
                orange_banner = 11,
                magenta_banner = 12,
                light_blue_banner = 13,
                yellow_banner = 14,
                lime_banner = 15,
                pink_banner = 16,
                gray_banner = 17,
                light_gray_banner = 18,
                cyan_banner = 19,
                purple_banner = 20,
                blue_banner = 21,
                brown_banner = 22,
                green_banner = 23,
                red_banner = 24,
                black_banner = 25,
                treasure_marker = 26,
                desert_village = 27,
                plains_village = 28,
                savanna_village = 29,
                snowy_village = 30,
                taiga_village = 31,
                jungle_temple = 32,
                swamp_hut = 33,
                trial_chambers = 34,
            };
            enum_as<type_e, var_int32> type = type_e::white_arrow;
            limited_num<int8_t, -128, 127> x;
            limited_num<int8_t, -128, 127> z;
            limited_num<int8_t, 0, 15> dir;
            std::optional<base_objects::chat> name = std::nullopt;
        };

        struct color_patch {
            depends_next<uint8_t> columns;
            uint8_t rows;
            uint8_t x;
            uint8_t z;
            list_array_no_size<uint8_t, &color_patch::columns, &color_patch::rows> data; //255 color palette
        };

        var_int32 map_id;
        int8_t scale;
        bool is_locked = false;
        std::optional<list_array<icon>> icons = std::nullopt;
        color_patch patch;
    };

    struct merchant_offers : public packet<0x32> {
        struct trade {
            struct trade_item {
                var_int32::item item_id;
                var_int32 item_count;
                list_array<base_objects::component> components;
            };

            trade_item input_0;
            slot output;
            std::optional<trade_item> input_1 = std::nullopt;
            bool trade_disabled = false;
            int trade_uses;
            int max_trade_uses;
            int xp;
            int special_price;
            float price_multiplier;
            int demand;
        };

        var_int32 window_id;
        list_array<trade> trades;
        var_int32 villager_level;
        var_int32 experience;
        bool is_regular_villager;
        bool can_restock;
    };

    struct move_entity_pos : public packet<0x33> {
        var_int32::entity_id id;
        short delta_x;
        short delta_y;
        short delta_z;
        bool on_ground;
    };

    struct move_entity_pos_rot : public packet<0x34> {
        var_int32::entity_id id;
        short delta_x;
        short delta_y;
        short delta_z;
        Angle yaw;
        Angle pitch;
        bool on_ground;
    };

    struct move_minecart_along_track : public packet<0x35> {
        struct step {
            double x;
            double y;
            double z;
            double velocity_x;
            double velocity_y;
            double velocity_z;
            Angle yaw;
            Angle pitch;
            float weight;
        };

        var_int32::entity_id id;
        list_array<step> steps;
    };

    struct move_entity_rot : public packet<0x36> {
        var_int32::entity_id id;
        Angle yaw;
        Angle pitch;
        bool on_ground;
    };

    struct move_vehicle : public packet<0x37> {
        double x;    //absolute
        double y;    //absolute
        double z;    //absolute
        Angle yaw;   //absolute
        Angle pitch; //absolute
    };

    struct open_book : public packet<0x38> {
        enum class hand_e : uint8_t {
            main = 0,
            off = 1,
        };
        enum_as<hand_e, var_int32> hand;
    };

    struct open_screen : public packet<0x39> {
        var_int32 window_id;
        var_int32::menu window_type;
        base_objects::chat window_title;
    };

    struct open_sign_editor : public packet<0x3A> {
        base_objects::position location;
        bool is_front_text;
    };

    struct ping : public packet<0x3B> {
        int32_t id;
    };

    struct pong_response : public packet<0x3C> {
        uint64_t id;
    };

    struct place_ghost_recipe : public packet<0x3D> {
        var_int32 window_id;
        recipe_display display;
    };

    struct player_abilities : public packet<0x3E> {
        enum class flags_f : uint8_t {
            invulnerable = 0x1,
            flying = 0x2,
            allow_flying = 0x4,
            creative_mode = 0x8,
        };
        using enum flags_f;

        enum_as_flag<flags_f, uint8_t> flags;
        float flying_speed;
        float fov_modifier;
    };

    struct player_chat : public packet<0x3F> { //TODO fix the encoding
        struct previous_message {
            value_optional<var_int32, std::array<uint8_t, 256>> message_id_or_signature;
        };

        struct no_filter : public enum_item<0> {};

        struct fully_filtered : public enum_item<1> {};

        struct partially_filtered : public enum_item<2> {
            bit_list_array<uint64_t> filtered_characters;
        };

        var_int32 global_index;
        base_objects::uuid sender;
        var_int32 index;
        std::optional<std::array<uint8_t, 256>> signature = std::nullopt;
        string_sized<256> message;
        uint64_t timestamp;
        uint64_t salt;
        list_array_sized<previous_message, 20> previous_messages;
        std::optional<base_objects::chat> unsigned_content = std::nullopt;
        enum_switch<var_int32, no_filter, fully_filtered, partially_filtered> filter;
        or_<var_int32::chat_type, chat_type> type;
        base_objects::chat sender_name;
        std::optional<base_objects::chat> target_name = std::nullopt;
    };

    struct player_combat_end : public packet<0x40> {
        var_int32 duration;
    };

    struct player_combat_enter : public packet<0x41> {};

    struct player_combat_kill : public packet<0x42> {
        var_int32 player_id;
        base_objects::chat message;
    };

    struct player_info_remove : public packet<0x43> {
        list_array<base_objects::uuid> uuids;
    };

    struct player_info_update : public packet<0x44> {
        struct add_player {
            string_sized<16> name;

            struct property {
                string_sized<64> name;
                string_sized<32767> value;
                std::optional<string_sized<1024>> signature = std::nullopt;
            };

            list_array_sized<property, 16> properties;
        };

        struct initialize_chat {
            base_objects::uuid chat_session_id;
            uint64_t pub_key_expiries_timestamp;
            list_array_fixed<uint8_t, 512> public_key;
            list_array_fixed<uint8_t, 4096> public_signature;
        };

        struct set_gamemode {
            var_int32 gamemode;
        };

        struct listed {
            bool should;
        };

        struct set_ping {
            var_int32 milliseconds;
        };

        struct set_display_name {
            std::optional<base_objects::chat> name = std::nullopt;
        };

        struct set_hat_visible {
            bool visible;
        };

        struct set_list_priority {
            var_int32 level;
        };

        struct header {
            base_objects::uuid uuid;
        };

        enum_set<
            header,
            add_player,
            initialize_chat,
            set_gamemode,
            listed,
            set_ping,
            set_display_name,
            set_list_priority,
            set_hat_visible>
            actions;
    };

    struct player_look_at : public packet<0x45> {
        enum class using_position_e : uint8_t {
            feet = 0,
            eyes = 1,
        };
        using enum using_position_e;

        struct entity_target {
            var_int32::entity_id id;
            enum_as<using_position_e, var_int32> using_position;
        };

        enum_as<using_position_e, var_int32> using_position;
        double target_x;
        double target_y;
        double target_z;
        std::optional<entity_target> entity = std::nullopt;
    };

    struct player_position : public packet<0x46> {
        ordered_id<var_int32, "sync/player_position"> teleport_id;
        double x;
        double y;
        double z;
        double velocity_x;
        double velocity_y;
        double velocity_z;
        float yaw;
        float pitch;
        teleport_flags flags;
    };

    struct player_rotation : public packet<0x47> {
        float yaw;
        bool yaw_is_relative;
        float pitch;
        bool pitch_is_relative;
    };

    struct recipe_book_add : public packet<0x48> {
        struct recipe {
            enum class flags_f : uint8_t {
                show_notification = 0x1,
                highlight_as_new = 0x2,
            };
            using enum flags_f;

            var_int32::recipe recipe_id;
            recipe_display display;
            var_int32 group_id;
            var_int32 category_id;
            std::optional<list_array<id_set<var_int32::item>>> ingredients = std::nullopt;
            enum_as_flag<flags_f, int8_t> flags;
        };

        list_array<recipe> recipes;
        bool replace;
    };

    struct recipe_book_remove : public packet<0x49> {
        list_array<var_int32::recipe> recipe_ids;
    };

    struct recipe_book_settings : public packet<0x4A> {
        bool crafting_recipe_open = false;
        bool crafting_recipe_filter_active = false;
        bool smelting_recipe_open = false;
        bool smelting_recipe_filter_active = false;
        bool blast_recipe_open = false;
        bool blast_recipe_filter_active = false;
        bool smoker_recipe_open = false;
        bool smoker_recipe_filter_active = false;
    };

    struct remove_entities : public packet<0x4B> {
        list_array<var_int32::entity_id> ids;
    };

    struct remove_mob_effect : public packet<0x4C> {
        var_int32::entity_id id;
        var_int32::mob_effect effect_id;
    };

    struct reset_score : public packet<0x4D> {
        string_sized<32767> entity_name;
        std::optional<string_sized<32767>> objective_name = std::nullopt;
    };

    struct resource_pack_pop : public packet<0x4E> {
        std::optional<base_objects::uuid> uuid = std::nullopt;
    };

    struct resource_pack_push : public packet<0x4F> {
        base_objects::uuid uuid;
        string_sized<32767> url;
        string_sized<40> hash; //0 or 40, other values waste bandwidth
        bool forced = false;
        std::optional<base_objects::chat> prompt_message = std::nullopt;
    };

    struct respawn : public packet<0x50> {
        struct death_location_t {
            identifier dimension_name;
            base_objects::position location;
        };

        var_int32::dimension_type dimension_type;
        identifier dimension_name;
        uint64_t seed_hashed;
        enum_as<gamemode_e, uint8_t> gamemode;
        enum_as<optional_gamemode_e, int8_t> previous_gamemode;
        bool is_debug = false;
        bool is_flat = false;
        std::optional<death_location_t> death_location = std::nullopt;
        var_int32 portal_cooldown;
        var_int32 sea_level;
        enum class flags_f : uint8_t {
            keep_attributes = 0x1,
            keep_metadata = 0x2,
        };
        using enum flags_f;

        enum_as_flag<flags_f, uint8_t> flags;
    };

    struct rotate_head : public packet<0x51> {
        var_int32::entity_id id;
        Angle head_yaw; //new angle
    };

    struct section_blocks_update : public packet<0x52> {
        struct position_t {
            uint64_t x : 22;
            uint64_t z : 22;
            uint64_t y : 20;

            uint64_t to_packet() const;

            static position_t from_packet(uint64_t value);
        };

        struct block_entry {
            uint32_t block_state : 20;
            uint32_t local_x : 4;
            uint32_t local_z : 4;
            uint32_t local_y : 4;

            var_int64 to_packet() const;

            static block_entry from_packet(var_int64 value);
        };

        position_t position;
        list_array<block_entry> block;
    };

    struct select_advancements_tab : public packet<0x53> {
        std::optional<identifier> id = std::nullopt;
    };

    struct server_data : public packet<0x54> {
        base_objects::chat motd;
        std::optional<list_array<uint8_t>> icon_png = std::nullopt;
    };

    struct set_action_bar_text : public packet<0x55> {
        base_objects::chat text;
    };

    struct set_border_center : public packet<0x56> {
        double x;
        double z;
    };

    struct set_border_lerp_size : public packet<0x57> {
        double old_diameter;
        double new_diameter;
        var_int64 speed_milliseconds;
    };

    struct set_border_size : public packet<0x58> {
        double diameter;
    };

    struct set_border_warning_delay : public packet<0x59> {
        var_int32 warn_time;
    };

    struct set_border_warning_distance : public packet<0x5A> {
        var_int32 meters;
    };

    struct set_camera : public packet<0x5B> {
        var_int32::entity_id id;
    };

    struct set_chunk_cache_center : public packet<0x5C> {
        var_int32 x;
        var_int32 z;
    };

    struct set_chunk_cache_radius : public packet<0x5D> {
        var_int32 distance;
    };

    struct set_cursor_item : public packet<0x5E> {
        slot item;
    };

    struct set_default_spawn_position : public packet<0x5F> {
        var_int32::dimension id;
        base_objects::position location;
        float yaw;
        float pitch;
    };

    struct set_display_objective : public packet<0x60> {
        enum class position_e : uint8_t {
            list = 0,
            sidebar = 1,
            below_name = 2,
            team_white = 4,
            team_orange = 5,
            team_magenta = 6,
            team_light_blue = 7,
            team_yellow = 8,
            team_lime = 9,
            team_pink = 10,
            team_gray = 11,
            team_light_gray = 12,
            team_cyan = 13,
            team_purple = 14,
            team_blue = 15,
            team_brown = 16,
            team_green = 17,
            team_red = 18,
            team_black = 19,
        };
        using enum position_e;
        enum_as<position_e, var_int32> position;
        string_sized<32767> name;
    };

    struct set_entity_data : public packet<0x61> {
        var_int32::entity_id id;

        struct metadata_item_t {
            uint8_t index = 0;
            base_objects::entity_metadata value;
        };

        list_array_siz_from_packet<metadata_item_t> metadata;
        constant_value<(uint8_t)0xFF> end_index; //this is safe because the sizeof(metadata_item_t) is more than 1 and this item would be not counted to metadata size because of the roundup by division in decoder
    };

    struct set_entity_link : public packet<0x62> {
        api::id::entity_id attached_id;
        api::id::entity_id holding_id;
    };

    struct set_entity_motion : public packet<0x63> {
        var_int32::entity_id id;
        base_objects::velocity velocity;
    };

    struct set_equipment : public packet<0x64> {
        struct equipment {
            enum class slot_place_e {
                main_hand = 0,
                off_hand = 1,
                boots = 2,
                leggings = 3,
                chestplate = 4,
                helmet = 5,
                body = 6,
                saddle = 7,
            };
            ignored<bool> has_next_item = false;

            item_depend<enum_as<slot_place_e, uint8_t>, 0x80, &equipment::has_next_item> slot_place;
            slot item;
        };

        var_int32::entity_id id;
        list_array_depend<equipment> equipments;
    };

    struct set_experience : public packet<0x65> {
        limited_num<float, 0.0f, 1.0f> bar;
        var_int32 level;
        var_int32 total_experience;
    };

    struct set_health : public packet<0x66> {
        float health;
        var_int32 food;
        limited_num<float, 0.0f, 5.0f> saturation;
    };

    struct set_held_slot : public packet<0x67> {
        var_int32 slot;
    };

    struct set_objective : public packet<0x68> {
        struct blank : public enum_item<0> {};

        struct styled : public enum_item<1> {
            enbt::compound styling;
        };

        struct fixed : public enum_item<2> {
            base_objects::chat content;
        };

        struct create : public enum_item<0> {
            base_objects::chat name;
            var_int32 type; //0 numbers, 1 - hearts
            std::optional<enum_switch<var_int32, blank, styled, fixed>> default_format;
        };

        struct remove : public enum_item<1> {};

        struct update : public enum_item<2> {
            base_objects::chat name;
            var_int32 type; //0 numbers, 1 - hearts
            std::optional<enum_switch<var_int32, blank, styled, fixed>> default_format;
        };

        string_sized<32767> name;
        enum_switch<int8_t, create, remove, update> mode;
    };

    struct set_passengers : public packet<0x69> {
        var_int32::entity_id id;
        list_array<var_int32::entity_id> passengers;
    };

    struct set_player_inventory : public packet<0x6A> {
        var_int32 slot;
        struct slot data;
    };

    struct set_player_team : public packet<0x6B> {
        enum class friendly_f : int8_t {
            allow_friendly_fire = 0x1,
            can_see_invisible = 0x2,
        };

        enum class name_tag_visibility_e {
            always,
            never,
            hide_for_others,
            hide_for_own,
        };

        enum class collision_rule_e {
            always,
            never,
            push_for_others,
            push_for_own,
        };

        struct create : public enum_item<0> {
            base_objects::chat display_name;
            enum_as_flag<friendly_f, int8_t> friendly;
            enum_as<name_tag_visibility_e, var_int32> name_tag_visibility;
            enum_as<collision_rule_e, var_int32> collision_rule;
            var_int32 team_color;
            base_objects::chat prefix;
            base_objects::chat suffix;
            list_array<string_sized<32767>> entries;
        };

        struct remove : public enum_item<1> {};

        struct update : public enum_item<2> {
            base_objects::chat display_name;
            enum_as_flag<friendly_f, int8_t> friendly;
            enum_as<name_tag_visibility_e, var_int32> name_tag_visibility;
            enum_as<collision_rule_e, var_int32> collision_rule;
            var_int32 team_color;
            base_objects::chat prefix;
            base_objects::chat suffix;
        };

        struct add_entries : public enum_item<3> {
            list_array<string_sized<32767>> entries;
        };

        struct remove_entries : public enum_item<4> {
            list_array<string_sized<32767>> entries;
        };

        string_sized<32767> name;
        enum_switch<int8_t, create, remove, update, add_entries, remove_entries> mode;
    };

    struct set_score : public packet<0x6C> {
        struct blank : public enum_item<0> {};

        struct styled : public enum_item<1> {
            enbt::compound styling;
        };

        struct fixed : public enum_item<2> {
            base_objects::chat content;
        };

        string_sized<32767> entry_name;
        string_sized<32767> objective_name;
        var_int32 value;
        std::optional<base_objects::chat> name = std::nullopt;
        std::optional<enum_switch<var_int32, blank, styled, fixed>> default_format = std::nullopt;
    };

    struct set_simulation_distance : public packet<0x6D> {
        var_int32 distance;
    };

    struct set_subtitle_text : public packet<0x6E> {
        base_objects::chat text;
    };

    struct set_time : public packet<0x6F> {
        uint64_t world_age;
        uint64_t time_of_day;
        bool time_of_day_increment;
    };

    struct set_title_text : public packet<0x70> {
        base_objects::chat text;
    };

    struct set_titles_animation : public packet<0x71> {
        int32_t fade_in;
        int32_t stay;
        int32_t fadeout;
    };

    struct sound_entity : public packet<0x72> {
        or_<var_int32::sound_event, base_objects::sound_event> sound;
        var_int32 category;
        var_int32::entity_id id;
        float volume;
        float pitch;
        int64_t seed;
    };

    struct sound : public packet<0x73> {
        or_<var_int32::sound_event, base_objects::sound_event> sound;
        var_int32 category;
        int32_t x;
        int32_t y;
        int32_t z;
        float volume;
        float pitch;
        int64_t seed;
    };

    struct start_configuration : public packet<0x74> {};

    struct stop_sound : public packet<0x75> {
        struct source : public flag_item<0x1, 0x1, 1> {
            var_int32 source;
        };

        struct sound_name : public flag_item<0x2, 0x2, 2> {
            identifier name;
        };

        flags_list<int8_t, source, sound_name> flags;
    };

    struct store_cookie : public packet<0x76> {
        identifier key;
        list_array_sized<uint8_t, 5120> payload;
    };

    struct system_chat : public packet<0x77> {
        base_objects::chat content;
        bool is_overlay = false;
    };

    struct tab_list : public packet<0x78> {
        base_objects::chat header;
        base_objects::chat footer;
    };

    struct tag_query : public packet<0x79> {
        var_int32 tag_query_id; //managed by client
        enbt::value nbt;
    };

    struct take_item_entity : public packet<0x7A> {
        var_int32::entity_id collected_id;
        var_int32::entity_id collectors_id;
        var_int32 items_count;
    };

    struct teleport_entity : public packet<0x7B> {
        var_int32::entity_id id;
        double x;
        double y;
        double z;
        double velocity_x;
        double velocity_y;
        double velocity_z;
        float yaw;
        float pitch;
        teleport_flags flags;
        bool on_ground;
    };

    struct test_instance_block_status : public packet<0x7C> {
        struct volume_t {
            double x;
            double y;
            double z;
        };

        base_objects::chat status;
        std::optional<volume_t> volume = std::nullopt;
    };

    struct ticking_state : public packet<0x7D> {
        float tick_rate;
        bool is_frozen;
    };

    struct ticking_step : public packet<0x7E> {
        var_int32 steps;
    };

    struct transfer : public packet<0x7F> {
        std::string host;
        var_int32 port;
    };

    struct update_advancements : public packet<0x80> {
        struct display {
            struct background_texture : public flag_item<0x1, 0x1, 1> {
                identifier texture;
            };

            struct show_toast : public flag_item<0x2, 0x2, 2> {};

            struct hidden : public flag_item<0x3, 0x3, 3> {};

            base_objects::chat title;
            base_objects::chat description;
            slot icon;
            var_int32 frame_type;
            flags_list<int32_t, background_texture, show_toast, hidden> flags;
            float x_cord;
            float y_cord;
        };

        struct advancement {
            std::optional<identifier> parent_id = std::nullopt;
            std::optional<display> display = std::nullopt;
            list_array<list_array<string_sized<32767>>> nested_requirements;
            bool send_telemetry = false;
        };

        struct progress {
            identifier criterion;
            std::optional<int64_t> date_of_archiving = std::nullopt;
        };

        struct advancement_mapping {
            identifier key;
            advancement value;
        };

        struct progress_mapping {
            identifier key;
            list_array<progress> value;
        };

        bool clear_prev;
        list_array<advancement_mapping> advancement_mappings;
        list_array<identifier> remove_advancements;
        list_array<progress_mapping> progress_mappings;
        bool show;
    };

    struct update_attributes : public packet<0x81> {
        //clients execute add operation first then add_percent and at the end multiply
        enum class operation_e {
            add = 0,
            add_percent = 1,
            multiply = 2
        };

        struct modifier {
            identifier id;
            double amount;
            enum_as<operation_e, int8_t> operation;
        };

        struct property {
            var_int32::attribute id;
            double value;
            list_array<modifier> modifiers;
        };

        var_int32::entity_id id;
        list_array<property> properties;
    };

    struct update_mob_effect : public packet<0x82> {
        enum class flags_f : int8_t {
            is_ambient = 0x1,
            show_particles = 0x2,
            show_icon = 0x4,
            blend = 0x8,
        };

        var_int32::entity_id id;
        var_int32::mob_effect effect;
        var_int32 amplifier;
        var_int32 duration;
        enum_as_flag<flags_f, int8_t> flags;
    };

    struct update_recipes : public packet<0x83> {
        struct property {
            identifier set_id;
            list_array<var_int32::item> items;
        };

        struct stonecuter_recipe {
            id_set<var_int32::item> ingredients;
            slot_display item;
        };

        list_array<property> property_sets;
        list_array<stonecuter_recipe> stonecuter_recipes;
    };

    struct update_tags : public packet<0x84> {
        struct tag {
            identifier tag_name;
            list_array<var_int32> values;
        };

        struct entry {
            identifier registry_id;
            list_array<tag> tags;
        };

        list_array<entry> entries;
    };

    struct projectile_power : public packet<0x85> {
        var_int32::entity_id id;
        double power;
    };

    struct custom_report_details : public packet<0x86> {
        struct detail {
            string_sized<128> title;
            string_sized<4096> description;
        };

        list_array_sized<detail, 32> details;
    };

    struct server_links : public packet<0x87> {
        enum class link_type : uint8_t {
            bug_report = 0,
            community_guidelines = 1,
            support = 2,
            status = 3,
            feedback = 4,
            community = 5,
            website = 6,
            forums = 7,
            news = 8,
            announcements = 9,
        };
        using enum link_type;

        struct link {
            bool_or<enum_as<link_type, var_int32>, base_objects::chat> label;
            std::string url;
        };

        list_array<link> links;
    };

    struct waypoint : public packet<0x88> {
        enum class operation_e {
            track = 0,
            untrack = 1,
            update = 2,
        };

        struct color_t {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };

        struct here : public enum_item<0> {};

        struct near : public enum_item<1> {
            var_int32 x;
            var_int32 y;
            var_int32 z;
        };

        struct far : public enum_item<2> {
            var_int32 x;
            var_int32 z;
        };

        struct far_away : public enum_item<3> {
            float azimuth;
        };

        bool_or<base_objects::uuid, std::string> id;
        identifier icon_style; //assets path
        std::optional<color_t> color = std::nullopt;
        enum_switch<var_int32, here, near, far, far_away> type;
    };

    struct clear_dialog : public packet<0x89> {};

    struct show_dialog : public packet<0x8A> {
        or_<var_int32::dialog, enbt::value> dialog;
    };

    using _play_packets = std::variant<
        bundle_delimiter,
        add_entity,
        animate,
        award_stats,
        block_changed_ack,
        block_destruction,
        block_entity_data,
        block_event,
        block_update,
        boss_event,
        change_difficulty,
        chunk_batch_finished,
        chunk_batch_start,
        chunks_biomes,
        clear_titles,
        command_suggestions,
        commands,
        container_close,
        container_set_content,
        container_set_data,
        container_set_slot,
        cookie_request,
        cooldown,
        custom_chat_completions,
        custom_payload,
        damage_event,
        debug__block_value,
        debug__chunk_value,
        debug__entity_value,
        debug__event,
        debug_sample,
        delete_chat,
        disconnect,
        disguised_chat,
        entity_event,
        entity_position_sync,
        explode,
        forget_level_chunk,
        game_event,
        game_test_highlight_pos,
        horse_screen_open,
        hurt_animation,
        initialize_border,
        keep_alive,
        level_chunk_with_light,
        level_event,
        level_particles,
        light_update,
        login,
        map_item_data,
        merchant_offers,
        move_entity_pos,
        move_entity_pos_rot,
        move_minecart_along_track,
        move_entity_rot,
        move_vehicle,
        open_book,
        open_screen,
        open_sign_editor,
        ping,
        pong_response,
        place_ghost_recipe,
        player_abilities,
        player_chat,
        player_combat_end,
        player_combat_enter,
        player_combat_kill,
        player_info_remove,
        player_info_update,
        player_look_at,
        player_position,
        player_rotation,
        recipe_book_add,
        recipe_book_remove,
        recipe_book_settings,
        remove_entities,
        remove_mob_effect,
        reset_score,
        resource_pack_pop,
        resource_pack_push,
        respawn,
        rotate_head,
        section_blocks_update,
        select_advancements_tab,
        server_data,
        set_action_bar_text,
        set_border_center,
        set_border_lerp_size,
        set_border_size,
        set_border_warning_delay,
        set_border_warning_distance,
        set_camera,
        set_chunk_cache_center,
        set_chunk_cache_radius,
        set_cursor_item,
        set_default_spawn_position,
        set_display_objective,
        set_entity_data,
        set_entity_link,
        set_entity_motion,
        set_equipment,
        set_experience,
        set_health,
        set_held_slot,
        set_objective,
        set_passengers,
        set_player_inventory,
        set_player_team,
        set_score,
        set_simulation_distance,
        set_subtitle_text,
        set_time,
        set_title_text,
        set_titles_animation,
        sound_entity,
        sound,
        start_configuration,
        stop_sound,
        store_cookie,
        system_chat,
        tab_list,
        tag_query,
        take_item_entity,
        teleport_entity,
        test_instance_block_status,
        ticking_state,
        ticking_step,
        transfer,
        update_advancements,
        update_attributes,
        update_mob_effect,
        update_recipes,
        update_tags,
        projectile_power,
        custom_report_details,
        server_links,
        waypoint,
        clear_dialog,
        show_dialog>;

    struct play_packet : public _play_packets {
        using base = _play_packets;
        using base::variant;
        using base::operator=;
    };
}

namespace copper_server::api::packets {
    extern template packet_ops<client_bound::play::bundle_delimiter>;
    extern template packet_ops<client_bound::play::add_entity>;
    extern template packet_ops<client_bound::play::animate>;
    extern template packet_ops<client_bound::play::award_stats>;
    extern template packet_ops<client_bound::play::block_changed_ack>;
    extern template packet_ops<client_bound::play::block_destruction>;
    extern template packet_ops<client_bound::play::block_entity_data>;
    extern template packet_ops<client_bound::play::block_event>;
    extern template packet_ops<client_bound::play::block_update>;
    extern template packet_ops<client_bound::play::boss_event>;
    extern template packet_ops<client_bound::play::change_difficulty>;
    extern template packet_ops<client_bound::play::chunk_batch_finished>;
    extern template packet_ops<client_bound::play::chunk_batch_start>;
    extern template packet_ops<client_bound::play::chunks_biomes>;
    extern template packet_ops<client_bound::play::clear_titles>;
    extern template packet_ops<client_bound::play::command_suggestions>;
    extern template packet_ops<client_bound::play::commands>;
    extern template packet_ops<client_bound::play::container_close>;
    extern template packet_ops<client_bound::play::container_set_content>;
    extern template packet_ops<client_bound::play::container_set_data>;
    extern template packet_ops<client_bound::play::container_set_slot>;
    extern template packet_ops<client_bound::play::cookie_request>;
    extern template packet_ops<client_bound::play::cooldown>;
    extern template packet_ops<client_bound::play::custom_chat_completions>;
    extern template packet_ops<client_bound::play::custom_payload>;
    extern template packet_ops<client_bound::play::damage_event>;
    extern template packet_ops<client_bound::play::debug__block_value>;
    extern template packet_ops<client_bound::play::debug__chunk_value>;
    extern template packet_ops<client_bound::play::debug__entity_value>;
    extern template packet_ops<client_bound::play::debug__event>;
    extern template packet_ops<client_bound::play::debug_sample>;
    extern template packet_ops<client_bound::play::delete_chat>;
    extern template packet_ops<client_bound::play::disconnect>;
    extern template packet_ops<client_bound::play::disguised_chat>;
    extern template packet_ops<client_bound::play::entity_event>;
    extern template packet_ops<client_bound::play::entity_position_sync>;
    extern template packet_ops<client_bound::play::explode>;
    extern template packet_ops<client_bound::play::forget_level_chunk>;
    extern template packet_ops<client_bound::play::game_event>;
    extern template packet_ops<client_bound::play::game_test_highlight_pos>;
    extern template packet_ops<client_bound::play::horse_screen_open>;
    extern template packet_ops<client_bound::play::hurt_animation>;
    extern template packet_ops<client_bound::play::initialize_border>;
    extern template packet_ops<client_bound::play::keep_alive>;
    extern template packet_ops<client_bound::play::level_chunk_with_light>;
    extern template packet_ops<client_bound::play::level_event>;
    extern template packet_ops<client_bound::play::level_particles>;
    extern template packet_ops<client_bound::play::light_update>;
    extern template packet_ops<client_bound::play::login>;
    extern template packet_ops<client_bound::play::map_item_data>;
    extern template packet_ops<client_bound::play::merchant_offers>;
    extern template packet_ops<client_bound::play::move_entity_pos>;
    extern template packet_ops<client_bound::play::move_entity_pos_rot>;
    extern template packet_ops<client_bound::play::move_minecart_along_track>;
    extern template packet_ops<client_bound::play::move_entity_rot>;
    extern template packet_ops<client_bound::play::move_vehicle>;
    extern template packet_ops<client_bound::play::open_book>;
    extern template packet_ops<client_bound::play::open_screen>;
    extern template packet_ops<client_bound::play::open_sign_editor>;
    extern template packet_ops<client_bound::play::ping>;
    extern template packet_ops<client_bound::play::pong_response>;
    extern template packet_ops<client_bound::play::place_ghost_recipe>;
    extern template packet_ops<client_bound::play::player_abilities>;
    extern template packet_ops<client_bound::play::player_chat>;
    extern template packet_ops<client_bound::play::player_combat_end>;
    extern template packet_ops<client_bound::play::player_combat_enter>;
    extern template packet_ops<client_bound::play::player_combat_kill>;
    extern template packet_ops<client_bound::play::player_info_remove>;
    extern template packet_ops<client_bound::play::player_info_update>;
    extern template packet_ops<client_bound::play::player_look_at>;
    extern template packet_ops<client_bound::play::player_position>;
    extern template packet_ops<client_bound::play::player_rotation>;
    extern template packet_ops<client_bound::play::recipe_book_add>;
    extern template packet_ops<client_bound::play::recipe_book_remove>;
    extern template packet_ops<client_bound::play::recipe_book_settings>;
    extern template packet_ops<client_bound::play::remove_entities>;
    extern template packet_ops<client_bound::play::remove_mob_effect>;
    extern template packet_ops<client_bound::play::reset_score>;
    extern template packet_ops<client_bound::play::resource_pack_pop>;
    extern template packet_ops<client_bound::play::resource_pack_push>;
    extern template packet_ops<client_bound::play::respawn>;
    extern template packet_ops<client_bound::play::rotate_head>;
    extern template packet_ops<client_bound::play::section_blocks_update>;
    extern template packet_ops<client_bound::play::select_advancements_tab>;
    extern template packet_ops<client_bound::play::server_data>;
    extern template packet_ops<client_bound::play::set_action_bar_text>;
    extern template packet_ops<client_bound::play::set_border_center>;
    extern template packet_ops<client_bound::play::set_border_lerp_size>;
    extern template packet_ops<client_bound::play::set_border_size>;
    extern template packet_ops<client_bound::play::set_border_warning_delay>;
    extern template packet_ops<client_bound::play::set_border_warning_distance>;
    extern template packet_ops<client_bound::play::set_camera>;
    extern template packet_ops<client_bound::play::set_chunk_cache_center>;
    extern template packet_ops<client_bound::play::set_chunk_cache_radius>;
    extern template packet_ops<client_bound::play::set_cursor_item>;
    extern template packet_ops<client_bound::play::set_default_spawn_position>;
    extern template packet_ops<client_bound::play::set_display_objective>;
    extern template packet_ops<client_bound::play::set_entity_data>;
    extern template packet_ops<client_bound::play::set_entity_link>;
    extern template packet_ops<client_bound::play::set_entity_motion>;
    extern template packet_ops<client_bound::play::set_equipment>;
    extern template packet_ops<client_bound::play::set_experience>;
    extern template packet_ops<client_bound::play::set_health>;
    extern template packet_ops<client_bound::play::set_held_slot>;
    extern template packet_ops<client_bound::play::set_objective>;
    extern template packet_ops<client_bound::play::set_passengers>;
    extern template packet_ops<client_bound::play::set_player_inventory>;
    extern template packet_ops<client_bound::play::set_player_team>;
    extern template packet_ops<client_bound::play::set_score>;
    extern template packet_ops<client_bound::play::set_simulation_distance>;
    extern template packet_ops<client_bound::play::set_subtitle_text>;
    extern template packet_ops<client_bound::play::set_time>;
    extern template packet_ops<client_bound::play::set_title_text>;
    extern template packet_ops<client_bound::play::set_titles_animation>;
    extern template packet_ops<client_bound::play::sound_entity>;
    extern template packet_ops<client_bound::play::sound>;
    extern template packet_ops<client_bound::play::start_configuration>;
    extern template packet_ops<client_bound::play::stop_sound>;
    extern template packet_ops<client_bound::play::store_cookie>;
    extern template packet_ops<client_bound::play::system_chat>;
    extern template packet_ops<client_bound::play::tab_list>;
    extern template packet_ops<client_bound::play::tag_query>;
    extern template packet_ops<client_bound::play::take_item_entity>;
    extern template packet_ops<client_bound::play::teleport_entity>;
    extern template packet_ops<client_bound::play::test_instance_block_status>;
    extern template packet_ops<client_bound::play::ticking_state>;
    extern template packet_ops<client_bound::play::ticking_step>;
    extern template packet_ops<client_bound::play::transfer>;
    extern template packet_ops<client_bound::play::update_advancements>;
    extern template packet_ops<client_bound::play::update_attributes>;
    extern template packet_ops<client_bound::play::update_mob_effect>;
    extern template packet_ops<client_bound::play::update_recipes>;
    extern template packet_ops<client_bound::play::update_tags>;
    extern template packet_ops<client_bound::play::projectile_power>;
    extern template packet_ops<client_bound::play::custom_report_details>;
    extern template packet_ops<client_bound::play::server_links>;
    extern template packet_ops<client_bound::play::waypoint>;
    extern template packet_ops<client_bound::play::clear_dialog>;
    extern template packet_ops<client_bound::play::show_dialog>;

    using client_bound_play_ops = state_ops<
        client_bound::play::bundle_delimiter,
        client_bound::play::add_entity,
        client_bound::play::animate,
        client_bound::play::award_stats,
        client_bound::play::block_changed_ack,
        client_bound::play::block_destruction,
        client_bound::play::block_entity_data,
        client_bound::play::block_event,
        client_bound::play::block_update,
        client_bound::play::boss_event,
        client_bound::play::change_difficulty,
        client_bound::play::chunk_batch_finished,
        client_bound::play::chunk_batch_start,
        client_bound::play::chunks_biomes,
        client_bound::play::clear_titles,
        client_bound::play::command_suggestions,
        client_bound::play::commands,
        client_bound::play::container_close,
        client_bound::play::container_set_content,
        client_bound::play::container_set_data,
        client_bound::play::container_set_slot,
        client_bound::play::cookie_request,
        client_bound::play::cooldown,
        client_bound::play::custom_chat_completions,
        client_bound::play::custom_payload,
        client_bound::play::damage_event,
        client_bound::play::debug__block_value,
        client_bound::play::debug__chunk_value,
        client_bound::play::debug__entity_value,
        client_bound::play::debug__event,
        client_bound::play::debug_sample,
        client_bound::play::delete_chat,
        client_bound::play::disconnect,
        client_bound::play::disguised_chat,
        client_bound::play::entity_event,
        client_bound::play::entity_position_sync,
        client_bound::play::explode,
        client_bound::play::forget_level_chunk,
        client_bound::play::game_event,
        client_bound::play::game_test_highlight_pos,
        client_bound::play::horse_screen_open,
        client_bound::play::hurt_animation,
        client_bound::play::initialize_border,
        client_bound::play::keep_alive,
        client_bound::play::level_chunk_with_light,
        client_bound::play::level_event,
        client_bound::play::level_particles,
        client_bound::play::light_update,
        client_bound::play::login,
        client_bound::play::map_item_data,
        client_bound::play::merchant_offers,
        client_bound::play::move_entity_pos,
        client_bound::play::move_entity_pos_rot,
        client_bound::play::move_minecart_along_track,
        client_bound::play::move_entity_rot,
        client_bound::play::move_vehicle,
        client_bound::play::open_book,
        client_bound::play::open_screen,
        client_bound::play::open_sign_editor,
        client_bound::play::ping,
        client_bound::play::pong_response,
        client_bound::play::place_ghost_recipe,
        client_bound::play::player_abilities,
        client_bound::play::player_chat,
        client_bound::play::player_combat_end,
        client_bound::play::player_combat_enter,
        client_bound::play::player_combat_kill,
        client_bound::play::player_info_remove,
        client_bound::play::player_info_update,
        client_bound::play::player_look_at,
        client_bound::play::player_position,
        client_bound::play::player_rotation,
        client_bound::play::recipe_book_add,
        client_bound::play::recipe_book_remove,
        client_bound::play::recipe_book_settings,
        client_bound::play::remove_entities,
        client_bound::play::remove_mob_effect,
        client_bound::play::reset_score,
        client_bound::play::resource_pack_pop,
        client_bound::play::resource_pack_push,
        client_bound::play::respawn,
        client_bound::play::rotate_head,
        client_bound::play::section_blocks_update,
        client_bound::play::select_advancements_tab,
        client_bound::play::server_data,
        client_bound::play::set_action_bar_text,
        client_bound::play::set_border_center,
        client_bound::play::set_border_lerp_size,
        client_bound::play::set_border_size,
        client_bound::play::set_border_warning_delay,
        client_bound::play::set_border_warning_distance,
        client_bound::play::set_camera,
        client_bound::play::set_chunk_cache_center,
        client_bound::play::set_chunk_cache_radius,
        client_bound::play::set_cursor_item,
        client_bound::play::set_default_spawn_position,
        client_bound::play::set_display_objective,
        client_bound::play::set_entity_data,
        client_bound::play::set_entity_link,
        client_bound::play::set_entity_motion,
        client_bound::play::set_equipment,
        client_bound::play::set_experience,
        client_bound::play::set_health,
        client_bound::play::set_held_slot,
        client_bound::play::set_objective,
        client_bound::play::set_passengers,
        client_bound::play::set_player_inventory,
        client_bound::play::set_player_team,
        client_bound::play::set_score,
        client_bound::play::set_simulation_distance,
        client_bound::play::set_subtitle_text,
        client_bound::play::set_time,
        client_bound::play::set_title_text,
        client_bound::play::set_titles_animation,
        client_bound::play::sound_entity,
        client_bound::play::sound,
        client_bound::play::start_configuration,
        client_bound::play::stop_sound,
        client_bound::play::store_cookie,
        client_bound::play::system_chat,
        client_bound::play::tab_list,
        client_bound::play::tag_query,
        client_bound::play::take_item_entity,
        client_bound::play::teleport_entity,
        client_bound::play::test_instance_block_status,
        client_bound::play::ticking_state,
        client_bound::play::ticking_step,
        client_bound::play::transfer,
        client_bound::play::update_advancements,
        client_bound::play::update_attributes,
        client_bound::play::update_mob_effect,
        client_bound::play::update_recipes,
        client_bound::play::update_tags,
        client_bound::play::projectile_power,
        client_bound::play::custom_report_details,
        client_bound::play::server_links,
        client_bound::play::waypoint,
        client_bound::play::clear_dialog,
        client_bound::play::show_dialog>;
}

inline copper_server::api::packets::client_bound::play::player_abilities::flags_f operator|(copper_server::api::packets::client_bound::play::player_abilities::flags_f a, copper_server::api::packets::client_bound::play::player_abilities::flags_f b) {
    return copper_server::api::packets::client_bound::play::player_abilities::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::client_bound::play::recipe_book_add::recipe::flags_f operator|(copper_server::api::packets::client_bound::play::recipe_book_add::recipe::flags_f a, copper_server::api::packets::client_bound::play::recipe_book_add::recipe::flags_f b) {
    return copper_server::api::packets::client_bound::play::recipe_book_add::recipe::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::client_bound::play::respawn::flags_f operator|(copper_server::api::packets::client_bound::play::respawn::flags_f a, copper_server::api::packets::client_bound::play::respawn::flags_f b) {
    return copper_server::api::packets::client_bound::play::respawn::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::client_bound::play::set_player_team::friendly_f operator|(copper_server::api::packets::client_bound::play::set_player_team::friendly_f a, copper_server::api::packets::client_bound::play::set_player_team::friendly_f b) {
    return copper_server::api::packets::client_bound::play::set_player_team::friendly_f(static_cast<int8_t>(a) | static_cast<int8_t>(b));
}

inline copper_server::api::packets::client_bound::play::update_mob_effect::flags_f operator|(copper_server::api::packets::client_bound::play::update_mob_effect::flags_f a, copper_server::api::packets::client_bound::play::update_mob_effect::flags_f b) {
    return copper_server::api::packets::client_bound::play::update_mob_effect::flags_f(static_cast<int8_t>(a) | static_cast<int8_t>(b));
}

#endif /* SRC_API_PACKETS_CLIENT_BOUND_PLAY */
