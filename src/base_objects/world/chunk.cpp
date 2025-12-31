/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/iostreams/filter/zstd.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include <library/fast_task/include/files.hpp>

#include <src/api/configuration.hpp>
#include <src/api/ecs/base_components.hpp>
#include <src/api/ecs/entity_definition.hpp>
#include <src/api/entity.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/world/chunk.hpp>
#include <src/storage/world_data.hpp>
#include <src/util/readers.hpp>

namespace copper_server::base_objects::world {
    template <class T>
    T convert_chunk_global_pos(T pos) {
        if (pos == 0)
            return 0;
        if (pos < 0)
            return (pos + 1) / 16 - 1;
        return pos / 16;
    }

    template <class T>
    T convert_chunk_local_pos(T pos) {
        if (pos == 0)
            return 0;
        if (pos < 0)
            return 16 + (pos % 16);
        return pos % 16;
    }

    chunk_data::chunk_data(int32_t chunk_x, int32_t chunk_z)
        : chunk_x(chunk_x), chunk_z(chunk_z) {}

    void chunk_data::update_height_map_on(uint8_t local_x, uint32_t local_y_block, uint8_t local_z) {
        if (local_y_block == 0)
            return;
        auto& leaves = api::tags::unfold_tag(api::tags::builtin_entry::block, "minecraft:block/leaves");
        auto bloc = get_block(local_x, local_y_block, local_z);
        if (!bloc.is_air()) {
            if (height_maps.ocean_floor.get(local_x, local_z) < local_y_block)
                height_maps.ocean_floor.set(local_x, local_z, local_y_block);
            if (bloc.is_liquid()) {
                if (height_maps.surface.get(local_x, local_z) < local_y_block)
                    height_maps.surface.set(local_x, local_z, local_y_block);
            }
            if (bloc.is_solid()) {
                if (height_maps.motion_blocking.get(local_x, local_z) < local_y_block)
                    height_maps.motion_blocking.set(local_x, local_z, local_y_block);

                if (!leaves.contains(bloc.general_block_id()))
                    if (height_maps.motion_blocking_no_leaves.get(local_x, local_z) < local_y_block)
                        height_maps.motion_blocking_no_leaves.set(local_x, local_z, local_y_block);
            }

        } else {
            uint32_t to_skip = local_y_block / 16 + bool(local_y_block % 16);
            auto end = sub_chunks.rend();

            for (auto beg = sub_chunks.rbegin(); beg != end; ++beg) {
                if (to_skip) {
                    --to_skip;
                    continue;
                }
                auto& schunk = *beg;
                for (int8_t y = 15; y >= 0; y--) {
                    auto block = schunk.get_block(local_x, y, local_z);
                    if (!block.is_air()) {
                        auto y_pos = y + local_y_block;

                        if (!height_maps.ocean_floor.get(local_x, local_z))
                            height_maps.ocean_floor.set(local_x, local_z, y_pos);

                        if (block.is_liquid())
                            if (!height_maps.surface.get(local_x, local_z))
                                height_maps.surface.set(local_x, local_z, y_pos);

                        if (block.is_solid()) {
                            if (!height_maps.motion_blocking.get(local_x, local_z))
                                height_maps.motion_blocking.set(local_x, local_z, y_pos);

                            if (!leaves.contains(block.general_block_id()))
                                if (!height_maps.motion_blocking_no_leaves.get(local_x, local_z))
                                    height_maps.motion_blocking_no_leaves.set(local_x, local_z, y_pos);
                        }
                    }
                }
                local_y_block -= 16;
            }
        }
    }

