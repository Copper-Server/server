/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/client.hpp>
#include <src/api/configuration.hpp>
#include <src/api/console.hpp>
#include <src/api/permissions.hpp>
#include <src/api/players.hpp>
#include <src/api/server.hpp>
#include <src/base_objects/commands.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    struct kick : public PluginAutoRegister<"tools/kick", kick> {
        void OnCommandsLoad(const PluginRegistrationPtr&, base_objects::command_root_browser& browser) override {
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;

            browser.add_child("kick")
                .add_child("<player>", cmd_pred_string::quotable_phrase)
                .set_callback("command.kick", [](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto target = api::players::get_player(
                        base_objects::SharedClientData::packets_state_t::protocol_state::play,
                        std::get<pred_string>(args[0]).value
                    );
                    if (!target) {
                        context.executor << api::client::play::system_chat{.content = "Player not found"};
                        return;
                    }
                    if (api::permissions::has_rights("misc.operator_protection.kick", *target)) {
                        context.executor << api::client::play::system_chat{.content = "You can't kick this player"};
                        return;
                    }
                    api::players::calls::on_player_kick({target, "kicked by admin"});
                })
                .add_child({"[reason]", "kick player with reason", "/kick <player> [reason]"}, cmd_pred_string::greedy_phrase)
                .set_callback("command.kick", [](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto target = api::players::get_player(
                        base_objects::SharedClientData::packets_state_t::protocol_state::play,
                        std::get<pred_string>(args[0]).value
                    );
                    if (!target) {
                        context.executor << api::client::play::system_chat{.content = "Player not found"};
                        return;
                    }
                    if (api::permissions::has_rights("misc.operator_protection.kick", *target)) {
                        context.executor << api::client::play::system_chat{.content = "You can't kick this player"};
                        return;
                    }
                    api::players::calls::on_player_kick({target, Chat::parseToChat(std::get<pred_string>(args[1]).value)});
                });
        }
    };
}
