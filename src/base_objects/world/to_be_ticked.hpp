#ifndef SRC_BASE_OBJECTS_WORLD_TO_BE_TICKED
#define SRC_BASE_OBJECTS_WORLD_TO_BE_TICKED
#include <cstdint>
namespace copper_server::base_objects::world {
    struct to_be_ticked {
        uint64_t scheduled_on;
        uint32_t block_id;//type not state
        int32_t priority;
        uint8_t x : 4;
        uint8_t y : 4;
        uint8_t z : 4;
    };
}

#endif /* SRC_BASE_OBJECTS_WORLD_TO_BE_TICKED */
