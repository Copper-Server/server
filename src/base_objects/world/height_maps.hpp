/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_WORLD_HEIGHT_MAPS
#define SRC_BASE_OBJECTS_WORLD_HEIGHT_MAPS
#include <cstdint>
#include <src/base_objects/palette_container.hpp>

namespace copper_server::base_objects::world {
    struct height_maps {
        palette_data_height_map surface;
        palette_data_height_map surface_wg;
        palette_data_height_map ocean_floor;
        palette_data_height_map ocean_floor_wg;
        palette_data_height_map motion_blocking;
        palette_data_height_map motion_blocking_no_leaves;

        height_maps() {}

        void make_zero() {
            size_t height = surface.data.size() / surface.bits_per_entry;
            surface = palette_data_height_map(height);
            surface_wg = palette_data_height_map(height);
            ocean_floor = palette_data_height_map(height);
            ocean_floor_wg = palette_data_height_map(height);
            motion_blocking = palette_data_height_map(height);
            motion_blocking_no_leaves = palette_data_height_map(height);
        }

        void set_height(int64_t new_height) {
            surface.set_height(new_height);
            surface_wg.set_height(new_height);
            ocean_floor.set_height(new_height);
            ocean_floor_wg.set_height(new_height);
            motion_blocking.set_height(new_height);
            motion_blocking_no_leaves.set_height(new_height);
        }
    };
}
#endif /* SRC_BASE_OBJECTS_WORLD_HEIGHT_MAPS */
