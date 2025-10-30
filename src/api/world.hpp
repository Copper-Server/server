/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_WORLD
#define SRC_API_WORLD

#include <functional>
#include <memory>
#include <src/storage/world_data.hpp>

namespace copper_server::base_objects {
    struct shared_client_data;
    using client_data_holder = std::shared_ptr<shared_client_data>;
}

namespace copper_server::api::world {
    void unload(int32_t world_id);
    void unload_all();
    void save(int32_t world_id);
    void save_all();
    void save_and_unload(int32_t world_id);
    void save_and_unload_all();

    size_t loaded_chunks_count();
    size_t loaded_chunks_count(int32_t world_id);
    size_t loaded_chunks_count(const std::string& name);

    void iterate(std::function<void(int32_t id, storage::world_data& world)> callback);
    void get(int32_t world_id, std::function<void(storage::world_data& world)> callback);
    void get(const std::string& name, std::function<void(storage::world_data& world)> callback);
    int32_t resolve_id(const std::string& name);
    std::string resolve_name(int32_t id);
    list_array<std::string> request_names();
    list_array<int32_t> request_ids();

    int32_t get_default_world_id();
    void set_default_world_id(int32_t id);

    //load world to cache
    void pre_load_world(int32_t world_id);

    //resolve name and load world to cache, if world does not exists then tries to use initialization callback, if nullptr then throws
    int32_t pre_load_world(
        const std::string& name,
        std::function<void(storage::world_data& world)> initialization = nullptr
    );

    bool exists(int32_t id);
    bool exists(const std::string& name);

    //creates new world and returns id
    int32_t create(const std::string& name);

    //creates new world, calls callback for initialization and returns id, safe for exceptions
    int32_t create(
        const std::string& name,
        std::function<void(storage::world_data& world)> callback
    );

    void remove(int32_t id);

    void for_each_world(const std::function<void(int32_t id, storage::world_data& world)>& func);


    //gets client world, checks if world exists, returns pair of id and name, if world does not exists then returns default world and sets default position for player in new world
    std::pair<int32_t, std::string> prepare_world(base_objects::shared_client_data& client_ref);
    void sync_settings(base_objects::shared_client_data& client_ref); //sends world settings to client

    void transfer(
        api::ecs::entity entity,
        int32_t world_id,
        util::vector position,
        util::angle_deg rotation,
        util::vector velocity,
        std::function<void(storage::world_data& world)> callback = nullptr
    );

    void transfer(
        api::ecs::entity entity,
        int32_t world_id,
        util::vector position,
        util::angle_deg rotation,
        std::function<void(storage::world_data& world)> callback = nullptr
    );

    void transfer(
        api::ecs::entity entity,
        int32_t world_id,
        util::vector position,
        std::function<void(storage::world_data& world)> callback = nullptr
    );

    void register_entity(int32_t world_id, api::ecs::entity entity_ref);
    void unregister_entity(int32_t world_id, api::ecs::entity entity_ref);

    base_objects::events::event<int32_t>& on_world_loaded();
    base_objects::events::event<int32_t>& on_world_unloaded();
    base_objects::events::event<double>& on_tps_changed();
    base_objects::events::event<uint64_t>& on_tick();

    base_objects::events::event<uint64_t>& ticking_clock(uint64_t notify_each = 1); //0 and 1 is same ticking speed, used global ticking speed

    bool registered();
}


#endif /* SRC_API_WORLD */
