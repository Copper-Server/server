#include "virtual_client.hpp"
#include "../protocolHelper/util.hpp"

namespace crafted_craft {
    namespace base_objects {

        void virtual_client::parse_packet(list_array<uint8_t>& packet) {
            if (packet.empty())
                return;
            ArrayStream arr(packet.data(), packet.size());
            uint8_t packet_id = arr.read();
            switch (packet_id) {
            case 0x00:
                return;
            case 0x01: {
                return;
            }
            case 0x02: {
                return;
            }
            case 0x03: {
            }
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
            case 0x08:
            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C:
            case 0x0D:
            case 0x0F:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
            case 0x14:
            case 0x15:
            case 0x16:
            case 0x17:
            case 0x18:
            case 0x19:
            case 0x1A:
            case 0x1B:
            case 0x1C:
            case 0x1D:
            case 0x1E: {
                if (!disguisedChatMessage)
                    return;
                list_array<uint8_t> message_text_component;
                list_array<uint8_t> sender_text_component;
                list_array<uint8_t> target_text_component;
                while (uint8_t it = arr.read())
                    message_text_component.push_back(it);
                int32_t chat_type = ReadVar<int32_t>(arr);
                while (uint8_t it = arr.read())
                    sender_text_component.push_back(it);

                if (arr.read()) {
                    while (uint8_t it = arr.read())
                        target_text_component.push_back(it);
                }
                message_text_component.push_back(0);
                sender_text_component.push_back(0);
                target_text_component.push_back(0);


                disguisedChatMessage(
                    Chat::fromTextComponent(message_text_component),
                    chat_type,
                    Chat::fromTextComponent(sender_text_component),
                    target_text_component.size() == 1 ? std::optional(Chat::fromTextComponent(target_text_component)) : std::nullopt
                );
                break;
            }
            case 0x1F:
            case 0x20:
            case 0x21:
            case 0x22:
            case 0x23:
            case 0x24:
            case 0x25:
            case 0x26:
            case 0x27:
            case 0x28:
            case 0x29:
            case 0x2A:
            case 0x2B:
            case 0x2C:
            case 0x2D:
            case 0x2F:
            case 0x30:
            case 0x31:
            case 0x32:
            case 0x33:
            case 0x34:
            case 0x35:
            case 0x36:
            case 0x37:
            case 0x38:
            case 0x39:
            case 0x3A:
            case 0x3B:
            case 0x3C:
            case 0x3D:
            case 0x3F:
            case 0x40:
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
            case 0x46:
            case 0x47:
            case 0x48:
            case 0x49:
            case 0x4A:
            case 0x4B:
            case 0x4C:
            case 0x4D:
            case 0x4F:
            case 0x50:
            case 0x51:
            case 0x52:
            case 0x53:
            case 0x54:
            case 0x55:
            case 0x56:
            case 0x57:
            case 0x58:
            case 0x59:
            case 0x5A:
            case 0x5B:
            case 0x5C:
            case 0x5D:
            case 0x5F:
            case 0x60:
            case 0x61:
            case 0x62:
            case 0x63:
            case 0x64:
            case 0x65:
            case 0x66:
            case 0x67:
            case 0x68:
            case 0x69:
            case 0x6A:
            case 0x6B:
            case 0x6C: { //systemChatMessage
                if (!systemChatMessage && !systemChatMessageOverlay)
                    return;
                size_t readen = 0;
                Chat message_text_component = Chat::fromEnbt(NBT::readNBT_asENBT(arr.data_read(), arr.size_read(), readen));
                arr.range_read(readen);

                if (arr.read()) {
                    if (systemChatMessageOverlay)
                        systemChatMessageOverlay(message_text_component);
                } else {
                    if (systemChatMessage)
                        systemChatMessage(message_text_component);
                }
                break;
            };
            case 0x6D:
            case 0x6F:
            case 0x70:
            case 0x71:
            case 0x72:
            case 0x73:
            case 0x74:
                break;
            }
        }
    }
}