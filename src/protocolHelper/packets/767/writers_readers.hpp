
#include <src/base_objects/slot.hpp>
#include <src/util/readers.hpp>

namespace copper_server {
    namespace packets {
        namespace release_767 {
            namespace reader {
                void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot_data& slot, int16_t protocol = 767);
                void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot& slot, int16_t protocol = 767);
                void WriteSlot(list_array<uint8_t>& data, const base_objects::slot& slot, int16_t protocol = 767);
                void WriteTradeItem(list_array<uint8_t>& data, const base_objects::slot_data& slot, int16_t protocol = 767);
                void WriteTradeItem(list_array<uint8_t>& data, const base_objects::slot& slot, int16_t protocol = 767);
                base_objects::slot ReadSlotItem(ArrayStream& data, int16_t protocol = 767);
                base_objects::slot ReadSlot(ArrayStream& data, int16_t protocol = 767);
            }
        }
    }
}
