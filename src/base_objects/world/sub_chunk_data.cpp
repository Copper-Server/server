/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/ecs/base_components.hpp>
#include <src/api/ecs/entity_construction.hpp>
#include <src/api/ecs/entity_definition.hpp>
#include <src/api/registers.hpp>
#include <src/base_objects/world/sub_chunk_data.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server::base_objects::world {
    sub_chunk_data::sub_chunk_data() = default;

    sub_chunk_data::~sub_chunk_data() = default;

    sub_chunk_data::sub_chunk_data(sub_chunk_data&& other) {
        *this = std::move(other);
    }

    sub_chunk_data& sub_chunk_data::operator=(sub_chunk_data&& other) {
        biomes = other.biomes;
        blocks = other.blocks;
        memcpy(&sky_light, &other.sky_light, sizeof(sky_light));
        memcpy(&block_light, &other.block_light, sizeof(block_light));
        block_entities = std::move(other.block_entities);
        active_blocks = other.active_blocks;
        has_tickable_blocks = other.has_tickable_blocks;
        need_to_recalculate_light = other.need_to_recalculate_light;
        sky_lighted = other.sky_lighted;
        block_lighted = other.block_lighted;
        return *this;
    }

    api::ecs::entity sub_chunk_data::get_block_entity(uint8_t local_x, uint8_t local_y, uint8_t local_z) {
        return block_entities.at(local_z | (local_y << 4) | (local_x << 8));
    }

    void sub_chunk_data::get_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, std::function<void(base_objects::block block)> on_normal, std::function<void(api::ecs::entity block_entity)> on_entity) {
        auto block = (base_objects::block)blocks.get(local_x, local_y, local_z);
        if (block.is_block_entity())
            on_entity(block_entities[local_z | (local_y << 4) | (local_x << 8)]);
        else
            on_normal(block);
    }

    base_objects::block sub_chunk_data::get_block(uint8_t local_x, uint8_t local_y, uint8_t local_z) {
        return (base_objects::block)blocks.get(local_x, local_y, local_z);
    }

    void sub_chunk_data::set_state(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block_id_t state, api::ecs::world_local_registry& world) {
        bool prev_active = !base_objects::block(blocks.get(local_x, local_y, local_z)).is_air();
        set_state_gen(local_x, local_y, local_z, state, world);
        bool now_active = !base_objects::block(blocks.get(local_x, local_y, local_z)).is_air();
        if (prev_active && !now_active)
            --active_blocks;
        else if (!prev_active && now_active)
            ++active_blocks;
    }

    void sub_chunk_data::set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block, api::ecs::world_local_registry& world) {
        bool prev_active = !base_objects::block(blocks.get(local_x, local_y, local_z)).is_air();
        set_block_gen(local_x, local_y, local_z, block, world);
        bool now_active = !base_objects::block(blocks.get(local_x, local_y, local_z)).is_air();
        if (prev_active && !now_active)
            --active_blocks;
        else if (!prev_active && now_active)
            ++active_blocks;
    }

    void sub_chunk_data::set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, api::ecs::entity&& block, api::ecs::world_local_registry& world) {
        bool prev_active = !base_objects::block(blocks.get(local_x, local_y, local_z)).is_air();
        set_block_gen(local_x, local_y, local_z, std::move(block), world);

        bool now_active = !base_objects::block(blocks.get(local_x, local_y, local_z)).is_air();
        if (prev_active && !now_active)
            --active_blocks;
        else if (!prev_active && now_active)
            ++active_blocks;
    }

    void sub_chunk_data::set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, const api::ecs::entity& block, api::ecs::world_local_registry& world) {
        auto res = block.copy_and_wait();
        if (!res)
            throw std::runtime_error("Failed to copy block_entity");
        set_block(local_x, local_y, local_z, std::move(*res), world);
    }

    void sub_chunk_data::set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, const base_objects::any_block& block, api::ecs::world_local_registry& world) {
        std::visit(
            [&](auto& block) {
                set_block(local_x, local_y, local_z, block, world);
            },
            block
        );
    }

    void sub_chunk_data::set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::any_block&& block, api::ecs::world_local_registry& world) {
        std::visit(
            [&](auto& block) {
                set_block(local_x, local_y, local_z, std::move(block), world);
            },
            block
        );
    }

    void sub_chunk_data::set_block_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, const api::ecs::entity& block, api::ecs::world_local_registry& world) {
        auto res = block.copy_and_wait();
        if (!res)
            throw std::runtime_error("Failed to copy block_entity");
        set_block_gen(local_x, local_y, local_z, std::move(*res), world);
    }

    void sub_chunk_data::set_block_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, api::ecs::entity&& block, api::ecs::world_local_registry& world) {
        if (!block.has<api::ecs::com::block_entity_tag>() || !block.has<api::ecs::com::block_entity::block_id>())
            throw std::runtime_error("Expected block entity, received other kind.");
        if (!world.transfer_entity_and_block(block))
            throw std::runtime_error("Failed to assign the entity to the new world.");

        blocks.set(local_x, local_y, local_z, block.get<api::ecs::com::block_entity::block_id>().id);
        auto it = block_entities.find(local_z | (local_y << 4) | (local_x << 8));
        if (it != block_entities.end()) {
            it->second.add<api::ecs::com::dead_mark>();
            it->second = block;
        } else
            block_entities[local_z | (local_y << 4) | (local_x << 8)] = block;
    }

    void sub_chunk_data::set_state_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block_id_t block, api::ecs::world_local_registry& world) {
        base_objects::block b(block);

        if (!b.is_block_entity()) {
            set_block_gen(local_x, local_y, local_z, b, world);
        } else {
            auto e_id_m = get_block_entity(local_x, local_y, local_z).modify<api::ecs::com::block_entity::block_id>();
            base_objects::block ee(e_id_m->id);
            if (ee.block_entity_id() != b.block_entity_id()) {
                set_block_gen(local_x, local_y, local_z, b, world);
            } else {
                e_id_m->id = block;
                blocks.set(local_x, local_y, local_z, block);
            }
        }
    }

    void sub_chunk_data::set_block_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block, api::ecs::world_local_registry& world) {
        blocks.set(local_x, local_y, local_z, block.id);
        auto it = block_entities.find(local_z | (local_y << 4) | (local_x << 8));
        if (it != block_entities.end()) {
            it->second.add<api::ecs::com::dead_mark>();
            block_entities.erase(it);
        }
        if (block.is_block_entity()) {
            api::ecs::entity_construction construct;
            construct.template set<api::ecs::com::block_entity_tag>();
            construct.template emplace<api::ecs::com::block_entity::block_id>(block.id);
            auto entity_dat = std::move(construct).create_and_wait(api::ecs::get_block_entity_definition(block.name()).get_recipe(), world.get_ecs_world_ref());
            block_entities[local_z | (local_y << 4) | (local_x << 8)] = entity_dat;
        }
    }

    int32_t sub_chunk_data::get_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z) {
        return biomes.get(local_x, local_y, local_z);
    }

    void sub_chunk_data::set_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z, int32_t id) {
        biomes.set(local_x, local_y, local_z, id);
    }

    void sub_chunk_data::for_each_block(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block)> func) const {
        for (uint8_t x = 0; x < 16; x++)
            for (uint8_t y = 0; y < 16; y++)
                for (uint8_t z = 0; z < 16; z++)
                    func(x, y, z, (base_objects::block)blocks.get(x, y, z));
    }

    void sub_chunk_data::for_each_block_entity(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, api::ecs::entity block_entity)> func) const {
        for (auto& [pos, data] : block_entities) {
            auto local_z = uint8_t(pos & 0xF);
            auto local_y = uint8_t((pos >> 4) & 0xF);
            auto local_x = uint8_t((pos >> 8) & 0xF);
            //TODO add check for id (base_objects::block)blocks.get(local_x, local_y, local_z)
            func(local_x, local_y, local_z, data);
        }
    }

    void sub_chunk_data::for_each_block(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block)> func) {
        for (uint8_t x = 0; x < 16; x++)
            for (uint8_t y = 0; y < 16; y++)
                for (uint8_t z = 0; z < 16; z++)
                    func(x, y, z, (base_objects::block)blocks.get(x, y, z));
    }

    void sub_chunk_data::for_each_block_entity(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, api::ecs::entity block_entity)> func) {
        for (auto& [pos, data] : block_entities) {
            auto local_z = uint8_t(pos & 0xF);
            auto local_y = uint8_t((pos >> 4) & 0xF);
            auto local_x = uint8_t((pos >> 8) & 0xF);
            //TODO add check for id (base_objects::block)blocks.get(local_x, local_y, local_z)
            func(local_x, local_y, local_z, data);
        }
    }
}