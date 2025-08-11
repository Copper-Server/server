/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <functional>
#include <src/api/configuration.hpp>
#include <src/api/packets.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server::api::world {
    storage::worlds_data* worlds_data = nullptr;

    void register_worlds_data(storage::worlds_data& worlds) {
        if (!worlds_data)
            worlds_data = &worlds;
        else
            throw std::runtime_error("Worlds already registered");
    }

    void unregister_worlds_data() {
        if (worlds_data)
            worlds_data = nullptr;
        else
            throw std::runtime_error("Worlds already unregistered");
    }

    bool registered() {
        return worlds_data;
    }

    storage::worlds_data& get_worlds() {
        if (worlds_data)
            return *worlds_data;
        else
            throw std::runtime_error("Worlds not yet registered");
    }

    void unload(int32_t world_id) {
        get_worlds().save_and_unload(world_id);
    }

    void save(int32_t world_id) {
        get_worlds().save(world_id);
    }

    void save_all() {
        get_worlds().save_all();
    }

    size_t loaded_chunks_count() {
        return get_worlds().loaded_chunks_count();
    }

    size_t loaded_chunks_count(int32_t world_id) {
        return get_worlds().loaded_chunks_count(world_id);
    }

    size_t loaded_chunks_count(const std::string& name) {
        return get_worlds().loaded_chunks_count(get_worlds().get_id(name));
    }

    void iterate(std::function<void(int32_t world, storage::world_data& data)> callback) {
        get_worlds().for_each_world(callback);
    }

    void get(int32_t world_id, std::function<void(storage::world_data& data)> callback) {
        callback(*get_worlds().get(world_id));
    }

    void get(const std::string& name, std::function<void(storage::world_data& data)> callback) {
        callback(*get_worlds().get(get_worlds().get_id(name)));
    }

    int32_t resolve_id(const std::string& name) {
        return get_worlds().get_id(name);
    }

    std::string resolve_name(int32_t id) {
        return get_worlds().get_name(id);
    }

    list_array<std::string> request_names() {
        return get_worlds().get_all_ids().convert_fn([](auto id) { return get_worlds().get_name(id); });
    }

    list_array<int32_t> request_ids() {
        return get_worlds().get_all_ids();
    }

    int32_t get_default_world_id() {
        return get_worlds().base_world_id;
    }

    void pre_load_world(int32_t world_id) {
        get_worlds().get(world_id);
    }

    int32_t pre_load_world(const std::string& name, std::function<void(storage::world_data& world)> initialization) {
        auto id = get_worlds().get_id(name);
        if (id == (int32_t)-1) {
            if (initialization)
                get_worlds().create(name, initialization);
            else
                throw std::runtime_error("World with name " + name + " does not exists.");
        } else
            get_worlds().get(id);
        return id;
    }

    int32_t create(const std::string& name) {
        return get_worlds().create(name);
    }

    int32_t create(
        const std::string& name,
        std::function<void(storage::world_data& world)> callback
    ) {
        return get_worlds().create(name, callback);
    }

    //gets client world, checks if world exists, returns pair of id and name, if world does not exists then returns default world and sets default position for player in new world
    std::pair<int32_t, std::string> prepare_world(base_objects::SharedClientData& client_ref) {
        auto id = get_worlds().get_id(client_ref.player_data.world_id);
        bool set_new_data = false;
        if (id == (int32_t)-1) {
            id = get_worlds().base_world_id;
            set_new_data = true;
        }
        auto world = get_worlds().get(id);

        if (set_new_data) {
            base_objects::cubic_bounds_block_radius rs{world->spawn_data.x, 0, world->spawn_data.z, world->spawn_data.radius};
            auto [x, y, z] = rs.random_point();
            int64_t pos_y = 0;
            world->get_height_maps_at(x, z, [&](base_objects::world::height_maps& height_maps) {
                auto mt = height_maps.motion_blocking[x % 16][z % 16];
                auto oc_flor = height_maps.ocean_floor[x % 16][z % 16];
                auto oc = height_maps.surface[x % 16][z % 16];
                pos_y = std::max(mt, std::max(oc_flor, oc));
            });

            client_ref.player_data.world_id = get_worlds().get(id)->world_name;

            client_ref.player_data.assigned_entity->position.x = 0.5 + (double)x;
            client_ref.player_data.assigned_entity->position.y = (double)pos_y;
            client_ref.player_data.assigned_entity->position.z = 0.5 + (double)z;
            client_ref.player_data.assigned_entity->rotation = {0, 0};
        }

        return {id, client_ref.player_data.world_id};
    }

    void sync_settings(base_objects::SharedClientData& client_ref) {
        auto id = get_worlds().get_id(client_ref.player_data.world_id);
        if (id == -1) {
            using enum_ = api::configuration::ServerConfiguration::World::world_not_found_for_client_e;
            switch (api::configuration::get().world.world_not_found_for_client) {
            case enum_::kick:
                throw std::runtime_error("World with id " + client_ref.player_data.world_id + " does not exists.");
            case enum_::transfer_to_default:
            case enum_::request_plugin_or_default:
            default:
                id = get_default_world_id();
                client_ref.player_data.world_id = get_worlds().get_name(id);
                break;
            }
        }
        auto world = get_worlds().get(id);

        client_ref << api::packets::client_bound::play::initialize_border{
            .x = world->border_center_x,
            .z = world->border_center_z,
            .old_diameter = world->border_size,
            .new_diameter = world->border_size,
            .speed_ms = (int64_t)world->border_lerp_time,
            .portal_teleport_boundary = world->portal_teleport_boundary,
            .warning_blocks = (int32_t)world->border_warning_blocks,
            .warning_time = (int32_t)world->border_warning_time,
        } << api::packets::client_bound::play::ticking_state{
            .tick_rate = (float)world->ticks_per_second,
            .is_frozen = world->ticking_frozen,
        } << api::packets::client_bound::play::ticking_step{
            .steps = 1,
        } << api::packets::client_bound::play::set_time{
            .world_age = (uint64_t)world->time,
            .time_of_day = (uint64_t)world->day_time,
            .time_of_day_increment = world->increase_time,
        } << api::packets::client_bound::play::set_default_spawn_position{
            .location = {(int32_t)world->spawn_data.x, (int32_t)world->spawn_data.y, (int32_t)world->spawn_data.z},
            .angle = world->spawn_data.angle,
        } << api::packets::client_bound::play::game_event{
            .event = {api::packets::client_bound::play::game_event::wait_for_level_chunks{}},
        };
    }

    void transfer(
        [[maybe_unused]] base_objects::client_data_holder& client_ref,
        [[maybe_unused]] int32_t world_id,
        [[maybe_unused]] util::VECTOR position,
        [[maybe_unused]] util::ANGLE_DEG rotation,
        [[maybe_unused]] util::VECTOR velocity,
        [[maybe_unused]] std::function<void(storage::world_data& world)> callback = nullptr
    ) {
        //TODO
    }

    void transfer(
        [[maybe_unused]] base_objects::client_data_holder& client_ref,
        [[maybe_unused]] int32_t world_id,
        [[maybe_unused]] util::VECTOR position,
        [[maybe_unused]] util::ANGLE_DEG rotation,
        [[maybe_unused]] std::function<void(storage::world_data& world)> callback = nullptr
    ) {
        //TODO
    }

    void transfer(
        [[maybe_unused]] base_objects::client_data_holder& client_ref,
        [[maybe_unused]] int32_t world_id,
        [[maybe_unused]] util::VECTOR position,
        [[maybe_unused]] std::function<void(storage::world_data& world)> callback = nullptr
    ) {
        //TODO
    }

    void register_entity(int32_t world_id, base_objects::entity_ref& entity_ref) {
        get_worlds().get(world_id)->register_entity(entity_ref);
    }

    void unregister_entity(int32_t world_id, base_objects::entity_ref& entity_ref) {
        get_worlds().get(world_id)->unregister_entity(entity_ref);
    }

    std::pair<int32_t, std::string> prepare_world(const std::string& name) {
        auto id = get_worlds().get_id(name);
        if (id == (int32_t)-1)
            id = get_worlds().base_world_id;
        return {id, get_worlds().get(id)->world_name};
    }

    base_objects::events::event<int32_t>& on_world_loaded() {
        return get_worlds().on_world_loaded;
    }

    base_objects::events::event<int32_t>& on_world_unloaded() {
        return get_worlds().on_world_unloaded;
    }

    base_objects::events::event<double>& on_tps_changed() {
        return get_worlds().on_tps_changed;
    }
}