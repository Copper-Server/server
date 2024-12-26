#ifndef SRC_API_WORLD
#define SRC_API_WORLD

#include <functional>
#include <src/storage/world_data.hpp>

namespace copper_server::api::world {
    void unload(uint64_t world_id);
    void save(uint64_t world_id);
    void save_all();

    size_t loaded_chunks_count();
    size_t loaded_chunks_count(uint64_t world_id);
    size_t loaded_chunks_count(const std::string& name);

    void iterate(std::function<void(uint64_t id, storage::world_data& world)> callback);
    void get(uint64_t world_id, std::function<void(storage::world_data& world)> callback);
    void get(const std::string& name, std::function<void(storage::world_data& world)> callback);
    uint64_t resolve_id(const std::string& name);

    uint64_t get_default_world_id();

    //load world to cache
    void pre_load_world(uint64_t world_id);

    //resolve name and load world to cache, if world does not exists then tries to use initialization callback, if nullptr then throws
    uint64_t pre_load_world(
        const std::string& name,
        std::function<void(storage::world_data& world)> initialization = nullptr
    );

    //creates new world and returns id
    uint64_t create(const std::string& name);

    //creates new world, calls callback for initialization and returns id, safe for exceptions
    uint64_t create(
        const std::string& name,
        std::function<void(storage::world_data& world)> callback
    );

    //gets client world, checks if world exists, returns pair of id and name, if world does not exists then returns default world and sets default position for player in new world
    std::pair<uint64_t, std::string> prepare_world(base_objects::client_data_holder& client_ref);

    base_objects::events::event<uint64_t>& on_world_loaded();
    base_objects::events::event<uint64_t>& on_world_unloaded();
    base_objects::events::event<double>& on_tps_changed();

    bool registered();
}


#endif /* SRC_API_WORLD */
