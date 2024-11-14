#include <src/ClientHandleHelper.hpp>
#include <src/api/configuration.hpp>
#include <src/api/console.hpp>
#include <src/api/internal/command.hpp>
#include <src/api/permissions.hpp>
#include <src/api/players.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/player.hpp>
#include <src/build_in_plugins/server.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/plugin/registration.hpp>
#include <src/registers.hpp>

namespace copper_server {
    class Server;

    namespace build_in_plugins {

        ServerPlugin::ServerPlugin()
            : players_data(api::configuration::get().server.get_storage_path() / "players") {}

        void ServerPlugin::OnRegister(const PluginRegistrationPtr& self) {
            log::info("Server", "starting server...");
        }

        void ServerPlugin::OnLoad(const PluginRegistrationPtr& self) {
            try {
                api::permissions::make_sync();
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
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;
            {
                browser.add_child({"help", "returns list of commands", ""})
                    .set_callback("command.help", [browser, this](const list_array<predicate>& args, base_objects::command_context& context) {
                        api::players::calls::on_system_message({context.executor, {"help for all commands:\n" + browser.get_documentation()}});
                    })
                    .add_child({"[command]", "returns help for command", "/help [command]"}, cmd_pred_string::greedy_phrase)
                    .set_callback("command.help", [browser, this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto command = browser.open(std::get<pred_string>(args[0]).value);
                        if (!command.is_valid())
                            api::players::calls::on_system_message({context.executor, {"Command not found"}});
                        else
                            api::players::calls::on_system_message({context.executor, {command.get_documentation()}});
                    });

                browser.add_child({"?", "help alias"}).set_redirect("help", [browser](base_objects::command& cmd, const list_array<predicate>& args, const std::string& left, base_objects::command_context& context) {
                    browser.get_manager().execute_command_from(left, cmd, context);
                });
            }
            {
                browser.add_child({"version", "returns server version", "/version"})
                    .set_callback("command.version", [this](const list_array<predicate>&, base_objects::command_context& context) {
                        api::players::calls::on_system_message({context.executor, {"Server version: 1.0.0. Build: " __DATE__ " " __TIME__}});
                    });
            }
            {
                browser.add_child("kick")
                    .add_child("<player>", cmd_pred_string::quotable_phrase)
                    .set_callback("command.kick", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto target = api::players::get_player(
                            SharedClientData::packets_state_t::protocol_state::play,
                            std::get<pred_string>(args[0]).value
                        );
                        if (!target) {
                            api::players::calls::on_system_message({context.executor, "Player not found"});
                            return;
                        }
                        if (api::permissions::has_rights("misc.operator_protection.kick", target)) {
                            api::players::calls::on_system_message({context.executor, "You can't kick this player"});
                            return;
                        }
                        api::players::calls::on_player_kick({target, "kicked by admin"});
                    })
                    .add_child({"[reason]", "kick player with reason", "/kick <player> [reason]"}, cmd_pred_string::greedy_phrase)
                    .set_callback("command.kick", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto target = api::players::get_player(
                            SharedClientData::packets_state_t::protocol_state::play,
                            std::get<pred_string>(args[0]).value
                        );
                        if (!target) {
                            api::players::calls::on_system_message({context.executor, "Player not found"});
                            return;
                        }
                        if (api::permissions::has_rights("misc.operator_protection.kick", target)) {
                            api::players::calls::on_system_message({context.executor, "You can't kick this player"});
                            return;
                        }
                        if (target)
                            api::players::calls::on_player_kick({target, Chat::parseToChat(std::get<pred_string>(args[1]).value)});
                        else
                            api::players::calls::on_system_message({context.executor, "Player not found"});
                    });
            }
            {
                browser.add_child({"stop", "stop server", "/stop"})
                    .set_callback("command.stop", [this](const list_array<predicate>&, base_objects::command_context& context) {
                        Server::instance().stop();
                    });
            }


            {
                auto _config = browser.add_child("config");
                _config
                    .add_child({"reload", "reloads config from file", "/config reload"})
                    .set_callback({"command.config.reload", {"console"}}, [&](const list_array<predicate>& args, base_objects::command_context& context) {
                        api::configuration::get().load(api::configuration::get().server.base_path);
                        pluginManagement.registeredPlugins().for_each([&](const PluginRegistrationPtr& plugin) {
                            plugin->OnConfigReload(plugin, api::configuration::get());
                        });
                    });
                _config.add_child("set")
                    .add_child("<config item>", cmd_pred_string::quotable_phrase)
                    .add_child({"<value>", "updates config in file and applies for program", "/config set <config item> <value>"}, cmd_pred_string::greedy_phrase)
                    .set_callback({"command.config.set", {"console"}}, [&](const list_array<predicate>& args, base_objects::command_context& context) {
                        api::configuration::get().set(api::configuration::get().server.base_path, std::get<pred_string>(args[0]).value, std::get<pred_string>(args[1]).value);
                        api::players::calls::on_system_message({context.executor, {"Config updated"}});
                        pluginManagement.registeredPlugins().for_each([&](const PluginRegistrationPtr& plugin) {
                            plugin->OnConfigReload(plugin, api::configuration::get());
                        });
                    });
                _config
                    .add_child("get")
                    .add_child({"<config item>", "returns config value", "/config get <config item>"}, cmd_pred_string::quotable_phrase)
                    .set_callback({"command.config.get", {"console"}}, [&](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto value = api::configuration::get().get(std::get<pred_string>(args[0]).value);
                        while (value.ends_with('\n') | value.ends_with('\r'))
                            value.pop_back();
                        if (value.contains("\n"))
                            api::players::calls::on_system_message({context.executor, {"Config value: \n" + value}});
                        else
                            api::players::calls::on_system_message({context.executor, {"Config value: " + value}});
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
            auto last_death_location = client.player_data.last_death_location ? std::optional(base_objects::packets::death_location_data(
                                                                                    client.player_data.last_death_location->world_id,
                                                                                    {(int32_t)client.player_data.last_death_location->x, (int32_t)client.player_data.last_death_location->y, (int32_t)client.player_data.last_death_location->z}
                                                                                ))
                                                                              : std::nullopt;


            response += packets::play::joinGame(
                client,
                0,
                client.player_data.hardcore_hearts,
                registers::dimensionTypes_cache.convert<std::string>([](auto& a) { return a->first; }),
                100,
                2,
                2,
                client.player_data.reduced_debug_info,
                true,
                false,
                0, //client.player_data.world_id,
                client.player_data.world_id,
                0,
                client.player_data.gamemode,
                client.player_data.prev_gamemode,
                false,
                false,
                last_death_location,
                0,
                false
            );
            response += packets::play::playerAbilities(
                client,
                client.player_data.abilities.flags.mask,
                client.player_data.abilities.flying_speed,
                client.player_data.abilities.field_of_view_modifier
            );
            client.packets_state.pending_teleport_ids.push_back(0);
            response += packets::play::synchronizePlayerPosition(
                client,
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
