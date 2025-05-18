#ifndef SRC_BASE_OBJECTS_ENTITY_ANIMATION
#define SRC_BASE_OBJECTS_ENTITY_ANIMATION
#include <cstdint>

namespace copper_server::base_objects {
    enum entity_animation : uint8_t {
        swing_main_arm = 0,
        unrecognized = 1,
        leave_bed = 2,
        swing_offhand = 3,
        critical_hit = 4,
        enchanted_hit = 5,
    };
}

#endif /* SRC_BASE_OBJECTS_ENTITY_ANIMATION */
