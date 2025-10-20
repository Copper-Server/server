/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/ban.hpp>
#include <src/api/configuration.hpp>
#include <src/api/log.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/api/packets/server_bound/play.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/commands.hpp>
#include <src/plugin/main.hpp>
#include <src/plugin/registration.hpp>
#include <src/storage/enbt_list_storage.hpp>

namespace copper_server::build_in_plugins::tools {
    class ban : public plugin_auto_register<"tools/ban", ban> {
        storage::enbt_list_storage banned_players;
        storage::enbt_list_storage banned_ips;

    public:
        ban()
            : banned_players(api::configuration::get().server.get_storage_path() / +"banned_players.c_enbt"),
              banned_ips(api::configuration::get().server.get_storage_path() / +"banned_ips.c_enbt") {
            if (!banned_players.is_loaded()) {
                api::log::error("BanPlugin", "Failed to load banned players list");
            }
            if (!banned_ips.is_loaded()) {
                api::log::error("BanPlugin", "Failed to load banned ips list");
            }
        }

        ~ban() noexcept {};

        void on_initialization(const plugin_registration_ptr&) override {
            api::configuration::get() ^ "ban" ^ "on_ban_message" |= enbt::compound{{"text", "You are banned from this server\nReason: %."}, {"color", "red"}};
            api::configuration::get() ^ "ban" ^ "on_ban_ip_message" |= enbt::compound{{"text", "You are banned from this server\nReason: %."}, {"color", "red"}};
        }

        void on_post_load(const plugin_registration_ptr&) override {
            register_event(api::ban::on_ban, base_objects::events::priority::low, [](const api::ban::ban_data& client) {
                api::players::calls::on_player_kick(
                    {api::players::get_player(base_objects::shared_client_data::packets_state_t::protocol_state::play, client.who),
                     Chat::from_enbt_with_format(api::configuration::get() ^ "ban" ^ "on_ban_message", {client.reason})}
                );
                return false;
            });
            register_event(api::ban::on_ban, base_objects::events::priority::low, [](const api::ban::ban_data& client) {
                api::players::calls::on_player_kick(
                    {api::players::get_player(base_objects::shared_client_data::packets_state_t::protocol_state::play, client.who),
                     Chat::from_enbt_with_format(api::configuration::get() ^ "ban" ^ "on_ban_ip_message", {client.reason})}
                );
                return false;
            });
        }

