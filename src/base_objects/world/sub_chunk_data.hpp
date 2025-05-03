#ifndef SRC_BASE_OBJECTS_WORLD_SUB_CHUNK
#define SRC_BASE_OBJECTS_WORLD_SUB_CHUNK
#include <cstdint>
#include <functional>
#include <unordered_map>

#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>

#include <src/base_objects/block.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/world/light_data.hpp>
namespace copper_server::base_objects::world {

    struct sub_chunk_data {
        base_objects::block blocks[16][16][16];
        uint32_t biomes[4][4][4];
        std::unordered_map<uint16_t, enbt::value> block_entities; //0xXYZ => block_entity
        list_array<base_objects::entity_ref> stored_entities;


        base_objects::world::light_data sky_light;
        base_objects::world::light_data block_light;
        bool has_tickable_blocks = false;
        bool need_to_recalculate_light = false;
        bool sky_lighted = false;   //set true if at least one block is lighted in this sub_chunk
        bool block_lighted = false; //set true if at least one block is lighted in this sub_chunk

        sub_chunk_data() {}

        enbt::value& get_block_entity_data(uint8_t local_x, uint8_t local_y, uint8_t local_z);
        void get_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, std::function<void(base_objects::block& block)> on_normal, std::function<void(base_objects::block& block, enbt::value& entity_data)> on_entity);
        void set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, const base_objects::full_block_data& block);
        void set_block(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::full_block_data&& block);
        uint32_t get_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z);
        void set_biome(uint8_t local_x, uint8_t local_y, uint8_t local_z, uint32_t id);
        void for_each_block(std::function<void(uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block& block)> func);
    };
}
#endif /* SRC_BASE_OBJECTS_WORLD_SUB_CHUNK */
