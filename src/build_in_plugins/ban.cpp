#include <src/api/ban.hpp>
#include <src/api/configuration.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/commands.hpp>
#include <src/build_in_plugins/ban.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/plugin/registration.hpp>
#include <src/storage/enbt_list_storage.hpp>

namespace copper_server {
    namespace build_in_plugins {
        BanPlugin::BanPlugin()
            : banned_players(api::configuration::get().server.get_storage_path() / +"banned_players.c_enbt"),
              banned_ips(api::configuration::get().server.get_storage_path() / +"banned_ips.c_enbt") {
            if (!banned_players.is_loaded()) {
                log::error("BanPlugin", "Failed to load banned players list");
            }
            if (!banned_ips.is_loaded()) {
                log::error("BanPlugin", "Failed to load banned ips list");
            }
        }

        void BanPlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;
            {
                browser.add_child({"ban", "", ""})
                    .add_child({"<player>", "ban player", "/ban <player>"}, cmd_pred_string::quotable_phrase)
                    .set_callback("command.ban", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto& player_name = std::get<pred_string>(args[0]).value;
                        if (api::ban::on_ban({player_name, context.executor->name, ""}))
                            return;

                        if (banned_players.contains(player_name)) {
                            api::players::calls::on_system_message({context.executor, {"Player " + player_name + " has been already banned."}});
                            return;
                        }
                        banned_players.add(player_name, {});
                        api::players::calls::on_system_message({context.executor, {"Player " + player_name + " has been banned."}});
                    })
                    .add_child({"[reason]", "ban player with reason", "/ban <player> [reason]"}, cmd_pred_string::greedy_phrase)
                    .set_callback("command.ban:reason", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto& player_name = std::get<pred_string>(args[0]).value;
                        auto& reason = std::get<pred_string>(args[1]).value;
                        if (api::ban::on_ban({player_name, context.executor->name, reason}))
                            return;
                        if (banned_players.contains(player_name)) {
                            api::players::calls::on_system_message({context.executor, {"Player " + player_name + " has been already banned."}});
                            return;
                        }
                        banned_players.add(player_name, reason);
                        api::players::calls::on_system_message({context.executor, {"Player " + player_name + " has been banned"}});
                    });
            }
            {
                auto& pardon = browser.add_child({"pardon", "", ""})
                                   .add_child({"<player>", "pardon player", "/pardon <player>"}, cmd_pred_string::quotable_phrase)
                                   .set_callback("command.pardon", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                                       auto& player_name = std::get<pred_string>(args[0]).value;
                                       if (api::ban::on_pardon({player_name, context.executor->name, ""}))
                                           return;
                                       if (!banned_players.contains(player_name)) {
                                           api::players::calls::on_system_message({context.executor, {"Player " + player_name + " has not been banned."}});
                                           return;
                                       }
                                       banned_players.remove(player_name);
                                       api::players::calls::on_system_message({context.executor, {"Player " + player_name + " has been pardoned."}});
                                   });
                browser.add_child({"unban", "pardon alias"}).add_child(pardon);
            }
            {
                auto ban_list = browser.add_child({"banlist", "list all banned players or ips", "/banlist ips|players"});
                {
                    auto players =
                        ban_list.add_child({"players", "list all banned players", "/banlist players"});


                    players.set_callback("command.banlist.players", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        bool max_reached = false;
                        auto banned = banned_players.keys(100, max_reached);
                        if (banned.size() == 0) {
                            api::players::calls::on_system_message({context.executor, {"There are no banned players."}});
                        } else if (banned.size() == 1) {
                            api::players::calls::on_system_message({context.executor, {"There is only one banned player:" + banned.back()}});
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
                            api::players::calls::on_system_message({context.executor, {message}});
                        }
                    });

                    players
                        .add_child({"detailed", "list all banned players with reasons", "/banlist players detailed"})
                        .set_callback("command.banlist.players.detailed", [this](const list_array<predicate>& args, base_objects::command_context& context) {
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
                            api::players::calls::on_system_message({context.executor, {message}});
                        });

                    players
                        .add_child({"contains", "", ""})
                        .add_child({"<player>", "returns if the player in list", "/banlist players contains <player>"}, cmd_pred_string::quotable_phrase)
                        .set_callback("command.banlist.players.contains", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& player_name = std::get<pred_string>(args[0]).value;
                            api::players::calls::on_system_message({context.executor, {"Player " + player_name + (banned_players.contains(player_name) ? " is in the list." : " is not in the list.")}});
                        });
                }
                {
                    auto ips =
                        ban_list.add_child({"ips", "list all banned ips", "/banlist ips"});

                    ips.set_callback("command.banlist.ips", [this](const list_array<predicate>&, base_objects::command_context& context) {
                        bool max_reached = false;
                        auto banned = banned_ips.keys(100, max_reached);
                        if (banned.size() == 0) {
                            api::players::calls::on_system_message({context.executor, {"There are no banned ips."}});
                        } else if (banned.size() == 1) {
                            api::players::calls::on_system_message({context.executor, {"There is only one banned ip:" + banned.back()}});
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
                            api::players::calls::on_system_message({context.executor, {message}});
                        }
                    });
                    ips
                        .add_child({"detailed", "list all banned ips with reasons", "/banlist ips detailed"})
                        .set_callback("command.banlist.ips.detailed", [this](const list_array<predicate>& args, base_objects::command_context& context) {
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
                            api::players::calls::on_system_message({context.executor, {message}});
                        });

                    ips
                        .add_child({"contains", "", ""})
                        .add_child({"<player>", "returns if IP in list", "/banlist ips contains <ip>"}, cmd_pred_string::quotable_phrase)
                        .set_callback("command.banlist.ips.contains", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& ip = std::get<pred_string>(args[0]).value;
                            api::players::calls::on_system_message({context.executor, {"IP " + ip + (banned_players.contains(ip) ? " is in the list." : " is not in the list.")}});
                        });
                }
            }
            {
                browser.add_child({"ban-ip"})
                    .add_child({"<ip>", "ban ip", "/ban-ip <ip>"}, cmd_pred_string::quotable_phrase)
                    .set_callback("command.ban-ip", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto& ip = std::get<pred_string>(args[0]).value;
                        api::ban::on_ban_ip({ip, context.executor->name, ""});

                        if (banned_ips.contains(ip)) {
                            api::players::calls::on_system_message({context.executor, {"IP " + ip + " has been already banned."}});
                            return;
                        }
                        banned_ips.add(ip, {});
                        api::players::calls::on_system_message({context.executor, {"IP " + ip + " has been banned."}});
                    })
                    .add_child({"[reason]", "ban ip with reason", "/ban-ip <ip> [reason]"}, cmd_pred_string::greedy_phrase)
                    .set_callback("command.ban-ip:reason", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto& ip = std::get<pred_string>(args[0]).value;
                        auto& reason = std::get<pred_string>(args[1]).value;
                        api::ban::on_ban_ip({ip, context.executor->name, reason});
                        if (banned_ips.contains(ip)) {
                            api::players::calls::on_system_message({context.executor, {"IP " + ip + " has been already banned."}});
                            return;
                        }
                        banned_ips.add(ip, reason);
                        api::players::calls::on_system_message({context.executor, {"IP " + ip + " has been banned."}});
                    });
            }
            {
                auto& pardon_ip = browser.add_child({"pardon-ip"})
                                      .add_child({"<ip>", "pardon ip", "/pardon-ip <ip>"}, cmd_pred_string::quotable_phrase)
                                      .set_callback("command.pardon-ip", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                                          auto& ip = std::get<pred_string>(args[0]).value;
                                          api::ban::on_pardon_ip({ip, context.executor->name, ""});

                                          if (!banned_ips.contains(ip)) {
                                              api::players::calls::on_system_message({context.executor, {"IP " + ip + " has not been banned."}});
                                              return;
                                          }

                                          banned_ips.remove(ip);
                                          api::players::calls::on_system_message({context.executor, {"IP " + ip + " has been pardoned."}});
                                      });
                browser.add_child({"unban-ip", "pardon-ip alias"}).add_child(pardon_ip);
            }
        }

        BanPlugin::plugin_response BanPlugin::OnPlay_initialize(base_objects::client_data_holder& client) {
            if (auto banned = banned_players.get(client->name); banned)
                api::players::calls::on_player_kick({client, {"You are banned from this server\nReason: " + banned->convert_to_str()}});
            if (auto banned = banned_ips.get(client->ip); banned)
                api::players::calls::on_player_kick({client, {"You are banned from this server\nReason: " + banned->convert_to_str()}});
            return std::nullopt;
        }
    }
}
