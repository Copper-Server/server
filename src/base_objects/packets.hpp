#ifndef SRC_BASE_OBJECTS_PACKETS
#define SRC_BASE_OBJECTS_PACKETS
#include <optional>
#include <src/base_objects/packets/enums.hpp>
#include <string>
#include <vector>

namespace copper_server::base_objects::packets {
    struct command_node {
        enum class parsers : uint32_t {
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
            std::string parser_names[] = {
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
            size_t index = static_cast<size_t>(parser);
            if (index >= (sizeof(parser_names) / sizeof(parser_names[0])))
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

        struct flag_t {
            node_type node_type : 2;
            bool is_executable : 1;
            bool has_redirect : 1;
            bool has_suggestion : 1;

            inline void set(uint8_t raw) {
                union u_t {
                    flag_t flag;
                    uint8_t r;
                } u{.r = raw};

                *this = u.flag;
            }

            inline uint8_t get() const {
                union u_t {
                    flag_t flag;
                    uint8_t r;
                } u{.flag = *this};

                return u.r;
            }
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

    int32_t java_name_to_protocol(const std::string& name_or_number);
    const char* protocol_to_java_name(int32_t id);
}
#endif /* SRC_BASE_OBJECTS_PACKETS */