    void chunk_data::update_height_map() {
        height_maps.make_zero();
        uint32_t local_y_block = (sub_chunks.size() - 1) * 16;
        auto& leaves = api::tags::unfold_tag(api::tags::builtin_entry::block, "minecraft:block/leaves");
        auto end = sub_chunks.rend();
        for (auto beg = sub_chunks.rbegin(); beg != end; ++beg) {
            auto& schunk = *beg;
            for (uint8_t x = 0; x < 16; x++) {
                for (int8_t y = 15; y >= 0; y--) {
                    for (uint8_t z = 0; z < 16; z++) {
                        auto block = schunk.get_block(x, y, z);
                        if (!block.is_air()) {
                            auto y_pos = y + local_y_block;

                            if (!height_maps.ocean_floor.get(x, z))
                                height_maps.ocean_floor.set(x, z, y_pos);

                            if (block.is_liquid())
                                if (!height_maps.surface.get(x, z))
                                    height_maps.surface.set(x, z, y_pos);

                            if (block.is_solid()) {
                                if (!height_maps.motion_blocking.get(x, z))
                                    height_maps.motion_blocking.set(x, z, y_pos);

                                if (!leaves.contains(block.general_block_id()))
                                    if (!height_maps.motion_blocking_no_leaves.get(x, z))
                                        height_maps.motion_blocking_no_leaves.set(x, z, y_pos);
                            }
                        }
                    }
                }
            }
            local_y_block -= 16;
        }
    }

    void chunk_data::calculate_active() {
        for (auto& schunk : sub_chunks) {
            schunk.active_blocks = 0;
            for (uint8_t x = 0; x < 16; x++)
                for (int8_t y = 0; y < 16; y++)
                    for (uint8_t z = 0; z < 16; z++)
                        schunk.active_blocks += !schunk.get_block(x, y, z).is_air();
        }
    }

    void chunk_data::update_metadata() {
        height_maps.make_zero();
        uint32_t local_y_block = (sub_chunks.size() - 1) * 16;
        auto& leaves = api::tags::unfold_tag(api::tags::builtin_entry::block, "minecraft:block/leaves");
        auto end = sub_chunks.rend();
        for (auto beg = sub_chunks.rbegin(); beg != end; ++beg) {
            auto& schunk = *beg;
            schunk.active_blocks = 0;
            for (uint8_t x = 0; x < 16; x++) {
                for (int8_t y = 15; y >= 0; y--) {
                    for (uint8_t z = 0; z < 16; z++) {
                        auto block = schunk.get_block(x, y, z);
                        if (!block.is_air()) {
                            schunk.active_blocks += 1;
                            auto y_pos = y + local_y_block;

                            if (!height_maps.ocean_floor.get(x, z))
                                height_maps.ocean_floor.set(x, z, y_pos);

                            if (block.is_liquid())
                                if (!height_maps.surface.get(x, z))
                                    height_maps.surface.set(x, z, y_pos);

                            if (block.is_solid()) {
                                if (!height_maps.motion_blocking.get(x, z))
                                    height_maps.motion_blocking.set(x, z, y_pos);

                                if (!leaves.contains(block.general_block_id()))
                                    if (!height_maps.motion_blocking_no_leaves.get(x, z))
                                        height_maps.motion_blocking_no_leaves.set(x, z, y_pos);
                            }
                        }
                    }
                }
            }
            local_y_block -= 16;
        }
    }

    void chunk_data::for_each_block_entity(const std::function<void(api::ecs::entity block_entity)>& func) {
        for (auto& sub_chunk : sub_chunks)
            for (auto& [_pos, data] : sub_chunk.block_entities) {
                base_objects::local_block_pos pos;
                pos.x = _pos >> 8;
                pos.y = (_pos >> 4) & 0xF;
                pos.z = _pos & 0xF;
                func(data);
            }
    }

    void chunk_data::for_each_block_entity(uint32_t local_y, const std::function<void(api::ecs::entity block_entity)>& func) {
        if (local_y < sub_chunks.size())
            for (auto& [_pos, data] : sub_chunks[local_y].block_entities) {
                base_objects::local_block_pos pos;
                pos.x = _pos >> 8;
                pos.y = (_pos >> 4) & 0xF;
                pos.z = _pos & 0xF;
                func(data);
            }
    }

