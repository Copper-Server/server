#ifndef SRC_PROTOCOLHELPER_PACKETS_765_WRITERS_READERS
#define SRC_PROTOCOLHELPER_PACKETS_765_WRITERS_READERS
#include <src/base_objects/slot.hpp>
#include <src/util/readers.hpp>

namespace copper_server::packets::release_765::reader {
    void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot& slot, int16_t protocol = 765);
    void WriteSlot(list_array<uint8_t>& data, const base_objects::slot& slot, int16_t protocol = 765);
    base_objects::slot ReadSlotItem(ArrayStream& data, int16_t protocol = 765);
    base_objects::slot ReadSlot(ArrayStream& data, int16_t protocol = 765);
}
#endif /* SRC_PROTOCOLHELPER_PACKETS_765_WRITERS_READERS */
