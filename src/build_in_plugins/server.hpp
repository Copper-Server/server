#ifndef SRC_BUILD_IN_PLUGINS_SERVER
#define SRC_BUILD_IN_PLUGINS_SERVER
#include "../base_objects/commands.hpp"
#include "../log.hpp"
#include "../plugin/registration.hpp"
#include "../protocolHelper/state_play.hpp"
#include "../storage/players_data.hpp"
namespace crafted_craft {
    namespace build_in_plugins {
        class ServerPlugin : public PluginRegistration {
            storage::players_data players_data;
            base_objects::command_manager manager;

        public:
            ServerPlugin(const std::string& path)
                : players_data(path + "/players") {
            }

            void OnLoad(const PluginRegistrationPtr& self) override {
                log::info("Server", "starting server...");
                manager.reload_commands();
                log::info("Server", "server handler loaded.");
            }

            void OnReload(const std::shared_ptr<PluginRegistration>&) override {
                manager.reload_commands();
                log::info("Server", "server handler reloaded.");
            }

            void OnUnload(const PluginRegistrationPtr& self) override {
                log::info("Server", "server handler unloaded.");
            }

            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override {
                {
                    browser.add_child({"reload", "signal to reload server", "/reload"})
                        .set_callback([](const list_array<std::string>&, SharedClientData& client) -> TCPclient::Response {
                            if (client.player_data.op_level < 4)
                                return {};
                            //kick all players
                            //unload all plugins
                            //load all plugins

                            //open back
                            return {};
                        });
                }
                {
                    browser.add_child({"help", "returns list of commands", ""})
                        .set_callback(
                            [browser](const list_array<std::string>& args, SharedClientData& client) -> TCPclient::Response {
                                return packets::play::systemChatMessage({"help for all commands:" + browser.get_documentation()});
                            }
                        )
                        .add_child({"<command>", "returns help for command", "/help <command>"})
                        .set_callback(
                            [browser](const list_array<std::string>& args, SharedClientData& client) -> TCPclient::Response {
                                auto command = browser.open(args[0]);
                                if (!command.is_valid())
                                    return packets::play::systemChatMessage({"Command not found"});
                                return packets::play::systemChatMessage({command.get_documentation()});
                            },
                            base_objects::packets::command_node::parsers::brigadier_string,
                            {.flags = 3}
                        );

                    browser.add_child({"?", "help alias"}).set_redirect("help", [browser](const list_array<std::string>& args, const std::string& left, SharedClientData& client) -> TCPclient::Response {
                        return browser.get_manager().execute_command("help" + left.size() ? " " + left : "", client);
                    });
                }
                {
                    browser.add_child({"version", "returns server version", "/version"})
                        .set_callback([](const list_array<std::string>&, SharedClientData& client) -> TCPclient::Response {
                            if (client.player_data.op_level < 4)
                                return {};
                            return packets::play::systemChatMessage({"Server version: 1.0.0"});
                        });
                }
                {
                    browser.add_child({"op"})
                        .add_child({"<player>", "op player", "/op <player>"})
                        .set_callback(
                            [](const list_array<std::string>& args, SharedClientData& client) -> TCPclient::Response {
                                if (client.player_data.op_level < 4)
                                    return {};
                                if (args.size() == 0)
                                    return packets::play::systemChatMessage({"Usage: /op <player>"});
                                //op player
                                return {};
                            },
                            base_objects::packets::command_node::parsers::brigadier_string,
                            {.flags = 2}
                        );
                }
                {
                    browser.add_child({"deop"})
                        .add_child({"<player>", "deop player", "/deop <player>"})
                        .set_callback([](const list_array<std::string>& args, SharedClientData& client) -> TCPclient::Response {
                            if (client.player_data.op_level < 4)
                                return {};
                            if (args.size() == 0)
                                return packets::play::systemChatMessage({"Usage: /deop <player>"});
                            return {};
                            //deop player
                        },
                                      base_objects::packets::command_node::parsers::brigadier_string,
                                      {.flags = 2});
                }
                {

                    browser.add_child({"kick"})
                        .add_child({"<player>"})
                        .set_callback(
                            [](const list_array<std::string>& args, SharedClientData& client) -> TCPclient::Response {
                                if (client.player_data.op_level < 4)
                                    return {};
                                if (args.size() == 0)
                                    return packets::play::systemChatMessage({"Usage: /kick <player>"});
                                //kick player
                                return {};
                            },
                            base_objects::packets::command_node::parsers::brigadier_string,
                            {.flags = 2}
                        )
                        .add_child({"<reason>", "kick player with reason", "/kick <player> [reason]"})
                        .set_callback(
                            [](const list_array<std::string>& args, SharedClientData& client) -> TCPclient::Response {
                                if (client.player_data.op_level < 4)
                                    return {};
                                if (args.size() == 0)
                                    return packets::play::systemChatMessage({"Usage: /kick <player> [reason]"});
                                //kick player
                                return {};
                            },
                            base_objects::packets::command_node::parsers::brigadier_string,
                            {.flags = 3}
                        );
                }
                {
                    browser.add_child({"stop", "stop server", "/stop"})
                        .set_callback([](const list_array<std::string>&, SharedClientData& client) -> TCPclient::Response {
                            if (client.player_data.op_level < 4)
                                return {};
                            //stop server
                            return {};
                        });
                }
            }

            plugin_response OnPlay_initialize(SharedClientData& client) override {
                {
                    auto data = players_data.get_player_data(client.str_uuid);
                    data.load();
                    client.player_data = data.player;
                }
                TCPclient::Response response;
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

            plugin_response OnPlay_uninitialized(SharedClientData& client) override {
                {
                    auto data = players_data.get_player_data(client.str_uuid);
                    data.player = client.player_data;
                    data.save();
                }
                return false;
            }
        };
    }
}


#endif /* SRC_BUILD_IN_PLUGINS_SERVER */
