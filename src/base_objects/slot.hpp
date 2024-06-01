#ifndef SRC_BASE_OBJECTS_SLOT
#define SRC_BASE_OBJECTS_SLOT

#include "../library/enbt.hpp"
#include <optional>

namespace crafted_craft {
    namespace base_objects {
        struct slot_data {
            std::optional<ENBT> nbt;
            int32_t id = 0;
            uint8_t count = 0;
        };

        using slot = std::optional<slot_data>;
    }
}

#endif /* SRC_BASE_OBJECTS_SLOT */
