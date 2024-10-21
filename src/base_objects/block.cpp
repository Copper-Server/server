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
                tickable = static_data.resolve_tickable();
                goto retry;
                break;
            default:
            case tick_opt::no_tick:
                break;
            }
        }

        block::tick_opt block::resolve_tickable(base_objects::block_id_t block_id) {
            return base_objects::block(block_id).getStaticData().resolve_tickable();
        }

        bool block::is_tickable() {
            switch (tickable) {
            case tick_opt::block_tickable:
            case tick_opt::entity_tickable:
                return true;
            case tick_opt::undefined:
                tickable = getStaticData().resolve_tickable();
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

        void static_block_data::reset_blocks() {
            block::access_full_block_data([](auto& i0, auto& i1) {
                i0.clear();
                i1.clear();
            });
        }

        void static_block_data::initialize_blocks() {
            block::access_full_block_data([&](std::vector<std::shared_ptr<static_block_data>>& arr, auto& ignored) {
                block_id_t id;
                internal_block_aliases.clear();
                for (auto& _block : arr) {
                    auto& block = *_block;
                    auto& local_aliases = internal_block_aliases[id];
                    for (auto& [protocol, assignations] : internal_block_aliases_protocol) {
                        if (assignations.find(block.name.get()) != assignations.end()) {
                            local_aliases[protocol] = assignations[block.name.get()];
                        } else {
                            bool found = false;
                            for (auto& alias : block.block_aliases) {
                                if (assignations.find(alias.get()) != assignations.end()) {
                                    local_aliases[protocol] = assignations[alias.get()];
                                    found = true;
                                    break;
                                }
                            }
                            if (!found)
                                throw std::runtime_error("Block alias for " + block.name.get() + '[' + std::to_string(id) + " not found in protocol " + std::to_string(protocol));
                        }
                    }
                    ++id;
                }
            });
        }


    } // namespace base_objects
}
