#ifndef SRC_BASE_OBJECTS_WORLD_CHUNK_REGION
#define SRC_BASE_OBJECTS_WORLD_CHUNK_REGION
#include <src/base_objects/world/chunk.hpp>
#include <array>
#include <memory>
#include <bitset>
#include <library/fast_task/include/task.hpp>

namespace copper_server::base_objects::world {
    struct chunk_region {
        static constexpr inline size_t chunk_count = 32*32;

        std::array<std::shared_ptr<chunk_data>, chunk_count> chunks;
        std::bitset<chunk_count> active_mask;
        std::bitset<chunk_count> dirty_mask;
        std::bitset<chunk_count> light_ready_mask;

        mutable fast_task::task_mutex mutex;

        const std::shared_ptr<chunk_data>& get_generator_unsafe(int32_t local_x, int32_t local_z) const;

        const std::shared_ptr<chunk_data>& get_unsafe(int32_t local_x, int32_t local_z) const;

        const std::shared_ptr<chunk_data>& get(int32_t local_x, int32_t local_z) const;

        void set(int32_t local_x, int32_t local_z, std::shared_ptr<chunk_data>&& chunk);

        void set(int32_t local_x, int32_t local_z, const std::shared_ptr<chunk_data>& chunk);

        void mark_dirty(int32_t local_x, int32_t local_z);

        void mark_clear(int32_t local_x, int32_t local_z);

        void mark_light_ready(int32_t local_x, int32_t local_z);

        void mark_active(int32_t local_x, int32_t local_z);

        void unload(int32_t local_x, int32_t local_z);

        //could be fully unloaded, has unloadable items
        std::pair<bool, bool> could_be_unloaded() const noexcept;
    };
}

#endif /* SRC_BASE_OBJECTS_WORLD_CHUNK_REGION */
