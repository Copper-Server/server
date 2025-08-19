/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/entity_id_map.hpp>
#include <src/api/permissions.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/player.hpp>
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

    void selector::build_selector_parse(std::string_view& view) {
        full_string = std::string(view);
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
                .where([](char ch) { return ch != ' ' || ch != '\n' || ch != '\t' || ch != '\r'; })
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
                    } else if (converted == "select_virtual") {
                        flags.select_virtual = true;
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
                    //x_inverted = is_inverted;
                } else if (key == "y") {
                    y = std::stod(std::string(value));
                    //y_inverted = is_inverted;
                } else if (key == "z") {
                    z = std::stod(std::string(value));
                    //z_inverted = is_inverted;
                } else if (key == "dx") {
                    dx = std::stod(std::string(value));
                    //dx_inverted = is_inverted;
                } else if (key == "dy") {
                    dy = std::stod(std::string(value));
                    //dy_inverted = is_inverted;
                } else if (key == "dz") {
                    dz = std::stod(std::string(value));
                    //dz_inverted = is_inverted;
                } else if (key == "x_rotation") {
                    x_rotation = double_range{};
                    auto dot = value.find('.');
                    if (dot == value.npos) {
                        x_rotation->min = std::stod(std::string(value));
                        x_rotation->max = x_rotation->min;
                    } else {
                        if (value[dot + 1] != '.')
                            throw std::runtime_error("Expected '..' as x_rotation");
                        auto min = value.substr(0, dot);
                        if (min.size())
                            x_rotation->min = std::stod(std::string(min));
                        auto max = value.substr(dot + 1);
                        if (max.size())
                            x_rotation->max = std::stod(std::string(max));
                    }
                    x_rotation->is_inverted = is_inverted;
                } else if (key == "y_rotation") {
                    y_rotation = double_range{};
                    auto dot = value.find('.');
                    if (dot == value.npos) {
                        y_rotation->min = std::stod(std::string(value));
                        y_rotation->max = y_rotation->min;
                    } else {
                        if (value[dot + 1] != '.')
                            throw std::runtime_error("Expected '..' as y_rotation");
                        auto min = value.substr(0, dot);
                        if (min.size())
                            y_rotation->min = std::stod(std::string(min));
                        auto max = value.substr(dot + 1);
                        if (max.size())
                            y_rotation->max = std::stod(std::string(max));
                    }
                    y_rotation->is_inverted = is_inverted;
                } else if (key == "distance") {
                    distance = integer_range{};
                    auto dot = value.find('.');
                    if (dot == value.npos) {
                        distance->min = std::stoi(std::string(value));
                        distance->max = distance->min;
                    } else {
                        if (value[dot + 1] != '.')
                            throw std::runtime_error("Expected '..' as distance");
                        auto min = value.substr(0, dot);
                        if (min.size())
                            distance->min = std::stoi(std::string(min));
                        auto max = value.substr(dot + 1);
                        if (max.size())
                            distance->max = std::stoi(std::string(max));
                    }
                    distance->is_inverted = is_inverted;
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
                        auto min = value.substr(0, dot);
                        if (min.size())
                            scores[std::string(score_name)].min = std::stoi(std::string(min));
                        auto max = value.substr(dot + 1);
                        if (max.size())
                            scores[std::string(score_name)].max = std::stoi(std::string(max));
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
                    gamemode_inverted = is_inverted;
                } else if (key == "level") {
                    level = integer_range{};
                    auto dot = value.find('.');
                    if (dot == value.npos) {
                        level->min = std::stoi(std::string(value));
                        level->max = level->min;
                    } else {
                        if (value[dot + 1] != '.')
                            throw std::runtime_error("Expected '..' as distance");
                        auto min = value.substr(0, dot);
                        if (min.size())
                            level->min = std::stoi(std::string(min));
                        auto max = value.substr(dot + 1);
                        if (max.size())
                            level->max = std::stoi(std::string(max));
                    }
                    level->is_inverted = is_inverted;
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

    void selector::build_selector(std::string_view selector_string) {
        build_selector_parse(selector_string);
    }

    bool selector::select(command_context& context, std::function<void(entity&)>&& fn) const {
        list_array<entity_ref> entities;
        entity_ref self_entity;
        if (context.other_data.contains("entity_id"))
            self_entity = api::entity_id_map::get_entity((int32_t)context.other_data.at("entity_id"));
        else
            self_entity = context.executor.player_data.assigned_entity;

        if (flags.nearest || distance || x || y || z) {
            double check_x = context.other_data.at("x");
            double check_y = context.other_data.at("y");
            double check_z = context.other_data.at("z");
            if (x)
                check_x = *x;
            if (y)
                check_y = *y;
            if (z)
                check_z = *z;

            if (!flags.self)
                api::world::get((int32_t)context.other_data.at("world_id"), [&](storage::world_data& world) {
                    if (distance) {
                        if (distance->min <= 0) {
                            spherical_bounds_block bounds{.x = (int64_t)check_x, .y = (int64_t)check_y, .z = (int64_t)check_z, .radius = (double)distance->max};
                            world.for_each_entity(bounds, [&entities](auto& entity) {
                                entities.push_back(entity);
                            });
                            if (distance->is_inverted) {
                                world.for_each_entity([&entities, filter_entities = entities.take()](auto& entity) {
                                    if (!filter_entities.contains(entity))
                                        entities.push_back(entity);
                                });
                            }
                        } else {
                            spherical_bounds_block_out bounds{.x = (int64_t)check_x, .y = (int64_t)check_y, .z = (int64_t)check_z, .radius_begin = (double)distance->min, .radius_end = (double)distance->max};
                            world.for_each_entity(bounds, [&entities](auto& entity) { entities.push_back(entity); });
                            if (distance->is_inverted) {
                                world.for_each_entity([&entities, filter_entities = entities.take()](auto& entity) {
                                    if (!filter_entities.contains(entity))
                                        entities.push_back(entity);
                                });
                            }
                        }
                    } else
                        world.for_each_entity([&entities](auto& entity) {
                            entities.push_back(entity);
                        });
                });
            else if (self_entity) {
                entities.push_back(self_entity);
            } else
                return false;


            if (dx)
                entities = entities.take().where([check_x, dx = *dx](auto& entity) {
                    return entity->hitboxes_touching_x(check_x, dx);
                });
            if (dy)
                entities = entities.take().where([check_y, dy = *dy](auto& entity) {
                    return entity->hitboxes_touching_y(check_y, dy);
                });
            if (dz)
                entities = entities.take().where([check_z, dz = *dz](auto& entity) {
                    return entity->hitboxes_touching_z(check_z, dz);
                });
            if (flags.nearest) {
                auto res = entities.min_index([pos = util::VECTOR(check_x, check_y, check_z)](auto& ent) {
                    return util::distance_sq(pos, ent->position);
                });
                if (res != entities.npos)
                    entities.push_back(
                        entities
                            .take()
                            .take(res)
                    );
            }
        } else if (!flags.self) {
            api::world::iterate([&](auto _, storage::world_data& world) {
                world.for_each_entity([&entities](auto& entity) {
                    entities.push_back(entity);
                });
            });
        } else if (self_entity)
            entities.push_back(self_entity);


        if (flags.only_players) {
            if (flags.only_entities)
                return false;
            else if (flags.select_virtual)
                entities = entities.take().where([](auto& entity) { return entity->assigned_player; });
            else {
                entities = entities.take().where([](auto& entity) {
                    if (entity->assigned_player)
                        return !entity->assigned_player->is_virtual;
                    else
                        return true;
                });
            }
        } else if (!flags.select_virtual && !flags.only_entities)
            entities = entities.take().where([](auto& entity) {
                if (entity->assigned_player)
                    return !entity->assigned_player->is_virtual;
                else
                    return true;
            });

        if (flags.only_entities)
            entities = entities.take().where([](auto& entity) {
                return !entity->assigned_player;
            });

        if (flags.except_self)
            entities.remove_one([&self_entity](auto& entity) { return entity == self_entity; });

        if (level) {
            if (level->is_inverted) {
                if (level->min && level->min)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->get_level() < level->min && entity->get_level() < level->max;
                    });
                else if (level->min)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->get_level() < level->min;
                    });
                else if (level->max)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->get_level() > level->max;
                    });
            } else {
                if (level->min && level->min)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->get_level() >= level->min && entity->get_level() >= level->max;
                    });
                else if (level->min)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->get_level() >= level->min;
                    });
                else if (level->max)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->get_level() <= level->max;
                    });
            }
        }
        if (x_rotation) {
            if (x_rotation->is_inverted) {
                if (x_rotation->min && x_rotation->min)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->rotation.x < x_rotation->min && entity->rotation.x < x_rotation->max;
                    });
                else if (x_rotation->min)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->rotation.x < x_rotation->min;
                    });
                else if (x_rotation->max)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->rotation.x > x_rotation->max;
                    });
            } else {
                if (x_rotation->min && x_rotation->min)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->rotation.x >= x_rotation->min && entity->rotation.x >= x_rotation->max;
                    });
                else if (x_rotation->min)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->rotation.x >= x_rotation->min;
                    });
                else if (x_rotation->max)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->rotation.x <= x_rotation->max;
                    });
            }
        }
        if (y_rotation) {
            if (y_rotation->is_inverted) {
                if (y_rotation->min && y_rotation->min)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->rotation.y < y_rotation->min && entity->rotation.y > y_rotation->max;
                    });
                else if (y_rotation->min)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->rotation.y < y_rotation->min;
                    });
                else if (y_rotation->max)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->rotation.y > y_rotation->max;
                    });
            } else {
                if (y_rotation->min && y_rotation->min)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->rotation.y >= y_rotation->min && entity->rotation.y >= y_rotation->max;
                    });
                else if (y_rotation->min)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->rotation.y >= y_rotation->min;
                    });
                else if (y_rotation->max)
                    entities = entities.take().where([&](auto& entity) {
                        return entity->rotation.y <= y_rotation->max;
                    });
            }
        }

        if (in_group.size()) {
            if (!flags.only_entities) {
                entities = entities.take().where([this](auto& entity) {
                    if (entity->assigned_player)
                        for (auto&& [group, is_inv] : in_group) {
                            if (api::permissions::is_in_group(group, *entity->assigned_player)) {
                                if (is_inv)
                                    return false;
                            } else if (!is_inv)
                                return false;
                        }
                    else
                        return false;
                    return true;
                });
            } else
                return false;
        }

        if (has_permission.size()) {
            if (!flags.only_entities) {
                entities = entities.take().where([this](auto& entity) {
                    if (entity->assigned_player)
                        for (auto& [perm, is_inv] : has_permission) {
                            if (api::permissions::has_rights(perm, *entity->assigned_player)) {
                                if (is_inv)
                                    return false;
                            } else if (!is_inv)
                                return false;
                        }
                    else
                        return false;
                    return true;
                });
            } else
                return false;
        }

        if (sort) {
            switch (*sort) {
            case selector_sort::nearest: {
                double check_x = context.other_data.at("x");
                double check_y = context.other_data.at("y");
                double check_z = context.other_data.at("z");
                if (x)
                    check_x = *x;
                if (y)
                    check_y = *y;
                if (z)
                    check_z = *z;

                entities
                    .sort([pos = util::VECTOR(check_x, check_y, check_z)](auto& prev, auto& next) {
                        return util::distance_sq(pos, prev->position) < util::distance_sq(pos, next->position);
                    });
                break;
            }
            case selector_sort::furthest: {
                double check_x = context.other_data.at("x");
                double check_y = context.other_data.at("y");
                double check_z = context.other_data.at("z");
                if (x)
                    check_x = *x;
                if (y)
                    check_y = *y;
                if (z)
                    check_z = *z;

                entities
                    .sort([pos = util::VECTOR(check_x, check_y, check_z)](auto& prev, auto& next) {
                        return util::distance_sq(pos, prev->position) > util::distance_sq(pos, next->position);
                    });
                break;
            }
            case selector_sort::random: {
                entities.sort([](auto& _, auto&) {
                    static std::mt19937_64 gen(std::random_device{}());
                    static std::uniform_int_distribution<> dis(0, 1);
                    return dis(gen) == 0;
                });
                break;
            }
            case selector_sort::arbitrary:
                break;
            }
        }

        if (limit) {
            if (limit_inverted) {
                if (entities.size() < *limit)
                    return false;
            } else if (entities.size() > *limit)
                entities.resize(*limit);
        }


        for (auto& entity : entities)
            fn(*entity);

        return true;
    }
}