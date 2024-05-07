#ifndef SRC_BUILD_IN_PLUGINS_BAN
#define SRC_BUILD_IN_PLUGINS_BAN
#include "../api/ban.hpp"
#include "../base_objects/commands.hpp"
#include "../log.hpp"
#include "../plugin/registration.hpp"
#include "../protocolHelper/state_play.hpp"
#include "../storage/enbt_list_storage.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        class BanPlugin : public PluginRegistration {
            storage::enbt_list_storage banned_players;
            storage::enbt_list_storage banned_ips;

        public:
            BanPlugin(const std::string& storage_path)
                : banned_players(storage_path + "/banned_players.c_enbt"),
                  banned_ips(storage_path + "/banned_ips.c_enbt") {
                if (!banned_players.is_loaded()) {
                    log::error("BanPlugin", "Failed to load banned players list");
                }
                if (!banned_ips.is_loaded()) {
                    log::error("BanPlugin", "Failed to load banned ips list");
                }
            }

            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override {
                {
                    browser.add_child({"ban", "", ""})
                        .add_child({"<player>", "ban player", "/ban <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                        .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            if (args.size() == 0) {
                                api::players::calls::on_system_message({client, {"Usage: /ban <player>"}});
                                return;
                            }
                            if (api::ban::on_ban({args[0], client->name, ""}))
                                return;

                            banned_players.add(args[0], {});
                            api::players::calls::on_system_message({client, {"Player " + args[0] + " has been banned."}});
                        })
                        .add_child({"<reason>", "ban player with reason", "/ban <player> [reason]"}, base_objects::command::parsers::brigadier_string, {.flags = 2})
                        .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            if (args.size() == 0) {
                                api::players::calls::on_system_message({client, {"Usage: /ban <player> [reason]"}});
                                return;
                            }
                            if (api::ban::on_ban({args[0], client->name, args[1]}))
                                return;
                            banned_players.add(args[0], args[1]);
                            api::players::calls::on_system_message({client, {"Player " + args[0] + " has been banned"}});
                        });
                }
                {
                    browser.add_child({"pardon", "", ""})
                        .add_child({"<player>", "pardon player", "/pardon <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                        .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            if (args.size() == 0) {
                                {
                                    api::players::calls::on_system_message({client, {"Usage: /pardon <player>"}});
                                    return;
                                }
                                return;
                            }
                            if (!api::ban::on_pardon({args[0], client->name, args[1]}))
                                return;
                            banned_players.remove(args[0]);
                            api::players::calls::on_system_message({client, {"Player " + args[0] + " has been pardoned."}});
                        });
                    browser.add_child({"unban", "pardon alias"}).set_redirect("pardon", [browser](const list_array<std::string>& args, const std::string& left, base_objects::client_data_holder& client) {
                        browser.get_manager().execute_command("pardon " + left, client);
                    });
                }
                {
                    auto ban_list = browser.add_child({"banlist", "list all banned players or ips", "/banlist ips|players"});
                    ban_list.add_child({"players", "list all banned players", "/banlist players"})
                        .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            auto banned = banned_players.keys();
                            if (banned.size() == 0) {
                                api::players::calls::on_system_message({client, {"There are no banned players."}});
                            } else if (banned.size() == 1) {
                                api::players::calls::on_system_message({client, {"There is only one banned player:" + banned.back()}});
                            } else {
                                std::string last_item = banned.back();
                                banned.pop_back();
                                std::string message = "There are " + std::to_string(banned.size()) + " total banned players:\n";
                                for (auto& player : banned)
                                    message += player + ", ";

                                message.erase(message.size() - 2, 2);
                                message += "and " + last_item + '.';
                                api::players::calls::on_system_message({client, {message}});
                            }
                        });
                    ban_list.add_child({"ips", "list all banned ips", "/banlist ips"})
                        .set_callback([this](const list_array<std::string>&, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            auto banned = banned_ips.keys();
                            if (banned.size() == 0) {
                                api::players::calls::on_system_message({client, {"There are no banned ips."}});
                            } else if (banned.size() == 1) {
                                api::players::calls::on_system_message({client, {"There is only one banned ip:" + banned.back()}});
                            } else {
                                std::string last_item = banned.back();
                                banned.pop_back();
                                std::string message = "There are " + std::to_string(banned.size()) + " total banned ips:\n";
                                for (auto& player : banned)
                                    message += player + ", ";

                                message.erase(message.size() - 2, 2);
                                message += "and " + last_item + '.';
                                api::players::calls::on_system_message({client, {message}});
                            }
                        });
                }
                {
                    browser.add_child({"ban-ip"})
                        .add_child({"<ip>", "ban ip", "/ban-ip <ip>"}, base_objects::packets::command_node::parsers::brigadier_string, {.flags = 1})
                        .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            if (args.size() == 0) {
                                {
                                    api::players::calls::on_system_message({client, {"Usage: /ban-ip <ip>"}});
                                    return;
                                }
                                return;
                            }
                            //ban ip
                            if (api::ban::on_ban_ip({args[0], client->name, ""}))
                                return;
                            banned_ips.add(args[0], {});
                            api::players::calls::on_system_message({client, {"IP " + args[0] + " has been banned."}});
                        })
                        .add_child({"<reason>", "ban ip with reason", "/ban-ip <ip> [reason]"}, base_objects::packets::command_node::parsers::brigadier_string, {.flags = 2})
                        .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            if (args.size() == 0) {
                                {
                                    api::players::calls::on_system_message({client, {"Usage: /ban-ip <ip> [reason]"}});
                                    return;
                                }
                                return;
                            }
                            //ban ip
                            if (api::ban::on_ban_ip({args[0], client->name, args[1]}))
                                return;
                            banned_ips.add(args[0], args[1]);
                            api::players::calls::on_system_message({client, {"IP " + args[0] + " has been banned."}});
                        });
                }
                {
                    browser.add_child({"pardon-ip"})
                        .add_child({"<ip>", "pardon ip", "/pardon-ip <ip>"}, base_objects::packets::command_node::parsers::brigadier_string, {.flags = 1})
                        .set_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            if (client->player_data.op_level < 4)
                                return;
                            if (args.size() == 0) {
                                api::players::calls::on_system_message({client, {"Usage: /pardon-ip <ip>"}});
                                return;
                            }
                            //pardon ip
                            if (!api::ban::on_pardon_ip({args[0], client->name, args[1]}))
                                return;
                            banned_ips.remove(args[0]);
                            api::players::calls::on_system_message({client, {"IP " + args[0] + " has been pardoned."}});
                        });
                    browser.add_child({"unban-ip", "pardon-ip alias"}).set_redirect("pardon-ip", [browser](const list_array<std::string>& args, const std::string& left, base_objects::client_data_holder& client) {
                        return browser.get_manager().execute_command("pardon-ip " + left, client);
                    });
                }
            }

            plugin_response OnPlay_initialize(base_objects::client_data_holder& client) override {
                if (auto banned = banned_players.get(client->name))
                    return packets::play::kick({"You are banned from this server\nReason: " + (std::string)*banned});
                if (auto banned = banned_ips.get(client->ip))
                    return packets::play::kick({"You are banned from this server\nReason: " + (std::string)*banned});
                return false;
            }
        };
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_BAN */
