/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_PARSERS
#define SRC_BASE_OBJECTS_PARSERS
#include <cstdint>
#include <library/enbt/enbt.hpp>
#include <src/base_objects/packets_help.hpp>
#include <string>
#include <unordered_map>

namespace copper_server::base_objects {
    namespace parsers {
        namespace command {
            struct _bool : public enum_item<0> {};

            struct _float : public enum_item<1> {
                struct min : public flag_item<1, 1, 1> {
                    float value;
                };

                struct max : public flag_item<2, 2, 2> {
                    float value;
                };

                flags_list<int8_t, min, max> flags;
            };

            struct _double : public enum_item<2> {
                struct min : public flag_item<1, 1, 1> {
                    double value;
                };

                struct max : public flag_item<2, 2, 2> {
                    double value;
                };

                flags_list<int8_t, min, max> flags;
            };

            struct _integer : public enum_item<3> {
                struct min : public flag_item<1, 1, 1> {
                    int32_t value;
                };

                struct max : public flag_item<2, 2, 2> {
                    int32_t value;
                };

                flags_list<int8_t, min, max> flags;
            };

            struct _long : public enum_item<4> {
                struct min : public flag_item<1, 1, 1> {
                    int64_t value;
                };

                struct max : public flag_item<2, 2, 2> {
                    int64_t value;
                };

                flags_list<int8_t, min, max> flags;
            };

            struct string : public enum_item<5> {
                enum class type_e : uint8_t {
                    single_word = 0,
                    quotable_phrase = 1,
                    greedy_phrase = 2,
                };
                using enum type_e;

                enum_as<type_e, var_int32> type;
            };

            struct entity : public enum_item<6> {
                enum class flags_f : int8_t {
                    only_one_entity = 0,
                    only_player_entity = 1
                };
                using enum flags_f;
                enum_as_flag<flags_f, int8_t> flag;
            };

            struct game_profile : public enum_item<7> {};

            struct block_pos : public enum_item<8> {};

            struct column_pos : public enum_item<9> {};

            struct vec2 : public enum_item<10> {};

            struct vec3 : public enum_item<11> {};

            struct block_state : public enum_item<12> {};

            struct block_predicate : public enum_item<13> {};

            struct item_stack : public enum_item<14> {};

            struct item_predicate : public enum_item<15> {};

            struct color : public enum_item<16> {};

            struct hex_color : public enum_item<17> {};

            struct component : public enum_item<18> {};

            struct style : public enum_item<19> {};

            struct message : public enum_item<20> {};

            struct nbt_compound_tag : public enum_item<21> {};

            struct nbt_tag : public enum_item<22> {};

            struct nbt_path : public enum_item<23> {};

            struct objective : public enum_item<24> {};

            struct objective_criteria : public enum_item<25> {};

            struct operation : public enum_item<26> {};

            struct particle : public enum_item<27> {};

            struct angle : public enum_item<28> {};

            struct rotation : public enum_item<29> {};

            struct scoreboard_slot : public enum_item<30> {};

            struct score_holder : public enum_item<31> {
                bool allow_multiple;
            };

            struct swizzle : public enum_item<32> {};

            struct team : public enum_item<33> {};

            struct item_slot : public enum_item<34> {};

            struct item_slots : public enum_item<35> {};

            struct resource_location : public enum_item<36> {};

            struct function : public enum_item<37> {};

            struct entity_anchor : public enum_item<38> {};

            struct int_range : public enum_item<39> {};

            struct float_range : public enum_item<40> {};

            struct dimension : public enum_item<41> {};

            struct gamemode : public enum_item<42> {};

            struct time : public enum_item<43> {
                int32_t min_ticks;
            };

            struct resource_or_tag : public enum_item<44> {
                identifier suggestion_registry;
            };

            struct resource_or_tag_key : public enum_item<45> {
                identifier suggestion_registry;
            };

            struct resource : public enum_item<46> {
                identifier suggestion_registry;
            };

            struct resource_key : public enum_item<47> {
                identifier suggestion_registry;
            };

            struct resource_selector : public enum_item<48> {
                identifier suggestion_registry;
            };

            struct template_mirror : public enum_item<49> {};

            struct template_rotation : public enum_item<50> {};

            struct heightmap : public enum_item<51> {};

            struct loot_table : public enum_item<52> {};

            struct loot_predicate : public enum_item<53> {};

            struct loot_modifier : public enum_item<54> {};

            struct dialog : public enum_item<55> {};

            struct uuid : public enum_item<56> {};
        }

        struct _bool {
            bool value;
        };

        struct _float {
            float value;
        };

        struct _double {
            double value;
        };

        struct _integer {
            int32_t value;
        };

