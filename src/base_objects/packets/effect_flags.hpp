#ifndef SRC_BASE_OBJECTS_PACKETS_EFFECT_FLAGS
#define SRC_BASE_OBJECTS_PACKETS_EFFECT_FLAGS

namespace copper_server::base_objects::packets {
    union effect_flags{
        struct{
            bool is_ambient:1;
            bool show_particles:1;
            bool show_icon:1;
            bool use_blend:1;
        };

        signed char raw;
    };
}

#endif /* SRC_BASE_OBJECTS_PACKETS_EFFECT_FLAGS */
