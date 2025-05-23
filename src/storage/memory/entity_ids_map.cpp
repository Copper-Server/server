#include <random>
#include <src/storage/memory/entity_ids_map.hpp>

namespace copper_server::storage::memory {
    int32_t entity_ids_map_storage::id_increment(){
        auto res = id_allocator++;
        if (id_allocator == INT32_MAX)
            id_allocator = 0;
        return res;
    }

    uint8_t entity_ids_map_storage::allocate_special(uint8_t required_ids) {
        uint8_t successfully_allocated = 0;
        auto old_id_allocator = id_allocator;
        while (ids_l.find(id_allocator) != ids_l.end())
            id_increment();

        while (successfully_allocated != required_ids && ids_l.find(id_allocator) == ids_l.end() && old_id_allocator < id_allocator) {
            id_increment();
            successfully_allocated++;
        }
        return successfully_allocated;
    }

    [[nodiscard]] std::pair<int32_t, enbt::raw_uuid> entity_ids_map_storage::allocate_id() {
        enbt::raw_uuid uuid = enbt::raw_uuid::generate_v4();
        std::unique_lock lock(mutex);
        if (ids_l.size() == INT32_MAX)
            throw std::runtime_error("Too many registered UUID's, can't allocate more");
        while (ids_r.find(uuid) != ids_r.end())
            uuid = enbt::raw_uuid::generate_v4();
        while (ids_l.find(id_allocator) != ids_l.end())
            id_increment();
        auto id = std::make_shared<id_s>(std::vector<int32_t>{id_allocator}, uuid);
        ids_l[id_allocator] = id;
        ids_r[uuid] = id;
        return {id_increment(), uuid};
    }

    [[nodiscard]] std::pair<int32_t, enbt::raw_uuid> entity_ids_map_storage::allocate_special_sequence(uint8_t required_ids) {
        enbt::raw_uuid uuid = enbt::raw_uuid::generate_v4();
        std::unique_lock lock(mutex);
        if (ids_l.size() == INT32_MAX)
            throw std::runtime_error("Too many registered UUID's, can't allocate more");
        while (ids_r.find(uuid) != ids_r.end())
            uuid = enbt::raw_uuid::generate_v4();
        auto id = std::make_shared<id_s>();
        while (allocate_special(required_ids) != required_ids)
            ;
        id->uuid = uuid;
        id->id.reserve(required_ids);
        auto id_off = id_allocator - required_ids;
        for (uint16_t i = 0; i < required_ids; i++) {
            id->id.push_back(id_off + i);
            ids_l[id_off + i] = id;
        }
        ids_r[uuid] = id;
        return {id_off, uuid};
    }

    [[nodiscard]] int32_t entity_ids_map_storage::allocate_id(const enbt::raw_uuid& uuid) {
        std::unique_lock lock(mutex);
        if (ids_l.size() == INT32_MAX)
            throw std::runtime_error("Too many registered UUID's, can't allocate more");
        if (ids_r.find(uuid) != ids_r.end())
            throw std::invalid_argument("UUID already registered");
        while (ids_l.find(id_allocator) != ids_l.end())
            id_increment();
        auto id = std::make_shared<id_s>(std::vector<int32_t>{id_allocator}, uuid);
        ids_l[id_allocator] = id;
        ids_r[uuid] = id;
        return id_increment();
    }

    [[nodiscard]] int32_t entity_ids_map_storage::allocate_special_sequence(const enbt::raw_uuid& uuid, uint8_t required_ids) {
        std::unique_lock lock(mutex);
        if (ids_l.size() == INT32_MAX)
            throw std::runtime_error("Too many registered UUID's, can't allocate more");
        if (ids_r.find(uuid) != ids_r.end())
            throw std::invalid_argument("UUID already registered");
        while (allocate_special(required_ids) != required_ids)
            ;
        auto id = std::make_shared<id_s>();
        id->uuid = uuid;
        id->id.reserve(required_ids);
        auto id_off = id_allocator - required_ids;
        for (uint16_t i = 0; i < required_ids; i++) {
            id->id.push_back(id_off + i);
            ids_l[id_off + i] = id;
        }
        ids_r[uuid] = id;
        return id_off;
    }

    void entity_ids_map_storage::remove_id(int32_t id) {
        std::unique_lock lock(mutex);
        if (auto it = ids_l.find(id); it != ids_l.end()) {
            auto id_ptr = it->second;
            ids_l.erase(it);
            ids_r.erase(id_ptr->uuid);
            for (auto& id : id_ptr->id)
                ids_l.erase(id);
        }
    }

    [[nodiscard]] int32_t entity_ids_map_storage::remove_id(const enbt::raw_uuid& uuid) {
        std::unique_lock lock(mutex);
        if (auto it = ids_r.find(uuid); it != ids_r.end()) {
            auto id_ptr = it->second;
            ids_r.erase(it);
            ids_l.erase(id_ptr->id[0]);
            for (auto& id : id_ptr->id)
                ids_l.erase(id);
            return id_ptr->id[0];
        }
        return -1;
    }

    [[nodiscard]] int32_t entity_ids_map_storage::get_id(const enbt::raw_uuid& uuid) {
        std::unique_lock lock(mutex);
        auto it = ids_r.find(uuid);
        if (it == ids_r.end())
            return -1;
        return it->second->id[0];
    }

    [[nodiscard]] enbt::raw_uuid entity_ids_map_storage::get_uuid(int32_t id) {
        std::unique_lock lock(mutex);
        auto it = ids_l.find(id);
        if (it == ids_l.end())
            return enbt::raw_uuid();
        return it->second->uuid;
    }

    void entity_ids_map_storage::assign_entity(int32_t id, base_objects::entity_ref entity) {
        std::unique_lock lock(mutex);
        auto it = ids_l.find(id);
        if (it == ids_l.end())
            throw std::runtime_error("ID not found");
        it->second->assigned_entity = entity;
    }

    void entity_ids_map_storage::assign_entity(const enbt::raw_uuid& uuid, base_objects::entity_ref entity) {
        std::unique_lock lock(mutex);
        auto it = ids_r.find(uuid);
        if (it == ids_r.end())
            throw std::runtime_error("UUID not found");
        it->second->assigned_entity = entity;
    }

    [[nodiscard]] base_objects::entity_ref entity_ids_map_storage::get_entity(int32_t id) {
        std::unique_lock lock(mutex);
        auto it = ids_l.find(id);
        if (it == ids_l.end())
            return nullptr;
        return it->second->assigned_entity;
    }

    [[nodiscard]] base_objects::entity_ref entity_ids_map_storage::get_entity(const enbt::raw_uuid& id) {
        std::unique_lock lock(mutex);
        auto it = ids_r.find(id);
        if (it == ids_r.end())
            return nullptr;
        return it->second->assigned_entity;
    }

    [[nodiscard]] bool entity_ids_map_storage::has_id(int32_t id) {
        std::unique_lock lock(mutex);
        return ids_l.contains(id);
    }

    [[nodiscard]] bool entity_ids_map_storage::has_uuid(const enbt::raw_uuid& uuid) {
        std::unique_lock lock(mutex);
        return ids_r.contains(uuid);
    }
}