    void chunk_data::for_each_sub_chunk(const std::function<void(sub_chunk_data& sub_chunk)>& func) {
        for (auto& sub_chunk : sub_chunks)
            func(sub_chunk);
    }

    void chunk_data::get_sub_chunk(uint32_t sub_chunk_y, const std::function<void(sub_chunk_data& sub_chunk)>& func) {
        if (sub_chunk_y < sub_chunks.size())
            func(sub_chunks[sub_chunk_y]);
    }

    void chunk_data::query_for_tick(uint8_t local_x, uint32_t global_y, uint8_t local_z, uint64_t on_tick, int32_t priority) {
        auto& sub_chunk = sub_chunks.at(convert_chunk_global_pos(global_y));
        sub_chunk.queried_for_tick.push_back(
            to_be_ticked{
                on_tick,
                get_block(local_x, global_y, local_z).general_block_id(),
                priority,
                local_x,
                uint8_t(global_y & 15),
                local_z
            }
        );
    }

    void chunk_data::query_for_liquid_tick(uint8_t local_x, uint32_t global_y, uint8_t local_z, uint64_t on_tick, int32_t priority) {
        auto& sub_chunk = sub_chunks.at(convert_chunk_global_pos(global_y));
        sub_chunk.queried_for_liquid_tick.push_back(
            to_be_ticked{
                on_tick,
                get_block(local_x, global_y, local_z).general_block_id(),
                priority,
                local_x,
                uint8_t(global_y & 15),
                local_z
            }
        );
    }

