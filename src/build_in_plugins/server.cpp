#include "server.hpp"
#include "../api/command.hpp"
#include "../api/console.hpp"
#include "../api/players.hpp"
#include "../log.hpp"
#include "../plugin/main.hpp"
#include "../plugin/registration.hpp"

namespace crafted_craft {
    class Server;

    namespace build_in_plugins {

        ServerPlugin::ServerPlugin()
            : players_data(Server::instance().config.server.get_storage_path() / "players") {}

        void ServerPlugin::OnRegister(const PluginRegistrationPtr& self) {
            log::info("Server", "starting server...");
        }

        void ServerPlugin::OnLoad(const PluginRegistrationPtr& self) {
            try {
                Server::instance().permissions_manager.sync();
            } catch (const std::exception& ex) {
                log::error("Server", std::string("[permissions] Failed to load permission file because: ") + ex.what());
            }
            api::command::register_manager(manager);
            manager.reload_commands();
            log::info("Server", "server handler loaded.");
        }

        void ServerPlugin::OnPostLoad(const std::shared_ptr<PluginRegistration>&) {
            if (api::console::console_enabled())
                api::console::execute_as_console("version");
        }

        void ServerPlugin::OnUnload(const PluginRegistrationPtr& self) {
            log::commands::unloadCommandSuggestion();
            PluginRegistration::OnUnload(self);
            log::info("Server", "server handler unloaded.");
        }

        void ServerPlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
            using predicate = base_objects::predicate;
            using pred_string = base_objects::predicates::string;
            using cmd_pred_string = base_objects::predicates::command::string;
            {
                browser.add_child({"help", "returns list of commands", ""})
                    .set_callback("command.help", [browser, this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        api::players::calls::on_system_message({client, {"help for all commands:\n" + browser.get_documentation()}});
                    })
                    .add_child({"[command]", "returns help for command", "/help [command]"}, cmd_pred_string::greedy_phrase)
                    .set_callback("command.help", [browser, this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        auto command = browser.open(std::get<pred_string>(args[0]).value);
                        if (!command.is_valid())
                            api::players::calls::on_system_message({client, {"Command not found"}});
                        else
                            api::players::calls::on_system_message({client, {command.get_documentation()}});
                    });

                browser.add_child({"?", "help alias"}).set_redirect("help", [browser](base_objects::command& cmd, const list_array<predicate>& args, const std::string& left, base_objects::client_data_holder& client) {
                    browser.get_manager().execute_command_from(left, cmd, client);
                });
            }
            {
                browser.add_child({"version", "returns server version", "/version"})
                    .set_callback("command.version", [this](const list_array<predicate>&, base_objects::client_data_holder& client) {
                        api::players::calls::on_system_message({client, {"Server version: 1.0.0. Build: " __DATE__ " " __TIME__}});
                    });
            }
            {
                browser.add_child("kick")
                    .add_child("<player>", cmd_pred_string::quotable_phrase)
                    .set_callback("command.kick", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        auto target = Server::instance().online_players.get_player(
                            SharedClientData::packets_state_t::protocol_state::play,
                            std::get<pred_string>(args[0]).value
                        );
                        if (!target) {
                            api::players::calls::on_system_message({client, "Player not found"});
                            return;
                        }
                        if (Server::instance().permissions_manager.has_rights("misc.operator_protection.kick", target)) {
                            api::players::calls::on_system_message({client, "You can't kick this player"});
                            return;
                        }
                        api::players::calls::on_player_kick({target, "kicked by admin"});
                    })
                    .add_child({"[reason]", "kick player with reason", "/kick <player> [reason]"}, cmd_pred_string::greedy_phrase)
                    .set_callback("command.kick", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        auto target = Server::instance().online_players.get_player(
                            SharedClientData::packets_state_t::protocol_state::play,
                            std::get<pred_string>(args[0]).value
                        );
                        if (!target) {
                            api::players::calls::on_system_message({client, "Player not found"});
                            return;
                        }
                        if (Server::instance().permissions_manager.has_rights("misc.operator_protection.kick", target)) {
                            api::players::calls::on_system_message({client, "You can't kick this player"});
                            return;
                        }
                        if (target)
                            api::players::calls::on_player_kick({target, Chat::parseToChat(std::get<pred_string>(args[1]).value)});
                        else
                            api::players::calls::on_system_message({client, "Player not found"});
                    });
            }
            {
                browser.add_child({"stop", "stop server", "/stop"})
                    .set_callback("command.stop", [this](const list_array<predicate>&, base_objects::client_data_holder& client) {
                        Server::instance().stop();
                    });
            }
            {
                auto _config = browser.add_child("config");
                _config
                    .add_child({"reload", "reloads config from file", "/config reload"})
                    .set_callback({"command.config.reload", {"console"}}, [&](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        Server::instance().config.load(Server::instance().config.server.base_path);
                        pluginManagement.registeredPlugins().for_each([&](const PluginRegistrationPtr& plugin) {
                            plugin->OnConfigReload(plugin, Server::instance().config);
                        });
                    });
                _config.add_child("set")
                    .add_child("<config item>", cmd_pred_string::quotable_phrase)
                    .add_child({"<value>", "updates config in file and applies for program", "/config set <config item> <value>"}, cmd_pred_string::greedy_phrase)
                    .set_callback({"command.config.set", {"console"}}, [&](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        Server::instance().config.set(Server::instance().config.server.base_path, std::get<pred_string>(args[0]).value, std::get<pred_string>(args[1]).value);
                        api::players::calls::on_system_message({client, {"Config updated"}});
                        pluginManagement.registeredPlugins().for_each([&](const PluginRegistrationPtr& plugin) {
                            plugin->OnConfigReload(plugin, Server::instance().config);
                        });
                    });
                _config
                    .add_child("get")
                    .add_child({"<config item>", "returns config value", "/config get <config item>"}, cmd_pred_string::quotable_phrase)
                    .set_callback({"command.config.get", {"console"}}, [&](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        auto value = Server::instance().config.get(std::get<pred_string>(args[0]).value);
                        while (value.ends_with('\n') | value.ends_with('\r'))
                            value.pop_back();
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


            //response += packets::play::joinGame(
            //    0,
            //    client.player_data.hardcore_hearts,
            //    registers::dimensionTypes.convert<std::string>([](auto& a) { return a.name; }),
            //    100,
            //    2,
            //    2,
            //    client.player_data.reduced_debug_info,
            //    true,
            //    false,
            //    0, //client.player_data.world_id.get(),
            //    client.player_data.world_id.get(),
            //    0,
            //    client.player_data.gamemode,
            //    client.player_data.prev_gamemode,
            //    false,
            //    false,
            //    last_death_location,
            //    0,
            //    false
            //);
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
