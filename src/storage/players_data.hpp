/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_STORAGE_PLAYERS_DATA
#define SRC_STORAGE_PLAYERS_DATA
#include <filesystem>
#include <library/enbt/enbt.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/slot.hpp>
#include <string>

namespace copper_server::storage {
    class player_data {
        std::filesystem::path path;

    public:
        player_data(const std::filesystem::path& path);
        base_objects::player player;

        void load();
        void save();
    };

    class players_data {
        std::filesystem::path base_path;

    public:
        players_data(const std::filesystem::path& base_path);

        player_data get_player_data(const std::string& player_uuid);
    };
}
#endif /* SRC_STORAGE_PLAYERS_DATA */
