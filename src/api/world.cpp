#include <functional>
#include <src/base_objects/player.hpp>
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

    void unload(uint64_t world_id) {
        get_worlds().save_and_unload(world_id);
    }

    void save(uint64_t world_id) {
        get_worlds().save(world_id);
    }

    void save_all() {
        get_worlds().save_all();
    }

    size_t loaded_chunks_count() {
        return get_worlds().loaded_chunks_count();
    }

    size_t loaded_chunks_count(uint64_t world_id) {
        return get_worlds().loaded_chunks_count(world_id);
    }

    size_t loaded_chunks_count(const std::string& name) {
        return get_worlds().loaded_chunks_count(get_worlds().get_id(name));
    }

    void iterate(std::function<void(uint64_t world, storage::world_data& data)> callback) {
        get_worlds().for_each_world(callback);
    }

    void get(uint64_t world_id, std::function<void(storage::world_data& data)> callback) {
        callback(*get_worlds().get(world_id));
    }

    void get(const std::string& name, std::function<void(storage::world_data& data)> callback) {
        callback(*get_worlds().get(get_worlds().get_id(name)));
    }

    uint64_t resolve_id(const std::string& name) {
        return get_worlds().get_id(name);
    }

    uint64_t get_default_world_id() {
        return get_worlds().base_world_id;
    }

    void pre_load_world(uint64_t world_id) {
        get_worlds().get(world_id);
    }

    uint64_t pre_load_world(const std::string& name, std::function<void(storage::world_data& world)> initialization) {
        auto id = get_worlds().get_id(name);
        if (id == -1) {
            if (initialization)
                get_worlds().create(name, initialization);
            else
                throw std::runtime_error("World with name " + name + " does not exists.");
        } else
            get_worlds().get(id);
        return id;
    }

    uint64_t create(const std::string& name) {
        return get_worlds().create(name);
    }

    uint64_t create(
        const std::string& name,
        std::function<void(storage::world_data& world)> callback
    ) {
        return get_worlds().create(name, callback);
    }

    //gets client world, checks if world exists, returns pair of id and name, if world does not exists then returns default world and sets default position for player in new world
    std::pair<uint64_t, std::string> prepare_world(base_objects::client_data_holder& client_ref) {
        auto id = get_worlds().get_id(client_ref->player_data.world_id);
        bool set_new_data = false;
        if (id == -1) {
            id = get_worlds().base_world_id;
            set_new_data = true;
        }
        auto world = get_worlds().get(id);

        if (set_new_data) {
            base_objects::cubic_bounds_block_radius rs{world->spawn_data.x, 0, world->spawn_data.z, world->spawn_data.radius};
            auto [x, y, z] = rs.random_point();
            int64_t pos_y = 0;
            world->get_height_maps_at(x, z, [&](storage::height_maps& height_maps) {
                auto mt = height_maps.motion_blocking[x % 16][z % 16];
                auto oc_flor = height_maps.ocean_floor[x % 16][z % 16];
                auto oc = height_maps.surface[x % 16][z % 16];
                pos_y = std::max(mt, std::max(oc_flor, oc));
            });

            client_ref->player_data.world_id = id;

            client_ref->player_data.assigned_entity->position.x = world->spawn_data.x;
            client_ref->player_data.assigned_entity->position.y = pos_y;
            client_ref->player_data.assigned_entity->position.z = world->spawn_data.z;
            client_ref->player_data.assigned_entity->rotation = {0, 0, 0};
        }


        return {id, get_worlds().get(id)->world_name};
    }

    std::pair<uint64_t, std::string> prepare_world(const std::string& name) {
        auto id = get_worlds().get_id(name);
        if (id == -1)
            id = get_worlds().base_world_id;
        return {id, get_worlds().get(id)->world_name};
    }

    base_objects::events::event<uint64_t>& on_world_loaded() {
        return get_worlds().on_world_loaded;
    }

    base_objects::events::event<uint64_t>& on_world_unloaded() {
        return get_worlds().on_world_unloaded;
    }

    base_objects::events::event<double>& on_tps_changed() {
        return get_worlds().on_tps_changed;
    }
}