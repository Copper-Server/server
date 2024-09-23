#ifndef SRC_BASE_OBJECTS_VIRTUAL_CLIENT
#define SRC_BASE_OBJECTS_VIRTUAL_CLIENT
#include "../protocolHelper/packets.hpp"
#include "shared_client_data.hpp"

namespace crafted_craft {
    class Server;

    namespace base_objects {
        struct virtual_client {
            client_data_holder client;
            Server& assigned_server;

            virtual_client(client_data_holder allocated, const std::string& name, const std::string& brand, Server& assigned_server)
                : client(allocated), assigned_server(assigned_server) {

                client->name = name;
                client->ip = "";
                client->client_brand = brand;
                client->locale = "en_US";

                client->player_data.local_data["virtual_client"] = name;
                client->player_data.gamemode = -1;
                client->player_data.op_level = 4;
                client->player_data.world_id = shared_string("virtual_client astral space");
                client->player_data.health = 999999.0f;
                client->player_data.on_ground = true;

                client->packets_state.protocol_version = 767;
                client->packets_state.state = SharedClientData::packets_state_t::protocol_state::play;
                client->special_callback = [this](SharedClientData& self) {
                    self.getPendingPackets().for_each([&](auto packet) {
                        packet.data.for_each([&](Response::Item& it) {
                            parse_packet(it.data);
                        });
                    });
                };
            }

            void parse_packet(list_array<uint8_t>&);

            std::function<void(int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions)> commandSuggestionsResponse;

            std::function<void(int32_t message_id)> deleteMessage;

            std::function<void(const Chat& message, int32_t chat_type, const Chat& sender, std::optional<Chat> target_name)> disguisedChatMessage;
            std::function<void(enbt::raw_uuid sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, std::optional<enbt::value> __UNDEFINED__FIELD__, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name)> playerChatMessage;

            std::function<void(const Chat& message)> systemChatMessage;
            std::function<void(const Chat& message)> systemChatMessageOverlay;
        };
    }
}


#endif /* SRC_BASE_OBJECTS_VIRTUAL_CLIENT */
