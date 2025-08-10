/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_PARTICLE_DATA
#define SRC_BASE_OBJECTS_PARTICLE_DATA
#include <src/base_objects/position.hpp>
#include <src/base_objects/slot.hpp>
#include <string>
#include <variant>
#include <vector>

namespace copper_server::base_objects {
    struct particle_data {
        using values = std::variant<int, float, std::string, slot, base_objects::position>;
        std::vector<values> data;

        static particle_data minecraft(int32_t id) {
            particle_data data;
            data.data.push_back(id);
            return data;
        }

        static particle_data minecraft_block(int32_t id) {
            particle_data data;
            data.data.push_back(id);
            return data;
        }

        static particle_data minecraft_block_marker(int32_t id) {
            particle_data data;
            data.data.push_back(id);
            return data;
        }

        static particle_data minecraft_dust_color(float r, float g, float b, float scale) {
            particle_data data;
            data.data.push_back(r);
            data.data.push_back(g);
            data.data.push_back(b);
            data.data.push_back(scale);
            return data;
        }

        static particle_data minecraft_dust_color_transition(float from_red, float from_green, float from_blue, float to_red, float to_green, float to_blue, float scale) {
            particle_data data;
            data.data.push_back(from_red);
            data.data.push_back(from_green);
            data.data.push_back(from_blue);
            data.data.push_back(to_red);
            data.data.push_back(to_green);
            data.data.push_back(to_blue);
            data.data.push_back(scale);
            return data;
        }

        //ARGB components
        static particle_data minecraft_entity_effect(int32_t color) {
            particle_data data;
            data.data.push_back(color);
            return data;
        }

        static particle_data minecraft_falling_dust(int32_t block_state) {
            particle_data data;
            data.data.push_back(block_state);
            return data;
        }

        static particle_data minecraft_item(slot item) {
            particle_data data;
            data.data.push_back(item);
            return data;
        }

        static particle_data minecraft_vibration(base_objects::position pos, int32_t ticks) {
            particle_data data;
            data.data.push_back(0i32);
            data.data.push_back(pos);
            data.data.push_back(ticks);
            return data;
        }

        static particle_data minecraft_vibration(int32_t entity_id, float eye_height, int32_t ticks) {
            particle_data data;
            data.data.push_back(1i32);
            data.data.push_back(entity_id);
            data.data.push_back(eye_height);
            data.data.push_back(ticks);
            return data;
        }

        static particle_data minecraft_shriek(int32_t delay) {
            particle_data data;
            data.data.push_back(delay);
            return data;
        }

        static particle_data minecraft_dust_pillar(int32_t block_state) {
            particle_data data;
            data.data.push_back(block_state);
            return data;
        }
    };
}
#endif /* SRC_BASE_OBJECTS_PARTICLE_DATA */
