#include "ban.hpp"
#include "../ClientHandleHelper.hpp"
#include "../api/ban.hpp"
#include "../api/players.hpp"
#include "../base_objects/commands.hpp"
#include "../log.hpp"
#include "../plugin/main.hpp"
#include "../plugin/registration.hpp"
#include "../storage/enbt_list_storage.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        BanPlugin::BanPlugin()
            : banned_players(Server::instance().config.server.get_storage_path() / +"banned_players.c_enbt"),
              banned_ips(Server::instance().config.server.get_storage_path() / +"banned_ips.c_enbt"),
              server(Server::instance()) {
            if (!banned_players.is_loaded()) {
                log::error("BanPlugin", "Failed to load banned players list");
            }
            if (!banned_ips.is_loaded()) {
                log::error("BanPlugin", "Failed to load banned ips list");
            }
        }

        void BanPlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
            {
                browser.add_child({"ban", "", ""})
                    .add_child({"<player>", "ban player", "/ban <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                    .set_callback("command.ban", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        if (args.size() == 0) {
                            api::players::calls::on_system_message({client, {"Usage: /ban <player>"}});
                            return;
                        }
                        if (api::ban::on_ban({args[0], client->name, ""}))
                            return;

                        if (banned_players.contains(args[0])) {
                            api::players::calls::on_system_message({client, {"Player " + args[0] + " has been already banned."}});
                            return;
                        }
                        banned_players.add(args[0], {});
                        api::players::calls::on_system_message({client, {"Player " + args[0] + " has been banned."}});
                    })
                    .add_child({"[reason]", "ban player with reason", "/ban <player> [reason]"}, base_objects::command::parsers::brigadier_string, {.flags = 2})
                    .set_callback("command.ban:reason", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        if (args.size() == 0) {
                            api::players::calls::on_system_message({client, {"Usage: /ban <player> [reason]"}});
                            return;
                        }
                        if (api::ban::on_ban({args[0], client->name, args[1]}))
                            return;
                        if (banned_players.contains(args[0])) {
                            api::players::calls::on_system_message({client, {"Player " + args[0] + " has been already banned."}});
                            return;
                        }
                        banned_players.add(args[0], args[1]);
                        api::players::calls::on_system_message({client, {"Player " + args[0] + " has been banned"}});
                    });
            }
            {
                auto& pardon = browser.add_child({"pardon", "", ""})
                                   .add_child({"<player>", "pardon player", "/pardon <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                                   .set_callback("command.pardon", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                                       if (args.size() == 0) {
                                           {
                                               api::players::calls::on_system_message({client, {"Usage: /pardon <player>"}});
                                               return;
                                           }
                                           return;
                                       }
                                       if (api::ban::on_pardon({args[0], client->name, ""}))
                                           return;
                                       if (!banned_players.contains(args[0])) {
                                           api::players::calls::on_system_message({client, {"Player " + args[0] + " has not been banned."}});
                                           return;
                                       }
                                       banned_players.remove(args[0]);
                                       api::players::calls::on_system_message({client, {"Player " + args[0] + " has been pardoned."}});
                                   });
                browser.add_child({"unban", "pardon alias"}).add_child(pardon);
            }
            {
                auto ban_list = browser.add_child({"banlist", "list all banned players or ips", "/banlist ips|players"});
                {
                    auto players =
                        ban_list.add_child({"players", "list all banned players", "/banlist players"});


                    players.set_callback("command.banlist.players", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        bool max_reached = false;
                        auto banned = banned_players.keys(100, max_reached);
                        if (banned.size() == 0) {
                            api::players::calls::on_system_message({client, {"There are no banned players."}});
                        } else if (banned.size() == 1) {
                            api::players::calls::on_system_message({client, {"There is only one banned player:" + banned.back()}});
                        } else {
                            std::string last_item = banned.back();
                            banned.pop_back();
                            std::string message = "There a total of " + std::to_string(banned.size() + 1) + " banned players:\n";
                            for (auto& player : banned)
                                message += player + ", ";

                            if (!max_reached) {
                                message.erase(message.size() - 2, 2);
                                message += "and " + last_item + '.';
                            } else
                                message += last_item + ", ...";
                            api::players::calls::on_system_message({client, {message}});
                        }
                    });

                    players
                        .add_child({"detailed", "list all banned players with reasons", "/banlist players detailed"})
                        .set_callback("command.banlist.players.detailed", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            std::string message = "List of banned players:\n";
                            banned_players.for_each(
                                100,
                                [&message](auto& it) {
                                    message += ("\t" + it.first + (it.second.is_none() ? "\n" : ("\n\t\tReason: " + (std::string)it.second + "\n")));
                                },
                                [&message]() {
                                    message += "...\n";
                                }
                            );
                            message.erase(message.size() - 1, 1);
                            api::players::calls::on_system_message({client, {message}});
                        });

                    players
                        .add_child({"contains", "", ""})
                        .add_child({"<player>", "returns if the player in list", "/banlist players contains <player>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                        .set_callback("command.banlist.players.contains", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            api::players::calls::on_system_message({client, {"Player " + args[0] + (banned_players.contains(args[0]) ? " is in the list." : " is not in the list.")}});
                        });
                }
                {
                    auto ips =
                        ban_list.add_child({"ips", "list all banned ips", "/banlist ips"});

                    ips.set_callback("command.banlist.ips", [this](const list_array<std::string>&, base_objects::client_data_holder& client) {
                        bool max_reached = false;
                        auto banned = banned_ips.keys(100, max_reached);
                        if (banned.size() == 0) {
                            api::players::calls::on_system_message({client, {"There are no banned ips."}});
                        } else if (banned.size() == 1) {
                            api::players::calls::on_system_message({client, {"There is only one banned ip:" + banned.back()}});
                        } else {
                            std::string last_item = banned.back();
                            banned.pop_back();
                            std::string message = "There a total of " + std::to_string(banned.size() + 1) + " banned IPs:\n";
                            for (auto& player : banned)
                                message += player + ", ";

                            if (!max_reached) {
                                message.erase(message.size() - 2, 2);
                                message += "and " + last_item + '.';
                            } else
                                message += last_item + ", ...";
                            api::players::calls::on_system_message({client, {message}});
                        }
                    });
                    ips
                        .add_child({"detailed", "list all banned ips with reasons", "/banlist ips detailed"})
                        .set_callback("command.banlist.ips.detailed", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            std::string message = "List of banned ips:\n";
                            banned_ips.for_each(
                                100,
                                [&message](auto& it) {
                                    message += "\t" + it.first + (it.second.is_none() ? "\n" : ("\n\t\tReason: " + (std::string)it.second + "\n"));
                                },
                                [&message]() {
                                    message += "...\n";
                                }
                            );
                            api::players::calls::on_system_message({client, {message}});
                        });

                    ips
                        .add_child({"contains", "", ""})
                        .add_child({"<player>", "returns if IP in list", "/banlist ips contains <ip>"}, base_objects::command::parsers::brigadier_string, {.flags = 1})
                        .set_callback("command.banlist.ips.contains", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            api::players::calls::on_system_message({client, {"IP " + args[0] + (banned_players.contains(args[0]) ? " is in the list." : " is not in the list.")}});
                        });
                }
            }
            {
                browser.add_child({"ban-ip"})
                    .add_child({"<ip>", "ban ip", "/ban-ip <ip>"}, base_objects::packets::command_node::parsers::brigadier_string, {.flags = 1})
                    .set_callback("command.ban-ip", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        if (args.size() == 0) {
                            {
                                api::players::calls::on_system_message({client, {"Usage: /ban-ip <ip>"}});
                                return;
                            }
                            return;
                        }
                        api::ban::on_ban_ip({args[0], client->name, ""});

                        if (banned_ips.contains(args[0])) {
                            api::players::calls::on_system_message({client, {"IP " + args[0] + " has been already banned."}});
                            return;
                        }
                        banned_ips.add(args[0], {});
                        api::players::calls::on_system_message({client, {"IP " + args[0] + " has been banned."}});
                    })
                    .add_child({"[reason]", "ban ip with reason", "/ban-ip <ip> [reason]"}, base_objects::packets::command_node::parsers::brigadier_string, {.flags = 2})
                    .set_callback("command.ban-ip:reason", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        if (args.size() == 0) {
                            {
                                api::players::calls::on_system_message({client, {"Usage: /ban-ip <ip> [reason]"}});
                                return;
                            }
                            return;
                        }
                        api::ban::on_ban_ip({args[0], client->name, args[1]});
                        if (banned_ips.contains(args[0])) {
                            api::players::calls::on_system_message({client, {"IP " + args[0] + " has been already banned."}});
                            return;
                        }
                        banned_ips.add(args[0], args[1]);
                        api::players::calls::on_system_message({client, {"IP " + args[0] + " has been banned."}});
                    });
            }
            {
                auto& pardon_ip = browser.add_child({"pardon-ip"})
                                      .add_child({"<ip>", "pardon ip", "/pardon-ip <ip>"}, base_objects::packets::command_node::parsers::brigadier_string, {.flags = 1})
                                      .set_callback("command.pardon-ip", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                                          if (args.size() == 0) {
                                              api::players::calls::on_system_message({client, {"Usage: /pardon-ip <ip>"}});
                                              return;
                                          }
                                          api::ban::on_pardon_ip({args[0], client->name, args[1]});

                                          if (!banned_ips.contains(args[0])) {
                                              api::players::calls::on_system_message({client, {"IP " + args[0] + " has not been banned."}});
                                              return;
                                          }

                                          banned_ips.remove(args[0]);
                                          api::players::calls::on_system_message({client, {"IP " + args[0] + " has been pardoned."}});
                                      });
                browser.add_child({"unban-ip", "pardon-ip alias"}).add_child(pardon_ip);
            }
        }

        BanPlugin::plugin_response BanPlugin::OnPlay_initialize(base_objects::client_data_holder& client) {
            if (auto banned = banned_players.get(client->name); banned)
                api::players::calls::on_player_kick({client, {"You are banned from this server\nReason: " + banned->convert_to_str()}});
            if (auto banned = banned_ips.get(client->ip); banned)
                api::players::calls::on_player_kick({client, {"You are banned from this server\nReason: " + banned->convert_to_str()}});
            return false;
        }
    }
}
