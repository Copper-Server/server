#ifndef SRC_BUILD_IN_PLUGINS_COMMUNICATION_CORE
#define SRC_BUILD_IN_PLUGINS_COMMUNICATION_CORE
#include "../api/players.hpp"
#include "../base_objects/event.hpp"
#include "../plugin/registration.hpp"
#include "../protocolHelper/packets.hpp"
#include "../storage/memory/online_player.hpp"
#include <unordered_map>

namespace crafted_craft {
    namespace build_in_plugins {
        class CommunicationCorePlugin : public PluginRegistration {
            base_objects::event_register_id on_player_chat;
            base_objects::event_register_id on_player_personal_chat;
            base_objects::event_register_id on_system_message_broadcast;
            base_objects::event_register_id on_system_message;
            base_objects::event_register_id on_system_message_overlay_broadcast;
            base_objects::event_register_id on_system_message_overlay;
            base_objects::event_register_id on_player_kick;
            base_objects::event_register_id on_player_ban;
            base_objects::event_register_id on_action_bar_message_broadcast;
            base_objects::event_register_id on_action_bar_message;
            base_objects::event_register_id on_title_message_broadcast;
            base_objects::event_register_id on_title_message;
            base_objects::event_register_id on_subtitle_message_broadcast;
            base_objects::event_register_id on_subtitle_message;
            base_objects::event_register_id on_title_times_broadcast;
            base_objects::event_register_id on_title_times;
            base_objects::event_register_id on_unsigned_message_broadcast;
            base_objects::event_register_id on_unsigned_message;

            storage::memory::online_player_storage& player_storage;

        public:
            CommunicationCorePlugin(storage::memory::online_player_storage& player_storage)
                : player_storage(player_storage) {}

            void OnLoad(const PluginRegistrationPtr& self) override {
                on_player_chat = api::players::calls::on_player_chat += ([this](const api::players::player_chat& chat) {
                    player_storage.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&chat](SharedClientData& client) {
                        client.sendPacket(
                            packets::play::playerChatMessage(
                                chat.sender->uuid,
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

                on_player_personal_chat = api::players::calls::on_player_personal_chat += ([this](const api::players::player_personal_chat& chat) {
                    chat.receiver->sendPacket(
                        packets::play::playerChatMessage(
                            chat.sender->uuid,
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
                            chat.sender->uuid,
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


                on_system_message_broadcast = api::players::calls::on_system_message_broadcast += ([this](const Chat& message) {
                    player_storage.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                        client.sendPacket(packets::play::systemChatMessage(message));
                        return false;
                    });
                    return false;
                });
                on_system_message = api::players::calls::on_system_message += ([this](const api::players::personal<Chat>& message) {
                    message.player->sendPacket(packets::play::systemChatMessage(message.data));
                    return false;
                });

                on_system_message_overlay_broadcast = api::players::calls::on_system_message_overlay_broadcast += ([this](const Chat& message) {
                    player_storage.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                        client.sendPacket(packets::play::systemChatMessageOverlay(message));
                        return false;
                    });
                    return false;
                });
                on_system_message_overlay = api::players::calls::on_system_message_overlay += ([this](const api::players::personal<Chat>& message) {
                    message.player->sendPacket(packets::play::systemChatMessageOverlay(message.data));
                    return false;
                });

                on_player_kick = api::players::calls::on_player_kick += ([this](const api::players::personal<Chat>& message) {
                    message.player->sendPacket(packets::play::kick(message.data));
                    player_storage.remove_player(message.player);
                    return false;
                });

                on_player_ban = api::players::calls::on_player_ban += ([this](const api::players::personal<Chat>& message) {
                    message.player->sendPacket(packets::play::kick(message.data));
                    player_storage.remove_player(message.player);
                    return false;
                });

                on_action_bar_message_broadcast = api::players::calls::on_action_bar_message_broadcast += ([this](const Chat& message) {
                    player_storage.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                        client.sendPacket(packets::play::setActionBarText(message));
                        return false;
                    });
                    return false;
                });
                on_action_bar_message = api::players::calls::on_action_bar_message += ([this](const api::players::personal<Chat>& message) {
                    message.player->sendPacket(packets::play::setActionBarText(message.data));
                    return false;
                });

                on_title_message_broadcast = api::players::calls::on_title_message_broadcast += ([this](const Chat& message) {
                    player_storage.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                        client.sendPacket(packets::play::setTitleText(message));
                        return false;
                    });
                    return false;
                });
                on_title_message = api::players::calls::on_title_message += ([this](const api::players::personal<Chat>& message) {
                    message.player->sendPacket(packets::play::setTitleText(message.data));
                    return false;
                });

                on_subtitle_message_broadcast = api::players::calls::on_subtitle_message_broadcast += ([this](const Chat& message) {
                    player_storage.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                        client.sendPacket(packets::play::setSubtitleText(message));
                        return false;
                    });
                    return false;
                });
                on_subtitle_message = api::players::calls::on_subtitle_message += ([this](const api::players::personal<Chat>& message) {
                    message.player->sendPacket(packets::play::setSubtitleText(message.data));
                    return false;
                });

