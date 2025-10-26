/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/configuration.hpp>
#include <src/api/console.hpp>
#include <src/api/log.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/api/packets/server_bound/play.hpp>
#include <src/api/permissions.hpp>
#include <src/api/players.hpp>
#include <src/api/server.hpp>
#include <src/base_objects/commands.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::tools {
    struct kick : public plugin_auto_register<"tools/kick", kick> {
        void on_commands_load(const plugin_registration_ptr&, base_objects::command_root_browser& browser) override {
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;

            browser.add_child("kick")
                .add_child("player", cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                .set_callback("command.kick", [](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto target = api::players::get_player(
                        base_objects::shared_client_data::packets_state_t::protocol_state::play,
                        std::get<pred_string>(args[0]).value
                    );
                    if (!target) {
                        context.executor << api::packets::client_bound::play::system_chat{.content = "Player not found"};
                        return false;
                    }
                    if (api::permissions::has_rights("misc.operator_protection.kick", *target)) {
                        context.executor << api::packets::client_bound::play::system_chat{.content = "You can't kick this player"};
                        return false;
                    }
                    api::players::calls::on_player_kick({target, "kicked by admin"});
                    return true;
                })
                .add_child({"reason", "kick player with reason", "/kick player reason"}, cmd_pred_string{.type = cmd_pred_string::greedy_phrase})
                .set_callback("command.kick", [](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto target = api::players::get_player(
                        base_objects::shared_client_data::packets_state_t::protocol_state::play,
                        std::get<pred_string>(args[0]).value
                    );
                    if (!target) {
                        context.executor << api::packets::client_bound::play::system_chat{.content = "Player not found"};
                        return false;
                    }
                    if (api::permissions::has_rights("misc.operator_protection.kick", *target)) {
                        context.executor << api::packets::client_bound::play::system_chat{.content = "You can't kick this player"};
                        return false;
                    }
                    api::players::calls::on_player_kick({target, base_objects::chat::parse_to_chat(std::get<pred_string>(args[1]).value)});
                    return true;
                });
        }
    };
}
