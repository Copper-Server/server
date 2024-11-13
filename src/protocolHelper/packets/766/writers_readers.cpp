#include <src/protocolHelper/packets/765/writers_readers.hpp>

namespace copper_server {
    namespace packets {
        namespace release_766 {
            namespace reader {
                void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot& slot) {
                    release_765::reader::WriteSlotItem(data, slot, 766);
                }

                void WriteSlot(list_array<uint8_t>& data, const base_objects::slot& slot) {
                    release_765::reader::WriteSlot(data, slot, 766);
                }

                base_objects::slot ReadSlotItem(ArrayStream& data) {
                    return release_765::reader::ReadSlotItem(data, 766);
                }

                base_objects::slot ReadSlot(ArrayStream& data) {
                    return release_765::reader::ReadSlot(data, 766);
                }
            }
        }
    }
}
