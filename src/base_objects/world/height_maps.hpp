#ifndef SRC_BASE_OBJECTS_WORLD_HEIGHT_MAPS
#define SRC_BASE_OBJECTS_WORLD_HEIGHT_MAPS
#include <cstdint>
namespace copper_server::base_objects::world {
    struct height_maps {
        uint64_t surface[16][16];
        uint64_t ocean_floor[16][16];
        uint64_t motion_blocking[16][16];
        uint64_t motion_blocking_no_leaves[16][16];

        void make_zero() {
            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 16; j++) {
                    surface[i][j] = 0;
                    ocean_floor[i][j] = 0;
                    motion_blocking[i][j] = 0;
                    motion_blocking_no_leaves[i][j] = 0;
                }
            }
        }

        height_maps() {
            make_zero();
        }
    };
}
#endif /* SRC_BASE_OBJECTS_WORLD_HEIGHT_MAPS */
