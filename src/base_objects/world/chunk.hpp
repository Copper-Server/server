/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_WORLD_CHUNK
#define SRC_BASE_OBJECTS_WORLD_CHUNK
#include <boost/unordered/unordered_flat_map.hpp>
#include <vector>
#include <random>

#include <filesystem>
#include <library/list_array.hpp>
#include <src/api/ecs.hpp>
#include <src/base_objects/world/height_maps.hpp>
#include <src/base_objects/world/sub_chunk_data.hpp>

namespace copper_server::storage {
    class world_data;
    struct chunk_tick_result;
}

namespace copper_server::base_objects::world {
    struct chunk_data {
        base_objects::world::height_maps height_maps;
        std::vector<base_objects::world::sub_chunk_data> sub_chunks;
        boost::unordered_flat_map<uint64_t, api::ecs::entity> stored_entities; //uses id from world

        std::chrono::milliseconds tick_speed{0};
        int64_t inhabited_time = 0;
        const int32_t chunk_x, chunk_z;
        uint8_t load_level = 44;
        uint8_t resume_gen_level = 255; //if load_level would be lower or equal than this, then generation would be resumed, used by generators
        uint8_t generator_stage = 0xFF; //0xFF == the chunk is complete and accessible, should be managed by generator

        chunk_data(int32_t chunk_x, int32_t chunk_z);

        void update_height_map_on(uint8_t local_x, uint32_t local_y_block, uint8_t local_z);
        void update_height_map();
        void calculate_active();
        void update_metadata(); //update_height_map + calculate_active (called automatically)

        void for_each_block_entity(const std::function<void(api::ecs::entity block_entity)>& func);
        void for_each_block_entity(uint32_t local_y, const std::function<void(api::ecs::entity block_entity)>& func);

        void for_each_sub_chunk(const std::function<void(base_objects::world::sub_chunk_data& sub_chunk)>& func);
        void get_sub_chunk(uint32_t local_y, const std::function<void(base_objects::world::sub_chunk_data& sub_chunk)>& func);

        //priority accepts only negative values
        void query_for_tick(uint8_t local_x, uint32_t local_y, uint8_t local_z, uint64_t on_tick, int32_t priority = -1);
        void query_for_liquid_tick(uint8_t local_x, uint32_t local_y, uint8_t local_z, uint64_t on_tick, int32_t priority = -1);

        void tick_players_sleep(storage::chunk_tick_result& rr, storage::world_data& world);
        void tick_scheduled_blocks(storage::chunk_tick_result& rr, storage::world_data& world);
        void tick_scheduled_fluids(storage::chunk_tick_result& rr, storage::world_data& world);
        void tick_raid(storage::chunk_tick_result& rr, storage::world_data& world);
        void tick_spawn_mobs(storage::chunk_tick_result& rr, storage::world_data& world);
        void tick_ice_snow(storage::chunk_tick_result& rr, storage::world_data& world);
        void tick_random_ticks(storage::chunk_tick_result& rr, storage::world_data& world, size_t random_tick_speed, std::mt19937& random_engine);
        void tick_poi(storage::chunk_tick_result& rr, storage::world_data& world);
        void tick_block_event(storage::chunk_tick_result& rr, storage::world_data& world);
        void tick_dragon(storage::chunk_tick_result& rr, storage::world_data& world);
        void tick_entity(storage::chunk_tick_result& rr, storage::world_data& world, std::mt19937& random_engine);
        void tick_block_entity(storage::chunk_tick_result& rr, storage::world_data& world);
        void tick_game_event(storage::chunk_tick_result& rr, storage::world_data& world);

        void set_state(uint8_t local_x, uint32_t local_y, uint8_t local_z, base_objects::block_id_t, api::ecs::world_local_registry& world);
        void set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, base_objects::block, api::ecs::world_local_registry& world);
        void set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, api::ecs::entity&&, api::ecs::world_local_registry& world);
        void set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, const api::ecs::entity&, api::ecs::world_local_registry& world);

        void set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, const base_objects::any_block& block, api::ecs::world_local_registry& world);
        void set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, base_objects::any_block&& block, api::ecs::world_local_registry& world);
        base_objects::block get_block(uint8_t local_x, uint32_t local_y, uint8_t local_z);
        api::ecs::entity get_block_entity(uint8_t local_x, uint32_t local_y, uint8_t local_z);

        //generator functions
        void gen_set_state(uint8_t local_x, uint32_t local_y, uint8_t local_z, base_objects::block_id_t, api::ecs::world_local_registry& world);
        void gen_set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, base_objects::block, api::ecs::world_local_registry& world);
        void gen_set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, api::ecs::entity&&, api::ecs::world_local_registry& world);
        void gen_set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, const api::ecs::entity&, api::ecs::world_local_registry& world);
        void gen_remove_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, api::ecs::world_local_registry& world);

        bool could_be_unloaded() const noexcept;

    private:
        friend class storage::world_data;
        bool load(const std::filesystem::path& path, uint64_t tick_counter, storage::world_data& world);
        bool load(const enbt::compound_const_ref& chunk_data, uint64_t tick_counter, storage::world_data& world);
        bool save(const std::filesystem::path& path, uint64_t tick_counter, storage::world_data& world);
    };
}

#endif /* SRC_BASE_OBJECTS_WORLD_CHUNK */
