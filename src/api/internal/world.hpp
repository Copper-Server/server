#ifndef SRC_API_INTERNAL_WORLD
#define SRC_API_INTERNAL_WORLD
#include "../../storage/world_data.hpp"

namespace crafted_craft::api::world {
    void register_worlds_data(storage::worlds_data& worlds);
    void unregister_worlds_data();
}

#endif /* SRC_API_INTERNAL_WORLD */
