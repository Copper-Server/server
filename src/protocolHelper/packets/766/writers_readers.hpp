#ifndef SRC_PROTOCOLHELPER_WRITERS_READERS_767_RELEASE
#define SRC_PROTOCOLHELPER_WRITERS_READERS_767_RELEASE
#include "../../../base_objects/slot.hpp"
#include "../../../util/readers.hpp"

namespace crafted_craft {
    namespace packets {
        namespace release_766 {
            namespace reader {
                void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot& slot);
                void WriteSlot(list_array<uint8_t>& data, const base_objects::slot& slot);
                base_objects::slot ReadSlotItem(ArrayStream& data);
                base_objects::slot ReadSlot(ArrayStream& data);
            }
        }
    }
}

#endif /* SRC_PROTOCOLHELPER_WRITERS_READERS_766_RELEASE */
