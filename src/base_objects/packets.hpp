#ifndef SRC_BASE_OBJECTS_PACKETS
#define SRC_BASE_OBJECTS_PACKETS
#include "chat.hpp"
#include "position.hpp"
#include "shared_string.hpp"
#include "slot.hpp"
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
                    minecraft_entity,
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
                    minecraft_resource_key,
                    minecraft_template_mirror,
                    minecraft_template_rotation,
                    minecraft_heightmap,
                    minecraft_uuid,
                };

                static parsers to_parser(const std::string& parser) {
                    static std::unordered_map<std::string, parsers> parsers = {
                        {"brigadier_bool", parsers::brigadier_bool},
                        {"brigadier_float", parsers::brigadier_float},
                        {"brigadier_double", parsers::brigadier_double},
                        {"brigadier_integer", parsers::brigadier_integer},
                        {"brigadier_long", parsers::brigadier_long},
                        {"brigadier_string", parsers::brigadier_string},
                        {"minecraft_entity", parsers::minecraft_entity},
                        {"minecraft_game_profile", parsers::minecraft_game_profile},
                        {"minecraft_block_pos", parsers::minecraft_block_pos},
                        {"minecraft_column_pos", parsers::minecraft_column_pos},
                        {"minecraft_vec3", parsers::minecraft_vec3},
                        {"minecraft_vec2", parsers::minecraft_vec2},
                        {"minecraft_block_state", parsers::minecraft_block_state},
                        {"minecraft_block_predicate", parsers::minecraft_block_predicate},
                        {"minecraft_item_stack", parsers::minecraft_item_stack},
                        {"minecraft_item_predicate", parsers::minecraft_item_predicate},
                        {"minecraft_color", parsers::minecraft_color},
                        {"minecraft_component", parsers::minecraft_component},
                        {"minecraft_style", parsers::minecraft_style},
                        {"minecraft_message", parsers::minecraft_message},
                        {"minecraft_nbt", parsers::minecraft_nbt},
                        {"minecraft_nbt_tag", parsers::minecraft_nbt_tag},
                        {"minecraft_nbt_path", parsers::minecraft_nbt_path},
                        {"minecraft_objective", parsers::minecraft_objective},
                        {"minecraft_objective_criteria", parsers::minecraft_objective_criteria},
                        {"minecraft_operation", parsers::minecraft_operation},
                        {"minecraft_particle", parsers::minecraft_particle},
                        {"minecraft_angle", parsers::minecraft_angle},
                        {"minecraft_rotation", parsers::minecraft_rotation},
                        {"minecraft_scoreboard_slot", parsers::minecraft_scoreboard_slot},
                        {"minecraft_score_holder", parsers::minecraft_score_holder},
                        {"minecraft_swizzle", parsers::minecraft_swizzle},
                        {"minecraft_team", parsers::minecraft_team},
                        {"minecraft_item_slot", parsers::minecraft_item_slot},
                        {"minecraft_resource_location", parsers::minecraft_resource_location},
                        {"minecraft_function", parsers::minecraft_function},
                        {"minecraft_entity_anchor", parsers::minecraft_entity_anchor},
                        {"minecraft_int_range", parsers::minecraft_int_range},
                        {"minecraft_float_range", parsers::minecraft_float_range},
                        {"minecraft_dimension", parsers::minecraft_dimension},
                        {"minecraft_gamemode", parsers::minecraft_gamemode},
                        {"minecraft_time", parsers::minecraft_time},
                        {"minecraft_resource_or_tag", parsers::minecraft_resource_or_tag},
                        {"minecraft_resource_or_tag_key", parsers::minecraft_resource_or_tag_key},
                        {"minecraft_resource", parsers::minecraft_resource},
                        {"minecraft_resource_key", parsers::minecraft_resource_key},
                        {"minecraft_template_mirror", parsers::minecraft_template_mirror},
                        {"minecraft_template_rotation", parsers::minecraft_template_rotation},
                        {"minecraft_heightmap", parsers::minecraft_heightmap},
                        {"minecraft_uuid", parsers::minecraft_uuid},
                    };

                    return parsers.at(parser);
                }

                static std::string from_parser(parsers parser) {
                    std::string parser_names[] =
                        {
                            "brigadier_bool",
                            "brigadier_float",
                            "brigadier_double",
                            "brigadier_integer",
                            "brigadier_long",
                            "brigadier_string",
                            "minecraft_entity",
                            "minecraft_game_profile",
                            "minecraft_block_pos",
                            "minecraft_column_pos",
                            "minecraft_vec3",
                            "minecraft_vec2",
                            "minecraft_block_state",
                            "minecraft_block_predicate",
                            "minecraft_item_stack",
                            "minecraft_item_predicate",
                            "minecraft_color",
                            "minecraft_component",
                            "minecraft_style",
                            "minecraft_message",
                            "minecraft_nbt",
                            "minecraft_nbt_tag",
                            "minecraft_nbt_path",
                            "minecraft_objective",
                            "minecraft_objective_criteria",
                            "minecraft_operation",
                            "minecraft_particle",
                            "minecraft_angle",
                            "minecraft_rotation",
                            "minecraft_scoreboard_slot",
                            "minecraft_score_holder",
                            "minecraft_swizzle",
                            "minecraft_team",
                            "minecraft_item_slot",
                            "minecraft_resource_location",
                            "minecraft_function",
                            "minecraft_entity_anchor",
                            "minecraft_int_range",
                            "minecraft_float_range",
                            "minecraft_dimension",
                            "minecraft_gamemode",
                            "minecraft_time",
                            "minecraft_resource_or_tag",
                            "minecraft_resource_or_tag_key",
                            "minecraft_resource",
                            "minecraft_resource_key",
                            "minecraft_template_mirror",
                            "minecraft_template_rotation",
                            "minecraft_heightmap",
                            "minecraft_uuid",
                        };
                    int index = static_cast<int>(parser);
                    if (index < 0 || index >= sizeof(parser_names) / sizeof(parser_names[0]))
                        throw std::invalid_argument("invalid parser id");
                    return parser_names[index];
                }


                enum class node_type : uint8_t {
                    root,
                    literal,
                    argument
                };

                struct properties_t {
                    std::optional<uint8_t> flags;
                    std::optional<std::variant<int64_t, int32_t, float, double>> min;
                    std::optional<std::variant<int64_t, int32_t, float, double>> max;
                    std::optional<std::string> registry;
                };

                union {
                    struct {
                        node_type node_type : 2;
                        bool is_executable : 1;
                        bool has_redirect : 1;
                        bool has_suggestion : 1;
                    };

                    uint8_t raw = 0;
                } flags;

                list_array<int32_t> children;
                std::optional<int32_t> redirect_node;
                std::optional<std::string> name;
                std::optional<parsers> parser_id;
                std::optional<properties_t> properties;

                //minecraft:ask_server
                //minecraft:all_recipes
                //minecraft:available_sounds
                //minecraft:summonable_entities
                std::optional<std::string> suggestion_type;
            };


            struct death_location_data {
                shared_string dimension;
                Position position;
            };

            struct map_icon {
                std::optional<Chat> display_name;
                int32_t type;
                int8_t x;
                int8_t z;
                int8_t direction;
            };

            struct trade_item {
                int32_t id;
                int32_t count;
                //TODO
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

                    std::optional<shared_string> background_texture;
                    float x;
                    float y;
                };

                shared_string key;
                std::optional<shared_string> parent;
                std::optional<advancement_display> display;
                list_array<list_array<shared_string>> requirements;
                bool sends_telemetry_data;
            };

            struct advancement_progress_item {
                struct criterion_progress {
                    bool achieved;
                    int64_t date;
                };

                shared_string criterion;
                criterion_progress progress;
            };

            struct advancement_progress {
                shared_string advancement;
                list_array<advancement_progress_item> criteria;
            };

            struct attributes {
                struct modifier {
                    ENBT::UUID uuid;
                    double amount;
                    int8_t operation; //0:addition/subtraction, 1:addition/subtraction by %, 2:multiplication by %
                };

                int32_t key;
                double value;
                list_array<modifier> modifiers;

                constexpr static std::string key_to_string(int32_t key) {
                    switch (key) {
                    case 0:
                        return "generic.armor";
                    case 1:
                        return "generic.armor_toughness";
                    case 2:
                        return "generic.attack_damage";
                    case 3:
                        return "generic.attack_knockback";
                    case 4:
                        return "generic.attack_speed";
                    case 5:
                        return "generic.block_break_speed";
                    case 6:
                        return "generic.block_interaction_range";
                    case 7:
                        return "generic.entity_interaction_range";
                    case 8:
                        return "generic.fall_damage_multiplier";
                    case 9:
                        return "generic.flying_speed";
                    case 10:
                        return "generic.follow_range";
                    case 11:
                        return "generic.gravity";
                    case 12:
                        return "generic.jump_strength";
                    case 13:
                        return "generic.knockback_resistance";
                    case 14:
                        return "generic.luck";
                    case 15:
                        return "generic.max_absorption";
                    case 16:
                        return "generic.max_health";
                    case 17:
                        return "generic.movement_speed";
                    case 18:
                        return "generic.safe_fall_distance";
                    case 19:
                        return "generic.scale";
                    case 20:
                        return "generic.spawn_reinforcements";
                    case 21:
                        return "generic.step_height";
                    default:
                        return "UNDEFINED";
                    }
                }
            };

            struct tag_mapping {
                struct entry {
                    shared_string tag_name;
                    list_array<int32_t> entires;
                };

                shared_string registry;
                list_array<entry> tags;
            };

            struct known_pack {
                std::string namespace_;
                std::string id;
                std::string version;
            };

            struct server_link {
                enum class label_type {
                    bug_report,
                    community_guidelines,
                    support,
                    status,
                    feedback,
                    community,
                    website,
                    forums,
                    news,
                    announcements,
                };

                std::variant<label_type, Chat> label;
                std::string url;
            };
        }
    }
}

#endif /* SRC_BASE_OBJECTS_PACKETS */
