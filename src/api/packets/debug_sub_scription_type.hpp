/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_DEBUG_SUB_SCRIPTION_TYPE
#define SRC_API_PACKETS_DEBUG_SUB_SCRIPTION_TYPE
#include <optional>
#include <src/api/packets/types.hpp>
#include <src/base_objects/position.hpp>
#include <src/util/calculations.hpp>

namespace copper_server::api::packets {
    struct debug_sub_scription_type {
        struct dedicated_server_tick_time : public enum_item<0> {};

        struct bees : public enum_item<1> {
            std::optional<base_objects::position> hive_pos;
            std::optional<base_objects::position> flower_pos;
            var_int32 travel_ticks;
            list_array<base_objects::position> black_listed_hives;
        };

        struct brains : public enum_item<2> {
            string_sized<32767> name;
            string_sized<32767> profession;
            int32_t xp;
            float health;
            float max_health;
            string_sized<32767> inventory;
            bool wants_golem;
            int32_t anger_level;
            list_array<string_sized<32767>> activities;
            list_array<string_sized<32767>> behaviors;
            list_array<string_sized<32767>> memories;
            list_array<string_sized<32767>> gossips;
            list_array<base_objects::position> pois;
            list_array<base_objects::position> potential_work_pois;
        };

        struct breezes : public enum_item<3> {
            std::optional<var_int32> attack_target;
            std::optional<base_objects::position> jump_target;
        };

        struct goal_selectors : public enum_item<4> {
            struct goal {
                var_int32 priority;
                bool is_running;
                string_sized<255> name;
            };

            list_array<goal> goals;
        };

        struct entity_paths : public enum_item<5> {
            struct path_t {
                struct node_t {
                    enum class type_t {
                        blocked,
                        open,
                        walkable,
                        walkable_door,
                        trapdoor,
                        powder_snow,
                        danger_powder_snow,
                        fence,
                        lava,
                        water,
                        water_border,
                        rail,
                        unpassable_rail,
                        danger_fire,
                        damage_fire,
                        danger_other,
                        damage_other,
                        door_open,
                        door_wood_closed,
                        door_iron_closed,
                        breach,
                        leaves,
                        sticky_honey,
                        cocoa,
                        damage_cautious,
                        danger_trapdoor,
                    };

                    int32_t x;
                    int32_t y;
                    int32_t z;
                    float path_lenght;
                    float penalty_value;
                    bool visited;
                    enum_as<type_t, var_int32> type;
                    float heap_weight;
                };

                bool reaches_target;
                int32_t current_node_index;
                base_objects::position target;
                list_array<node_t> nodes;
            };

            path_t path;
            float max_node_distance;
        };

        struct entity_block_intersections : public enum_item<6> {
            enum class type_t {
                in_block = 0,
                in_fluid = 1,
                in_air
            };
            enum_as<type_t, var_int32> type;
        };

        struct bee_hives : public enum_item<7> {
            var_int32::block_type type;
            var_int32 bees_in;
            var_int32 honey_level;
            bool is_sedated;
        };

        struct pois : public enum_item<8> {
            base_objects::position target;
            var_int32::point_of_interest_type type;
            var_int32 free_ticked_count;
        };

        struct redstone_wire_orientations : public enum_item<9> {
            enum class direction : uint8_t {
                down = 0,
                up = 1,
                north = 2,
                south = 3,
                west = 4,
                east = 5,
            };

            enum class side_bias : uint8_t {
                left = 0,
                right = 1,
            };

            uint8_t up_val : 3;
            uint8_t front_ortho_idx : 2;
            uint8_t bias_val : 1;

            direction get_up() const;
            side_bias get_bias() const;
            direction get_front() const;

            void set_up(direction up);
            void set_bias(side_bias bias);
            void set_front(direction front);

            uint8_t to_packet() const;
            static redstone_wire_orientations from_packet(uint8_t val);
        };

        struct village_sections : public enum_item<10> {};

        struct raids : public enum_item<11> {
            list_array<base_objects::position> value;
        };

        struct structures : public enum_item<12> {
            struct block_box {
                base_objects::position min;
                base_objects::position max;
            };

            struct piece {
                block_box bounding;
                bool is_start;
            };

            struct item {
                block_box bounding;
                list_array<piece> pieces;
            };

            list_array<item> value;
        };

        struct game_event_listeners : public enum_item<13> {
            var_int32 listener_radius;
        };

        struct neighbor_updates : public enum_item<14> {
            base_objects::position pos;
        };

        struct game_events : public enum_item<15> {
            var_int32::game_event event;
            util::VECTOR pos;
        };

        using value = enum_switch<
            var_int32,
            dedicated_server_tick_time,
            bees,
            brains,
            breezes,
            goal_selectors,
            entity_paths,
            entity_block_intersections,
            bee_hives,
            pois,
            redstone_wire_orientations,
            village_sections,
            raids,
            structures,
            game_event_listeners,
            neighbor_updates,
            game_events>;

        using optional = enum_switch< //TODO add new type `optional_enum_switch` to represent enum_id->optional->value
            var_int32,
            dedicated_server_tick_time,
            bees,
            brains,
            breezes,
            goal_selectors,
            entity_paths,
            entity_block_intersections,
            bee_hives,
            pois,
            redstone_wire_orientations,
            village_sections,
            raids,
            structures,
            game_event_listeners,
            neighbor_updates,
            game_events>;
    };
}
#endif /* SRC_API_PACKETS_DEBUG_SUB_SCRIPTION_TYPE */
