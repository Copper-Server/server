
#include "../../../base_objects/slot.hpp"
#include "../../../util/readers.hpp"

namespace crafted_craft {
    namespace packets {
        namespace release_765 {
            namespace reader {
                void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot& slot) {
                    auto converted = slot->pack();
                    WriteVar<int32_t>(converted.id, data);
                    data.push_back(converted.count);
                    if (converted.nbt)
                        data.push_back(NBT::build(converted.nbt.value()).get_as_network());
                    else
                        data.push_back(0); //TAG_End
                }

                void WriteSlot(list_array<uint8_t>& data, const base_objects::slot& slot) {
                    data.push_back((bool)slot);
                    if (slot) {
                        WriteSlotItem(data, slot);
                    }
                }

                base_objects::slot ReadSlotItem(ArrayStream& data) {
                    base_objects::slot_data_storage old_slot;
                    old_slot.id = ReadVar<int32_t>(data);
                    old_slot.count = data.read();
                    if (data.peek() == 0) {
                        size_t readed = 0;
                        old_slot.nbt = NBT::extract_from_array(data.data_read(), readed, data.size_read());
                        data.r += readed;
                    }
                    return old_slot.unpack();
                }

                base_objects::slot ReadSlot(ArrayStream& data) {
                    if (!data.read())
                        return std::nullopt;
                    else
                        return ReadSlotItem(data);
                }
            }
        }
    }
}
