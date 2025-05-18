#ifndef SRC_BASE_OBJECTS_WORLD_LOADING_POINT_TICKET
#define SRC_BASE_OBJECTS_WORLD_LOADING_POINT_TICKET
#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include <src/base_objects/bounds.hpp>

namespace copper_server::storage {
    class world_data;
}

namespace copper_server::base_objects::world {
    struct loading_point_ticket {
        //returns true if ticket not expired
        using callback = std::function<bool(copper_server::storage::world_data&, size_t, loading_point_ticket&)>;

        struct entity_bound_ticket {
            size_t id;
        };

        std::variant<uint16_t, callback, entity_bound_ticket> expiration;
        base_objects::cubic_bounds_chunk_radius point;
        std::string name;
        int8_t level;

        //sets the whole cubic point to specified level and propagates to neighbors by default
    };
}
#endif /* SRC_BASE_OBJECTS_WORLD_LOADING_POINT_TICKET */
