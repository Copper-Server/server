#include "communication_core.hpp"
#include "../ClientHandleHelper.hpp"
#include "../api/players.hpp"
#include "../library/list_array.hpp"
#include "../protocolHelper/packets.hpp"
#include "../storage/memory/online_player.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        CommunicationCorePlugin::CommunicationCorePlugin() {}

        void CommunicationCorePlugin::OnLoad(const PluginRegistrationPtr& self) {
            register_event(api::players::calls::on_player_chat, [this](const api::players::player_chat& chat) {
                api::players::iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&chat](SharedClientData& client) {
                    client.sendPacket(
                        packets::play::playerChatMessage(
                            client,
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
                        *chat.receiver,
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
                        *chat.sender,
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
                api::players::iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                    client.sendPacket(packets::play::systemChatMessage(client,message));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_system_message, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::systemChatMessage(*message.player, message.data));
                return false;
            });

            register_event(api::players::calls::on_system_message_overlay_broadcast, [this](const Chat& message) {
                api::players::iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                    client.sendPacket(packets::play::systemChatMessageOverlay(client, message));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_system_message_overlay, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::systemChatMessageOverlay(*message.player, message.data));
                return false;
            });

            register_event(api::players::calls::on_player_kick, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::kick(*message.player,message.data));
                api::players::remove_player(message.player);
                return false;
            });

            register_event(api::players::calls::on_player_ban, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::kick(*message.player, message.data));
                api::players::remove_player(message.player);
                return false;
            });

            register_event(api::players::calls::on_action_bar_message_broadcast, [this](const Chat& message) {
                api::players::iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                    client.sendPacket(packets::play::setActionBarText(client, message));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_action_bar_message, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::setActionBarText(*message.player, message.data));
                return false;
            });

            register_event(api::players::calls::on_title_message_broadcast, [this](const Chat& message) {
                api::players::iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                    client.sendPacket(packets::play::setTitleText(client, message));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_title_message, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::setTitleText(*message.player, message.data));
                return false;
            });

            register_event(api::players::calls::on_title_clear_broadcast, [this](bool to_reset) {
                api::players::iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&to_reset](SharedClientData& client) {
                    client.sendPacket(packets::play::clearTitles(client, to_reset));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_title_clear, [this](const api::players::personal<bool>& message) {
                message.player->sendPacket(packets::play::clearTitles(*message.player, message.data));
                return false;
            });


            register_event(api::players::calls::on_subtitle_message_broadcast, [this](const Chat& message) {
                api::players::iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                    client.sendPacket(packets::play::setSubtitleText(client, message));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_subtitle_message, [this](const api::players::personal<Chat>& message) {
                message.player->sendPacket(packets::play::setSubtitleText(*message.player, message.data));
                return false;
            });

            register_event(api::players::calls::on_title_times_broadcast, [this](const api::players::titles_times& times) {
                api::players::iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&times](SharedClientData& client) {
                    client.sendPacket(packets::play::setTitleAnimationTimes(client, times.fade_in, times.stay, times.fade_out));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_title_times, [this](const api::players::personal<api::players::titles_times>& times) {
                times.player->sendPacket(packets::play::setTitleAnimationTimes(*times.player, times.data.fade_in, times.data.stay, times.data.fade_out));
                return false;
            });

            register_event(api::players::calls::on_unsigned_message_broadcast, [this](const api::players::unsigned_chat& message) {
                api::players::iterate_players(SharedClientData::packets_state_t::protocol_state::play, [&message](SharedClientData& client) {
                    client.sendPacket(packets::play::disguisedChatMessage(client, message.message, message.chat_type_id, message.sender_name, message.receiver_name));
                    return false;
                });
                return false;
            });
            register_event(api::players::calls::on_unsigned_message, [this](const api::players::personal<api::players::unsigned_chat>& message) {
                message.player->sendPacket(packets::play::disguisedChatMessage(*message.player, message.data.message, message.data.chat_type_id, message.data.sender_name, message.data.receiver_name));
                return false;
            });
        }

        void CommunicationCorePlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;
            using cmd_pred_entity = base_objects::parsers::command::entity;
            browser.add_child("broadcast")
                .add_child({"<message>", "broadcast <message>", "Broadcast a message to all players"}, cmd_pred_string::greedy_phrase)
                .set_callback("command.broadcast", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    api::players::calls::on_system_message_broadcast(Chat::parseToChat(std::get<pred_string>(args[0]).value));
                });
            browser.add_child("msg")
                .add_child("<target>", cmd_pred_string::quotable_phrase)
                .add_child({"<message>", "msg <target> <message>", "Send private message to specified player"}, cmd_pred_string::greedy_phrase)
                .set_callback("command.msg", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto target = api::players::get_player(std::get<pred_string>(args[0]).value);
                    if (!target) {
                        api::players::calls::on_system_message({context.executor, "Player not found"});
                        return;
                    }
                    Chat message = Chat::parseToChat(std::get<pred_string>(args[1]).value);
                    api::players::calls::on_system_message({context.executor, {"To " + target->name + ": ", message}});
                    api::players::calls::on_system_message({target, {"From " + context.executor->name + ": ", message}});
                });
            browser.add_child("chat")
                .add_child({"<message>", "chat <message>", "Send message to chat"}, cmd_pred_string::greedy_phrase)
                .set_callback("command.chat", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    api::players::calls::on_system_message_broadcast({"[" + context.executor->name + "] ", Chat::parseToChat(std::get<pred_string>(args[0]).value)});
                });
            browser.add_child("whoami")
                .set_callback("command.whoami", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    api::players::calls::on_system_message({context.executor, "You are " + context.executor->name});
                });
            browser.add_child("tellraw")
                .add_child({"<message>", "tellraw <message>", "Broadcast raw message for everyone."}, cmd_pred_string::greedy_phrase)
                .set_callback("command.tellraw", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    api::players::calls::on_system_message_broadcast(Chat::fromStr(std::get<pred_string>(args[0]).value));
                });
            {
                auto title = browser
                                 .add_child("title")
                                 .add_child("<target>", cmd_pred_entity{.only_player_entity = true});
                title.add_child({"clear", "title <target> clear", "Clear title"})
                    .set_callback("command.title.clear", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        //TODO
                        //api::players::iterate_online([&context](SharedClientData& context) {
                        //    return false;
                        //});
                    });
            }
        }
    } // namespace build_in_plugins

} // namespace crafted_craft
