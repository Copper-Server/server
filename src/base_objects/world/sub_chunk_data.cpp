/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/registers.hpp>
#include <src/base_objects/world/sub_chunk_data.hpp>

namespace copper_server::base_objects::world {
    sub_chunk_data::sub_chunk_data() {
        memset(biomes, 0, sizeof(biomes));
        memset(blocks, 0, sizeof(blocks));
    }

    sub_chunk_data::~sub_chunk_data() {
    }

    sub_chunk_data::sub_chunk_data(sub_chunk_data&& other) {
        *this = std::move(other);
    }

    sub_chunk_data& sub_chunk_data::operator=(sub_chunk_data&& other) {
        memcpy(biomes, other.biomes, sizeof(biomes));
        memcpy(blocks, other.blocks, sizeof(blocks));
        memcpy(&sky_light, &other.sky_light, sizeof(sky_light));
        memcpy(&block_light, &other.block_light, sizeof(block_light));
        block_entities = std::move(other.block_entities);
        block_palette = std::move(other.block_palette);
        biome_palette = std::move(other.biome_palette);
        active_blocks = other.active_blocks;
        has_tickable_blocks = other.has_tickable_blocks;
        need_to_recalculate_light = other.need_to_recalculate_light;
        sky_lighted = other.sky_lighted;
        block_lighted = other.block_lighted;
        return *this;
    }


    enbt::value& sub_chunk_data::get_block_entity_data(uint8_t local_x, uint8_t local_y, uint8_t local_z) {
        return block_entities[local_z | (local_y << 4) | (local_x << 8)];
    }

    void sub_chunk_data::get_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, std::function<void(base_objects::block& block)> on_normal, std::function<void(base_objects::block& block, enbt::value& entity_data)> on_entity) {
        auto& block = blocks[local_y][local_x][local_z];
        if (block.is_block_entity())
            on_entity(block, get_block_entity_data(local_x, local_y, local_z));
        else
            on_normal(block);
    }

    void sub_chunk_data::set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, const base_objects::full_block_data& block) {
        bool prev_active = !blocks[local_x][local_y][local_z].is_air();
        std::visit(
            [&](auto& block) {
                using T = std::decay_t<decltype(block)>;
                if constexpr (std::is_same_v<T, base_objects::block>) {
                    blocks[local_y][local_x][local_z] = block;
                    block_entities.erase(local_z | (local_y << 4) | (local_x << 8));
                } else {
                    blocks[local_y][local_x][local_z] = block.block;
                    get_block_entity_data(local_x, local_y, local_z) = block.data;
                }
            },
            block
        );
        bool now_active = !blocks[local_x][local_y][local_z].is_air();
        if (prev_active && !now_active)
            --active_blocks;
        else if (!prev_active && now_active)
            ++active_blocks;
        block_palette.reset();
    }

    void sub_chunk_data::set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::full_block_data&& block) {
        bool prev_active = !blocks[local_x][local_y][local_z].is_air();
        std::visit(
            [&](auto& block) {
                using T = std::decay_t<decltype(block)>;
                if constexpr (std::is_same_v<T, base_objects::block>) {
                    blocks[local_y][local_x][local_z] = block;
                    block_entities.erase(local_z | (local_y << 4) | (local_x << 8));
                } else {
                    blocks[local_y][local_x][local_z] = block.block;
                    get_block_entity_data(local_x, local_y, local_z) = std::move(block.data);
                }
            },
            block
        );
        bool now_active = !blocks[local_y][local_x][local_z].is_air();
        if (prev_active && !now_active)
            --active_blocks;
        else if (!prev_active && now_active)
            ++active_blocks;
        block_palette.reset();
    }

    void sub_chunk_data::set_block_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, const base_objects::block_entity& block) {
        blocks[local_y][local_x][local_z] = block.block;
        get_block_entity_data(local_x, local_y, local_z) = block.data;
    }

    void sub_chunk_data::set_block_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block_entity&& block) {
        blocks[local_y][local_x][local_z] = block.block;
        get_block_entity_data(local_x, local_y, local_z) = std::move(block.data);
    }

    void sub_chunk_data::set_block_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block) {
        blocks[local_y][local_x][local_z] = block;
        block_entities.erase(local_z | (local_y << 4) | (local_x << 8));
    }

    int32_t sub_chunk_data::get_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z) {
        return biomes[2 >> local_y][2 >> local_z][2 >> local_z];
    }

    void sub_chunk_data::set_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z, int32_t id) {
        biomes[2 >> local_y][2 >> local_x][2 >> local_z] = id;
    }

    void sub_chunk_data::for_each_block(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block)> func) const {
        for (uint8_t x = 0; x < 16; x++)
            for (uint8_t y = 0; y < 16; y++)
                for (uint8_t z = 0; z < 16; z++)
                    func(x, y, z, blocks[y][x][z]);
    }

    void sub_chunk_data::for_each_block_entity(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block, const enbt::value& entity_data)> func) const {
        for (auto& [pos, data] : block_entities) {
            auto local_z = uint8_t(pos & 0xF);
            auto local_y = uint8_t((pos >> 4) & 0xF);
            auto local_x = uint8_t((pos >> 8) & 0xF);
            func(local_x, local_y, local_z, blocks[local_y][local_x][local_z], data);
        }
    }

    void sub_chunk_data::for_each_block(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block& block)> func) {
        for (uint8_t x = 0; x < 16; x++)
            for (uint8_t y = 0; y < 16; y++)
                for (uint8_t z = 0; z < 16; z++)
                    func(x, y, z, blocks[y][x][z]);
    }

    void sub_chunk_data::for_each_block_entity(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block& block, enbt::value& entity_data)> func) {
        for (auto& [pos, data] : block_entities) {
            auto local_z = uint8_t(pos & 0xF);
            auto local_y = uint8_t((pos >> 4) & 0xF);
            auto local_x = uint8_t((pos >> 8) & 0xF);
            func(local_x, local_y, local_z, blocks[local_y][local_x][local_z], data);
        }
    }

    const base_objects::palette_container_block& sub_chunk_data::get_block_pallete() const {
        if (!block_palette) {
            base_objects::palette_container_block blocks_(base_objects::block::block_states_size());
            //blocks_.reserve(4096);
            blocks_.add_range((int32_t*)blocks, 4096);
            //for (int i = 0; i < 4096; ++i) // y = i >> 8; x = (i >> 4) & 15; z = i & 15;
            //    blocks_.add(blocks[(i >> 4) & 15][i >> 8][i & 15].id);
            block_palette = std::make_unique<base_objects::palette_container_block>(std::move(blocks_));
        }
        return *block_palette;
    }

    const base_objects::palette_container_biome& sub_chunk_data::get_biome_pallete() const {
        if (!biome_palette) {
            base_objects::palette_container_biome biomes_(api::registers::biomes.size());
            //biomes_.reserve(64);
            biomes_.add_range((int32_t*)biomes, 64);
            //for (int i = 0; i < 64; ++i) // y = i >> 4; z = (i >> 2) & 3; x = i & 3;
            //    biomes_.add(biomes[i & 3][(i >> 2) & 3][i >> 4]);
            biome_palette = std::make_unique<base_objects::palette_container_biome>(std::move(biomes_));
        }
        return *biome_palette;
    }
}