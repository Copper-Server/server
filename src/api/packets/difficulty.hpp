/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_DIFFICULTY
#define SRC_API_PACKETS_DIFFICULTY
#include <cstdint>

namespace copper_server::api::packets {
    enum class difficulty_e : uint8_t {
        peaceful = 0,
        easy = 1,
        normal = 2,
        hard = 3,
    };
}

#endif /* SRC_API_PACKETS_DIFFICULTY */
