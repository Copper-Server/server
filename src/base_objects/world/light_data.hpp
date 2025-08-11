/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_WORLD_LIGHT_DATA
#define SRC_BASE_OBJECTS_WORLD_LIGHT_DATA
#include <cstdint>
namespace copper_server::base_objects::world {
    struct light_data {
        union light_item {
            uint8_t light_point;
        };

        light_item light_map[16][16][16];

        light_data()
            : light_map() {}
    };
}
#endif /* SRC_BASE_OBJECTS_WORLD_LIGHT_DATA */
