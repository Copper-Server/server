/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_WORLD_SUB_CHUNK_DATA
#define SRC_BASE_OBJECTS_WORLD_SUB_CHUNK_DATA
#include <boost/unordered/unordered_flat_map.hpp>
#include <cstdint>
#include <functional>

#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>

#include <src/api/ecs.hpp>
#include <src/base_objects/any_block.hpp>
#include <src/base_objects/block.hpp>
#include <src/base_objects/palette_container.hpp>
#include <src/base_objects/world/light_data.hpp>

namespace copper_server::base_objects::world {
    struct sub_chunk_data {
        base_objects::palette_container_block blocks;
        base_objects::palette_container_biome biomes;
        boost::unordered_flat_map<uint16_t, api::ecs::entity> block_entities; //0xXYZ => block_entity

        base_objects::world::light_data sky_light;
        base_objects::world::light_data block_light;

        uint16_t active_blocks = 0; //if zero, the sub chunk is not rendered for clients
        bool has_tickable_blocks = false;
        bool need_to_recalculate_light = false;
        bool sky_lighted = false;   //set true if at least one block is lighted in this sub_chunk
        bool block_lighted = false; //set true if at least one block is lighted in this sub_chunk

        sub_chunk_data();
        sub_chunk_data(sub_chunk_data&&);
        ~sub_chunk_data();

        sub_chunk_data& operator=(sub_chunk_data&&);

        void get_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, std::function<void(base_objects::block block)> on_normal, std::function<void(api::ecs::entity block_entity)> on_entity);
        api::ecs::entity get_block_entity(uint8_t local_x, uint8_t local_y, uint8_t local_z);
        base_objects::block get_block(uint8_t local_x, uint8_t local_y, uint8_t local_z);

        //this function used to change the block state without destroying the block entity.
        //if stated doesn't belong for block entity or block entity id is not same for this state id then the function behaves same as set_block
        void set_state(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block_id_t state, api::ecs::world_local_registry& world);
        void set_state_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block_id_t block, api::ecs::world_local_registry& world);

        void set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block, api::ecs::world_local_registry& world);
        void set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, api::ecs::entity&& block, api::ecs::world_local_registry& world);
        void set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, const api::ecs::entity& block, api::ecs::world_local_registry& world);

        void set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, const base_objects::any_block& block, api::ecs::world_local_registry& world);
        void set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::any_block&& block, api::ecs::world_local_registry& world);

        void set_block_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block, api::ecs::world_local_registry& world);
        void set_block_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, const api::ecs::entity& block, api::ecs::world_local_registry& world);
        void set_block_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, api::ecs::entity&& block, api::ecs::world_local_registry& world);

        int32_t get_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z);
        void set_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z, int32_t id);
        void for_each_block(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block)> func);
        void for_each_block_entity(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, api::ecs::entity block_entity)> func);

        void for_each_block(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block)> func) const;
        void for_each_block_entity(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, api::ecs::entity block_entity)> func) const;
    };
}

#endif /* SRC_BASE_OBJECTS_WORLD_SUB_CHUNK_DATA */
