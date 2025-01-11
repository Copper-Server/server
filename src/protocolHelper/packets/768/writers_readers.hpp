#ifndef SRC_PROTOCOLHELPER_PACKETS_768_WRITERS_READERS
#define SRC_PROTOCOLHELPER_PACKETS_768_WRITERS_READERS
#include <src/base_objects/recipe.hpp>
#include <src/base_objects/slot.hpp>
#include <src/base_objects/slot_display.hpp>
#include <src/util/readers.hpp>

namespace copper_server::packets::release_768::reader {
    void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot_data& slot, int16_t protocol = 768);
    void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot& slot, int16_t protocol = 768);
    void WriteSlot(list_array<uint8_t>& data, const base_objects::slot& slot, int16_t protocol = 768);
    void WriteTradeItem(list_array<uint8_t>& data, const base_objects::slot_data& slot, int16_t protocol = 768);
    void WriteTradeItem(list_array<uint8_t>& data, const base_objects::slot& slot, int16_t protocol = 768);
    void WriteIngredient(list_array<uint8_t>& data, const base_objects::slot_display& item);
    base_objects::slot ReadSlotItem(ArrayStream& data, int16_t protocol = 768);
    base_objects::slot ReadSlot(ArrayStream& data, int16_t protocol = 768);

    bool WriteRecipeDisplay(list_array<uint8_t>& data, const base_objects::recipe& display);
}
#endif /* SRC_PROTOCOLHELPER_PACKETS_768_WRITERS_READERS */
