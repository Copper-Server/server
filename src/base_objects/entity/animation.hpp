/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_ENTITY_ANIMATION
#define SRC_BASE_OBJECTS_ENTITY_ANIMATION
#include <cstdint>

namespace copper_server::base_objects {
    enum entity_animation : uint8_t {
        swing_main_arm = 0,
        unrecognized = 1,
        leave_bed = 2,
        swing_offhand = 3,
        critical_hit = 4,
        enchanted_hit = 5,
    };
}

#endif /* SRC_BASE_OBJECTS_ENTITY_ANIMATION */
