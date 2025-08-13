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
#include <src/base_objects/commands.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    struct scheduled : public PluginAutoRegister<"tools/scheduled", scheduled> {
        void OnInitialization(const PluginRegistrationPtr&) override {
            api::configuration::get() ^ "scheduled" ^ "on_start" ^ "command" |= enbt::fixed_array{enbt::value("version")};
            api::configuration::get() ^ "scheduled" ^ "on_stop" ^ "command" |= enbt::fixed_array{};
            //TODO add more flexibility
        }

        void OnPostLoad(const PluginRegistrationPtr&) override {
            if (api::console::console_enabled()) {
                const enbt::value& command = api::configuration::get() ^ "scheduled" ^ "on_start" ^ "command";
                for (auto& commands : command.as_array())
                    api::console::on_command(commands.as_string());
            }
        }

        void OnUnload(const PluginRegistrationPtr&) override {
            if (api::console::console_enabled()) {
                const enbt::value& command = api::configuration::get() ^ "scheduled" ^ "on_stop" ^ "command";
                for (auto& commands : command.as_array())
                    api::console::on_command(commands.as_string());
            }
        }
    };
}
