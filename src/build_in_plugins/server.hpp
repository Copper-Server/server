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
            storage::memory::online_player_storage& player_storage;
            storage::players_data players_data;
            base_objects::command_manager manager;

        public:
            ServerPlugin(const std::string& path, storage::memory::online_player_storage& player_storage)
                : players_data(path + "/players"), player_storage(player_storage) {
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
                        .set_callback([](const list_array<std::string>&, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            //kick all players
                            //unload all plugins
                            //load all plugins

                            //open back
                            return;
                        });
                }
                {
                    browser.add_child({"help", "returns list of commands", ""})
                        .set_callback([browser](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            return packets::play::systemChatMessage({"help for all commands:" + browser.get_documentation()});
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
                        browser.get_manager().execute_command("help" + left.size() ? " " + left : "", client);
                    });
                }
                {
                    browser.add_child({"version", "returns server version", "/version"})
                        .set_callback([](const list_array<std::string>&, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            api::players::calls::on_system_message({client, {"Server version: 1.0.0"}});
                        });
                }
                {
                    browser.add_child({"op"})
                        .add_child({"<player>", "op player", "/op <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                        .set_callback([](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            if (args.size() == 0) {
                                api::players::calls::on_system_message({client, {"Usage: /op <player>"}});
                                return;
                            }
                            //op player
                        });
                }
                {
                    browser.add_child({"deop"})
                        .add_child({"<player>", "deop player", "/deop <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                        .set_callback([](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            if (args.size() == 0) {
                                api::players::calls::on_system_message({client, {"Usage: /deop <player>"}});
                                return;
                            }
                            //deop player
                        });
                }
                {

                    browser.add_child({"kick"})
                        .add_child({"<player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                        .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            if (args.size() == 0) {
                                api::players::calls::on_system_message({client, {"Usage: /kick <player>"}});
                                return;
                            }
                            auto target = player_storage.get_player(
                                SharedClientData::packets_state_t::protocol_state::play,
                                args[0]
                            );
                            api::players::calls::on_player_kick({target, "kicked by admin"});
                        })
                        .add_child({"<reason>", "kick player with reason", "/kick <player> [reason]"}, base_objects::command::parsers::brigadier_string, {.flags = 2})
                        .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            if (args.size() == 0) {
                                api::players::calls::on_system_message({client, {"Usage: /kick <player>"}});
                                return;
                            }
                            auto target = player_storage.get_player(
                                SharedClientData::packets_state_t::protocol_state::play,
                                args[0]
                            );
                            if (target)
                                api::players::calls::on_player_kick({target, Chat::parseToChat(args[1])});
                            else
                                api::players::calls::on_system_message({client, "Player not found"});
                        });
                }
                {
                    browser.add_child({"stop", "stop server", "/stop"})
                        .set_callback([](const list_array<std::string>&, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            //stop server
                        });
                }
            }

            plugin_response OnPlay_initialize(base_objects::client_data_holder& client_ref) override {
                auto& client = *client_ref;
                {
                    auto data = players_data.get_player_data(client.str_uuid);
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

            plugin_response OnPlay_uninitialized(base_objects::client_data_holder& client) override {
                {
                    auto data = players_data.get_player_data(client->str_uuid);
                    data.player = client->player_data;
                    data.save();
                }
                return false;
            }
        };
    }
}


#endif /* SRC_BUILD_IN_PLUGINS_SERVER */