        void on_commands_load(const plugin_registration_ptr&, base_objects::command_root_browser& browser) override {
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;
            {
                browser.add_child({"ban", "", ""})
                    .add_child({"player", "ban player", "/ban player"}, cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                    .set_callback("command.ban", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto& player_name = std::get<pred_string>(args[0]).value;
                        if (api::ban::on_ban({player_name, context.executor.name, ""}))
                            return false;

                        if (banned_players.contains(player_name)) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"Player " + player_name + " has been already banned."}};
                            return false;
                        }
                        banned_players.add(player_name, {});
                        context.executor << api::packets::client_bound::play::system_chat{.content = {"Player " + player_name + " has been banned."}};
                        return true;
                    })
                    .add_child({"reason", "ban player with reason", "/ban player reason"}, cmd_pred_string{.type = cmd_pred_string::greedy_phrase})
                    .set_callback("command.ban:reason", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto& player_name = std::get<pred_string>(args[0]).value;
                        auto& reason = std::get<pred_string>(args[1]).value;
                        if (api::ban::on_ban({player_name, context.executor.name, reason}))
                            return false;
                        if (banned_players.contains(player_name)) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"Player " + player_name + " has been already banned."}};
                            return false;
                        }
                        banned_players.add(player_name, reason);
                        context.executor << api::packets::client_bound::play::system_chat{.content = {"Player " + player_name + " has been banned"}};
                        return true;
                    });
            }
            {
                auto& pardon = browser.add_child({"pardon", "", ""})
                                   .add_child({"player", "pardon player", "/pardon player"}, cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                                   .set_callback("command.pardon", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                                       auto& player_name = std::get<pred_string>(args[0]).value;
                                       if (api::ban::on_pardon({player_name, context.executor.name, ""}))
                                           return false;
                                       if (!banned_players.contains(player_name)) {
                                           context.executor << api::packets::client_bound::play::system_chat{.content = {"Player " + player_name + " has not been banned."}};
                                           return false;
                                       }
                                       banned_players.remove(player_name);
                                       context.executor << api::packets::client_bound::play::system_chat{.content = {"Player " + player_name + " has been pardoned."}};
                                       return true;
                                   });
                browser.add_child({"unban", "pardon alias"}).add_child(pardon);
            }
            {
                auto ban_list = browser.add_child({"banlist", "list all banned players or ips", "/banlist ips|players"});
                {
                    auto players = ban_list.add_child({"players", "list all banned players", "/banlist players"});


                    players.set_callback("command.banlist.players", [this](const list_array<predicate>&, base_objects::command_context& context) -> size_t {
                        bool max_reached = false;
                        auto banned = banned_players.keys(100, max_reached);
                        if (banned.size() == 0) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"There are no banned players."}};
                            return 0;
                        } else if (banned.size() == 1) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"There is only one banned player:" + banned.back()}};
                            return 1;
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
                            context.executor << api::packets::client_bound::play::system_chat{.content = {message}};
                            return banned.size() + 1;
                        }
                    });

                    players
                        .add_child({"detailed", "list all banned players with reasons", "/banlist players detailed"})
                        .set_callback("command.banlist.players.detailed", [this](const list_array<predicate>&, base_objects::command_context& context) {
                            std::string message = "List of banned players:\n";
                            size_t count = 0;
                            banned_players.for_each(
                                100,
                                [&message, &count](auto& it) {
                                    message += ("\t" + it.first + (it.second.is_none() ? "\n" : ("\n\t\tReason: " + (std::string)it.second + "\n")));
                                    ++count;
                                },
                                [&message, &count]() {
                                    message += "...\n";
                                    ++count;
                                }
                            );
                            message.erase(message.size() - 1, 1);
                            context.executor << api::packets::client_bound::play::system_chat{.content = {message}};
                            return count;
                        });

                    players
                        .add_child({"contains", "", ""})
                        .add_child({"player", "returns if the player in list", "/banlist players contains player"}, cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                        .set_callback("command.banlist.players.contains", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& player_name = std::get<pred_string>(args[0]).value;
                            bool res = banned_players.contains(player_name);
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"Player " + player_name + (res ? " is in the list." : " is not in the list.")}};
                            return res;
                        });
                }
                {
                    auto ips = ban_list.add_child({"ips", "list all banned ips", "/banlist ips"});

                    ips.set_callback("command.banlist.ips", [this](const list_array<predicate>&, base_objects::command_context& context) -> size_t {
                        bool max_reached = false;
                        auto banned = banned_ips.keys(100, max_reached);
                        if (banned.size() == 0) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"There are no banned ips."}};
                            return 0;
                        } else if (banned.size() == 1) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"There is only one banned ip:" + banned.back()}};
                            return 1;
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
                            context.executor << api::packets::client_bound::play::system_chat{.content = {message}};
                            return banned.size() + 1;
                        }
                    });
                    ips
                        .add_child({"detailed", "list all banned ips with reasons", "/banlist ips detailed"})
                        .set_callback("command.banlist.ips.detailed", [this](const list_array<predicate>&, base_objects::command_context& context) {
                            size_t count = 0;
                            std::string message = "List of banned ips:\n";
                            banned_ips.for_each(
                                100,
                                [&message, &count](auto& it) {
                                    message += "\t" + it.first + (it.second.is_none() ? "\n" : ("\n\t\tReason: " + (std::string)it.second + "\n"));
                                    ++count;
                                },
                                [&message, &count]() {
                                    message += "...\n";
                                    ++count;
                                }
                            );
                            context.executor << api::packets::client_bound::play::system_chat{.content = {message}};
                            return count;
                        });

                    ips
                        .add_child({"contains", "", ""})
                        .add_child({"player", "returns if IP in list", "/banlist ips contains ip"}, cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                        .set_callback("command.banlist.ips.contains", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& ip = std::get<pred_string>(args[0]).value;
                            bool res = banned_ips.contains(ip);
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"IP " + ip + (res ? " is in the list." : " is not in the list.")}};
                            return res;
                        });
                }
            }
            {
                browser.add_child({"ban-ip"})
                    .add_child({"ip", "ban ip", "/ban-ip ip"}, cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                    .set_callback("command.ban-ip", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto& ip = std::get<pred_string>(args[0]).value;
                        if (api::ban::on_ban_ip({ip, context.executor.name, ""}))
                            return false;

                        if (banned_ips.contains(ip)) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"IP " + ip + " has been already banned."}};
                            return false;
                        }
                        banned_ips.add(ip, {});
                        context.executor << api::packets::client_bound::play::system_chat{.content = {"IP " + ip + " has been banned."}};
                        return true;
                    })
                    .add_child({"reason", "ban ip with reason", "/ban-ip ip reason"}, cmd_pred_string{.type = cmd_pred_string::greedy_phrase})
                    .set_callback("command.ban-ip:reason", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto& ip = std::get<pred_string>(args[0]).value;
                        auto& reason = std::get<pred_string>(args[1]).value;
                        if (api::ban::on_ban_ip({ip, context.executor.name, reason}))
                            return false;
                        if (banned_ips.contains(ip)) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"IP " + ip + " has been already banned."}};
                            return false;
                        }
                        banned_ips.add(ip, reason);
                        context.executor << api::packets::client_bound::play::system_chat{.content = {"IP " + ip + " has been banned."}};
                        return true;
                    });
            }
            {
                auto& pardon_ip = browser.add_child({"pardon-ip"})
                                      .add_child({"ip", "pardon ip", "/pardon-ip ip"}, cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                                      .set_callback("command.pardon-ip", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                                          auto& ip = std::get<pred_string>(args[0]).value;
                                          if (api::ban::on_pardon_ip({ip, context.executor.name, ""}))
                                              return false;

                                          if (!banned_ips.contains(ip)) {
                                              context.executor << api::packets::client_bound::play::system_chat{.content = {"IP " + ip + " has not been banned."}};
                                              return false;
                                          }

                                          banned_ips.remove(ip);
                                          context.executor << api::packets::client_bound::play::system_chat{.content = {"IP " + ip + " has been pardoned."}};
                                          return true;
                                      });
                browser.add_child({"unban-ip", "pardon-ip alias"}).add_child(pardon_ip);
            }
        }

        void on_play_initialize(base_objects::shared_client_data& client) override {
            if (auto banned = banned_players.get(client.name); banned)
                api::players::calls::on_player_kick({api::players::get_player(client), Chat::from_enbt_with_format(api::configuration::get() ^ "ban" ^ "on_ban_message", {banned->convert_to_str()})});
            if (auto banned = banned_ips.get(client.ip); banned)
                api::players::calls::on_player_kick({api::players::get_player(client), Chat::from_enbt_with_format(api::configuration::get() ^ "ban" ^ "on_ban_ip_message", {banned->convert_to_str()})});
        }
    };
}
