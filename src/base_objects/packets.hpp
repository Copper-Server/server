#ifndef SRC_BASE_OBJECTS_PACKETS
#define SRC_BASE_OBJECTS_PACKETS
#include "chat.hpp"
#include "position.hpp"
#include <vector>

namespace crafted_craft {
    namespace base_objects {
        namespace packets {
            struct command_suggestion {
                std::string suggestion;
                std::optional<Chat> tooltip;
            };

            struct statistics {
                int32_t category_id;
                int32_t statistic_id;
                int32_t value;
            };

            struct command_node {
                enum class parsers : int32_t {
                    brigadier_bool,
                    brigadier_float,
                    brigadier_double,
                    brigadier_integer,
                    brigadier_long,
                    brigadier_string,
                    brigadier_entity,
                    minecraft_game_profile,
                    minecraft_block_pos,
                    minecraft_column_pos,
                    minecraft_vec3,
                    minecraft_vec2,
                    minecraft_block_state,
                    minecraft_block_predicate,
                    minecraft_item_stack,
                    minecraft_item_predicate,
                    minecraft_color,
                    minecraft_component,
                    minecraft_style,
                    minecraft_message,
                    minecraft_nbt,
                    minecraft_nbt_tag,
                    minecraft_nbt_path,
                    minecraft_objective,
                    minecraft_objective_criteria,
                    minecraft_operation,
                    minecraft_particle,
                    minecraft_angle,
                    minecraft_rotation,
                    minecraft_scoreboard_slot,
                    minecraft_score_holder,
                    minecraft_swizzle,
                    minecraft_team,
                    minecraft_item_slot,
                    minecraft_resource_location,
                    minecraft_function,
                    minecraft_entity_anchor,
                    minecraft_int_range,
                    minecraft_float_range,
                    minecraft_dimension,
                    minecraft_gamemode,
                    minecraft_time,
                    minecraft_resource_or_tag,
                    minecraft_resource_or_tag_key,
                    minecraft_resource,
                    minecraft_resource_tag,
                    minecraft_template_mirror,
                    minecraft_template_rotation,
                    minecraft_heightmap,
                    minecraft_uuid,
                };
                enum class node_type : uint8_t {
                    root,
                    literal,
                    argument
                };

                struct properties {
                    std::optional<uint8_t> flags;
                    std::optional<std::variant<int32_t, float, double>> min;
                    std::optional<std::variant<int32_t, float, double>> max;
                    std::optional<std::string> registry;
                };

                union {
                    struct {
                        node_type node_type : 2;
                        bool is_executable : 1;
                        bool has_redirect : 1;
                        bool has_suggestion : 1;
                    };

                    uint8_t raw;
                } flags;

                list_array<int32_t> children;
                std::optional<int32_t> redirect_node;
                std::optional<std::string> name;
                std::optional<parsers> parser_id;
                std::optional<properties> properties;
                std::optional<std::string> suggestion_type;
            };

            struct slot_data {
                std::optional<ENBT> nbt;
                int32_t id;
                uint8_t count;
            };

            using slot = std::optional<slot_data>;

            struct death_location_data {
                std::string dimension;
                Position position;
            };

            struct map_icon {
                std::optional<Chat> display_name;
                int32_t type;
                int8_t x;
                int8_t z;
                int8_t direction;
            };

            struct trade {
                slot input_item1;
                slot output_item;
                slot input_item2;
                int32_t max_uses;
                int32_t uses;
                int32_t experience;
                int32_t special_price;
                float price_multiplier;
                int32_t demand;
                bool trade_disabled;
            };

            struct player_actions_add {
                struct property {
                    std::string name;
                    std::string value;
                    std::optional<std::string> signature;
                };

                ENBT::UUID player_id;
                std::string name;
                list_array<property> properties;
            };

            struct player_actions_initialize_chat {
                ENBT::UUID player_id;
                std::optional<ENBT::UUID> chat_session_id;
                int64_t public_key_expiry_time;
                //max 512 bytes
                list_array<uint8_t> public_key;
                //max 4096 bytes
                list_array<uint8_t> public_key_signature;
            };

            struct player_actions_update_gamemode {
                ENBT::UUID player_id;
                int32_t gamemode;
            };

            struct player_actions_update_listed {
                ENBT::UUID player_id;
                bool listed;
            };

            struct player_actions_update_latency {
                ENBT::UUID player_id;
                int32_t latency; //ms
            };

            struct player_actions_update_display_name {
                ENBT::UUID player_id;
                std::optional<Chat> display_name;
            };

            struct advancements_maping {
                struct advancement_display {
                    Chat title;
                    Chat description;
                    slot icon;
                    int32_t frame_type; //0: task, 1: challenge, 2: goal

                    enum flags_t : int32_t {
                        has_background_texture = 0x01,
                        show_toast = 0x02,
                        hidden = 0x04,
                    } flags;

                    std::optional<std::string> background_texture;
                    float x;
                    float y;
                };

                std::string key;
                std::optional<std::string> parent;
                std::optional<advancement_display> display;
                list_array<list_array<std::string>> requirements;
                bool sends_telemetry_data;
            };

            struct advancement_progress_item {
                struct criterion_progress {
                    bool achieved;
                    int64_t date;
                };

                std::string criterion;
                criterion_progress progress;
            };

            struct advancement_progress {
                std::string advancement;
                list_array<advancement_progress_item> criteria;
            };

            struct attributes {
                struct modifier {
                    ENBT::UUID uuid;
                    double amount;
                    int8_t operation; //0:addition/subtraction, 1:addition/subtraction by %, 2:multiplication by %
                };

                std::string key;
                double value;
                list_array<modifier> modifiers;
            };

            struct tag_mapping {
                struct entry {
                    std::string tag_name;
                    list_array<int32_t> entires;
                };

                std::string registry;
                list_array<entry> tags;
            };
        }
    }
}

#endif /* SRC_BASE_OBJECTS_PACKETS */
