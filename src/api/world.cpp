#include <functional>
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

    base_objects::event<uint64_t>& on_world_loaded() {
        return get_worlds().on_world_loaded;
    }

    base_objects::event<uint64_t>& on_world_unloaded() {
        return get_worlds().on_world_unloaded;
    }

    base_objects::event<double>& on_tps_changed() {
        return get_worlds().on_tps_changed;
    }
}