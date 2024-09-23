#ifndef SRC_BASE_OBJECTS_PREDICATES
#define SRC_BASE_OBJECTS_PREDICATES
#include "../library/enbt.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>

namespace crafted_craft {
    namespace base_objects {
        namespace predicates {
            namespace command {
                class custom_virtual_base {
                public:
                    virtual ~custom_virtual_base() {}
                    virtual std::string name() = 0;
                    virtual custom_virtual_base* make_copy() const = 0;
                };

                struct custom_virtual {
                    custom_virtual_base* value;

                    custom_virtual(custom_virtual_base* value)
                        : value(value) {}

                    ~custom_virtual() {
                        if (value)
                            delete value;
                    }

                    custom_virtual(custom_virtual&& other)
                        : value(other.value) {
                        other.value = nullptr;
                    }

                    custom_virtual(const custom_virtual& other)
                        : value(other.value->make_copy()) {}

                    custom_virtual& operator=(custom_virtual&& other) {
                        std::swap(value, other.value);
                        return *this;
                    }

                    custom_virtual& operator=(const custom_virtual& other) {
                        value = other.value->make_copy();
                        return *this;
                    }
                };

                struct _bool {};

                struct _double {
                    std::optional<double> min;
                    std::optional<double> max;
                };

                struct _float {
                    std::optional<float> min;
                    std::optional<float> max;
                };

                struct _integer {
                    std::optional<int32_t> min;
                    std::optional<int32_t> max;
                };

                struct _long {
                    std::optional<int64_t> min;
                    std::optional<int64_t> max;
                };

                enum class string : uint8_t {
                    single_word = 0,
                    quotable_phrase = 1,
                    greedy_phrase = 2,
                };

                struct angle {
                };

                struct block {
                };

                struct state {
                };

                struct color {
                };

                struct column_pos {
                };

                struct component {};

                struct dimension {};

                struct entity {
                    bool only_one_entity : 1;
                    bool only_player_entity : 1;
                };

                struct entity_anchor {
                };

                struct float_range {
                };

                struct function {};

                struct game_profile {
                };

                struct gamemode {
                };

                struct heightmap {
                };

                struct int_range {
                };

                struct item {
                };

                struct item_slot {
                };

                struct item_stack {
                };

                struct message {};

                struct nbt_compound_tag {};

                struct nbt_path {};

                struct nbt {};

                struct objective {};

                struct objective_criteria {};

                struct operation {
                };

                struct particle {
                };

                struct resource {
                    std::string suggestion_registry;
                };

                struct resource_key {
                    std::string suggestion_registry;
                };

                struct resource_location {};

                struct resource_or_tag {
                    std::string suggestion_registry;
                };

                struct resource_or_tag_key {
                    std::string suggestion_registry;
                };

                struct rotation {
                };

                struct score_holder {
                    bool allow_multiple;
                };

                struct scoreboard_slot {};

                struct swizzle {
                };

                struct team {
                };

                struct template_mirror {
                };

                struct template_rotation {
                };

                struct time {
                    int32_t min;
                };

                struct uuid {};

                struct vec2 {
                };

                struct vec3 {
                };
            };

            class custom_virtual_base {
            public:
                virtual ~custom_virtual_base() {}
                virtual std::string name() = 0;
            };

            struct custom_virtual {
                custom_virtual_base* value;

                custom_virtual(custom_virtual_base* value)
                    : value(value) {}

                ~custom_virtual() {
                    if (value)
                        delete value;
                }

                custom_virtual(custom_virtual&& other)
                    : value(other.value) {
                    other.value = nullptr;
                }
            };

            struct _bool {
                bool value;
            };

            struct _double {
                double value;
            };

            struct _float {
                float value;
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

            struct angle {
                float yaw;
                bool relative = false;
            };

            struct block {
                std::string block_id;
                std::unordered_map<std::string, std::string> states;
                enbt::value data_tags;
            };

            struct state {
                std::string block_id;
                std::unordered_map<std::string, std::string> states;
                //enbt::value data_tags;still parsed but ignored
            };

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

            struct column_pos {
                float x;
                float z;
                bool x_relative = false;
                bool z_relative = false;
            };

            struct component {
                std::string json_value;
            };

            struct dimension {
                std::string string;
            };

            struct entity {
                std::string value;

                enum class type_t : uint8_t {
                    name,
                    selector,
                    uuid,
                } type;
            };