                on_title_times_broadcast = api::players::calls::on_title_times_broadcast += ([this](const api::players::titles_times& times) {
                    player_storage.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&times](SharedClientData& client) {
                        client.sendPacket(packets::play::setTitleAnimationTimes(times.fade_in, times.stay, times.fade_out));
                        return false;
                    });
                    return false;
                });
                on_title_times = api::players::calls::on_title_times += ([this](const api::players::personal<api::players::titles_times>& times) {
                    times.player->sendPacket(packets::play::setTitleAnimationTimes(times.data.fade_in, times.data.stay, times.data.fade_out));
                    return false;
                });

                on_unsigned_message_broadcast = api::players::calls::on_unsigned_message_broadcast += ([this](const api::players::unsigned_chat& message) {
                    player_storage.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                        client.sendPacket(packets::play::disguisedChatMessage(message.message, message.chat_type_id, message.sender_name, message.receiver_name));
                        return false;
                    });
                    return false;
                });
                on_unsigned_message = api::players::calls::on_unsigned_message += ([this](const api::players::personal<api::players::unsigned_chat>& message) {
                    message.player->sendPacket(packets::play::disguisedChatMessage(message.data.message, message.data.chat_type_id, message.data.sender_name, message.data.receiver_name));
                    return false;
                });
            }

            void OnUnload(const PluginRegistrationPtr& self) override {
                api::players::calls::on_player_chat.leave(on_player_chat);
                api::players::calls::on_player_personal_chat.leave(on_player_personal_chat);
                api::players::calls::on_system_message_broadcast.leave(on_system_message_broadcast);
                api::players::calls::on_system_message.leave(on_system_message);
                api::players::calls::on_system_message_overlay_broadcast.leave(on_system_message_overlay_broadcast);
                api::players::calls::on_system_message_overlay.leave(on_system_message_overlay);
                api::players::calls::on_player_kick.leave(on_player_kick);
                api::players::calls::on_player_ban.leave(on_player_ban);
                api::players::calls::on_action_bar_message_broadcast.leave(on_action_bar_message_broadcast);
                api::players::calls::on_action_bar_message.leave(on_action_bar_message);
                api::players::calls::on_title_message_broadcast.leave(on_title_message_broadcast);
                api::players::calls::on_title_message.leave(on_title_message);
                api::players::calls::on_subtitle_message_broadcast.leave(on_subtitle_message_broadcast);
                api::players::calls::on_subtitle_message.leave(on_subtitle_message);
                api::players::calls::on_title_times_broadcast.leave(on_title_times_broadcast);
                api::players::calls::on_title_times.leave(on_title_times);
                api::players::calls::on_unsigned_message_broadcast.leave(on_unsigned_message_broadcast);
                api::players::calls::on_unsigned_message.leave(on_unsigned_message);
            }

            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override {
                browser.add_child({"broadcast"})
                    .add_child({"<message>", "broadcast <message>", "Broadcast a message to all players"})
                    .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& ignored__) {
                        player_storage.iterate_players(SharedClientData::packets_state_t::protocol_state::play, [args](SharedClientData& client) {
                            client.sendPacket(packets::play::systemChatMessage(args[0]));
                            return false;
                        });
                    });
            }
        };
    } // namespace build_in_plugins

} // namespace crafted_craft


#endif /* SRC_BUILD_IN_PLUGINS_COMMUNICATION_CORE */
