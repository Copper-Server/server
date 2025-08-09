#include <src/base_objects/entity.hpp>
#include <src/storage/memory/entity_ids_map.hpp>

namespace copper_server::api::entity_id_map {
    storage::memory::entity_ids_map_storage mem;

    int32_t allocate_special_sequence(const enbt::raw_uuid& uuid, uint8_t required_ids) {
        return mem.allocate_special_sequence(uuid, required_ids);
    }

    std::pair<int32_t, enbt::raw_uuid> allocate_special_sequence(uint8_t required_ids) {
        return mem.allocate_special_sequence(required_ids);
    }

    std::pair<int32_t, enbt::raw_uuid> allocate_id() {
        return mem.allocate_id();
    }

    int32_t allocate_id(const enbt::raw_uuid& uuid) {
        return mem.allocate_id(uuid);
    }

    void remove_id(int32_t id) {
        return mem.remove_id(id);
    }

    int32_t remove_id(const enbt::raw_uuid& uuid) {
        return mem.remove_id(uuid);
    }

    /*nodiscard*/ void assign_entity(int32_t id, base_objects::entity_ref entity) {
        return mem.assign_entity(id, entity);
    }

    void assign_entity(const enbt::raw_uuid& uuid, base_objects::entity_ref entity) {
        return mem.assign_entity(uuid, entity);
    }

    base_objects::entity_ref get_entity(int32_t id) {
        return mem.get_entity(id);
    }

    base_objects::entity_ref get_entity(const enbt::raw_uuid& uuid) {
        return mem.get_entity(uuid);
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

    void apply_selector(base_objects::SharedClientData& caller, const std::string& selector, std::function<void(base_objects::entity&)>&& callback) {
        return mem.apply_selector(caller, selector, std::move(callback));
    }
}
