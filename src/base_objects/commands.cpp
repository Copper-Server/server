#include <library/enbt/senbt.hpp>
#include <library/list_array.hpp>
#include <src/api/permissions.hpp>
#include <src/base_objects/block.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/packets.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>
#include <src/util/calculations.hpp>
#include <src/util/conversions.hpp>

namespace copper_server::base_objects {
    void command_context::apply_executor_data() {
        if (!executor->player_data.assigned_entity)
            return;
        auto pos = executor->player_data.assigned_entity->position;
        auto rot = util::to_yaw_pitch_256(executor->player_data.assigned_entity->rotation);
        auto mot = executor->player_data.assigned_entity->motion;

        other_data["x"] = pos.x;
        other_data["y"] = pos.y;
        other_data["z"] = pos.z;

        other_data["yaw"] = rot.x;
        other_data["pitch"] = rot.y;

        other_data["motion_x"] = mot.x;
        other_data["motion_y"] = mot.y;
        other_data["motion_y"] = mot.y;
    }

    void next_token(std::string& part, std::string& path) {
        auto split = path.find(' ');
        if (split == std::string::npos)
            std::swap(part, path);
        else {
            part = path.substr(0, split);
            path = path.substr(split + 1);
        }
    }

    namespace pred {
        std::optional<parsers::angle> parse(command_manager& manager, parsers::command::angle& cfg, std::string& part, std::string& path) {
            parsers::angle res;
            if (part.starts_with('~')) {
                res.relative = true;
                res.yaw = part.empty() ? 0 : std::stof(part.substr(1));
            } else if (part.size())
                res.yaw = std::stof(part.substr(1));
            else
                return std::nullopt;

            return res;
        }

        std::optional<parsers::block> parse(command_manager& manager, parsers::command::block& cfg, std::string& part, std::string& path) {
            parsers::block res;
            auto id = part;
            next_token(part, path);
            try {
                auto& block_data = base_objects::block::get_block(id);
                if (part.starts_with('[')) {
                    //TODO
                }
                if (part.starts_with('{')) {
                    part += part;
                    std::string_view view = part;
                    auto enbt = senbt::parse_mod(view);
                    auto comp = enbt.as_compound();

                    //TODO
                }
                path = part + ' ' + path;
            } catch (const std::out_of_range&) {
                throw std::runtime_error("The block " + part + "not found");
            }
            return res;
        }

        std::optional<parsers::color> parse(command_manager& manager, parsers::command::color& cfg, std::string& part, std::string& path) {
            if (part == "white")
                return parsers::color::white;
            else if (part == "orange")
                return parsers::color::orange;
            else if (part == "magenta")
                return parsers::color::magenta;
            else if (part == "light_blue")
                return parsers::color::light_blue;
            else if (part == "yellow")
                return parsers::color::yellow;
            else if (part == "lime")
                return parsers::color::lime;
            else if (part == "pink")
                return parsers::color::pink;
            else if (part == "gray")
                return parsers::color::gray;
            else if (part == "light_gray")
                return parsers::color::light_gray;
            else if (part == "cyan")
                return parsers::color::cyan;
            else if (part == "purple")
                return parsers::color::purple;
            else if (part == "blue")
                return parsers::color::blue;
            else if (part == "brown")
                return parsers::color::brown;
            else if (part == "green")
                return parsers::color::green;
            else if (part == "red")
                return parsers::color::red;
            else if (part == "black")
                return parsers::color::black;
            else
                return std::nullopt;
        }

        std::optional<parsers::block_pos> parse(command_manager& manager, parsers::command::block_pos& cfg, std::string& part, std::string& path) {
            parsers::block_pos res;
            if (part.starts_with('~')) {
                res.x_relative = true;
                res.x = part.empty() ? 0 : std::stof(part.substr(1));
            } else if (part.size())
                res.x = std::stof(part);
            else
                return std::nullopt;

            next_token(part, path);
            if (part.starts_with('~')) {
                res.y_relative = true;
                res.y = part.empty() ? 0 : std::stof(part.substr(1));
            } else if (part.size())
                res.y = std::stof(part);
            else
                return std::nullopt;
            next_token(part, path);
            if (part.starts_with('~')) {
                res.z_relative = true;
                res.z = part.empty() ? 0 : std::stof(part.substr(1));
            } else if (part.size())
                res.z = std::stof(part);
            else
                return std::nullopt;
            return res;
        }

        std::optional<parsers::column_pos> parse(command_manager& manager, parsers::command::column_pos& cfg, std::string& part, std::string& path) {
            parsers::column_pos res;
            if (part.starts_with('~')) {
                res.x_relative = true;
                res.x = part.empty() ? 0 : std::stof(part.substr(1));
            } else if (part.size())
                res.x = std::stof(part);
            else
                return std::nullopt;

            next_token(part, path);
            if (part.starts_with('~')) {
                res.z_relative = true;
                res.z = part.empty() ? 0 : std::stof(part.substr(1));
            } else if (part.size())
                res.z = std::stof(part);
            else
                return std::nullopt;
            return res;
        }

