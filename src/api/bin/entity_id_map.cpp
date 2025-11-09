/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <library/enbt/enbt.hpp>
#include <library/fast_task.hpp>
#include <memory>
#include <src/api/ecs.hpp>
#include <src/api/selector.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/uuid.hpp>

namespace copper_server::api::entity_id_map {
    struct id_s {
        std::vector<int32_t> id;
        base_objects::uuid uuid;
        api::ecs::entity assigned_entity;
    };

    using id_sp = std::shared_ptr<id_s>;

    std::unordered_map<int32_t, id_sp> ids_l;
    std::unordered_map<base_objects::uuid, id_sp> ids_r;
    fast_task::task_mutex mutex;
    int32_t id_allocator = 0;

    int32_t id_increment() {
        auto res = id_allocator++;
        if (id_allocator == INT32_MAX)
            id_allocator = 0;
        return res;
    }

    uint8_t allocate_special(uint8_t required_ids) {
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

    std::pair<int32_t, base_objects::uuid> allocate_id() {
        base_objects::uuid uuid = base_objects::uuid::generate_v4();
        std::unique_lock lock(mutex);
        if (ids_l.size() == INT32_MAX)
            throw std::runtime_error("Too many registered UUID's, can't allocate more");
        while (ids_r.find(uuid) != ids_r.end())
            uuid = base_objects::uuid::generate_v4();
        while (ids_l.find(id_allocator) != ids_l.end())
            id_increment();
        auto id = std::make_shared<id_s>(std::vector<int32_t>{id_allocator}, uuid);
        ids_l[id_allocator] = id;
        ids_r[uuid] = id;
        return {id_increment(), uuid};
    }

    std::pair<int32_t, base_objects::uuid> allocate_special_sequence(uint8_t required_ids) {
        base_objects::uuid uuid = base_objects::uuid::generate_v4();
        std::unique_lock lock(mutex);
        if (ids_l.size() == INT32_MAX)
            throw std::runtime_error("Too many registered UUID's, can't allocate more");
        while (ids_r.find(uuid) != ids_r.end())
            uuid = base_objects::uuid::generate_v4();
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

    int32_t allocate_id(const base_objects::uuid& uuid) {
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

    int32_t allocate_special_sequence(const base_objects::uuid& uuid, uint8_t required_ids) {
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

    void remove_id(int32_t id) {
        std::unique_lock lock(mutex);
        if (auto it = ids_l.find(id); it != ids_l.end()) {
            auto id_ptr = it->second;
            ids_l.erase(it);
            ids_r.erase(id_ptr->uuid);
            for (auto& rem_id : id_ptr->id)
                ids_l.erase(rem_id);
        }
    }

    int32_t remove_id(const base_objects::uuid& uuid) {
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

    int32_t get_id(const base_objects::uuid& uuid) {
        std::unique_lock lock(mutex);
        auto it = ids_r.find(uuid);
        if (it == ids_r.end())
            return -1;
        return it->second->id[0];
    }

    base_objects::uuid get_uuid(int32_t id) {
        std::unique_lock lock(mutex);
        auto it = ids_l.find(id);
        if (it == ids_l.end())
            return base_objects::uuid();
        return it->second->uuid;
    }

    void assign_entity(int32_t id, api::ecs::entity entity) {
        std::unique_lock lock(mutex);
        auto it = ids_l.find(id);
        if (it == ids_l.end())
            throw std::runtime_error("ID not found");
        it->second->assigned_entity = entity;
    }

    void assign_entity(const base_objects::uuid& uuid, api::ecs::entity entity) {
        std::unique_lock lock(mutex);
        auto it = ids_r.find(uuid);
        if (it == ids_r.end())
            throw std::runtime_error("UUID not found");
        it->second->assigned_entity = entity;
    }

    std::optional<api::ecs::entity> get_entity(int32_t id) {
        std::unique_lock lock(mutex);
        auto it = ids_l.find(id);
        if (it == ids_l.end())
            return std::nullopt;
        return it->second->assigned_entity;
    }

    std::optional<api::ecs::entity> get_entity(const base_objects::uuid& id) {
        std::unique_lock lock(mutex);
        auto it = ids_r.find(id);
        if (it == ids_r.end())
            return std::nullopt;
        return it->second->assigned_entity;
    }

    bool has_id(int32_t id) {
        std::unique_lock lock(mutex);
        return ids_l.contains(id);
    }

    bool has_uuid(const base_objects::uuid& uuid) {
        std::unique_lock lock(mutex);
        return ids_r.contains(uuid);
    }

    list_array<int32_t> query_ids() {
        list_array<int32_t> res;
        std::unique_lock lock(mutex);
        res.reserve(ids_l.size());
        for (auto& [id, data] : ids_l)
            res.push_back(id);
        return res;
    }

    uint8_t id_index(int32_t id) {
        std::unique_lock lock(mutex);
        auto& id_range = ids_l.at(id)->id;
        uint8_t i = 0;
        while (i != UINT8_MAX)
            if (id_range[i] == id)
                return i;
            else
                ++i;
        throw std::runtime_error("Id not found");
    }

    void apply_selector(base_objects::shared_client_data& caller, const std::string& selector, std::function<void(api::ecs::entity)>&& callback) {
        api::selector sel;
        sel.build_selector(selector);
        base_objects::command_context context(caller, true);
        sel.flags.only_players = true;
        sel.flags.only_entities = false;
        sel.select(context, std::move(callback));
    }
}
