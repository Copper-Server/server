#include "block.hpp"
#include "../storage/world_data.hpp"

namespace crafted_craft {
    namespace base_objects {
        std::vector<std::shared_ptr<static_block_data>> block::full_block_data_;
        std::unordered_map<shared_string, std::shared_ptr<static_block_data>> block::named_full_block_data;

        void block::tick(storage::world_data& world, storage::sub_chunk_data& sub_chunk, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z, bool random_ticked) {
        retry:
            auto& static_data = getStaticData();
            switch (tickable) {
            case tick_opt::block_tickable:
                static_data.on_tick(world, sub_chunk, *this, chunk_x, sub_chunk_y, chunk_z, local_x, local_y, local_z, random_ticked);
                return;
            case tick_opt::entity_tickable:
                static_data.as_entity_on_tick(world, sub_chunk, *this, sub_chunk.get_block_entity_data(local_x, local_y, local_z), chunk_x, sub_chunk_y, chunk_z, local_x, local_y, local_z, random_ticked);
                return;
            case tick_opt::undefined:
                tickable = resolve_tickable(id);
                goto retry;
                break;
            default:
            case tick_opt::no_tick:
                break;
            }
        }

        block::tick_opt block::resolve_tickable(base_objects::block_id_t block_id) {
            auto& staic_data = base_objects::block(block_id).getStaticData();
            if (staic_data.on_tick)
                return tick_opt::block_tickable;
            if (staic_data.as_entity_on_tick)
                return tick_opt::entity_tickable;
            return tick_opt::no_tick;
        }

        bool block::is_tickable() {
            switch (tickable) {
            case tick_opt::block_tickable:
            case tick_opt::entity_tickable:
                return true;
            case tick_opt::undefined:
                tickable = resolve_tickable(id);
                return tickable != tick_opt::no_tick;
            default:
            case tick_opt::no_tick:
                return false;
            }
        }

        bool block::is_tickable() const {
            switch (tickable) {
            case tick_opt::block_tickable:
            case tick_opt::entity_tickable:
            case tick_opt::undefined:
                return true;
            default:
            case tick_opt::no_tick:
                return false;
            }
        }

        void block::initialize() {
            static bool inited = false;
            if (inited)
                return;
            inited = true;
            auto& air = full_block_data_.emplace_back();
        }

    } // namespace base_objects
}
