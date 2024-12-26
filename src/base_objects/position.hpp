#ifndef SRC_BASE_OBJECTS_POSITION
#define SRC_BASE_OBJECTS_POSITION

namespace copper_server::base_objects {
    union position {
        struct {
            int x : 26;
            int z : 26;
            int y : 12;
        };

        unsigned long long raw;

        bool operator==(const position& other) const {
            return raw == other.raw;
        }

        bool operator!=(const position& other) const {
            return raw != other.raw;
        }
    };
}

#endif /* SRC_BASE_OBJECTS_POSITION */
