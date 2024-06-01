#include "block.hpp"
#include "../storage/world_data.hpp"

namespace crafted_craft {
    namespace base_objects {
        void block_entity_tick_transfer(
            storage::world_data& world,
            storage::sub_chunk_data& sub_chunk,
            storage::block_data& data,
            int64_t chunk_x,
            uint64_t sub_chunk_y,
            int64_t chunk_z,
            uint8_t local_x,
            uint8_t local_y,
            uint8_t local_z
        ) {
            auto& block_entity = sub_chunk.block_entities[data.block_state_data];
            block_entity.block_id.getStaticData().as_entity_on_tick(
                world,
                sub_chunk,
                block_entity,
                chunk_x,
                sub_chunk_y,
                chunk_z,
                local_x,
                local_y,
                local_z
            );
        }

        void block::initialize() {
            static bool inited = false;
            if (inited)
                return;
            inited = true;
            auto& air = full_block_data_[0] = full_block_data(0, 0, 0, 0, 0, 0, 0, 0, 0);
            auto& block_entity = full_block_data_[0x7FFF] = full_block_data(1, 0, 0, 0, 0, 0, 0, 0, 0);
            block_adder = 1;
            tags["minecraft::air"].push_back(block(0));
            std::function fun = block_entity_tick_transfer;
            block_entity.on_tick = fun;
        }

    } // namespace base_objects

}