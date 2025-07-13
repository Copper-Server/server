#ifndef SRC_BUILD_IN_PLUGINS_PROTOCOL_770_WRITERS_READERS
#define SRC_BUILD_IN_PLUGINS_PROTOCOL_770_WRITERS_READERS
#include <src/base_objects/recipe.hpp>
#include <src/base_objects/slot.hpp>
#include <src/base_objects/slot_display.hpp>
#include <src/util/readers.hpp>

namespace copper_server::build_in_plugins::network::tcp::protocol::play_770::reader {
    void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot_data& slot);
    void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot& slot);
    void WriteSlot(list_array<uint8_t>& data, const base_objects::slot& slot);
    void WriteTradeItem(list_array<uint8_t>& data, const base_objects::slot_data& slot);
    void WriteTradeItem(list_array<uint8_t>& data, const base_objects::slot& slot);
    void WriteIngredient(list_array<uint8_t>& data, const base_objects::slot_display& item);
    base_objects::slot ReadSlotItem(ArrayStream& data);
    base_objects::slot ReadSlot(ArrayStream& data);

    bool WriteRecipeDisplay(list_array<uint8_t>& data, const base_objects::recipe& display);

    Chat fromTextComponent(const list_array<uint8_t>& enbt);
    Chat fromTextComponent(ArrayStream& data);
    list_array<uint8_t> toTextComponent(const Chat&);
}
#endif /* SRC_BUILD_IN_PLUGINS_PROTOCOL_770_WRITERS_READERS */
