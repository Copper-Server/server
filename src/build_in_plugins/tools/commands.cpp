/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/console.hpp>
#include <src/api/client.hpp>
#include <src/api/internal/command.hpp>
#include <src/api/permissions.hpp>
#include <src/base_objects/commands.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    struct commands : public PluginAutoRegister<"tools/commands", commands> {
        base_objects::command_manager manager;

        void OnRegister(const PluginRegistrationPtr&) override {
            api::command::register_manager(manager);
        }

        void OnUnregister(const PluginRegistrationPtr&) override {
            api::command::unregister_manager();
        }

        void OnLoad(const PluginRegistrationPtr&) override {
            manager.reload_commands();
        }

        void OnCommandsLoad(const PluginRegistrationPtr&, base_objects::command_root_browser& browser) override {
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;
            {
                browser.add_child({"help", "returns list of commands", ""})
                    .set_callback("command.help", [browser](const list_array<predicate>&, base_objects::command_context& context) {
                        context.executor << api::client::play::system_chat{.content = "help for all commands:\n" + browser.get_documentation()};
                    })
                    .add_child({"[command]", "returns help for command", "/help [command]"}, cmd_pred_string::greedy_phrase)
                    .set_callback("command.help", [browser](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto command = browser.open(std::get<pred_string>(args[0]).value);
                        if (!command.is_valid())
                            context.executor << api::client::play::system_chat{.content = "Command not found"};
                        else
                            context.executor << api::client::play::system_chat{.content = command.get_documentation()};
                    });

                browser.add_child({"?", "help alias"}).set_redirect("help", [browser](base_objects::command& cmd, const list_array<predicate>&, const std::string& left, base_objects::command_context& context) {
                    browser.get_manager().execute_command_from(left, cmd, context);
                });
            }
            {
                browser.add_child({"version", "returns server version", "/version"})
                    .set_callback("command.version", [](const list_array<predicate>&, base_objects::command_context& context) {
                        context.executor << api::client::play::system_chat{.content = "Server version: 1.0.0. Build: InDev-" __DATE__ " " __TIME__};
                    });
            }
        }

        void OnPlay_initialize(base_objects::SharedClientData& client) override {
            client << api::client::play::commands::create(manager);
        }
    };
}