        std::optional<parsers::component> parse(command_manager& manager, parsers::command::component& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::dimension> parse(command_manager& manager, parsers::command::dimension& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::entity> parse(command_manager& manager, parsers::command::entity& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::entity_anchor> parse(command_manager& manager, parsers::command::entity_anchor& cfg, std::string& part, std::string& path) {
            if (part == "eyes")
                return parsers::entity_anchor::eyes;
            else if (part == "feet")
                return parsers::entity_anchor::feet;
            else
                return std::nullopt;
        }

        std::optional<parsers::float_range> parse(command_manager& manager, parsers::command::float_range& cfg, std::string& part, std::string& path) {
            size_t pos = part.find("..");
            if (pos == std::string::npos)
                return std::nullopt;
            parsers::float_range res;

            std::string begin = part.substr(0, pos);
            std::string end = part.substr(pos + 1);
            if (begin.empty())
                res.without_begin = true;
            else
                res.begin = std::stof(begin);
            if (end.empty())
                res.without_end = true;
            else
                res.end = std::stof(end);
            return res;
        }

        std::optional<parsers::function> parse(command_manager& manager, parsers::command::function& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::game_profile> parse(command_manager& manager, parsers::command::game_profile& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::gamemode> parse(command_manager& manager, parsers::command::gamemode& cfg, std::string& part, std::string& path) {
            if (part == "survival")
                return parsers::gamemode::survival;
            else if (part == "creative")
                return parsers::gamemode::creative;
            else if (part == "adventure")
                return parsers::gamemode::adventure;
            else if (part == "spectator")
                return parsers::gamemode::spectator;
            else
                return std::nullopt;
        }

        std::optional<parsers::heightmap> parse(command_manager& manager, parsers::command::heightmap& cfg, std::string& part, std::string& path) {
            if (part == "motion_blocking")
                return parsers::heightmap::motion_blocking;
            else if (part == "motion_blocking_no_leaves")
                return parsers::heightmap::motion_blocking_no_leaves;
            else if (part == "ocean_floor")
                return parsers::heightmap::ocean_floor;
            else if (part == "world_surface")
                return parsers::heightmap::world_surface;
            else
                return std::nullopt;
        }

        std::optional<parsers::int_range> parse(command_manager& manager, parsers::command::int_range& cfg, std::string& part, std::string& path) {
            size_t pos = part.find("..");
            if (pos == std::string::npos)
                return std::nullopt;
            parsers::int_range res;

            std::string begin = part.substr(0, pos);
            std::string end = part.substr(pos + 1);
            if (begin.empty())
                res.without_begin = true;
            else
                res.begin = std::stoi(begin);
            if (end.empty())
                res.without_end = true;
            else
                res.end = std::stoi(end);
            return res;
        }

        std::optional<parsers::item> parse(command_manager& manager, parsers::command::item& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::item_slot> parse(command_manager& manager, parsers::command::item_slot& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::item_stack> parse(command_manager& manager, parsers::command::item_stack& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::message> parse(command_manager& manager, parsers::command::message& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::nbt> parse(command_manager& manager, parsers::command::nbt& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::nbt_compound_tag> parse(command_manager& manager, parsers::command::nbt_compound_tag& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::nbt_path> parse(command_manager& manager, parsers::command::nbt_path& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::objective> parse(command_manager& manager, parsers::command::objective& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::objective_criteria> parse(command_manager& manager, parsers::command::objective_criteria& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::operation> parse(command_manager& manager, parsers::command::operation& cfg, std::string& part, std::string& path) {
            if (part == "=")
                return parsers::operation::assignment;
            else if (part == "+=")
                return parsers::operation::addition;
            else if (part == "-=")
                return parsers::operation::subtraction;
            else if (part == "*=")
                return parsers::operation::multiplication;
            else if (part == "/=")
                return parsers::operation::floor_division;
            else if (part == "%=")
                return parsers::operation::modulus;
            else if (part == "><")
                return parsers::operation::swapping;
            else if (part == "<")
                return parsers::operation::minimum;
            else if (part == ">")
                return parsers::operation::maximum;
            else
                return std::nullopt;
        }

        std::optional<parsers::particle> parse(command_manager& manager, parsers::command::particle& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::resource> parse(command_manager& manager, parsers::command::resource& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::resource_key> parse(command_manager& manager, parsers::command::resource_key& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::resource_location> parse(command_manager& manager, parsers::command::resource_location& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::resource_or_tag> parse(command_manager& manager, parsers::command::resource_or_tag& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::resource_or_tag_key> parse(command_manager& manager, parsers::command::resource_or_tag_key& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::rotation> parse(command_manager& manager, parsers::command::rotation& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::score_holder> parse(command_manager& manager, parsers::command::score_holder& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::scoreboard_slot> parse(command_manager& manager, parsers::command::scoreboard_slot& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::state> parse(command_manager& manager, parsers::command::state& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::string> parse(command_manager& manager, parsers::command::string& cfg, std::string& part, std::string& path) {
            switch (cfg) {
            case parsers::command::string::single_word:
                return parsers::string(part);
            case parsers::command::string::quotable_phrase:
                if (part[0] == '"') {
                    std::string phrase = part + ' ' + path;
                    auto pos = util::conversions::string::direct_find(phrase, '"');
                    if (pos == std::string::npos)
                        throw std::invalid_argument("invalid quoted string");
                    part = phrase.substr(1, pos - 1);
                    bool has_space = phrase.find(' ', pos + 1) != std::string::npos;
                    path = phrase.substr(pos + 1 + has_space);
                }
                return parsers::string(part);

            case parsers::command::string::greedy_phrase: {
                auto phrase = part + ' ' + path;
                path.clear();
                return parsers::string(phrase);
            }
            default:
                return std::nullopt;
            }
        }

        std::optional<parsers::swizzle> parse(command_manager& manager, parsers::command::swizzle& cfg, std::string& part, std::string& path) {
            parsers::swizzle result;
            parsers::swizzle::coord current = parsers::swizzle::coord::undefined;
            bool v0_set = false;
            bool v1_set = false;
            bool v2_set = false;
            for (char ch : part) {
                switch (ch) {
                case 'x':
                    current = parsers::swizzle::coord::x;
                    break;
                case 'y':
                    current = parsers::swizzle::coord::y;
                    break;
                case 'z':
                    current = parsers::swizzle::coord::z;
                    break;
                default:
                    return std::nullopt;
                }

                if (!v0_set) {
                    result.v0 = current;
                    v0_set = true;
                } else if (!v1_set) {
                    if (result.v0 == current)
                        return std::nullopt;
                    result.v1 = current;
                    v1_set = true;
                } else if (!v2_set) {
                    if (result.v0 == current || result.v1 == current)
                        return std::nullopt;
                    result.v2 = current;
                    v2_set = true;
                } else
                    return std::nullopt;
            }
            return result;
        }

        std::optional<parsers::team> parse(command_manager& manager, parsers::command::team& cfg, std::string& part, std::string& path) {
            return {}; //TODO
        }

        std::optional<parsers::template_mirror> parse(command_manager& manager, parsers::command::template_mirror& cfg, std::string& part, std::string& path) {
            if (part == "none")
                return parsers::template_mirror::none;
            else if (part == "front_back")
                return parsers::template_mirror::front_back;
            else if (part == "left_right")
                return parsers::template_mirror::left_right;
            else
                return std::nullopt;
        }

        std::optional<parsers::template_rotation> parse(command_manager& manager, parsers::command::template_rotation& cfg, std::string& part, std::string& path) {
            if (part == "none")
                return parsers::template_rotation::none;
            else if (part == "clockwise_90")
                return parsers::template_rotation::clockwise_90;
            else if (part == "counterclockwise_90")
                return parsers::template_rotation::counterclockwise_90;
            else if (part == "180")
                return parsers::template_rotation::_180;
            else
                return std::nullopt;
        }

        std::optional<parsers::time> parse(command_manager& manager, parsers::command::time& cfg, std::string& part, std::string& path) {
            int64_t value = 0;
            try {
                if (part.ends_with('d')) {
                    value = int64_t(std::stod(part.substr(0, part.size() - 2)) * 24000);
                } else if (part.ends_with('s')) {
                    value = int64_t(std::stod(part.substr(0, part.size() - 2)) * 20);
                } else if (part.ends_with('t')) {
                    value = std::stoll(part.substr(0, part.size() - 2));
                } else
                    value = std::stoll(part);
            } catch (const std::invalid_argument&) {
                return std::nullopt;
            } catch (const std::out_of_range&) {
                return std::nullopt;
            }
            if (value < cfg.min)
                return std::nullopt;
            return parsers::time{value};
        }

        std::optional<parsers::uuid> parse(command_manager& manager, parsers::command::uuid& cfg, std::string& part, std::string& path) {
            return parsers::uuid{util::conversions::uuid::from(part)};
        }

        std::optional<parsers::vec2> parse(command_manager& manager, parsers::command::vec2& cfg, std::string& part, std::string& path) {
            parsers::vec2 res;
            if (part.starts_with('~')) {
                res.relative[0] = true;
                res.v[0] = part.empty() ? 0 : std::stof(part.substr(1));
            } else if (part.size())
                res.v[0] = std::stof(part);
            else
                return std::nullopt;

            next_token(part, path);
            if (part.starts_with('~')) {
                res.relative[1] = true;
                res.v[1] = part.empty() ? 0 : std::stof(part.substr(1));
            } else if (part.size())
                res.v[1] = std::stof(part);
            else
                return std::nullopt;
            return res;
        }

        std::optional<parsers::vec3> parse(command_manager& manager, parsers::command::vec3& cfg, std::string& part, std::string& path) {
            parsers::vec3 res;
            if (part.starts_with('~')) {
                res.relative[0] = true;
                res.v[0] = part.empty() ? 0 : std::stof(part.substr(1));
            } else if (part.size())
                res.v[0] = std::stof(part);
            else
                return std::nullopt;

            next_token(part, path);
            if (part.starts_with('~')) {
                res.relative[1] = true;
                res.v[1] = part.empty() ? 0 : std::stof(part.substr(1));
            } else if (part.size())
                res.v[1] = std::stof(part);
            else
                return std::nullopt;
            next_token(part, path);
            if (part.starts_with('~')) {
                res.relative[2] = true;
                res.v[2] = part.empty() ? 0 : std::stof(part.substr(1));
            } else if (part.size())
                res.v[2] = std::stof(part);
            else
                return std::nullopt;

            return res;
        }

        std::optional<parsers::_bool> parse(command_manager& manager, parsers::command::_bool& cfg, std::string& part, std::string& path) {
            if (part == "true" || part == "false")
                return parsers::_bool{part == "true"};
            else
                return std::nullopt;
        }

        std::optional<parsers::_double> parse(command_manager& manager, parsers::command::_double& cfg, std::string& part, std::string& path) {
            try {
                double value = std::stod(part);
                if (cfg.min)
                    if (value < *cfg.min)
                        return std::nullopt;
                if (cfg.max)
                    if (value > *cfg.max)
                        return std::nullopt;
                return parsers::_double{value};
            }

            catch (...) {
                return std::nullopt;
            }
        }

        std::optional<parsers::_float> parse(command_manager& manager, parsers::command::_float& cfg, std::string& part, std::string& path) {
            try {
                float value = std::stof(part);
                if (cfg.min)
                    if (value < *cfg.min)
                        return std::nullopt;
                if (cfg.max)
                    if (value > *cfg.max)
                        return std::nullopt;
                return parsers::_float{value};
            } catch (...) {
                return std::nullopt;
            }
        }

        std::optional<parsers::_integer> parse(command_manager& manager, parsers::command::_integer& cfg, std::string& part, std::string& path) {
            try {
                int32_t value = std::stoi(part);
                if (cfg.min)
                    if (value < *cfg.min)
                        return std::nullopt;
                if (cfg.max)
                    if (value > *cfg.max)
                        return std::nullopt;
                return parsers::_integer{value};
            } catch (...) {
                return std::nullopt;
            }
        }

        std::optional<parsers::_long> parse(command_manager& manager, parsers::command::_long& cfg, std::string& part, std::string& path) {
            try {
                int32_t value = std::stoll(part);
                if (cfg.min)
                    if (value < *cfg.min)
                        return std::nullopt;
                if (cfg.max)
                    if (value > *cfg.max)
                        return std::nullopt;
                return parsers::_long{value};
            } catch (...) {
                return std::nullopt;
            }
        }

        std::optional<parsers::custom_virtual> parse(command_manager& manager, parsers::command::custom_virtual& cfg, std::string& part, std::string& path) {
            return manager.get_parser(cfg.value->name()).parse(cfg, part, path);
        }
    }

    action_provider::action_provider(const char* tag)
        : action_tag(tag) {}

    action_provider::action_provider(const std::string& tag)
        : action_tag(tag) {}

    action_provider::action_provider(std::string&& tag)
        : action_tag(std::move(tag)) {}

    action_provider::action_provider(const std::string& tag, const list_array<std::string>& requirement)
        : action_tag(tag), required_permissions_tag(requirement) {}

    action_provider::action_provider(std::string&& tag, const list_array<std::string>& requirement)
        : action_tag(std::move(tag)), required_permissions_tag(requirement) {}

    action_provider::action_provider(const std::string& tag, list_array<std::string>&& requirement)
        : action_tag(tag), required_permissions_tag(std::move(requirement)) {}

    action_provider::action_provider(std::string&& tag, list_array<std::string>&& requirement)
        : action_tag(std::move(tag)), required_permissions_tag(std::move(requirement)) {}

    int32_t command::get_child(list_array<command>& command_nodes, const std::string& name) {
        if (childs_cache.size() == childs.size()) {
            auto it = childs_cache.find(name);
            if (it == childs_cache.end())
                return -1;
            if (command_nodes[it->second].name != name) {
                childs_cache.clear();
                return get_child(command_nodes, name);
            } else
                return it->second;
        } else {
            childs_cache.clear();
            for (size_t i = 0; i < childs.size(); i++) {
                auto& check_name = command_nodes[childs[i]].name;
                if (childs_cache.find(check_name) != childs_cache.end())
                    throw std::invalid_argument("invalid command children tree, duplicate name found");
                childs_cache[check_name] = i;
            }
            return get_child(command_nodes, name);
        }
    }

    int32_t get_index(list_array<command>& command_nodes, const std::string& path, int32_t current_ = 0) {
        if (current_ < 0)
            throw std::invalid_argument("invalid command id");
        auto real_path = list_array<char>(path)
                             .split_by(' ')
                             .convert<std::string>([](const list_array<char>& a) {
                                 return std::string(a.data(), a.size());
                             });

        if (real_path.size() == 0) {
            return current_;
        } else {
            int32_t current = current_;
            for (auto& part : real_path) {
                current = command_nodes[current].get_child(command_nodes, part);
                if (current == -1)
                    throw std::invalid_argument("invalid command path");
            }
            return current;
        }
    }

    void apply_options(packets::command_node::parsers parser, packets::command_node::properties_t& parser_data) {
        switch (parser) {
        case packets::command_node::parsers::brigadier_float:
        case packets::command_node::parsers::brigadier_double:
        case packets::command_node::parsers::brigadier_integer:
        case packets::command_node::parsers::brigadier_long: {
            parser_data.flags = 0;
            if (parser_data.min)
                *parser_data.flags |= 1;
            if (parser_data.max)
                *parser_data.flags |= 2;
            parser_data.registry = std::nullopt;
            break;
        }
        case packets::command_node::parsers::brigadier_string: {
            if (!parser_data.flags)
                parser_data.flags = 0;
            parser_data.min = std::nullopt;
            parser_data.max = std::nullopt;
            parser_data.registry = std::nullopt;
            break;
        }
        case packets::command_node::parsers::minecraft_entity: {
            if (!parser_data.flags)
                parser_data.flags = 0;
            parser_data.min = std::nullopt;
            parser_data.max = std::nullopt;
            parser_data.registry = std::nullopt;
            break;
        }
        case packets::command_node::parsers::minecraft_score_holder: {
            if (!parser_data.flags)
                parser_data.flags = 0;
            parser_data.min = std::nullopt;
            parser_data.max = std::nullopt;
            parser_data.registry = std::nullopt;
            break;
        }
        case packets::command_node::parsers::minecraft_time: {
            if (!parser_data.min)
                parser_data.min = std::numeric_limits<int32_t>::max();
            parser_data.flags = std::nullopt;
            parser_data.max = std::nullopt;
            parser_data.registry = std::nullopt;
            break;
        }
        case packets::command_node::parsers::minecraft_resource_or_tag:
        case packets::command_node::parsers::minecraft_resource_or_tag_key:
        case packets::command_node::parsers::minecraft_resource:
        case packets::command_node::parsers::minecraft_resource_key: {
            if (!parser_data.registry)
                throw std::invalid_argument("this parser should have registry");
            parser_data.flags = std::nullopt;
            parser_data.min = std::nullopt;
            parser_data.max = std::nullopt;
            break;
        }
        default:
            parser_data.flags = std::nullopt;
            parser_data.min = std::nullopt;
            parser_data.max = std::nullopt;
            parser_data.registry = std::nullopt;
            break;
        }
    }

    int32_t get_index(const list_array<command>& command_nodes, const std::string& path, int32_t current_ = 0) {
        return get_index(const_cast<list_array<command>&>(command_nodes), path, current_);
    }

    std::optional<parser> find_argument_no_except(command_manager& mngr, list_array<command>& command_nodes, command*& command, std::string& string, std::string& rest) {
        for (auto child_id : command->childs) {
            auto& child = command_nodes[child_id];
            if (child.argument_predicate) {
                auto res = std::visit(
                    [&](auto& predicate_cfg) -> std::optional<parser> {
                        auto res = pred::parse(mngr, predicate_cfg, string, rest);
                        if (res) {
                            command = &child;
                            return std::move(*res);
                        } else
                            return std::nullopt;
                    },
                    *child.argument_predicate
                );
                if (res)
                    return std::move(*res);
            }
        }
        return std::nullopt;
    }

    parser find_argument(command_manager& mngr, list_array<command>& command_nodes, command*& command, std::string& string, std::string& rest) {
        auto res = find_argument_no_except(mngr, command_nodes, command, string, rest);
        if (res)
            return std::move(*res);
        else
            throw std::invalid_argument("invalid argument");
    }

    void command_manager::remove(size_t id) {
        if (id >= command_nodes.size())
            return;
        command_nodes.erase(id);
        for (command& cmd : command_nodes) {
            bool make_erase = false;
            bool update_cache = false;
            for (auto& child : cmd.childs) {
                if (child == id)
                    make_erase = true;
                else if (child > id) {
                    --child;
                    update_cache = true;
                }
            }
            if (make_erase) {
                cmd.childs.erase(id);
                auto begin = cmd.childs_cache.begin();
                auto end = cmd.childs_cache.end();
                for (; begin != end; begin++)
                    if (begin->second == id) {
                        cmd.childs_cache.erase(begin);
                        break;
                    }
            }
            if (update_cache) {
                for (auto& [name, child_id] : cmd.childs_cache)
                    if (child_id > id)
                        --child_id;
            }
        }
    }

    command_manager::command_manager() {
        command_nodes.push_back({});
        graph_ready = false;
    }

    void command_manager::register_named_suggestion_provider(const std::string& name, const named_suggestion_provider& provider) {
        if (named_suggestion_providers.contains(name))
            return;
        else
            named_suggestion_providers[name] = provider;
    }

    void command_manager::remove_named_suggestion_provider(const std::string& name) {
        if (name == "minecraft:ask_server")
            return;
        else
            named_suggestion_providers.erase(name);
    }

    void command_manager::register_parser(const std::shared_ptr<command_custom_parser>& parser) {
        if (parser)
            custom_parsers[parser->name()] = parser;
    }

    command_custom_parser& command_manager::get_parser(const std::string& name) {
        auto it = custom_parsers.find(name);
        if (it == custom_parsers.end())
            throw std::runtime_error("Parser not found");
        return *it->second;
    }

    void command_manager::unregister_parser(const std::string& name) {
        custom_parsers.erase(name);
    }

    void command_manager::execute_command(const std::string& command_string, command_context& data) {
        execute_command_from(command_string, command_nodes[0], data);
    }

    void command_manager::execute_command_from(const std::string& command_string, command& cmd, command_context& data) {
        if (!belongs(&cmd))
            return;
        std::string path = command_string;
        list_array<parser> args;
        command* current = &cmd;


        while (path.size() > 0 && !current->redirect) {
            try {
                auto split = path.find(' ');
                std::string part;
                if (split == std::string::npos) {
                    std::swap(part, path);
                } else {
                    part = path.substr(0, split);
                    path = path.substr(split + 1);
                }

                if (part.empty())
                    break;

                int32_t child = current->get_child(command_nodes, part);
                if (child == -1)
                    args.push_back(find_argument(*this, command_nodes, current, part, path));
                else {
                    auto& command = command_nodes[child];
                    if (command.argument_predicate)
                        args.push_back(find_argument(*this, command_nodes, current, part, path));
                    else
                        current = &command_nodes[child];
                }
            } catch (...) {
                throw command_exception(std::current_exception(), command_string.size() - path.size());
            }
        }
        if (current->redirect) {
            auto routine = current->redirect;
            return (routine->redirect_routine)(command_nodes[routine->target_command], args, path, data);
        } else {
            if (current->executable) {
                if (api::permissions::has_rights(current->action_name, data.executor))
                    return (*current->executable)(args, data);
                else
                    throw std::exception("Not enough permissions for this command.");
            } else {
                if (current->childs.size() == 1) {
                    auto& usage = command_nodes[current->childs[0]].usage;
                    if (!usage.empty())
                        throw std::invalid_argument("Incomplete command, usage: " + usage);
                }
                if (!current->usage.empty())
                    throw std::invalid_argument("Incomplete command, usage: " + current->usage);
                throw std::invalid_argument("Invalid command, command is not executable");
            }
        }
    }

    bool has_accessible_callbacks_child(command* current, list_array<command>& command_nodes, command_context& data) {
        return current->childs.contains_one([&](int32_t id) {
            auto& command = command_nodes[id];
            if (command.executable)
                return api::permissions::has_rights(command.action_name, data.executor);
            else
                return has_accessible_callbacks_child(&command, command_nodes, data);
        });
    }

    list_array<std::string> extract_suggestions(command* current, std::unordered_map<std::string, named_suggestion_provider>& named_suggestion_providers, list_array<command>& command_nodes, const std::string& part, command_context& data) {
        list_array<std::string> suggestions;
        current->childs.for_each([&](int32_t id) {
            auto& command = command_nodes[id];
            if (command.childs.size())
                if (!has_accessible_callbacks_child(&command, command_nodes, data))
                    return;
            if (command.executable)
                if (!api::permissions::has_rights(command.action_name, data.executor))
                    return;

            if (command.name.starts_with(part)) {
                std::visit(
                    [&](auto& it) {
                        using data_T = std::decay_t<decltype(it)>;
                        if constexpr (std::is_same_v<data_T, command_suggestion>) {
                            if (it)
                                suggestions.push_back(it(part, data));
                            else
                                suggestions.push_back(command.name);
                        } else {
                            auto provider = named_suggestion_providers.find(it);
                            if (provider == named_suggestion_providers.end())
                                suggestions.push_back(command.name);
                            else
                                suggestions.push_back(provider->second(command, provider->first, part, data));
                        }
                    },
                    command.suggestions
                );
            }
        });
        return suggestions;
    }

    list_array<std::string> command_manager::request_suggestions(const std::string& command_string, command_context& data) {
        bool ends_with_space = command_string.ends_with(' ');
        std::string path = command_string;
        list_array<parser> args;
        command* current = &command_nodes[0];

        while (path.size() > 0 || !current->redirect) {
            auto split = path.find(' ');
            std::string part;
            if (split == std::string::npos) {
                std::swap(part, path);
            } else {
                part = path.substr(0, split);
                path = path.substr(split + 1);
            }

            try {
                if (part.empty())
                    return extract_suggestions(current, named_suggestion_providers, command_nodes, part, data);

                int32_t child = current->get_child(command_nodes, part);
                if (child == -1) {
                    auto res = find_argument_no_except(*this, command_nodes, current, part, path);
                    if (res)
                        args.push_back(std::move(*res));
                    else
                        return extract_suggestions(current, named_suggestion_providers, command_nodes, part, data);
                } else {
                    auto& command = command_nodes[child];
                    if (command.argument_predicate) {
                        auto res = find_argument_no_except(*this, command_nodes, current, part, path);
                        if (res)
                            args.push_back(std::move(*res));
                        else
                            return extract_suggestions(current, named_suggestion_providers, command_nodes, part, data);
                    } else {
                        current = &command_nodes[child];
                        if (path.empty() && !ends_with_space)
                            return extract_suggestions(current, named_suggestion_providers, command_nodes, path, data).transform([&](std::string&& s) {
                                s = part + ' ' + s;
                                return s;
                            });
                    }
                    if (!current->redirect)
                        continue;
                }
            } catch (...) {
                return {};
            }
        }
        return {};
    }

    void build_parser(packets::command_node& node, const command_parser& command) {
        std::visit(
            [&](auto& it) {
                using T = std::decay_t<decltype(it)>;
                if constexpr (std::is_same_v<T, parsers::command::_bool>) {
                    node.parser_id = packets::command_node::parsers::brigadier_bool;
                } else if constexpr (
                    std::is_same_v<T, parsers::command::_double>
                    || std::is_same_v<T, parsers::command::_float>
                    || std::is_same_v<T, parsers::command::_integer>
                    || std::is_same_v<T, parsers::command::_long>
                ) {
                    node.parser_id = packets::command_node::parsers::brigadier_long;
                    packets::command_node::properties_t prop;
                    uint8_t flags = 0;
                    if (it.min) {
                        prop.min = *it.min;
                        flags &= 1;
                    }
                    if (it.max) {
                        prop.min = *it.max;
                        flags &= 2;
                    }
                    prop.flags = flags;
                    node.properties = prop;
                } else if constexpr (std::is_same_v<T, parsers::command::string>) {
                    node.parser_id = packets::command_node::parsers::brigadier_string;
                    node.properties = {.flags = (int8_t)it};
                } else if constexpr (std::is_same_v<T, parsers::command::angle>) {
                    node.parser_id = packets::command_node::parsers::minecraft_angle;
                } else if constexpr (std::is_same_v<T, parsers::command::block>) {
                    node.parser_id = packets::command_node::parsers::minecraft_block_predicate;
                } else if constexpr (std::is_same_v<T, parsers::command::color>) {
                    node.parser_id = packets::command_node::parsers::minecraft_color;
                } else if constexpr (std::is_same_v<T, parsers::command::column_pos>) {
                    node.parser_id = packets::command_node::parsers::minecraft_column_pos;
                } else if constexpr (std::is_same_v<T, parsers::command::component>) {
                    node.parser_id = packets::command_node::parsers::minecraft_component;
                } else if constexpr (std::is_same_v<T, parsers::command::dimension>) {
                    node.parser_id = packets::command_node::parsers::minecraft_dimension;
                } else if constexpr (std::is_same_v<T, parsers::command::entity>) {
                    node.parser_id = packets::command_node::parsers::minecraft_entity;
                    node.properties = {.flags = (int(it.only_one_entity) | (int(it.only_player_entity) << 1))};
                } else if constexpr (std::is_same_v<T, parsers::command::entity_anchor>) {
                    node.parser_id = packets::command_node::parsers::minecraft_entity_anchor;
                } else if constexpr (std::is_same_v<T, parsers::command::heightmap>) {
                    node.parser_id = packets::command_node::parsers::minecraft_heightmap;
                } else if constexpr (std::is_same_v<T, parsers::command::item>) {
                    node.parser_id = packets::command_node::parsers::minecraft_item_predicate;
                } else if constexpr (std::is_same_v<T, parsers::command::item_slot>) {
                    node.parser_id = packets::command_node::parsers::minecraft_item_slot;
                } else if constexpr (std::is_same_v<T, parsers::command::item_stack>) {
                    node.parser_id = packets::command_node::parsers::minecraft_item_stack;
                } else if constexpr (std::is_same_v<T, parsers::command::message>) {
                    node.parser_id = packets::command_node::parsers::minecraft_message;
                } else if constexpr (std::is_same_v<T, parsers::command::nbt_compound_tag>) {
                    node.parser_id = packets::command_node::parsers::minecraft_nbt_tag;
                } else if constexpr (std::is_same_v<T, parsers::command::nbt_path>) {
                    node.parser_id = packets::command_node::parsers::minecraft_nbt_path;
                } else if constexpr (std::is_same_v<T, parsers::command::nbt>) {
                    node.parser_id = packets::command_node::parsers::minecraft_nbt;
                } else if constexpr (std::is_same_v<T, parsers::command::objective>) {
                    node.parser_id = packets::command_node::parsers::minecraft_objective;
                } else if constexpr (std::is_same_v<T, parsers::command::objective_criteria>) {
                    node.parser_id = packets::command_node::parsers::minecraft_objective_criteria;
                } else if constexpr (std::is_same_v<T, parsers::command::operation>) {
                    node.parser_id = packets::command_node::parsers::minecraft_operation;
                } else if constexpr (std::is_same_v<T, parsers::command::particle>) {
                    node.parser_id = packets::command_node::parsers::minecraft_particle;
                } else if constexpr (std::is_same_v<T, parsers::command::resource>) {
                    node.parser_id = packets::command_node::parsers::minecraft_resource;
                    node.properties = {.registry = it.suggestion_registry};
                } else if constexpr (std::is_same_v<T, parsers::command::resource_key>) {
                    node.parser_id = packets::command_node::parsers::minecraft_resource_key;
                    node.properties = {.registry = it.suggestion_registry};
                } else if constexpr (std::is_same_v<T, parsers::command::resource_location>) {
                    node.parser_id = packets::command_node::parsers::minecraft_resource_location;
                } else if constexpr (std::is_same_v<T, parsers::command::resource_or_tag>) {
                    node.parser_id = packets::command_node::parsers::minecraft_resource_or_tag;
                    node.properties = {.registry = it.suggestion_registry};
                } else if constexpr (std::is_same_v<T, parsers::command::rotation>) {
                    node.parser_id = packets::command_node::parsers::minecraft_rotation;
                } else if constexpr (std::is_same_v<T, parsers::command::score_holder>) {
                    node.parser_id = packets::command_node::parsers::minecraft_score_holder;
                } else if constexpr (std::is_same_v<T, parsers::command::team>) {
                    node.parser_id = packets::command_node::parsers::minecraft_team;
                } else if constexpr (std::is_same_v<T, parsers::command::template_mirror>) {
                    node.parser_id = packets::command_node::parsers::minecraft_template_mirror;
                } else if constexpr (std::is_same_v<T, parsers::command::time>) {
                    node.parser_id = packets::command_node::parsers::minecraft_time;
                } else if constexpr (std::is_same_v<T, parsers::command::uuid>) {
                    node.parser_id = packets::command_node::parsers::minecraft_uuid;
                } else if constexpr (std::is_same_v<T, parsers::command::vec2>) {
                    node.parser_id = packets::command_node::parsers::minecraft_vec2;
                } else if constexpr (std::is_same_v<T, parsers::command::vec3>) {
                    node.parser_id = packets::command_node::parsers::minecraft_vec3;
                } else if constexpr (std::is_same_v<T, parsers::command::custom_virtual>) {
                    node.parser_id = packets::command_node::parsers::brigadier_string;
                    node.properties = {.flags = (int8_t)parsers::command::string::single_word};
                } else if constexpr (std::is_same_v<T, parsers::command::block_pos>) {
                    node.parser_id = packets::command_node::parsers::minecraft_block_pos;
                } else {
                    assert((false && __LINE__ && __FILE__ " Failed to convert command predicate to parser"));
                }
            },
            command

        );
    }

    packets::command_node build_node(const command& command, bool is_root) {
        packets::command_node result;
        result.name = command.name;
        if (command.redirect) {
            result.flags.has_redirect = true;
            result.redirect_node = command.redirect->target_command;
        }
        if (command.has_suggestion()) {
            result.flags.has_suggestion = true;
            result.suggestion_type = command.is_custom_suggestion() ? "minecraft:ask_server" : command.get_named_suggestion();
        }
        result.flags.is_executable = (bool)command.executable;

        bool is_argument = (bool)command.argument_predicate;
        if (is_root) {
            result.flags.node_type = packets::command_node::node_type::root;
            return result;
        } else if (is_argument) {
            result.flags.node_type = packets::command_node::node_type::argument;
            build_parser(result, *command.argument_predicate);
        } else
            result.flags.node_type = packets::command_node::node_type::literal;
        return result;
    }

    const list_array<uint8_t>& command_manager::compile_to_graph() {
        if (graph_ready)
            return graph_cache;
        list_array<uint8_t> res;
        bool is_root = true;

        WriteVar<int32_t>(command_nodes.size(), res);
        for (auto& command : command_nodes) {
            auto node = build_node(command, is_root);
            is_root = false;
            res.push_back((uint8_t)node.flags.raw);
            res.push_back((uint8_t)command.childs.size());
            for (auto& child : command.childs)
                WriteVar<int32_t>(child, res);
            if (node.flags.has_redirect)
                WriteVar<int32_t>(*node.redirect_node, res);
            if (node.flags.node_type != packets::command_node::node_type::root)
                WriteString(res, *node.name, 32767);
            if (node.flags.node_type == packets::command_node::node_type::argument) {
                WriteVar<int32_t>((int32_t)*node.parser_id, res);
                if (node.properties) {
                    auto& parser_data = *node.properties;
                    if (parser_data.flags)
                        WriteVar<int32_t>(*parser_data.flags, res);
                    if (parser_data.min)
                        std::visit([&res](auto val) { WriteValue<decltype(val)>(val, res); }, *parser_data.min);
                    if (parser_data.max)
                        std::visit([&res](auto val) { WriteValue<decltype(val)>(val, res); }, *parser_data.max);
                    if (parser_data.registry)
                        WriteIdentifier(res, *parser_data.registry);
                }
            }
            if (node.flags.has_suggestion)
                WriteIdentifier(res, *node.suggestion_type);
        }
        res.commit();
        graph_ready = true;
        return graph_cache = res;
    }

    bool command_manager::is_graph_fresh() const {
        return graph_ready;
    }

    bool command_manager::belongs(command* _command) {
        return command_nodes.contains_one([_command](const command& c) { return &c == _command; });
    }

    void command_manager::reload_commands() {
        command_nodes.clear();
        graph_ready = false;
        command_nodes.push_back({});
        command_root_browser browser(*this);
        for (auto& plugin : pluginManagement.registeredPlugins())
            plugin->OnCommandsLoad(plugin, browser);

        for (auto& plugin : pluginManagement.registeredPlugins())
            plugin->OnCommandsLoadComplete(plugin, browser);

        command_nodes.commit();
    }

    std::optional<parser> command_manager::parse_string(command_parser&& config, const std::string& string) {
        return std::visit(
            [string = string, self = this](auto& config) mutable -> std::optional<parser> {
                std::string part;
                next_token(part, string);
                return pred::parse(*self, config, part, string);
            },
            config
        );
    }

    command_root_browser::command_root_browser(command_manager& manager)
        : manager(manager) {}

    command_browser command_root_browser::add_child(command&& command) {
        if (manager.command_nodes.size() >= INT32_MAX)
            throw std::runtime_error("command nodes limit reached");


        auto& root = manager.command_nodes[0];

        if (root.childs_cache.find(command.name) != root.childs_cache.end())
            throw std::invalid_argument("This command already defined");

        root.childs.push_back(manager.command_nodes.size());
        root.childs_cache[command.name] = manager.command_nodes.size();
        manager.command_nodes.push_back(command);
        manager.graph_ready = false;
        return command_browser(manager, int32_t(manager.command_nodes.size() - 1));
    }

    bool command_root_browser::remove_child(const std::string& name) {
        auto id = manager.command_nodes.find_if([&](const command& cmd) {
            return cmd.name == name;
        });
        if (list_array<command>::npos != id)
            manager.remove(id);
        return list_array<command>::npos != id;
    }

    list_array<command_browser> command_root_browser::get_childs() {
        list_array<command_browser> res;
        for (auto& child : manager.command_nodes[0].childs)
            res.push_back(command_browser(manager, child));
        return res;
    }

    command_browser command_root_browser::open(const std::string& path) const {
        return command_browser(manager, path);
    }

    std::string command_root_browser::get_documentation() const {
        std::string res;
        for (auto& child : manager.command_nodes[0].childs)
            res += command_browser(manager, child).get_documentation() + "\n\n";
        return res;
    }

    command_manager& command_root_browser::get_manager() const {
        return manager;
    }

    command_browser command_browser::open(const std::string& path) const {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");

        return command_browser(manager, get_index(manager.command_nodes, path, current_id));
    }

    std::string command_browser::get_documentation() const {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");
        std::string res = current_command.name;
        if (current_command.description.size() > 0)
            res += " - " + current_command.description;

        for (auto& child : current_command.childs) {
            command_browser browser(manager, child);
            std::string child_modify = browser.get_documentation();
            if (child_modify.size() > 0) {
                auto& cmd = browser.look_up();
                res.reserve(child_modify.size() + 4 + cmd.name.size());
                res.push_back('\n');
                res.push_back('\t');
                for (auto& ch : child_modify) {
                    res.push_back(ch);
                    if (ch == '\n')
                        res.push_back('\t');
                }
                if (res.back() == '\t')
                    res.pop_back();
                if (res.back() == '\n')
                    res.pop_back();
            }
        }
        return res;
    }

    const command& command_browser::look_up() {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");
        return current_command;
    }

    command_browser::command_browser(command_manager& manager, int32_t id)
        : manager(manager), current_id(id), current_command(manager.command_nodes[id]) {}

    command_browser::command_browser(command_manager& manager, const std::string& path)
        : manager(manager),
          current_command(manager.command_nodes[current_id = get_index(manager.command_nodes, path)]) {}

    command_browser::command_browser(command_browser& browser, const std::string& path)
        : manager(browser.manager),
          current_command(manager.command_nodes[current_id = get_index(manager.command_nodes, path, browser.current_id)]) {}

    command_browser::command_browser(command_browser&& browser) noexcept
        : manager(browser.manager),
          current_id(browser.current_id),
          current_command(browser.current_command) {}

    command_browser command_browser::add_child(command&& command) {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");

        if (manager.command_nodes.size() >= INT32_MAX)
            throw std::runtime_error("command nodes limit reached");

        if (current_command.childs_cache.find(command.name) != current_command.childs_cache.end())
            throw std::invalid_argument("This command already defined");

        current_command.childs.push_back(manager.command_nodes.size());
        current_command.childs_cache[command.name] = manager.command_nodes.size();
        manager.command_nodes.push_back(command);
        manager.graph_ready = false;


        return command_browser(manager, int32_t(manager.command_nodes.size() - 1));
    }

    command_browser command_browser::add_child(command&& command, command_parser pred) {
        command.argument_predicate = pred;
        return add_child(std::move(command));
    }

    command_browser command_browser::add_child(command_browser& command) {
        if (&command.manager != &manager)
            throw std::invalid_argument("command_browser from different manager");
        if (!is_valid() && !command.is_valid())
            throw std::runtime_error("command has been deleted");

        if (manager.command_nodes.size() >= INT32_MAX)
            throw std::runtime_error("command nodes limit reached");

        if (current_command.childs_cache.find(command.current_command.name) != current_command.childs_cache.end())
            throw std::invalid_argument("This command already defined");

        current_command.childs.push_back(command.current_id);
        ++command.current_command.links;
        current_command.childs_cache[command.current_command.name] = command.current_id;
        manager.graph_ready = false;


        return command_browser(manager, int32_t(manager.command_nodes.size() - 1));
    }

    bool command_browser::remove_child(const std::string& name) {
        auto tmp = current_command.childs_cache.find(current_command.name);
        if (tmp != current_command.childs_cache.end()) {
            auto& node = manager.command_nodes[tmp->second];
            if (!--node.links)
                manager.remove(tmp->second);
            manager.graph_ready = false;
            return true;
        }
        return false;
    }

    list_array<command_browser> command_browser::get_childs() {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");

        list_array<command_browser> res;
        for (auto& child : current_command.childs)
            res.push_back(command_browser(manager, child));
        return res;
    }

    void apply_action_command(command& current_command, const action_provider& action) {
        if (!action.action_tag.empty()) {
            current_command.action_name = action.action_tag;
            if (!api::permissions::has_action(action.action_tag))
                api::permissions::register_action(action.action_tag, action.required_permissions_tag);
        }
    }

    void apply_action_command(command& current_command, action_provider&& action) {
        if (!action.action_tag.empty()) {
            current_command.action_name = action.action_tag;
            if (!api::permissions::has_action(action.action_tag))
                api::permissions::register_action(action.action_tag, std::move(action.required_permissions_tag));
        }
    }

    command_browser& command_browser::set_redirect(const std::string& path, command_redirect target_command) {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");
        command_browser target(manager, path);
        redirect_command redirect;
        redirect.redirect_routine = target_command;
        redirect.target_command = target.current_id;
        current_command.redirect = std::move(redirect);
        manager.graph_ready = false;
        return *this;
    }

    command_browser& command_browser::remove_redirect() {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");
        current_command.redirect = std::nullopt;
        return *this;
    }

    command_browser& command_browser::set_argument_type(const command_parser& pred) {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");

        current_command.argument_predicate = pred;
        return *this;
    }

    command_browser& command_browser::remove_argument() {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");

        current_command.argument_predicate = std::nullopt;
        return *this;
    }

    command_browser& command_browser::set_callback(const action_provider& action, const command_callback& callback) {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");

        current_command.executable = callback;
        apply_action_command(current_command, action);
        return *this;
    }

    command_browser& command_browser::remove_callback() {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");

        current_command.executable = std::nullopt;
        current_command.action_name.clear();
        return *this;
    }

    command_browser& command_browser::set_suggestion(const std::string& suggestion_type) {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");
        current_command.suggestions = suggestion_type;
        manager.graph_ready = false;
        return *this;
    }

    command_browser& command_browser::set_suggestion_callback(const command_suggestion& suggestions) {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");
        current_command.suggestions = suggestions;
        manager.graph_ready = false;
        return *this;
    }

    command_browser& command_browser::remove_suggestion() {
        if (!is_valid())
            throw std::runtime_error("command has been deleted");
        current_command.suggestions = {};
        manager.graph_ready = false;
        return *this;
    }

    command_browser& command_browser::modify_command(const command& _command) {
        return modify_command(command(_command));
    }

    command_browser& command_browser::modify_command(command&& _command) {
        if (&current_command == &_command)
            return *this;
        if (!is_valid())
            throw std::runtime_error("command has been deleted");


        auto old_childs = manager.command_nodes[current_id].childs;
        auto& new_command = manager.command_nodes[current_id] = std::move(_command);
        new_command.childs = old_childs;
        manager.graph_ready = false;

        *reinterpret_cast<command**>(&current_command) = &new_command;
        return *this;
    }

    bool command_browser::is_valid() const {
        if (current_id == -1)
            return false;
        if (manager.command_nodes.size() < current_id)
            return false;
        return manager.belongs(&current_command);
    }
}
