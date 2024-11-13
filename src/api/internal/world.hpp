#ifndef SRC_API_INTERNAL_WORLD
#define SRC_API_INTERNAL_WORLD
#include <src/storage/world_data.hpp>

namespace copper_server::api::world {
    void register_worlds_data(storage::worlds_data& worlds);
    void unregister_worlds_data();
}

#endif /* SRC_API_INTERNAL_WORLD */
