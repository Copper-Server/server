/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_GAMEMODE
#define SRC_API_PACKETS_GAMEMODE
#include <cstdint>

namespace copper_server::api::packets {
enum class gamemode_e : uint8_t {
            survival = 0,
            creative = 1,
            adventure = 2,
            spectator = 3,
        };

        enum class optional_gamemode_e : int8_t {
            undefined = -1,
            survival = 0,
            creative = 1,
            adventure = 2,
            spectator = 3,
        };
    }
#endif /* SRC_API_PACKETS_GAMEMODE */
