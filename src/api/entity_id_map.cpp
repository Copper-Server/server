#include <src/storage/memory/entity_ids_map.hpp>

namespace copper_server::api::entity_id_map {
    storage::memory::entity_ids_map_storage mem;

    std::pair<int32_t, enbt::raw_uuid> allocate_id() {
        return mem.allocate_id();
    }

    int32_t allocate_id(const enbt::raw_uuid& uuid) {
        return mem.allocate_id(uuid);
    }

    void remove_id(int32_t id) {
        mem.remove_id(id);
    }

    int32_t remove_id(const enbt::raw_uuid& uuid) {
        return mem.remove_id(uuid);
    }

    int32_t get_id(const enbt::raw_uuid& uuid) {
        return mem.get_id(uuid);
    }

    enbt::raw_uuid get_uuid(int32_t id) {
        return mem.get_uuid(id);
    }

    bool has_id(int32_t id) {
        return mem.has_id(id);
    }

    bool has_uuid(const enbt::raw_uuid& uuid) {
        return mem.has_uuid(uuid);
    }
}
