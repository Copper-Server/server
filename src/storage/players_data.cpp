/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <filesystem>
#include <library/enbt/io.hpp>
#include <library/fast_task/include/files.hpp>
#include <src/base_objects/entity.hpp>
#include <src/storage/players_data.hpp>

namespace copper_server::storage {
    static void extract_slot(const enbt::value& data, base_objects::slot& slot) {
        if (!data.contains())
            slot = std::nullopt;
        else
            slot = base_objects::slot_data::from_enbt(data.as_compound());
    }

    static enbt::optional compact_slot(base_objects::slot& slot) {
        return slot ? enbt::optional(slot->to_enbt()) : enbt::optional();
    }

    player_data::player_data(const std::filesystem::path& path)
        : path(path) {}

    void player_data::load() {
        fast_task::files::async_iofstream file(
            path,
            fast_task::files::open_mode::read,
            fast_task::files::on_open_action::open_exists,
            fast_task::files::_sync_flags{}
        );
        if (!file.is_open()) {
            if (std::filesystem::exists(path))
                throw std::runtime_error("Failed to open file: " + path.string());
            else {
                player.assigned_entity = base_objects::entity::create("minecraft:player");
                save();
            }
            return;
        }
        enbt::io_helper::value_read_stream stream(file);
        stream.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& stream) {
            if (name == "abilities") {
                stream.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& stream) {
                    if (name == "flags") {
                        stream.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& stream) {
                            if (name == "invulnerable")
                                player.abilities.flags.invulnerable = stream.read();
                            else if (name == "flying")
                                player.abilities.flags.flying = stream.read();
                            else if (name == "allow_flying")
                                player.abilities.flags.allow_flying = stream.read();
                            else if (name == "creative_mode")
                                player.abilities.flags.creative_mode = stream.read();
                            else if (name == "flying_speed")
                                player.abilities.flags.flying_speed = stream.read();
                            else if (name == "walking_speed")
                                player.abilities.flags.walking_speed = stream.read();
                        });
                    } else if (name == "flying_speed")
                        player.abilities.flying_speed = stream.read();
                    else if (name == "field_of_view_modifier")
                        player.abilities.field_of_view_modifier = stream.read();
                });
            } else if (name == "flags") {
                stream.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& stream) {
                    if (name == "hardcore_hearts")
                        player.hardcore_hearts = stream.read();
                    else if (name == "reduced_debug_info")
                        player.reduced_debug_info = stream.read();
                    else if (name == "show_death_screen")
                        player.show_death_screen = stream.read();
                });
            } else if (name == "gamemode") {
                stream.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& stream) {
                    if (name == "op_level")
                        player.op_level = stream.read();
                    else if (name == "gamemode")
                        player.gamemode = stream.read();
                    else if (name == "prev_gamemode")
                        player.prev_gamemode = stream.read();
                });
            } else if (name == "permission_groups") {
                stream.iterate([&](enbt::io_helper::value_read_stream& stream) {
                    player.permission_groups.push_back(stream.read());
                });
                player.permission_groups.commit();
            } else if (name == "death_location") {
                stream.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& stream) {
                    if (name == "x")
                        player.last_death_location->x = stream.read();
                    else if (name == "y")
                        player.last_death_location->y = stream.read();
                    else if (name == "z")
                        player.last_death_location->z = stream.read();
                    else if (name == "world_id")
                        player.last_death_location->world_id = stream.read().as_string();
                });
            } else if (name == "local_data")
                player.local_data = stream.read();
            else if (name == "assigned_entity") {
                auto entity = stream.read();
                auto entity_comp = entity.as_compound();
                entity_comp.erase("bound_world");
                player.assigned_entity = base_objects::entity::load_from_enbt(entity_comp);
            }
        });

        if (!player.assigned_entity)
            player.assigned_entity = base_objects::entity::create("minecraft:player");
    }

    void player_data::save() {
        enbt::compound as_file_data;
        {
            enbt::compound abilities;
            {
                enbt::compound flags;
                flags["invulnerable"] = player.abilities.flags.invulnerable;
                flags["flying"] = player.abilities.flags.flying;
                flags["allow_flying"] = player.abilities.flags.allow_flying;
                flags["creative_mode"] = player.abilities.flags.creative_mode;
                flags["flying_speed"] = player.abilities.flags.flying_speed;
                flags["walking_speed"] = player.abilities.flags.walking_speed;
                abilities["flags"] = flags;
            }
            abilities["flying_speed"] = player.abilities.flying_speed;
            abilities["field_of_view_modifier"] = player.abilities.field_of_view_modifier;
            as_file_data["abilities"] = abilities;
        }
        {
            enbt::compound flags;
            flags["hardcore_hearts"] = player.hardcore_hearts;
            flags["reduced_debug_info"] = player.reduced_debug_info;
            flags["show_death_screen"] = player.show_death_screen;
            as_file_data["flags"] = flags;
        }
        {
            enbt::compound gamemode;
            gamemode["op_level"] = player.op_level;
            gamemode["gamemode"] = player.gamemode;
            gamemode["prev_gamemode"] = player.prev_gamemode;
            as_file_data["gamemode"] = gamemode;
        }
        {
            enbt::fixed_array permissions(player.permission_groups.size());
            for (size_t i = 0; i < player.permission_groups.size(); i++)
                permissions.set(i, player.permission_groups[i]);
            as_file_data["permission_groups"] = permissions;
        }
        if (player.last_death_location.has_value()) {
            enbt::compound death_location;
            death_location["x"] = player.last_death_location->x;
            death_location["y"] = player.last_death_location->y;
            death_location["z"] = player.last_death_location->z;
            death_location["world_id"] = player.last_death_location->world_id;
            as_file_data["death_location"] = death_location;
        }

        as_file_data["local_data"] = player.local_data;
        if (player.assigned_entity)
            as_file_data["assigned_entity"] = player.assigned_entity->copy_to_enbt();

        fast_task::files::async_iofstream file(
            path,
            fast_task::files::open_mode::write,
            fast_task::files::on_open_action::always_new,
            fast_task::files::_sync_flags{}
        );
        if (!file.is_open())
            throw std::runtime_error("Failed to open file: " + path.string());
        enbt::io_helper::write_token(file, as_file_data);
        file.flush();
    }

    players_data::players_data(const std::filesystem::path& base_path)
        : base_path(base_path) {
        std::filesystem::create_directories(base_path);
    }

    player_data players_data::get_player_data(const std::string& player_uuid) {
        return player_data(base_path / (player_uuid + ".e_dat"));
    }
}
