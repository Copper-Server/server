#ifndef SRC_BUILD_IN_PLUGINS_COMMUNICATION_CORE
#define SRC_BUILD_IN_PLUGINS_COMMUNICATION_CORE
#include "../api/players.hpp"
#include "../base_objects/event.hpp"
#include "../plugin/registration.hpp"
#include "../protocolHelper/packets.hpp"
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

        public:
            fast_task::task_rw_mutex mutex;
            std::unordered_map<std::string, SharedClientData*> clients;

            plugin_response OnPlay_initialize(SharedClientData& client) override {
                fast_task::write_lock lock(mutex);

                if (auto it = clients.find(client.name); it != clients.end()) {
                    if (it->second == &client)
                        return false;
                    it->second->pending_packets.push_back(packets::play::kick({"Someone joined with your name"}));
                    it->second->on_disconnect = nullptr;
                    it->second = &client;
                } else
                    clients[client.name] = &client;
                client.on_disconnect = [this, &client]() {
                    {
                        fast_task::write_lock lock(mutex);
                        clients.erase(client.name);
                    }
                    api::players::handlers::on_player_leave.async_notify(client.name);
                };
                api::players::handlers::on_player_join.async_notify(client.name);
                return false;
            }

            void OnLoad(const PluginRegistrationPtr& self) override {
                on_player_chat = api::players::calls::on_player_chat += ([this](const api::players::player_chat& chat) {
                    fast_task::read_lock lock(mutex);
                    for (auto& [name, client] : clients) {
                        client->pending_packets.push_back(
                            packets::play::playerChatMessage(
                                chat.sender,
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
                                name,
                                std::nullopt
                            )
                        );
                    }
                    return false;
                });

                on_player_personal_chat = api::players::calls::on_player_personal_chat += ([this](const api::players::player_personal_chat& chat) {
                    fast_task::read_lock lock(mutex);
                    if (auto sender_it = clients.find(chat.sender); sender_it != clients.end())
                        if (auto receiver_it = clients.find(chat.receiver); receiver_it != clients.end()) {
                            auto& sender = *sender_it->second;
                            auto& receiver = *receiver_it->second;
                            receiver.pending_packets.push_back(
                                packets::play::playerChatMessage(
                                    sender.uuid,
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
                                    sender.name,
                                    receiver.uuid
                                )
                            );
                        }

                    return false;
                });


                on_system_message_broadcast = api::players::calls::on_system_message_broadcast += ([this](const Chat& message) {
                    fast_task::read_lock lock(mutex);
                    for (auto& [name, client] : clients)
                        client->pending_packets.push_back(packets::play::systemChatMessage(message));
                    return false;
                });
                on_system_message = api::players::calls::on_system_message += ([this](const api::players::personal<Chat>& message) {
                    fast_task::read_lock lock(mutex);
                    if (auto it = clients.find(message.player); it != clients.end())
                        it->second->pending_packets.push_back(packets::play::systemChatMessage(message.data));
                    return false;
                });

                on_system_message_overlay_broadcast = api::players::calls::on_system_message_overlay_broadcast += ([this](const Chat& message) {
                    fast_task::read_lock lock(mutex);
                    for (auto& [name, client] : clients)
                        client->pending_packets.push_back(packets::play::systemChatMessageOverlay(message));
                    return false;
                });
                on_system_message_overlay = api::players::calls::on_system_message_overlay += ([this](const api::players::personal<Chat>& message) {
                    fast_task::read_lock lock(mutex);
                    if (auto it = clients.find(message.player); it != clients.end())
                        it->second->pending_packets.push_back(packets::play::systemChatMessageOverlay(message.data));
                    return false;
                });

                on_player_kick = api::players::calls::on_player_kick += ([this](const api::players::personal<Chat>& message) {
                    fast_task::read_lock lock(mutex);
                    if (auto it = clients.find(message.player); it != clients.end()) {
                        it->second->pending_packets.push_back(packets::play::kick(message.data));
                        it->second->on_disconnect = nullptr;
                        clients.erase(it);
                    }
                    return false;
                });

                on_player_ban = api::players::calls::on_player_ban += ([this](const api::players::personal<Chat>& message) {
                    fast_task::read_lock lock(mutex);
                    if (auto it = clients.find(message.player); it != clients.end()) {
                        it->second->pending_packets.push_back(packets::play::kick(message.data));
                        it->second->on_disconnect = nullptr;
                        clients.erase(it);
                    }
                    return false;
                });

                on_action_bar_message_broadcast = api::players::calls::on_action_bar_message_broadcast += ([this](const Chat& message) {
                    fast_task::read_lock lock(mutex);
                    for (auto& [name, client] : clients)
                        client->pending_packets.push_back(packets::play::setActionBarText(message));
                    return false;
                });
                on_action_bar_message = api::players::calls::on_action_bar_message += ([this](const api::players::personal<Chat>& message) {
                    fast_task::read_lock lock(mutex);
                    if (auto it = clients.find(message.player); it != clients.end())
                        it->second->pending_packets.push_back(packets::play::setActionBarText(message.data));
                    return false;
                });

                on_title_message_broadcast = api::players::calls::on_title_message_broadcast += ([this](const Chat& message) {
                    fast_task::read_lock lock(mutex);
                    for (auto& [name, client] : clients)
                        client->pending_packets.push_back(packets::play::setTitleText(message));
                    return false;
                });
                on_title_message = api::players::calls::on_title_message += ([this](const api::players::personal<Chat>& message) {
                    fast_task::read_lock lock(mutex);
                    if (auto it = clients.find(message.player); it != clients.end())
                        it->second->pending_packets.push_back(packets::play::setTitleText(message.data));
                    return false;
                });

                on_subtitle_message_broadcast = api::players::calls::on_subtitle_message_broadcast += ([this](const Chat& message) {
                    fast_task::read_lock lock(mutex);
                    for (auto& [name, client] : clients)
                        client->pending_packets.push_back(packets::play::setSubtitleText(message));
                    return false;
                });
                on_subtitle_message = api::players::calls::on_subtitle_message += ([this](const api::players::personal<Chat>& message) {
                    fast_task::read_lock lock(mutex);
                    if (auto it = clients.find(message.player); it != clients.end())
                        it->second->pending_packets.push_back(packets::play::setSubtitleText(message.data));
                    return false;
                });

                on_title_times_broadcast = api::players::calls::on_title_times_broadcast += ([this](const api::players::titles_times& times) {
                    fast_task::read_lock lock(mutex);
                    for (auto& [name, client] : clients)
                        client->pending_packets.push_back(packets::play::setTitleAnimationTimes(times.fade_in, times.stay, times.fade_out));
                    return false;
                });
                on_title_times = api::players::calls::on_title_times += ([this](const api::players::personal<api::players::titles_times>& times) {
                    fast_task::read_lock lock(mutex);
                    if (auto it = clients.find(times.player); it != clients.end())
                        it->second->pending_packets.push_back(packets::play::setTitleAnimationTimes(times.data.fade_in, times.data.stay, times.data.fade_out));
                    return false;
                });

                on_unsigned_message_broadcast = api::players::calls::on_unsigned_message_broadcast += ([this](const api::players::unsigned_chat& message) {
                    fast_task::read_lock lock(mutex);
                    for (auto& [name, client] : clients)
                        client->pending_packets.push_back(packets::play::disguisedChatMessage(message.message, message.chat_type_id, message.sender_name, message.receiver_name));
                    return false;
                });
                on_unsigned_message = api::players::calls::on_unsigned_message += ([this](const api::players::personal<api::players::unsigned_chat>& message) {
                    fast_task::read_lock lock(mutex);
                    if (auto it = clients.find(message.player); it != clients.end())
                        it->second->pending_packets.push_back(packets::play::disguisedChatMessage(message.data.message, message.data.chat_type_id, message.data.sender_name, message.data.receiver_name));
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
        };
    } // namespace build_in_plugins

} // namespace crafted_craft


#endif /* SRC_BUILD_IN_PLUGINS_COMMUNICATION_CORE */
