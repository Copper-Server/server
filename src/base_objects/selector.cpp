#include <src/base_objects/entity.hpp>
#include <src/base_objects/selector.hpp>

namespace copper_server::base_objects {

    size_t next_select(std::string_view view) {
        size_t pos = 0;
        int depth = 0;
        bool inString = false;

        while (pos < view.size()) {
            char c = view[pos];

            // Handle strings
            if (c == '"')
                inString = !inString;
            else if (!inString) {
                // Handle nesting
                if (c == '{')
                    depth++;
                else if (c == '}') {
                    depth--;
                    if (depth == 0 && pos > 0) {
                        pos++;
                        break;
                    }
                } else if (depth == 0 && (c == ',' || c == ']'))
                    break;
            }

            pos++;
        }

        if (depth != 0 || (pos == view.size() && view.back() != ']'))
            return std::string_view::npos;
        return pos;
    }

    void selector::build_selector(const std::string& full_string) {
        this->full_string = full_string;
        std::string_view view(full_string);
        if (view[0] != '@')
            throw std::runtime_error("Expected '@' at the start of selector");
        switch (view[1]) {
        case 'p':
            flags.only_players = true;
            flags.nearest = true;
            flags.except_self = true;
            break;
        case 'r':
            flags.only_players = true;
            flags.random = true;
            break;
        case 'a':
            flags.only_players = true;
            break;
        case 'e':
            break;
        case 's':
            flags.self = true;
            break;
        case 'R':
            flags.random = true;
            break;
        case 'E':
            flags.only_entities = true;
            break;
        case 'n':
            flags.nearest = true;
            flags.except_self = true;
            break;
        case 'o':
            flags.random = true;
            flags.only_one = true;
            break;
        case 't':
            flags.only_entities = true;
            flags.random = true;
            break;
        case '<': {
            auto close = view.find('>', 2);
            if (close == view.npos)
                throw std::runtime_error("Expected '>' in selector");
            std::string_view selected = view.substr(2, close - 1);
            view = view.substr(close + 1);
            list_array<char>(selected)
                .where([](char ch) { return ch != ' ' | ch != '\n' | ch != '\t' | ch != '\r'; })
                .split_by(',')
                .for_each([&](list_array<char>&& str) {
                    std::string converted = str.to_container<std::string>();
                    if (converted == "only_players")
                        flags.only_players = true;
                    else if (converted == "only_entities") {
                        flags.only_entities = true;
                    } else if (converted == "random") {
                        flags.random = true;
                    } else if (converted == "nearest") {
                        flags.nearest = true;
                    } else if (converted == "only_one") {
                        flags.only_one = true;
                    } else if (converted == "self") {
                        flags.self = true;
                    } else if (converted == "except_self") {
                        flags.except_self = true;
                    } else
                        throw std::runtime_error("Unknown selector flag: " + converted);
                });
            break;
        }
        default:
            throw std::runtime_error("Unknown selector: " + std::string(view.substr(0, 2)));
        }

        if (view.starts_with('[')) {
            view = view.substr(1);
            while (view[0] != ']') {
                auto equal = view.find_first_of("!=");
                if (equal == view.npos)
                    throw std::runtime_error("Expected '=' or '!=' in selector");
                bool is_inverted = view[equal] == '!';
                std::string_view key = view.substr(0, equal);
                view = view.substr(equal + 1 + is_inverted);
                auto comma = next_select(view);
                if (comma == view.npos)
                    throw std::runtime_error("Expected ',' or ']' at the end of selector property");

                std::string_view value = view.substr(0, comma);
                view = view.substr(comma + 1);
                if (key == "x") {
                    x = std::stod(std::string(value));
                    x_inverted = is_inverted;
                } else if (key == "y") {
                    y = std::stod(std::string(value));
                    y_inverted = is_inverted;
                } else if (key == "z") {
                    z = std::stod(std::string(value));
                    z_inverted = is_inverted;
                } else if (key == "dx") {
                    dx = std::stod(std::string(value));
                    dx_inverted = is_inverted;
                } else if (key == "dy") {
                    dy = std::stod(std::string(value));
                    dy_inverted = is_inverted;
                } else if (key == "dz") {
                    dz = std::stod(std::string(value));
                    dz_inverted = is_inverted;
                } else if (key == "x_rotation") {
                    x_rotation = std::stod(std::string(value));
                    x_rotation_inverted = is_inverted;
                } else if (key == "y_rotation") {
                    y_rotation = std::stod(std::string(value));
                    y_rotation_inverted = is_inverted;
                } else if (key == "distance") {
                    auto dot = value.find('.');
                    if (dot == value.npos) {
                        distance.min = std::stoi(std::string(value));
                        distance.max = distance.min;
                    } else {
                        if (value[dot + 1] != '.')
                            throw std::runtime_error("Expected '..' as distance");
                        distance.min = std::stoi(std::string(value.substr(0, dot)));
                        distance.max = std::stoi(std::string(value.substr(dot + 1)));
                    }
                    distance.is_inverted = is_inverted;
                } else if (key == "scores") {
                    auto colon = value.find(':');
                    if (colon == value.npos)
                        throw std::runtime_error("Expected ':' in scores");
                    std::string_view score_name = value.substr(0, colon);
                    value = value.substr(colon + 1);
                    auto dot = value.find('.');
                    if (dot == value.npos) {
                        scores[std::string(score_name)].min = std::stoi(std::string(value));
                        scores[std::string(score_name)].max = scores[std::string(score_name)].min;
                    } else {
                        if (value[dot + 1] != '.')
                            throw std::runtime_error("Expected '..' as distance");
                        scores[std::string(score_name)].min = std::stoi(std::string(value.substr(0, dot)));
                        scores[std::string(score_name)].max = std::stoi(std::string(value.substr(dot + 1)));
                    }
                    scores[std::string(score_name)].is_inverted = is_inverted;
                } else if (key == "tags") {
                    tags.push_back({std::string(value), is_inverted});
                } else if (key == "team") {
                    team.push_back({std::string(value), is_inverted});
                } else if (key == "name") {
                    name.push_back({std::string(value), is_inverted});
                } else if (key == "type") {
                    type.push_back({std::string(value), is_inverted});
                } else if (key == "family") {
                    family.push_back({std::string(value), is_inverted});
                } else if (key == "predicate") {
                    predicate.push_back({std::string(value), is_inverted});
                } else if (key == "nbt") {
                    nbt = std::string(value);
                    nbt_inverted = is_inverted;
                } else if (key == "abilities") {
                    abilities = std::string(value);
                    abilities_inverted = is_inverted;
                } else if (key == "has_item") {
                    has_item.push_back({std::string(value), is_inverted});
                } else if (key == "gamemode") {
                    gamemode = value;
                } else if (key == "level") {
                    level = std::stod(std::string(value));
                } else if (key == "advancements") {
                    advancements[std::string(key)] = !is_inverted;
                } else if (key == "has_permission") {
                    has_permission[std::string(key)] = !is_inverted;
                } else if (key == "in_group") {
                    in_group.push_back({std::string(value), is_inverted});
                } else if (key == "limit") {
                    limit = std::stoi(std::string(value));
                    limit_inverted = is_inverted;
                } else if (key == "sort") {
                    if (value == "nearest")
                        sort = selector_sort::nearest;
                    else if (value == "furthest")
                        sort = selector_sort::furthest;
                    else if (value == "random")
                        sort = selector_sort::random;
                    else if (value == "arbitrary")
                        sort = selector_sort::arbitrary;
                    else
                        throw std::runtime_error("Unknown sort type: " + std::string(value));
                } else {
                    throw std::runtime_error("Unknown selector property: " + std::string(key));
                }
            }
        }
    }

    bool selector::select(class Entity& entity) const {
        if (flags.self)
            return false;
        return false;
    }

    bool selector::select(client_data_holder& player) const {
        if (flags.self)
            return false;
        return false;
    }

    //entity selects
    bool selector::select(Entity& caller, class Entity& entity) const {
        return false;
    }

    bool selector::select(Entity& caller, client_data_holder& player) const {
        return false;
    }

    //player selects
    bool selector::select(client_data_holder& caller, class Entity& entity) const {
        return false;
    }

    bool selector::select(client_data_holder& caller, client_data_holder& player) const {
        return false;
    }
}
