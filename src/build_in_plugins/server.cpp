#include "server.hpp"
#include "../api/players.hpp"
#include "../log.hpp"
#include "../plugin/registration.hpp"
#include "../protocolHelper/state_play.hpp"

namespace crafted_craft {
    class TCPserver;

    namespace build_in_plugins {

        ServerPlugin::ServerPlugin(const std::filesystem::path& base_path, const std::filesystem::path& storage_path, TCPserver& server)
            : players_data(storage_path / "players"),
              base_path(base_path),
              console_data(server.online_players.allocate_player(), "Console", "Console", server),
              server(server) {
        }

        void ServerPlugin::OnLoad(const PluginRegistrationPtr& self) {
            log::info("Server", "starting server...");
            manager.reload_commands();
            register_event(log::commands::on_command, base_objects::event_priority::heigh, [&](const std::string& command) {
                try {
                    manager.execute_command(command, console_data.client);
                } catch (const std::exception& ex) {
                    log::error("Server", "[command] " + command + "\n Failed to execute command, reason:\n\t" + ex.what());
                    return false;
                }
                log::info("Server", "[command] " + command);
                return true;
            });
            log::commands::registerCommandSuggestion([this](const std::string& line, int position) {
                auto tmp = line;
                if (uint32_t(position) <= line.size())
                    tmp.resize(position);
                else
                    tmp += ' ';
                auto insertion_ = tmp;

                if (!insertion_.starts_with(' ')) {
                    auto it = insertion_.find_last_of(' ');
                    if (insertion_.npos == it)
                        insertion_.clear();
                    else
                        insertion_ = insertion_.substr(0, it) + ' ';
                }
                return manager
                    .request_suggestions(tmp, console_data.client)
                    .convert<std::string>([&insertion_](auto&& suggestion) { return insertion_ + suggestion.insertion; })
                    .to_container<std::vector<std::string>>();
            });
            console_data.systemChatMessage = [this](const Chat& message) {
                log::info("Server", message.to_ansi_console());
            };
            console_data.systemChatMessageOverlay = [this](const Chat& message) {
                log::info("Server", message.to_ansi_console());
            };
            console_data.disguisedChatMessage = [this](const Chat& message, int32_t chat_type, const Chat& sender, std::optional<Chat> target_name) {
                if (!target_name)
                    log::info("Server", "[" + sender.to_ansi_console() + "] " + message.to_ansi_console());
                else
                    log::info("Server", "[" + sender.to_ansi_console() + " -> " + target_name->to_ansi_console() + "] " + message.to_ansi_console());
            };


            log::info("Server", "server handler loaded.");
        }

        void ServerPlugin::OnUnload(const PluginRegistrationPtr& self) {
            log::commands::unloadCommandSuggestion();
            PluginRegistration::OnUnload(self);
            log::info("Server", "server handler unloaded.");
        }

