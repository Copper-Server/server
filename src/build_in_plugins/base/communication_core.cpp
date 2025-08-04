#include <library/list_array.hpp>
#include <src/api/client.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/commands.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/storage/memory/online_player.hpp>

namespace copper_server::build_in_plugins {
    //provides and manages chat system
    struct CommunicationCorePlugin : public PluginAutoRegister<"base/communication_core", CommunicationCorePlugin> {
        void OnLoad(const PluginRegistrationPtr& self) override {
            register_event(api::players::calls::on_player_kick, base_objects::events::priority::low, [this](const api::players::personal<Chat>& message) {
                switch (message.player->packets_state.state) {
                case base_objects::SharedClientData::packets_state_t::protocol_state::handshake:
                case base_objects::SharedClientData::packets_state_t::protocol_state::status:
                    message.player->sendPacket(base_objects::network::response::disconnect());
                    break;
                case base_objects::SharedClientData::packets_state_t::protocol_state::login:
                    *message.player << api::client::login::login_disconnect{.reason = message.data.ToStr()};
                    break;
                case base_objects::SharedClientData::packets_state_t::protocol_state::configuration:
                    *message.player << api::client::configuration::disconnect{.reason = message.data};
                    break;
                case base_objects::SharedClientData::packets_state_t::protocol_state::play:
                    *message.player << api::client::configuration::disconnect{.reason = message.data};
                    break;
                }
                return false;
            });

            register_event(api::players::calls::on_player_ban, base_objects::events::priority::low, [this](const api::players::personal<Chat>& message) {
                switch (message.player->packets_state.state) {
                case base_objects::SharedClientData::packets_state_t::protocol_state::handshake:
                case base_objects::SharedClientData::packets_state_t::protocol_state::status:
                    message.player->sendPacket(base_objects::network::response::disconnect());
                    break;
                case base_objects::SharedClientData::packets_state_t::protocol_state::login:
                    *message.player << api::client::login::login_disconnect{.reason = message.data.ToStr()};
                    break;
                case base_objects::SharedClientData::packets_state_t::protocol_state::configuration:
                    *message.player << api::client::configuration::disconnect{.reason = message.data};
                    break;
                case base_objects::SharedClientData::packets_state_t::protocol_state::play:
                    *message.player << api::client::configuration::disconnect{.reason = message.data};
                    break;
                }
                return false;
            });
            log::info("Communication Core", "chat handlers registered.");
        }

        void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override {
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;
            using cmd_pred_entity = base_objects::parsers::command::entity;
            browser.add_child("broadcast")
                .add_child({"<message>", "broadcast <message>", "Broadcast a message to all players"}, cmd_pred_string::greedy_phrase)
                .set_callback("command.broadcast", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto msg = Chat::parseToChat(std::get<pred_string>(args[0]).value);
                    api::players::iterate_online([&msg](base_objects::SharedClientData& context) {
                        context << api::client::play::system_chat{.content = msg};
                        return false;
                    });
                });
            browser.add_child("msg")
                .add_child("<target>", cmd_pred_string::quotable_phrase)
                .add_child({"<message>", "msg <target> <message>", "Send private message to specified player"}, cmd_pred_string::greedy_phrase)
                .set_callback("command.msg", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto target = api::players::get_player(std::get<pred_string>(args[0]).value);
                    if (!target) {
                        context.executor << api::client::play::system_chat{.content = "Player not found"};
                        return;
                    }
                    Chat message = Chat::parseToChat(std::get<pred_string>(args[1]).value);
                    context.executor << api::client::play::system_chat{.content = {"To " + target->name + ": ", message}};
                    *target << api::client::play::system_chat{.content = {"From " + context.executor.name + ": ", message}};
                });
            browser.add_child("chat")
                .add_child({"<message>", "chat <message>", "Send message to chat"}, cmd_pred_string::greedy_phrase)
                .set_callback("command.chat", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto msg = Chat{"[" + context.executor.name + "] ", Chat::parseToChat(std::get<pred_string>(args[0]).value)};
                    api::players::iterate_online([&msg](base_objects::SharedClientData& context) {
                        context << api::client::play::system_chat{.content = msg};
                        return false;
                    });
                });
            browser.add_child("whoami")
                .set_callback("command.whoami", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    context.executor << api::client::play::system_chat{.content = "You are " + context.executor.name};
                });
            browser.add_child("tellraw")
                .add_child({"<message>", "tellraw <message>", "Broadcast raw message for everyone."}, cmd_pred_string::greedy_phrase)
                .set_callback("command.tellraw", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto msg = Chat::fromStr(std::get<pred_string>(args[0]).value);
                    api::players::iterate_online([&msg](base_objects::SharedClientData& context) {
                        context << api::client::play::system_chat{.content = msg};
                        return false;
                    });
                });
            {
                auto title = browser
                                 .add_child("title")
                                 .add_child("<target>", cmd_pred_entity{.only_player_entity = true});
                title.add_child({"clear", "title <target> clear", "Clear title"})
                    .set_callback("command.title.clear", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        //TODO
                        //api::players::iterate_online([&context](base_objects::SharedClientData& context) {
                        //    return false;
                        //});
                    });
            }
        }
    };
}