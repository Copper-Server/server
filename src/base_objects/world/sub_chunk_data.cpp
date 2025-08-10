/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/base_objects/entity.hpp>
#include <src/base_objects/world/sub_chunk_data.hpp>

namespace copper_server::base_objects::world {


    sub_chunk_data::sub_chunk_data() {
    }

    sub_chunk_data::~sub_chunk_data() {
    }

    enbt::value& sub_chunk_data::get_block_entity_data(uint8_t local_x, uint8_t local_y, uint8_t local_z) {
        return block_entities[local_z | (local_y << 4) | (local_x << 8)];
    }

    void sub_chunk_data::get_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, std::function<void(base_objects::block& block)> on_normal, std::function<void(base_objects::block& block, enbt::value& entity_data)> on_entity) {
        auto& block = blocks[local_x][local_y][local_z];
        if (block.is_block_entity())
            on_entity(block, get_block_entity_data(local_x, local_y, local_z));
        else
            on_normal(block);
    }

    void sub_chunk_data::set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, const base_objects::full_block_data& block) {
        std::visit(
            [&](auto& block) {
                using T = std::decay_t<decltype(block)>;
                if constexpr (std::is_same_v<T, base_objects::block>) {
                    blocks[local_x][local_y][local_z] = block;
                    block_entities.erase(local_z | (local_y << 4) | (local_x << 8));
                } else {
                    blocks[local_x][local_y][local_z] = block.block;
                    get_block_entity_data(local_x, local_y, local_z) = block.data;
                }
            },
            block
        );
    }

    void sub_chunk_data::set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::full_block_data&& block) {
        std::visit(
            [&](auto& block) {
                using T = std::decay_t<decltype(block)>;
                if constexpr (std::is_same_v<T, base_objects::block>) {
                    blocks[local_x][local_y][local_z] = block;
                    block_entities.erase(local_z | (local_y << 4) | (local_x << 8));
                } else {
                    blocks[local_x][local_y][local_z] = block.block;
                    get_block_entity_data(local_x, local_y, local_z) = std::move(block.data);
                }
            },
            block
        );
    }

    uint32_t sub_chunk_data::get_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z) {
        return biomes[2 >> local_x][2 >> local_y][2 >> local_z];
    }

    void sub_chunk_data::set_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z, uint32_t id) {
        biomes[2 >> local_x][2 >> local_y][2 >> local_z] = id;
    }

    void sub_chunk_data::for_each_block(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block)> func) const {
        for (uint8_t x = 0; x < 16; x++)
            for (uint8_t y = 0; y < 16; y++)
                for (uint8_t z = 0; z < 16; z++)
                    func(x, y, z, blocks[x][y][z]);
    }

    void sub_chunk_data::for_each_block_entity(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block, const enbt::value& entity_data)> func) const {
        for (auto& [pos, data] : block_entities) {
            auto local_z = uint8_t(pos & 0xF);
            auto local_y = uint8_t((pos >> 4) & 0xF);
            auto local_x = uint8_t((pos >> 8) & 0xF);
            func(local_x, local_y, local_z, blocks[local_x][local_y][local_z], data);
        }
    }

    void sub_chunk_data::for_each_block(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block& block)> func) {
        for (uint8_t x = 0; x < 16; x++)
            for (uint8_t y = 0; y < 16; y++)
                for (uint8_t z = 0; z < 16; z++)
                    func(x, y, z, blocks[x][y][z]);
    }

    void sub_chunk_data::for_each_block_entity(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block& block, enbt::value& entity_data)> func) {
        for (auto& [pos, data] : block_entities) {
            auto local_z = uint8_t(pos & 0xF);
            auto local_y = uint8_t((pos >> 4) & 0xF);
            auto local_x = uint8_t((pos >> 8) & 0xF);
            func(local_x, local_y, local_z, blocks[local_x][local_y][local_z], data);
        }
    }
}