        void ServerPlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
            {
                browser.add_child({"help", "returns list of commands", ""})
                    .set_callback([browser](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        api::players::calls::on_system_message({client, {"help for all commands:\n" + browser.get_documentation()}});
                    })
                    .add_child({"<command>", "returns help for command", "/help <command>"}, base_objects::command::parsers::brigadier_string, {.flags = 2})
                    .set_callback([browser](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        auto command = browser.open(args[0]);
                        if (!command.is_valid())
                            api::players::calls::on_system_message({client, {"Command not found"}});
                        else
                            api::players::calls::on_system_message({client, {command.get_documentation()}});
                    });

                browser.add_child({"?", "help alias"}).set_redirect("help", [browser](const list_array<std::string>& args, const std::string& left, base_objects::client_data_holder& client) {
                    browser.get_manager().execute_command("help" + (left.size() ? " " + left : ""), client);
                });
            }
            {
                browser.add_child({"version", "returns server version", "/version"})
                    .set_callback([](const list_array<std::string>&, base_objects::client_data_holder& client) {
                        if (client->player_data.op_level < 4)
                            throw std::exception("Not enough permissions for this.");
                        api::players::calls::on_system_message({client, {"Server version: 1.0.0. Build: " __DATE__ " " __TIME__}});
                    });
            }
            {
                browser.add_child({"op"})
                    .add_child({"<player>", "op player", "/op <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                    .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        if (client->player_data.op_level < 5)
                            throw std::exception("Not enough permissions for this.");
                        if (args.size() == 0) {
                            api::players::calls::on_system_message({client, {"Usage: /op <player>"}});
                            return;
                        }
                        auto target = server.online_players.get_player(
                            SharedClientData::packets_state_t::protocol_state::play,
                            args[0]
                        );
                        if (!target) {
                            api::players::calls::on_system_message({client, "Player not found"});
                            return;
                        }
                        if (target->player_data.op_level > 4) {
                            api::players::calls::on_system_message({client, "This player has more permissions than OP"});
                            return;
                        }
                        target->player_data.op_level = 4;
                        api::players::calls::on_system_message_broadcast({"Player " + target->name + "(" + target->data->uuid_str + ") is now OP"});
                    });
            }
            {
                browser.add_child({"deop"})
                    .add_child({"<player>", "deop player", "/deop <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                    .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        if (client->player_data.op_level < 5)
                            throw std::exception("Not enough permissions for this.");
                        if (args.size() == 0) {
                            api::players::calls::on_system_message({client, {"Usage: /deop <player>"}});
                            return;
                        }
                        auto target = server.online_players.get_player(
                            SharedClientData::packets_state_t::protocol_state::play,
                            args[0]
                        );
                        if (!target) {
                            api::players::calls::on_system_message({client, "Player not found or offline"});
                            return;
                        }
                        if (target->player_data.op_level > 4) {
                            api::players::calls::on_system_message({client, "This player has more permissions than OP"});
                            return;
                        }
                        target->player_data.op_level = 4;
                        api::players::calls::on_system_message_broadcast({"Player " + target->name + "(" + target->data->uuid_str + ") is now OP"});
                    });
            }
            {

                browser.add_child({"kick"})
                    .add_child({"<player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                    .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        if (client->player_data.op_level < 4)
                            throw std::exception("Not enough permissions for this.");
                        if (args.size() == 0) {
                            api::players::calls::on_system_message({client, {"Usage: /kick <player>"}});
                            return;
                        }
                        auto target = server.online_players.get_player(
                            SharedClientData::packets_state_t::protocol_state::play,
                            args[0]
                        );
                        if (!target) {
                            api::players::calls::on_system_message({client, "Player not found"});
                            return;
                        }
                        if (target->player_data.op_level > 4) {
                            api::players::calls::on_system_message({client, "You can't kick this player"});
                            return;
                        }
                        api::players::calls::on_player_kick({target, "kicked by admin"});
                    })
                    .add_child({"<reason>", "kick player with reason", "/kick <player> [reason]"}, base_objects::command::parsers::brigadier_string, {.flags = 2})
                    .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        if (client->player_data.op_level < 4)
                            throw std::exception("Not enough permissions for this.");
                        if (args.size() == 0) {
                            api::players::calls::on_system_message({client, {"Usage: /kick <player>"}});
                            throw std::exception("Not enough permissions for this.");
                        }
                        auto target = server.online_players.get_player(
                            SharedClientData::packets_state_t::protocol_state::play,
                            args[0]
                        );
                        if (!target) {
                            api::players::calls::on_system_message({client, "Player not found"});
                            return;
                        }
                        if (target->player_data.op_level > 4) {
                            api::players::calls::on_system_message({client, "You can't kick this player"});
                            return;
                        }
                        if (target)
                            api::players::calls::on_player_kick({target, Chat::parseToChat(args[1])});
                        else
                            api::players::calls::on_system_message({client, "Player not found"});
                    });
            }
            {
                browser.add_child({"stop", "stop server", "/stop"})
                    .set_callback([this](const list_array<std::string>&, base_objects::client_data_holder& client) {
                        if (client->player_data.op_level < 4)
                            throw std::exception("Not enough permissions for this.");
                        server.stop();
                    });
            }
            {
                auto _config = browser.add_child({"config"});
                _config.add_child({"reload", "reloads config from file", "/config reload"}).set_callback([&](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                    if (client->player_data.op_level < 4)
                        throw std::exception("Not enough permissions for this.");
                    server.server_config.load(base_path);
                    pluginManagement.registeredPlugins().forEach([&](const PluginRegistrationPtr& plugin) {
                        plugin->OnConfigReload(plugin, server.server_config);
                    });
                });
                _config.add_child({"set"}).add_child({"[config item]"}, base_objects::command::parsers::brigadier_string, {.flags = 1}).add_child({"[value]", "updates config in file and applies for program", "/config set [config item] [value]"}, base_objects::command::parsers::brigadier_string, {.flags = 2}).set_callback([&](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                    if (client->player_data.op_level < 4)
                        throw std::exception("Not enough permissions for this.");
                    if (args.size() < 2) {
                        api::players::calls::on_system_message({client, {"Usage: /config set <config item> <value>"}});
                        return;
                    }
                    server.server_config.set(base_path, args[0], args[1]);
                    api::players::calls::on_system_message({client, {"Config updated"}});
                    pluginManagement.registeredPlugins().forEach([&](const PluginRegistrationPtr& plugin) {
                        plugin->OnConfigReload(plugin, server.server_config);
                    });
                });
                _config.add_child({"get"}).add_child({"[config item]", "returns config value", "/config get [config item]"}, base_objects::command::parsers::brigadier_string, {.flags = 1}).set_callback([&](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                    if (client->player_data.op_level < 4)
                        throw std::exception("Not enough permissions for this.");
                    if (args.size() == 0) {
                        api::players::calls::on_system_message({client, {"Usage: /config get <config item>"}});
                        return;
                    }
                    auto value = server.server_config.get(args[0]);
                    if (value.contains("\n"))
                        api::players::calls::on_system_message({client, {"Config value: \n" + value}});
                    else
                        api::players::calls::on_system_message({client, {"Config value: " + value}});
                });
            }
        }

        ServerPlugin::plugin_response ServerPlugin::OnPlay_initialize(base_objects::client_data_holder& client_ref) {
            auto& client = *client_ref;
            {
                auto data = players_data.get_player_data(client.data->uuid_str);
                data.load();
                client.player_data = data.player;
            }
            Response response;
            auto last_death_location =
                client.player_data.last_death_location ? std::optional(base_objects::packets::death_location_data(
                                                             client.player_data.last_death_location->world_id,
                                                             {client.player_data.last_death_location->x, client.player_data.last_death_location->y, client.player_data.last_death_location->z}
                                                         ))
                                                       : std::nullopt;


            response += packets::play::joinGame(
                0,
                client.player_data.hardcore_hearts,
                registers::dimensionTypes.convert<std::string>([](auto& a) { return a.name; }),
                100,
                2,
                2,
                client.player_data.reduced_debug_info,
                true,
                false,
                client.player_data.world_id,
                client.player_data.world_id,
                0,
                client.player_data.gamemode,
                client.player_data.prev_gamemode,
                false,
                false,
                last_death_location,
                0
            );
            response += packets::play::playerAbilities(
                client.player_data.abilities.flags.mask,
                client.player_data.abilities.flying_speed,
                client.player_data.abilities.field_of_view_modifier
            );
            client.packets_state.pending_teleport_ids.push_back(0);
            response += packets::play::synchronizePlayerPosition(
                calc::VECTOR(client.player_data.position.x, client.player_data.position.y, client.player_data.position.z),
                client.player_data.position.yaw,
                client.player_data.position.pitch,
                0x1F,
                0
            );
            return response;
        }

        ServerPlugin::plugin_response ServerPlugin::OnPlay_uninitialized(base_objects::client_data_holder& client) {
            {
                auto data = players_data.get_player_data(client->data->uuid_str);
                data.player = client->player_data;
                data.save();
            }
            return false;
        }
    }
}
