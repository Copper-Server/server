#ifndef SRC_BASE_OBJECTS_WORLD_BLOCK_ACTION
#define SRC_BASE_OBJECTS_WORLD_BLOCK_ACTION
#include <cstdint>
#include <variant>

namespace copper_server::base_objects::world {
    struct block_action {
        struct noteblock_activated {};

        enum class direction : uint8_t {
            down,
            up,
            south,
            west,
            north,
            east
        };

        struct piston_extend {
            direction dir;
        };

        struct piston_retract {
            direction dir;
        };

        struct piston_canceled {
            direction dir;
        };

        struct chest_opened {
            uint32_t count = 0;
        };

        struct reset_spawner {};

        struct end_gateway_activated {};

        struct shulker_box_closed {};

        struct shulker_box_opened {
            uint32_t count = 0;
        };

        struct bell_ring {
            direction dir;
        };

        struct decorated_block_woble {
            bool successful;
        };

        std::variant<
            noteblock_activated,
            piston_extend,
            piston_retract,
            piston_canceled,
            chest_opened,
            reset_spawner,
            end_gateway_activated,
            shulker_box_closed,
            shulker_box_opened,
            bell_ring,
            decorated_block_woble>
            action;
    };
}


#endif /* SRC_BASE_OBJECTS_WORLD_BLOCK_ACTION */
