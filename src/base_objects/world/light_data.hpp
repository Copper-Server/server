#ifndef SRC_BASE_OBJECTS_WORLD_LIGHT_DATA
#define SRC_BASE_OBJECTS_WORLD_LIGHT_DATA
#include <cstdint>
namespace copper_server::base_objects::world {
    struct light_data {
        union light_item {
            uint8_t light_point;
        };

        light_item light_map[16][16][16];

        light_data()
            : light_map() {}
    };
}
#endif /* SRC_BASE_OBJECTS_WORLD_LIGHT_DATA */
