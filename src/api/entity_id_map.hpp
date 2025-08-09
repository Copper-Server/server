#ifndef SRC_API_ENTITY_ID_MAP
#define SRC_API_ENTITY_ID_MAP
#include <library/enbt/enbt.hpp>
#include <src/base_objects/atomic_holder.hpp>
#include <functional>

namespace copper_server::base_objects {
    struct entity;
    struct SharedClientData;
    using entity_ref = atomic_holder<entity>;
}

namespace copper_server::api::entity_id_map {
    //used for ender_dragon and other entities, when the client uses entity_id + offset to specify part of entity it has interacted
    [[nodiscard]] int32_t allocate_special_sequence(const enbt::raw_uuid& uuid, uint8_t required_ids);
    [[nodiscard]] std::pair<int32_t, enbt::raw_uuid> allocate_special_sequence(uint8_t required_ids);
    [[nodiscard]] std::pair<int32_t, enbt::raw_uuid> allocate_id();
    [[nodiscard]] int32_t allocate_id(const enbt::raw_uuid& uuid);
    /*nodiscard*/ void remove_id(int32_t id);
    [[nodiscard]] int32_t remove_id(const enbt::raw_uuid& uuid);
    [[nodiscard]] int32_t get_id(const enbt::raw_uuid& uuid);
    [[nodiscard]] enbt::raw_uuid get_uuid(int32_t id);
    /*nodiscard*/ void assign_entity(int32_t id, base_objects::entity_ref entity);
    /*nodiscard*/ void assign_entity(const enbt::raw_uuid& uuid, base_objects::entity_ref entity);
    [[nodiscard]] base_objects::entity_ref get_entity(int32_t id);
    [[nodiscard]] base_objects::entity_ref get_entity(const enbt::raw_uuid& uuid);
    [[nodiscard]] bool has_id(int32_t id);
    [[nodiscard]] bool has_uuid(const enbt::raw_uuid& uuid);
    void apply_selector(base_objects::SharedClientData& caller, const std::string& selector, std::function<void(base_objects::entity&)>&& callback);
}

#endif /* SRC_API_ENTITY_ID_MAP */
