/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/base_objects/entity.hpp>
#include <src/base_objects/player.hpp>

namespace copper_server::base_objects {

    player& player::operator=(player&& other) {
        abilities = std::move(other.abilities);
        world_id = std::move(other.world_id);
        player_name = std::move(other.player_name);
        hardcore_hearts = other.hardcore_hearts;
        reduced_debug_info = other.reduced_debug_info;
        show_death_screen = other.show_death_screen;
        op_level = other.op_level;
        gamemode = other.gamemode;
        prev_gamemode = other.prev_gamemode;
        last_death_location = other.last_death_location;
        permission_groups = std::move(other.permission_groups);
        permissions = std::move(other.permissions);
        instant_granted_actions = std::move(other.instant_granted_actions);
        assigned_entity = std::move(other.assigned_entity);
        local_data = std::move(other.local_data);
        return *this;
    }

    player::player() = default;
    player::~player() = default;
}