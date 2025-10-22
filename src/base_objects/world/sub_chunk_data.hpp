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
#include <cstdint>
#include <functional>
#include <boost/unordered/unordered_flat_map.hpp>

#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>

#include <src/base_objects/block.hpp>
#include <src/base_objects/palette_container.hpp>
#include <src/base_objects/world/light_data.hpp>

namespace copper_server::base_objects {
    namespace world {
        struct sub_chunk_data {
            base_objects::block blocks[16][16][16];
            int32_t biomes[4][4][4];
            boost::unordered_flat_map<uint16_t, enbt::value> block_entities; //0xXYZ => block_entity


            base_objects::world::light_data sky_light;
            base_objects::world::light_data block_light;
            mutable std::unique_ptr<base_objects::palette_container_block> block_palette;
            mutable std::unique_ptr<base_objects::palette_container_biome> biome_palette;

            uint16_t active_blocks = 0; //if zero, the sub chunk is not rendered for clients
            bool has_tickable_blocks = false;
            bool need_to_recalculate_light = false;
            bool sky_lighted = false;   //set true if at least one block is lighted in this sub_chunk
            bool block_lighted = false; //set true if at least one block is lighted in this sub_chunk

            sub_chunk_data();
            sub_chunk_data(sub_chunk_data&&);
            ~sub_chunk_data();

            sub_chunk_data& operator=(sub_chunk_data&&);


            enbt::value& get_block_entity_data(uint8_t local_x, uint8_t local_y, uint8_t local_z);
            void get_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, std::function<void(base_objects::block& block)> on_normal, std::function<void(base_objects::block& block, enbt::value& entity_data)> on_entity);
            void set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, const base_objects::full_block_data& block);
            void set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::full_block_data&& block);
            void set_block_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, const base_objects::block_entity& block);
            void set_block_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block_entity&& block);
            void set_block_gen(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block);
            int32_t get_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z);
            void set_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z, int32_t id);
            void for_each_block(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block& block)> func);
            void for_each_block_entity(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block& block, enbt::value& entity_data)> func);

            void for_each_block(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block)> func) const;
            void for_each_block_entity(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block, const enbt::value& entity_data)> func) const;

            const base_objects::palette_container_block& get_block_pallete() const;
            const base_objects::palette_container_biome& get_biome_pallete() const;
        };
    }
}
#endif /* SRC_BASE_OBJECTS_WORLD_SUB_CHUNK_DATA */
