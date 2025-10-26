/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/allowlist.hpp>
#include <src/api/configuration.hpp>
#include <src/api/log.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/api/packets/server_bound/play.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/commands.hpp>
#include <src/plugin/main.hpp>
#include <src/storage/unordered_list_storage.hpp>

namespace copper_server::build_in_plugins::tools {
    struct allow_list_plugin : public plugin_auto_register<"tools/allow_list", allow_list_plugin> {
        storage::unordered_list_storage allow_list{api::configuration::get().server.get_storage_path() / "allow_list.txt"};
        api::allowlist::allowlist_mode mode = api::allowlist::allowlist_mode::off;

        ~allow_list_plugin() noexcept {};

        void on_initialization(const plugin_registration_ptr&) override {
            api::configuration::get() ^ "allow_list" ^ "on_kick_message" |= enbt::compound{{"text", "You are not in allowlist."}, {"color", "red"}};
        }

        void on_post_load(const plugin_registration_ptr&) override {
            register_event(api::allowlist::on_mode_change, base_objects::events::priority::high, [this](api::allowlist::allowlist_mode mode) {
                if (mode == api::allowlist::allowlist_mode::block) {
                    bool reached = false;
                    auto set = allow_list.entrys((size_t)-1, reached);
                    api::players::get_players().for_each([&set](auto& client) {
                        if (set.contains(client->name))
                            api::allowlist::on_kick(client);
                        return false;
                    });
                }
                this->mode = mode;
                return false;
            });
            register_event(api::allowlist::on_kick, base_objects::events::priority::low, [](const base_objects::client_data_holder& client) {
                api::players::calls::on_player_kick({client, base_objects::chat::from_enbt(api::configuration::get() ^ "allow_list" ^ "on_kick_message")});
                return false;
            });
            register_event(api::allowlist::on_add, base_objects::events::priority::low, [this](const std::string name) {
                switch (mode) {
                case api::allowlist::allowlist_mode::block:
                    if (allow_list.contains(name))
                        api::allowlist::on_kick(api::players::get_player(base_objects::shared_client_data::packets_state_t::protocol_state::play, name));
                    break;
                case api::allowlist::allowlist_mode::allow:
                    if (!allow_list.contains(name))
                        api::allowlist::on_kick(api::players::get_player(base_objects::shared_client_data::packets_state_t::protocol_state::play, name));
                    break;
                default:
                    break;
                }
                return false;
            });
        }

        void on_commands_load(const plugin_registration_ptr&, base_objects::command_root_browser& browser) override {
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;
            {
                auto allowlist = browser.add_child("allowlist");
                allowlist.add_child("add")
                    .add_child({"player", "add player to allowlist", "/allowlist add player"}, cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                    .set_callback("command.allowlist.add", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto& player_name = std::get<pred_string>(args[0]).value;
                        if (player_name.contains("\n")) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"Player name contains newline character"}};
                            return false;
                        }
                        if (api::allowlist::on_add(player_name)) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"Player " + player_name + " is not added to allowlist"}};
                            return false;
                        }

                        allow_list.add(player_name);
                        context.executor << api::packets::client_bound::play::system_chat{.content = {"Player " + player_name + " added to allowlist"}};
                        return true;
                    });
                allowlist.add_child("remove")
                    .add_child({"player", "remove player from allowlist", "/allowlist remove player"}, cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                    .set_callback("command.allowlist.remove", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto& player_name = std::get<pred_string>(args[0]).value;
                        if (player_name.contains("\n")) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"Player name contains newline character"}};
                            return false;
                        }
                        if (api::allowlist::on_remove(player_name)) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"Player " + player_name + " is not removed from allowlist"}};
                            return false;
                        }
                        allow_list.remove(player_name);
                        context.executor << api::packets::client_bound::play::system_chat{.content = {"Player " + player_name + " removed from allowlist"}};
                        return true;
                    });
                allowlist.add_child({"list", "list all players in allowlist", "/allowlist list"})
                    .set_callback("command.allowlist.list", [this](const list_array<predicate>&, base_objects::command_context& context) {
                        bool max_reached = false;
                        auto listed = allow_list.entrys(100, max_reached);
                        if (listed.size() == 0) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"There are no listed player."}};
                        } else if (listed.size() == 1) {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"There is only one player in the list:" + *listed.begin()}};
                        } else {
                            std::string message = "There a total of " + std::to_string(listed.size()) + " listed players:\n";
                            size_t i = 0;
                            size_t m = listed.size();
                            for (auto& player : listed) {
                                if (++i == m) {
                                    if (max_reached)
                                        message += "and " + player + '.';
                                    else
                                        message += player + ", ...";
                                    break;
                                } else
                                    message += player + ", ";
                            }
                            context.executor << api::packets::client_bound::play::system_chat{.content = {message}};
                        }
                        return listed.size();
                    });
                allowlist.add_child({"mode"})
                    .add_child({"mode", "set allowlist mode", "/allowlist mode block|allow|off"}, cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                    .set_callback("command.allowlist.mode", [](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto& new_mode = std::get<pred_string>(args[0]).value;
                        if (new_mode == "block")
                            api::allowlist::on_mode_change(api::allowlist::allowlist_mode::block);
                        else if (new_mode == "allow")
                            api::allowlist::on_mode_change(api::allowlist::allowlist_mode::allow);
                        else if (new_mode == "off")
                            api::allowlist::on_mode_change(api::allowlist::allowlist_mode::off);
                        else {
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"Usage: /allowlist mode block|allow|off"}};
                            return false;
                        }
                        context.executor << api::packets::client_bound::play::system_chat{.content = {"Allowlist mode set to " + new_mode}};
                        return true;
                    });
                allowlist.add_child("off")
                    .set_callback("command.allowlist.off", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        bool changed = this->mode != api::allowlist::allowlist_mode::off;
                        if (changed) {
                            api::allowlist::on_mode_change(api::allowlist::allowlist_mode::off);
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"Allowlist mode set to off"}};
                        } else
                            context.executor << api::packets::client_bound::play::system_chat{.content = base_objects::chat("Allowlist mode is already off").set_color("red")};
                        return changed;
                    });
                allowlist.add_child("on")
                    .set_callback("command.allowlist.off", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        bool changed = this->mode != api::allowlist::allowlist_mode::off;
                        if (changed) {
                            api::allowlist::on_mode_change(api::allowlist::allowlist_mode::off);
                            context.executor << api::packets::client_bound::play::system_chat{.content = {"Allowlist mode set to block"}};
                        } else
                            context.executor << api::packets::client_bound::play::system_chat{.content = base_objects::chat("Allowlist mode is already block").set_color("red")};
                        return changed;
                    });

                browser.add_child("whitelist")
                    .set_redirect("allowlist", [&browser](base_objects::command& cmd, const list_array<predicate>&, const std::string& left, base_objects::command_context& context) {
                        browser.get_manager().execute_command_from(left, cmd, context);
                        return std::move(context.other_data["result"]);
                    });
            }
        }

        void on_play_initialize(base_objects::shared_client_data& client) override {
            switch (mode) {
            case api::allowlist::allowlist_mode::block:
                if (allow_list.contains(client.name))
                    api::allowlist::on_kick(api::players::get_player(client));
                break;
            case api::allowlist::allowlist_mode::allow:
                if (!allow_list.contains(client.name))
                    api::allowlist::on_kick(api::players::get_player(client));
                break;
            default:
                break;
            }
        }
    };
}
