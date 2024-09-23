#ifndef SRC_API_WORLD
#define SRC_API_WORLD

#include "../ClientHandleHelper.hpp"
#include "../storage/world_data.hpp"
#include <functional>

namespace crafted_craft {
    namespace api {
        namespace world {
            void register_worlds_data(storage::worlds_data& worlds);

            void unload(uint64_t world_id);
            void save(uint64_t world_id);
            void save_all();

            void iterate(std::function<void(uint64_t id, storage::world_data& world)> callback);
            void get(uint64_t world_id, std::function<void(storage::world_data& world)> callback);
            void get(const std::string& name, std::function<void(storage::world_data& world)> callback);
            uint64_t resolve_id(const std::string& name);

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
        }
    }
}


#endif /* SRC_API_WORLD */