        struct _long {
            int64_t value;
        };

        struct string {
            std::string value;
            //
        };

        struct entity {
            enum class type_t : uint8_t {
                name,
                selector,
                uuid,
            };

            std::string value;
            type_t type;
        };

        struct game_profile {
            enum class type_t : uint8_t {
                name,
                selector,
                uuid,
            };
            std::string string;
            type_t type;
        };

        struct block_pos {
            int64_t x;
            int64_t y;
            int64_t z;
            bool x_relative = false;
            bool y_relative = false;
            bool z_relative = false;
        };

        struct column_pos {
            float x;
            float z;
            bool x_relative = false;
            bool z_relative = false;
        };

        struct vec2 {
            float v[2];
            bool relative[2]{false, false};
        };

        struct vec3 {
            float v[3];
            bool relative[3]{false, false, false};
        };

        struct block_state {
            std::string block_id;
            std::unordered_map<std::string, std::string> states;
            enbt::value data_tags;
        };

        struct block_predicate {}; //TODO

        struct item_stack {
            std::string value;
            enbt::value data_tags;
        };

        struct item_predicate {}; //TODO

        enum class color : uint8_t {
            white = 0,
            orange = 1,
            magenta = 2,
            light_blue = 3,
            yellow = 4,
            lime = 5,
            pink = 6,
            gray = 7,
            light_gray = 8,
            cyan = 9,
            purple = 10,
            blue = 11,
            brown = 12,
            green = 13,
            red = 14,
            black = 15,
        };

        struct hex_color {
            int32_t rgb;
        };

        struct component {}; //TODO

        struct style {}; //TODO

        struct message {
            std::string str;
        };

        struct nbt_compound_tag {
            enbt::compound nbt;
        };

        struct nbt_tag {
            enbt::value any_nbt;
        };

        struct nbt_path {
            std::string path;
        };

        struct objective {
            std::string value;
        };

        struct objective_criteria {
            std::string value;
        };

        enum class operation : uint8_t {
            assignment,     //=
            addition,       //+=
            subtraction,    //-=
            multiplication, //*=
            floor_division, // /=
            modulus,        //%=
            swapping,       //><
            minimum,        //<
            maximum,        //>
        };

        struct particle { //TODO update

            struct block {
                std::string block_id;
                std::unordered_map<std::string, std::string> states;
            };

            struct block_marker {
                std::string block_id;
                std::unordered_map<std::string, std::string> states;
            };

            struct falling_dust {
                std::string block_id;
                std::unordered_map<std::string, std::string> states;
            };

            struct dust {
                float r;
                float g;
                float b;
                float size;
            };

            struct dust_color_transition {
                float begin_r;
                float begin_g;
                float begin_b;
                float end_r;
                float end_g;
                float end_b;
                float size;
            };

            struct item {
                std::string item_id;
                enbt::value nbt;
            };

            struct sculk_charge {
                float angle;
            };

            struct shriek {
                float angle;
            };

            struct vibration {
                double destination_x;
                double destination_y;
                double destination_z;
                int64_t ticks;
            };

            struct any_other {
                std::string name;
            };

            std::variant<
                block,
                block_marker,
                falling_dust,
                dust,
                dust_color_transition,
                item,
                sculk_charge,
                shriek,
                vibration>
                particle;
        };

        struct angle {
            float yaw;
            bool relative = false;
        };

        struct rotation {
            double yaw;
            double pitch;
        };

        struct scoreboard_slot {
            std::string value;
        };

        struct score_holder {
            struct selector {
                std::string value;
            };

            struct name {
                std::string value;
            };

            struct uuid {
                std::string value;
            };

            struct anything {
            };

            std::variant<selector, name, uuid, anything> value;
        };

        struct swizzle {
            enum class coord : uint8_t {
                undefined,
                x,
                y,
                z,
            };
            coord v0 : 2 = coord::undefined;
            coord v1 : 2 = coord::undefined;
            coord v2 : 2 = coord::undefined;
        };

        struct team {
            //[-+._A-Za-z0-9]*
            std::string value;
        };

        struct item_slot {
            std::optional<std::string> container;
            std::optional<int32_t> number;
        };

        struct item_slots {
            list_array<item_slot> slots;
        };

        struct resource_location {
            std::string value;
        };

        struct function {
            std::string value;
        };

        enum class entity_anchor : uint8_t {
            eyes,
            feet,
        };

        struct int_range {
            std::optional<int32_t> begin;
            std::optional<int32_t> end;
        };

        struct float_range {
            std::optional<float> begin;
            std::optional<float> end;
        };

        struct dimension {
            std::string string;
        };

