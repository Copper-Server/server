#ifndef SRC_BASE_OBJECTS_VIRTUAL_CLIENT
#define SRC_BASE_OBJECTS_VIRTUAL_CLIENT
#include <src/base_objects/shared_client_data.hpp>
#include <src/protocolHelper/packets.hpp>

namespace copper_server::base_objects {
    struct virtual_client {
        client_data_holder client;

        virtual_client(client_data_holder allocated, const std::string& name, const std::string& brand);

        void parse_packet(list_array<uint8_t>&);

        std::function<void(int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions)> commandSuggestionsResponse;

        std::function<void(int32_t message_id)> deleteMessage;

        std::function<void(const Chat& message, int32_t chat_type, const Chat& sender, std::optional<Chat> target_name)> disguisedChatMessage;
        std::function<void(enbt::raw_uuid sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, std::optional<enbt::value> unsigned_content, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name)> playerChatMessage;

        std::function<void(const Chat& message)> systemChatMessage;
        std::function<void(const Chat& message)> systemChatMessageOverlay;
    };
}
#endif /* SRC_BASE_OBJECTS_VIRTUAL_CLIENT */
