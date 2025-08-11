/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_STORAGE_SCOREBOARD
#define SRC_STORAGE_SCOREBOARD
#include <filesystem>
#include <src/base_objects/scoreboard.hpp>

namespace copper_server::storage {
    class scoreboard {
        std::filesystem::path path;

    public:
        scoreboard(const std::filesystem::path& path);
        base_objects::scoreboard data;

        void load();
        void save();
    };
}

#endif /* SRC_STORAGE_SCOREBOARD */
