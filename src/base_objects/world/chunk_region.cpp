#include <src/base_objects/world/chunk_region.hpp>

namespace copper_server::base_objects::world {

    static constexpr size_t WIDTH_SHIFT = 5;
    static constexpr size_t MASK = 31; // 31

    const std::shared_ptr<chunk_data>& chunk_region::get_generator_unsafe(int32_t local_x, int32_t local_z) const {
        size_t idx = (static_cast<uint8_t>(local_x) & MASK) | ((static_cast<uint8_t>(local_z) & MASK) << WIDTH_SHIFT);
        return chunks[idx];
    }

    const std::shared_ptr<chunk_data>& chunk_region::get_unsafe(int32_t local_x, int32_t local_z) const {
        size_t idx = (static_cast<uint8_t>(local_x) & MASK) | ((static_cast<uint8_t>(local_z) & MASK) << WIDTH_SHIFT);
        return active_mask.test(idx) ? chunks[idx] : nullptr;
    }

    const std::shared_ptr<chunk_data>& chunk_region::get(int32_t local_x, int32_t local_z) const {
        std::lock_guard<fast_task::task_mutex> lock(mutex);
        return get_unsafe(local_x, local_z);
    }

    void chunk_region::set(int32_t local_x, int32_t local_z, std::shared_ptr<chunk_data>&& chunk) {
        size_t idx = (static_cast<uint8_t>(local_x) & MASK) | ((static_cast<uint8_t>(local_z) & MASK) << WIDTH_SHIFT);
        std::lock_guard<fast_task::task_mutex> lock(mutex);

        if (chunk) {
            chunks[idx] = chunk;
            active_mask.reset(idx);
            dirty_mask.reset(idx);
            light_ready_mask.reset(idx);
        } else {
            chunks[idx].reset();
            active_mask.reset(idx);
        }
    }

    void chunk_region::set(int32_t local_x, int32_t local_z, const std::shared_ptr<chunk_data>& chunk) {
        set(local_x, local_z, std::shared_ptr<chunk_data>(chunk));
    }

    void chunk_region::mark_dirty(int32_t local_x, int32_t local_z) {
        std::lock_guard<fast_task::task_mutex> lock(mutex);
        dirty_mask.set((static_cast<uint8_t>(local_x) & MASK) | ((static_cast<uint8_t>(local_z) & MASK) << WIDTH_SHIFT));
    }

    void chunk_region::mark_clear(int32_t local_x, int32_t local_z) {
        std::lock_guard<fast_task::task_mutex> lock(mutex);
        dirty_mask.reset((static_cast<uint8_t>(local_x) & MASK) | ((static_cast<uint8_t>(local_z) & MASK) << WIDTH_SHIFT));
    }

    void chunk_region::mark_light_ready(int32_t local_x, int32_t local_z) {
        std::lock_guard<fast_task::task_mutex> lock(mutex);
        light_ready_mask.set((static_cast<uint8_t>(local_x) & MASK) | ((static_cast<uint8_t>(local_z) & MASK) << WIDTH_SHIFT));
    }

    void chunk_region::mark_active(int32_t local_x, int32_t local_z) {
        std::lock_guard<fast_task::task_mutex> lock(mutex);
        active_mask.set((static_cast<uint8_t>(local_x) & MASK) | ((static_cast<uint8_t>(local_z) & MASK) << WIDTH_SHIFT));
    }

    void chunk_region::unload(int32_t local_x, int32_t local_z) {
        size_t idx = (static_cast<uint8_t>(local_x) & MASK) | ((static_cast<uint8_t>(local_z) & MASK) << WIDTH_SHIFT);
        std::lock_guard<fast_task::task_mutex> lock(mutex);
        chunks[idx].reset();
        active_mask.reset(idx);
    }

    //could be fully unloaded, has unloadable items
    std::pair<bool, bool> chunk_region::could_be_unloaded() const noexcept {
        bool has_unloadable = false;
        for(auto& chunk : chunks)
            if(chunk)
                if(!chunk->could_be_unloaded())
                    return {false, has_unloadable};
                else
                    has_unloadable = true;
        return {true, has_unloadable};
    }
}