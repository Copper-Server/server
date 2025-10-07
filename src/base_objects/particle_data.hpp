/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_PACKETS_PARTICLE_DATA
#define SRC_BASE_OBJECTS_PACKETS_PARTICLE_DATA
#include <src/base_objects/chat.hpp>
#include <src/base_objects/packets_help.hpp>
#include <src/base_objects/position.hpp>
#include <src/base_objects/slot.hpp>
#include <string>
#include <variant>
#include <vector>

namespace copper_server::base_objects {
    struct particle_data {
        struct block : public enum_item<1> {
            var_int32::block_state id;
        };

        struct block_marker : public enum_item<2> {
            var_int32::block_state id;
        };

        struct dust : public enum_item<13> {
            int32_t rgb;
            float scale;
        };

        struct dust_color_transition : public enum_item<14> {
            int32_t from_rgb;
            int32_t to_rgb;
            float scale;
        };

        struct entity_effect : public enum_item<20> {
            int32_t argb;
        };

        struct falling_dust : public enum_item<28> {
            var_int32::block_state id;
        };

        struct tinted_leaves : public enum_item<35> {
            int32_t rgb;
        };

        struct sculk_charge : public enum_item<37> {
            float roll;
        };

        struct item : public enum_item<46> {
            base_objects::slot item;
        };

        struct vibration : public enum_item<47> {
            struct block : public enum_item<0> {
                position block_pos;
            };

            struct entity : public enum_item<1> {
                var_int32::entity_id id;
                float eye_height;
            };

            partial_enum_switch<var_int32::position_source_type, block, entity> data;
            var_int32 travel_ticks;
        };

        struct trail : public enum_item<48> {
            double x;
            double y;
            double z;
            int32_t rgb;
            var_int32 duration;
        };

        struct shriek : public enum_item<102> {
            var_int32 delay;
        };

        struct dust_pillar : public enum_item<108> {
            var_int32::block_state id;
        };

        struct block_crumble : public enum_item<112> {
            var_int32::block_state id;
        };

        partial_enum_switch<
            var_int32::particle_type,
            block,
            block_marker,
            dust,
            dust_color_transition,
            entity_effect,
            falling_dust,
            tinted_leaves,
            sculk_charge,
            item,
            vibration,
            trail,
            shriek,
            dust_pillar,
            block_crumble>
            data;
    };
}
#endif /* SRC_BASE_OBJECTS_PARTICLE_DATA */
