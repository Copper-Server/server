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
    //this structure directly represents the layout of the light in packet
    struct light_data { 

        struct light_item {
            uint8_t i0 : 4;
            uint8_t i1 : 4;
        };

        light_item light_map[16][16][8];

        inline uint8_t get(size_t x, size_t y, size_t z) {
            return z & 1 ? light_map[x][y][z >> 1].i1 : light_map[x][y][z >> 1].i0;
        }

        inline void set(size_t x, size_t y, size_t z, uint8_t value) {
            (z & 1 ? light_map[x][y][z >> 1].i1 : light_map[x][y][z >> 1].i0) = value;
        }

        light_data()
            : light_map() {}
    };
}
#endif /* SRC_BASE_OBJECTS_WORLD_LIGHT_DATA */
