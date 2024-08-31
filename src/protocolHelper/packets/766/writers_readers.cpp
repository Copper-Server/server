#include "../765/writers_readers.hpp"

namespace crafted_craft {
    namespace packets {
        namespace release_766 {
            namespace reader {
                void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot& slot) {
                    release_765::reader::WriteSlotItem(data, slot);
                }

                void WriteSlot(list_array<uint8_t>& data, const base_objects::slot& slot) {
                    release_765::reader::WriteSlot(data, slot);
                }

                base_objects::slot ReadSlotItem(ArrayStream& data) {
                    return release_765::reader::ReadSlotItem(data);
                }

                base_objects::slot ReadSlot(ArrayStream& data) {

                    return release_765::reader::ReadSlot(data);
                }
            }
        }
    }
}