            enum class entity_anchor : uint8_t {
                eyes,
                feet,
            };

            struct float_range {
                float begin;
                float end;
                bool without_begin;
                bool without_end;
            };

            struct function {
                std::string string;
            };

            struct game_profile {
                std::string string;

                enum class type_t : uint8_t {
                    name,
                    selector,
                    uuid,
                } type;
            };

            enum class gamemode : uint8_t {
                survival,
                creative,
                adventure,
                spectator,
            };

            enum class heightmap : uint8_t {
                world_surface,
                motion_blocking,
                motion_blocking_no_leaves,
                ocean_floor,
            };

            struct int_range {
                int32_t begin;
                int32_t end;
                bool without_begin;
                bool without_end;
            };

            struct item {
                std::string value;
                enbt::value data_tags;
            };

            struct item_slot {
                std::optional<std::string> container;
                std::optional<int32_t> number;
            };

            struct item_stack {
                std::string value;
                enbt::value data_tags;
            };

            struct message {
                std::string value; //can contain selector
            };

            struct nbt_compound_tag {
                enbt::value nbt;
            };

            struct nbt_path {
                std::string path;
            };

            struct nbt {
                enbt::value any_nbt;
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

            struct particle {
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

            struct resource {
                std::string location;
            };

            struct resource_key {
                std::string location;
            };

            struct resource_location {
                std::string location;
            };

            struct resource_or_tag {
                struct resource {
                    std::string value;
                };

                struct tag {
                    std::string value;
                };

                std::variant<resource, tag> value;
            };

            struct resource_or_tag_key {
                struct resource {
                    std::string value;
                };

                struct tag_key {
                    std::string value;
                };

                std::variant<resource, tag_key> value;
            };

            struct rotation {
                double yaw;
                double pitch;
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

            struct scoreboard_slot {
                std::string value;
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

            struct uuid {
                enbt::raw_uuid value;
            };

            struct vec2 {
                float v[2];
                bool relative[2]{false, false};
            };

            struct vec3 {
                float v[3];
                bool relative[3]{false, false, false};
            };
        }

        using predicate = std::variant<
            predicates::_bool,
            predicates::_double,
            predicates::_float,
            predicates::_integer,
            predicates::_long,
            predicates::string,
            predicates::angle,
            predicates::block,
            predicates::color,
            predicates::column_pos,
            predicates::component,
            predicates::dimension,
            predicates::entity,
            predicates::entity_anchor,
            predicates::heightmap,
            predicates::item,
            predicates::item_slot,
            predicates::item_stack,
            predicates::message,
            predicates::nbt_compound_tag,
            predicates::nbt_path,
            predicates::nbt,
            predicates::objective,
            predicates::objective_criteria,
            predicates::operation,
            predicates::particle,
            predicates::resource,
            predicates::resource_key,
            predicates::resource_location,
            predicates::resource_or_tag,
            predicates::rotation,
            predicates::score_holder,
            predicates::team,
            predicates::template_mirror,
            predicates::time,
            predicates::uuid,
            predicates::vec2,
            predicates::vec3,
            predicates::custom_virtual

            >;


        using command_predicate = std::variant<
            predicates::command::_bool,
            predicates::command::_double,
            predicates::command::_float,
            predicates::command::_integer,
            predicates::command::_long,
            predicates::command::string,
            predicates::command::angle,
            predicates::command::block,
            predicates::command::color,
            predicates::command::column_pos,
            predicates::command::component,
            predicates::command::dimension,
            predicates::command::entity,
            predicates::command::entity_anchor,
            predicates::command::heightmap,
            predicates::command::item,
            predicates::command::item_slot,
            predicates::command::item_stack,
            predicates::command::message,
            predicates::command::nbt_compound_tag,
            predicates::command::nbt_path,
            predicates::command::nbt,
            predicates::command::objective,
            predicates::command::objective_criteria,
            predicates::command::operation,
            predicates::command::particle,
            predicates::command::resource,
            predicates::command::resource_key,
            predicates::command::resource_location,
            predicates::command::resource_or_tag,
            predicates::command::rotation,
            predicates::command::score_holder,
            predicates::command::team,
            predicates::command::template_mirror,
            predicates::command::time,
            predicates::command::uuid,
            predicates::command::vec2,
            predicates::command::vec3,
            predicates::command::custom_virtual>;
    }
}

#endif /* SRC_BASE_OBJECTS_PREDICATES */
