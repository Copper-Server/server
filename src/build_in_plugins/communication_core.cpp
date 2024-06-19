#include "communication_core.hpp"
#include "../ClientHandleHelper.hpp"
#include "../api/players.hpp"
#include "../library/list_array.hpp"
#include "../protocolHelper/packets.hpp"
#include "../storage/memory/online_player.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        CommunicationCorePlugin::CommunicationCorePlugin()
            : server(TCPserver::get_global_instance()) {}

        void CommunicationCorePlugin::OnLoad(const PluginRegistrationPtr& self) {
            register_event(api::players::calls::on_player_chat, [this](const api::players::player_chat& chat) {
                server.online_players.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&chat](SharedClientData& client) {
                    client.sendPacket(
                        packets::play::playerChatMessage(
                            chat.sender->data->uuid,
                            0,
                            chat.signature,
                            chat.message,
                            std::chrono::milliseconds(std::chrono::seconds(std::time(NULL))).count(),
                            chat.salt,
                            chat.previous_messages,
                            std::nullopt,
                            0,
                            {},
                            chat.chat_type_id,
                            chat.sender_decorated_name.empty() ? chat.sender->name : chat.sender_decorated_name,
                            std::nullopt
                        )
                    );
                    return false;
                });
                return false;
            });

            register_event(api::players::calls::on_player_personal_chat, [this](const api::players::player_personal_chat& chat) {
                chat.receiver->sendPacket(
                    packets::play::playerChatMessage(
                        chat.sender->data->uuid,
                        0,
                        chat.signature,
                        chat.message,
                        std::chrono::milliseconds(std::chrono::seconds(std::time(NULL))).count(),
                        chat.salt,
                        chat.previous_messages,
                        std::nullopt,
                        0,
                        {},
                        chat.receiver_chat_type_id,
                        chat.sender_decorated_name.empty() ? chat.sender->name : chat.sender_decorated_name,
                        chat.receiver_decorated_name.empty() ? chat.receiver->name : chat.receiver_decorated_name
                    )
                );

                chat.sender->sendPacket(
                    packets::play::playerChatMessage(
                        chat.sender->data->uuid,
                        0,
                        chat.signature,
                        chat.message,
                        std::chrono::milliseconds(std::chrono::seconds(std::time(NULL))).count(),
                        chat.salt,
                        chat.previous_messages,
                        std::nullopt,
                        0,
                        {},
                        chat.chat_type_id,
                        chat.sender_decorated_name.empty() ? chat.sender->name : chat.sender_decorated_name,
                        chat.receiver_decorated_name.empty() ? chat.receiver->name : chat.receiver_decorated_name
                    )
                );
                return false;
            });


            register_event(api::players::calls::on_system_message_broadcast, [this](const Chat& message) {
                server.online_players.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                    client.sendPacket(packets::play::systemChatMessage(message));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_system_message, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::systemChatMessage(message.data));
                return false;
            });

            register_event(api::players::calls::on_system_message_overlay_broadcast, [this](const Chat& message) {
                server.online_players.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                    client.sendPacket(packets::play::systemChatMessageOverlay(message));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_system_message_overlay, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::systemChatMessageOverlay(message.data));
                return false;
            });

            register_event(api::players::calls::on_player_kick, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::kick(message.data));
                server.online_players.remove_player(message.player);
                return false;
            });

            register_event(api::players::calls::on_player_ban, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::kick(message.data));
                server.online_players.remove_player(message.player);
                return false;
            });

            register_event(api::players::calls::on_action_bar_message_broadcast, [this](const Chat& message) {
                server.online_players.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                    client.sendPacket(packets::play::setActionBarText(message));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_action_bar_message, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::setActionBarText(message.data));
                return false;
            });

            register_event(api::players::calls::on_title_message_broadcast, [this](const Chat& message) {
                server.online_players.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                    client.sendPacket(packets::play::setTitleText(message));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_title_message, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::setTitleText(message.data));
                return false;
            });

            register_event(api::players::calls::on_subtitle_message_broadcast, [this](const Chat& message) {
                server.online_players.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                    client.sendPacket(packets::play::setSubtitleText(message));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_subtitle_message, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::setSubtitleText(message.data));
                return false;
            });

            register_event(api::players::calls::on_title_times_broadcast, [this](const api::players::titles_times& times) {
                server.online_players.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&times](SharedClientData& client) {
                    client.sendPacket(packets::play::setTitleAnimationTimes(times.fade_in, times.stay, times.fade_out));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_title_times, [this](const api::players::personal<api::players::titles_times>& times) {
                times.player->sendPacket(packets::play::setTitleAnimationTimes(times.data.fade_in, times.data.stay, times.data.fade_out));
                return false;
            });

            register_event(api::players::calls::on_unsigned_message_broadcast, [this](const api::players::unsigned_chat& message) {
                server.online_players.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                    client.sendPacket(packets::play::disguisedChatMessage(message.message, message.chat_type_id, message.sender_name, message.receiver_name));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_unsigned_message, [this](const api::players::personal<api::players::unsigned_chat>& message) {
                message.player->sendPacket(packets::play::disguisedChatMessage(message.data.message, message.data.chat_type_id, message.data.sender_name, message.data.receiver_name));
                return false;
            });
        }

        void CommunicationCorePlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
            browser.add_child({"broadcast"})
                .add_child({"<message>", "broadcast <message>", "Broadcast a message to all players"})
                .set_callback("command.broadcast", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                    api::players::calls::on_system_message_broadcast(Chat::parseToChat(args[0]));
                });
        }
    } // namespace build_in_plugins

} // namespace crafted_craft
