/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_ENTITY_ID_MAP
#define SRC_API_ENTITY_ID_MAP
#include <functional>
#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>
#include <src/api/ecs.hpp>
#include <src/base_objects/atomic_holder.hpp>

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
    /*nodiscard*/ void assign_entity(int32_t id, api::ecs::entity entity);
    /*nodiscard*/ void assign_entity(const enbt::raw_uuid& uuid, api::ecs::entity entity);
    [[nodiscard]] std::optional<api::ecs::entity> get_entity(int32_t id);
    [[nodiscard]] std::optional<api::ecs::entity> get_entity(const enbt::raw_uuid& uuid);
    [[nodiscard]] bool has_id(int32_t id);
    [[nodiscard]] bool has_uuid(const enbt::raw_uuid& uuid);
    [[nodiscard]] list_array<int32_t> query_ids();
    [[nodiscard]] uint8_t id_index(int32_t id); //gets index of allocated id

    void apply_selector(base_objects::shared_client_data& caller, const std::string& selector, std::function<void(api::ecs::entity)>&& callback);
}

#endif /* SRC_API_ENTITY_ID_MAP */