    void chunk_data::tick_players_sleep(storage::chunk_tick_result& rr, storage::world_data& world) {
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_scheduled_blocks(storage::chunk_tick_result& rr, storage::world_data& world) {
        if (load_level > 32)
            return;
        uint32_t sub_chunk_i = 0;
        for (auto& sub_chunk : sub_chunks) {
            for (auto item : sub_chunk.queried_for_tick
                                 .take([tick_counter = world.tick_counter](auto& it) {
                                     return it.scheduled_on >= tick_counter;
                                 })
                                 .sort([](auto& a, auto& b) {
                                     return a.priority < b.priority;
                                 })) {

                auto block = sub_chunk.get_block(
                    item.x,
                    item.y,
                    item.z
                );
                if (block.general_block_id() == item.block_id)
                    block.tick(world, sub_chunk, chunk_x, sub_chunk_i, chunk_z, item.x, item.y, item.z, false);
            }
            ++sub_chunk_i;
        }
    }

    void chunk_data::tick_scheduled_fluids(storage::chunk_tick_result& rr, storage::world_data& world) {
        if (load_level > 32)
            return;
        uint32_t sub_chunk_i = 0;
        for (auto& sub_chunk : sub_chunks) {
            for (auto item : sub_chunk.queried_for_liquid_tick
                                 .take([tick_counter = world.tick_counter](auto& it) {
                                     return it.scheduled_on >= tick_counter;
                                 })
                                 .sort([](auto& a, auto& b) {
                                     return a.priority < b.priority;
                                 })) {

                auto block = sub_chunk.get_block(
                    item.x,
                    item.y,
                    item.z
                );
                if (block.id == item.block_id)
                    block.tick(world, sub_chunk, chunk_x, sub_chunk_i, chunk_z, item.x, item.y, item.z, false);
            }
            ++sub_chunk_i;
        }
    }

    void chunk_data::tick_raid(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_spawn_mobs(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_ice_snow(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_random_ticks(storage::chunk_tick_result& rr, storage::world_data& world, size_t random_tick_speed, std::mt19937& random_engine) {
        if (load_level > 32)
            return;

        uint32_t sub_chunk_y = 0;
        for (auto& sub_chunk : sub_chunks) {
            auto max_random_tick_per_sub_chunk = random_tick_speed;
            while (sub_chunk.has_tickable_blocks && max_random_tick_per_sub_chunk) {
                union {
                    struct {
                        uint8_t x;
                        uint8_t y;
                        uint8_t z;
                    } dec;

                    uint32_t value;
                } pos;

                pos.value = random_engine();
                auto block = sub_chunk.get_block(pos.dec.x, pos.dec.y, pos.dec.z);
                if (block.is_tickable())
                    block.tick(world, sub_chunk, chunk_x, sub_chunk_y, chunk_z, pos.dec.x, pos.dec.y, pos.dec.z, true);
                --max_random_tick_per_sub_chunk;
            }
            sub_chunk_y++;
        }
    }

    void chunk_data::tick_poi(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_block_event(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_dragon(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::tick_entity(storage::chunk_tick_result& rr, storage::world_data& world, std::mt19937& random_engine) {
        auto max_inactivity = api::configuration::get().game_play.entity.despawn.despawn_after_inactivity;
        auto despawn_chance = api::configuration::get().game_play.entity.despawn.despawn_chance;
        std::normal_distribution<> dis(0.0, 1.0);
        for (auto& [id, entity] : stored_entities) {
            if (entity.is_assigned_to_world(world.world_id)) {
                auto sd = entity.modify<api::ecs::com::entities::world_syncing>();
                if (entity.has<api::ecs::com::entities::assigned_player>())
                    ; //skip check
                else if (sd->despawn_immune)
                    ; //skip check
                else if (sd->state == api::ecs::com::entities::world_syncing::state_e::scheduled_for_despawn) {
                    world.unregister_entity(entity);
                    rr.unrelated_entities.push_back(id);
                } else if (sd->state == api::ecs::com::entities::world_syncing::state_e::no_player) {
                    sd->state = api::ecs::com::entities::world_syncing::state_e::scheduled_for_despawn;
                } else if (sd->inactivity_counter > max_inactivity) {
                    if (dis(random_engine) >= despawn_chance)
                        sd->state = api::ecs::com::entities::world_syncing::state_e::scheduled_for_despawn;
                } else
                    sd->state = api::ecs::com::entities::world_syncing::state_e::no_player;

                if (entity.has<api::ecs::com::entities::assigned_player>()) {
                    auto& pos = entity.get<api::ecs::com::entities::position>();
                    world.for_each_entity(
                        base_objects::spherical_bounds_block{
                            (int32_t)pos.x,
                            (int32_t)pos.y,
                            (int32_t)pos.z,
                            api::configuration::get().game_play.entity.despawn_mobs_outside
                        },
                        [&pos, t_m_r = api::configuration::get().game_play.entity.squared_values.tick_mobs_in_range](auto mark_entity) {
                            auto& mark_pos = mark_entity.get<api::ecs::com::entities::position>();
                            if (mark_entity.has<api::ecs::com::entities::assigned_player>())
                                return;
                            auto sd = mark_entity.modify<api::ecs::com::entities::world_syncing>();
                            if (sd->despawn_immune || sd->inactivity_immune)
                                return;
                            switch (sd->state) {
                            case api::ecs::com::entities::world_syncing::state_e::init:
                            case api::ecs::com::entities::world_syncing::state_e::no_player: {
                                auto dist_sq = util::distance_sq(pos, mark_pos); //how to be with fish? fish has different despawn range
                                sd->state = dist_sq > t_m_r ? api::ecs::com::entities::world_syncing::state_e::player_far : api::ecs::com::entities::world_syncing::state_e::player_near;
                                if (sd->state == api::ecs::com::entities::world_syncing::state_e::player_near)
                                    sd->inactivity_counter = 0;
                                else
                                    ++sd->inactivity_counter;
                                break;
                            }
                            default:
                                break;
                            }
                        }
                    );
                }
                if (load_level > 31)
                    continue;

                if (entity.has<api::ecs::com::entities::ride_entity>()) {
                    if (!entity.get<api::ecs::com::entities::ride_entity>().other) {
                        entity.get<api::ecs::com::entities::entity_type>().tick(entity);
                        if (entity.has<api::ecs::com::entities::ride_by_entity>())
                            for (auto ride_entity : entity.get<api::ecs::com::entities::ride_by_entity>().ride_by)
                                if (ride_entity.is_resolved())
                                    ride_entity.get_entity().get<api::ecs::com::entities::entity_type>().tick(ride_entity.get_entity());
                    }
                } else {
                    entity.get<api::ecs::com::entities::entity_type>().tick(entity);
                    if (entity.has<api::ecs::com::entities::ride_by_entity>())
                        for (auto ride_entity : entity.get<api::ecs::com::entities::ride_by_entity>().ride_by)
                            if (ride_entity.is_resolved())
                                ride_entity.get_entity().get<api::ecs::com::entities::entity_type>().tick(ride_entity.get_entity());
                }
                continue;
            } else
                rr.unrelated_entities.push_back(id);
        }
        for (auto id : rr.unrelated_entities)
            stored_entities.erase(id);
    }

    void chunk_data::tick_block_entity(storage::chunk_tick_result& rr, storage::world_data& world) {
        if (load_level > 32)
            return;
        uint32_t y = 0;
        for (auto& sub_chunk : sub_chunks) {
            for (auto& [_pos, data] : sub_chunk.block_entities) {
                base_objects::local_block_pos pos;
                pos.x = _pos >> 8;
                pos.y = (_pos >> 4) & 0xF;
                pos.z = _pos & 0xF;
                auto block = sub_chunk.get_block(pos.x, pos.y, pos.z);
                if (block.is_tickable())
                    block.tick(world, sub_chunk, chunk_x, y, chunk_z, pos.x, pos.y, pos.z, false);
            }
            ++y;
        }
    }

    void chunk_data::tick_game_event(storage::chunk_tick_result& rr, storage::world_data& world) { //TODO
        if (load_level > 32)
            return;
    }

    void chunk_data::set_state(uint8_t local_x, uint32_t local_y, uint8_t local_z, base_objects::block_id_t id, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_state(local_x, local_y & 15, local_z, id, world);
    }

    void chunk_data::set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, base_objects::block block, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block(local_x, local_y & 15, local_z, block, world);
    }

    void chunk_data::set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, api::ecs::entity&& block, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block(local_x, local_y & 15, local_z, std::move(block), world);
    }

    void chunk_data::set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, const api::ecs::entity& block, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block(local_x, local_y & 15, local_z, block, world);
    }

    void chunk_data::set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, const base_objects::any_block& block, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block(local_x, local_y & 15, local_z, block, world);
    }

    void chunk_data::set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, base_objects::any_block&& block, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block(local_x, local_y & 15, local_z, std::move(block), world);
    }

    base_objects::block chunk_data::get_block(uint8_t local_x, uint32_t local_y, uint8_t local_z) {
        return sub_chunks.at(local_y >> 4).get_block(local_x, local_y & 15, local_z);
    }

    api::ecs::entity chunk_data::get_block_entity(uint8_t local_x, uint32_t local_y, uint8_t local_z) {
        return sub_chunks.at(local_y >> 4).get_block_entity(local_x, local_y & 15, local_z);
    }

    //generator functions
    void chunk_data::gen_set_state(uint8_t local_x, uint32_t local_y, uint8_t local_z, base_objects::block_id_t id, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_state_gen(local_x, local_y & 15, local_z, id, world);
    }

    void chunk_data::gen_set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, base_objects::block block, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block_gen(local_x, local_y & 15, local_z, block, world);
    }

    void chunk_data::gen_set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, api::ecs::entity&& block, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block_gen(local_x, local_y & 15, local_z, std::move(block), world);
    }

    void chunk_data::gen_set_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, const api::ecs::entity& block, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block_gen(local_x, local_y & 15, local_z, block, world);
    }

    void chunk_data::gen_remove_block(uint8_t local_x, uint32_t local_y, uint8_t local_z, api::ecs::world_local_registry& world) {
        sub_chunks.at(local_y >> 4).set_block_gen(local_x, local_y & 15, local_z, base_objects::block(), world);
    }

    bool chunk_data::could_be_unloaded() const noexcept {
        return load_level > 44;
    }
}