        enum class gamemode : uint8_t {
            survival,
            creative,
            adventure,
            spectator,
        };

        struct time {
            int64_t ticks;

            //d = day = 24000 ticks
            //s = seconds = 20 ticks
            //t = tick = 1 tick (default)
            //0d float
            //0s float
            //0t int64_t
            //0  int64_t
        };

        struct resource_or_tag {
            std::string value;
        };

        struct resource_or_tag_key {
            std::string value;
        };

        struct resource {
            std::string value;
        };

        struct resource_key {
            std::string value;
        };

        struct resource_selector {
            std::string value;
        };

        enum class template_mirror : uint8_t {
            none,
            front_back,
            left_right,
        };

        enum class template_rotation : uint8_t {
            none,                //0
            clockwise_90,        //90
            counterclockwise_90, //270
            _180,                //180
        };

        enum class heightmap : uint8_t {
            world_surface,
            motion_blocking,
            motion_blocking_no_leaves,
            ocean_floor,
        };

        struct loot_table {}; //TODO

        struct loot_predicate {}; //TODO

        struct loot_modifier {}; //TODO

        struct dialog {}; //TODO

        struct uuid {
            enbt::raw_uuid value;
        };
    }

    using parser = std::variant<
        parsers::_bool,
        parsers::_float,
        parsers::_double,
        parsers::_integer,
        parsers::_long,
        parsers::string,
        parsers::entity,
        parsers::game_profile,
        parsers::block_pos,
        parsers::column_pos,
        parsers::vec2,
        parsers::vec3,
        parsers::block_state,
        parsers::block_predicate,
        parsers::item_stack,
        parsers::item_predicate,
        parsers::color,
        parsers::hex_color,
        parsers::component,
        parsers::style,
        parsers::message,
        parsers::nbt_compound_tag,
        parsers::nbt_tag,
        parsers::nbt_path,
        parsers::objective,
        parsers::objective_criteria,
        parsers::operation,
        parsers::particle,
        parsers::angle,
        parsers::rotation,
        parsers::scoreboard_slot,
        parsers::score_holder,
        parsers::swizzle,
        parsers::team,
        parsers::item_slot,
        parsers::item_slots,
        parsers::resource_location,
        parsers::function,
        parsers::entity_anchor,
        parsers::int_range,
        parsers::float_range,
        parsers::dimension,
        parsers::gamemode,
        parsers::time,
        parsers::resource_or_tag,
        parsers::resource_or_tag_key,
        parsers::resource,
        parsers::resource_key,
        parsers::resource_selector,
        parsers::template_mirror,
        parsers::template_rotation,
        parsers::heightmap,
        parsers::loot_table,
        parsers::loot_predicate,
        parsers::loot_modifier,
        parsers::dialog,
        parsers::uuid>;


    using command_parser = enum_switch<
        var_int32,
        parsers::command::_bool,
        parsers::command::_float,
        parsers::command::_double,
        parsers::command::_integer,
        parsers::command::_long,
        parsers::command::string,
        parsers::command::entity,
        parsers::command::game_profile,
        parsers::command::block_pos,
        parsers::command::column_pos,
        parsers::command::vec2,
        parsers::command::vec3,
        parsers::command::block_state,
        parsers::command::block_predicate,
        parsers::command::item_stack,
        parsers::command::item_predicate,
        parsers::command::color,
        parsers::command::hex_color,
        parsers::command::component,
        parsers::command::style,
        parsers::command::message,
        parsers::command::nbt_compound_tag,
        parsers::command::nbt_tag,
        parsers::command::nbt_path,
        parsers::command::objective,
        parsers::command::objective_criteria,
        parsers::command::operation,
        parsers::command::particle,
        parsers::command::angle,
        parsers::command::rotation,
        parsers::command::scoreboard_slot,
        parsers::command::score_holder,
        parsers::command::swizzle,
        parsers::command::team,
        parsers::command::item_slot,
        parsers::command::item_slots,
        parsers::command::resource_location,
        parsers::command::function,
        parsers::command::entity_anchor,
        parsers::command::int_range,
        parsers::command::float_range,
        parsers::command::dimension,
        parsers::command::gamemode,
        parsers::command::time,
        parsers::command::resource_or_tag,
        parsers::command::resource_or_tag_key,
        parsers::command::resource,
        parsers::command::resource_key,
        parsers::command::resource_selector,
        parsers::command::template_mirror,
        parsers::command::template_rotation,
        parsers::command::heightmap,
        parsers::command::loot_table,
        parsers::command::loot_predicate,
        parsers::command::loot_modifier,
        parsers::command::dialog,
        parsers::command::uuid>;
}

#endif /* SRC_BASE_OBJECTS_PARSERS */
