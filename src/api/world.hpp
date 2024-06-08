#ifndef SRC_API_WORLD
#define SRC_API_WORLD

#include "../ClientHandleHelper.hpp"
#include "../storage/world_data.hpp"
#include <functional>

namespace crafted_craft {
    namespace api {
        struct world {
            world(TCPserver* server_ref);

            void load_world(uint64_t world_id, std::function<void()> callback = nullptr);
            void unload_world(uint64_t world_id, std::function<void()> callback = nullptr);
            void save_world(uint64_t world_id, std::function<void()> callback = nullptr);
            void save_all_worlds(std::function<void()> callback = nullptr);

            void load_chunk(uint64_t world_id, int64_t x, int64_t z, std::function<void(const storage::chunk_data& data)> callback = nullptr);
            void unload_chunk(uint64_t world_id, int64_t x, int64_t z, std::function<void()> callback = nullptr);

            void set_block(uint64_t world_id, int64_t x, int64_t y, int64_t z, base_objects::block block, std::function<void()> callback = nullptr);
            void set_block_range(uint64_t world_id, int64_t x1, int64_t y1, int64_t z1, int64_t x2, int64_t y2, int64_t z2, base_objects::block block, std::function<void()> callback = nullptr);
            void set_block_range(uint64_t world_id, int64_t x1, int64_t y1, int64_t z1, int64_t x2, int64_t y2, int64_t z2, std::function<base_objects::block(int64_t x, int64_t y, int64_t z, base_objects::block block)> callback = nullptr);
            void set_block_range(uint64_t world_id, int64_t x1, int64_t y1, int64_t z1, int64_t x2, int64_t y2, int64_t z2, list_array<base_objects::block> blocks, std::function<void()> callback = nullptr);

            void set_block_entity(uint64_t world_id, int64_t x, int64_t y, int64_t z, base_objects::block_entity entity, std::function<void()> callback = nullptr);
            void set_block_entity(uint64_t world_id, int64_t x, int64_t y, int64_t z, std::function<base_objects::block_entity(int64_t x, int64_t y, int64_t z, base_objects::block_entity entity)> callback = nullptr);
            void set_block_entity(uint64_t world_id, int64_t x, int64_t y, int64_t z, list_array<base_objects::block_entity> entities, std::function<void()> callback = nullptr);

            void get_block(uint64_t world_id, int64_t x, int64_t y, int64_t z, std::function<void(base_objects::block block)> callback);
            void get_chunk(uint64_t world_id, int64_t x, int64_t z, std::function<void(const storage::chunk_data& data)> callback);
            void get_block_entity(uint64_t world_id, int64_t x, int64_t y, int64_t z, std::function<void(base_objects::block_entity entity)> callback);
            void get_block_entities(uint64_t world_id, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, std::function<void(list_array<base_objects::block_entity> entities)> callback);

            void iterate_worlds(std::function<void(uint64_t world, storage::world_data& data)> callback);
            void get_world(uint64_t world_id, std::function<void(storage::world_data& data)> callback);

        private:
            TCPserver* server;
        };
    }
}


#endif /* SRC_API_WORLD */
