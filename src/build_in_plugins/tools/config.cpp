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
#include <src/base_objects/commands.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    struct config : public PluginAutoRegister<"tools/config", config> {
        void OnCommandsLoad(const PluginRegistrationPtr&, base_objects::command_root_browser& browser) override {
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;
            {
                auto _config = browser.add_child("config");
                _config
                    .add_child({"reload", "reloads config from file", "/config reload"})
                    .set_callback({"command.config.reload", {"console"}}, [&](const list_array<predicate>&, base_objects::command_context& context) {
                        api::configuration::load(true);
                        pluginManagement.registeredPlugins().for_each([&](const PluginRegistrationPtr& plugin) {
                            plugin->OnConfigReload(plugin);
                        });
                        context.executor << api::client::play::system_chat{.content = "Config reloaded"};
                    });
                _config.add_child("set")
                    .add_child("config_item", cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                    .add_child({"value", "updates config in file and applies for program", "/config set config_item value"}, cmd_pred_string{.type = cmd_pred_string::greedy_phrase})
                    .set_callback({"command.config.set", {"console"}}, [&](const list_array<predicate>& args, base_objects::command_context& context) {
                        api::configuration::set_item(std::get<pred_string>(args[0]).value, std::get<pred_string>(args[1]).value);
                        context.executor << api::client::play::system_chat{.content = "Config updated"};
                        pluginManagement.registeredPlugins().for_each([&](const PluginRegistrationPtr& plugin) {
                            plugin->OnConfigReload(plugin);
                        });
                    });
                _config
                    .add_child("get")
                    .add_child({"config_item", "returns config value", "/config get config_item"}, cmd_pred_string{.type = cmd_pred_string::quotable_phrase})
                    .set_callback({"command.config.get", {"console"}}, [&](const list_array<predicate>& args, base_objects::command_context& context) {
                        auto value = api::configuration::get().get(std::get<pred_string>(args[0]).value);
                        while (value.ends_with('\n') || value.ends_with('\r'))
                            value.pop_back();
                        if (value.contains("\n"))
                            context.executor << api::client::play::system_chat{.content = "Config value: \n" + value};
                        else
                            context.executor << api::client::play::system_chat{.content = "Config value: " + value};
                    });
            }
        }
    };
}
