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

namespace copper_server::build_in_plugins::tools {
    struct scheduled : public plugin_auto_register<"tools/scheduled", scheduled> {
        void on_initialization(const plugin_registration_ptr&) override {
            api::configuration::get() ^ "scheduled" ^ "on_start" ^ "command" |= list_array<util::nbt>{util::nbt("version")};
            api::configuration::get() ^ "scheduled" ^ "on_stop" ^ "command" |= list_array<util::nbt>{};
            //TODO add more flexibility
        }

        void on_post_load(const plugin_registration_ptr&) override {
            if (api::console::console_enabled()) {
                const util::nbt& command = api::configuration::get() ^ "scheduled" ^ "on_start" ^ "command";
                for (auto& commands : command.get_list())
                    api::console::on_command(commands.as_string());
            }
        }

        void on_unload(const plugin_registration_ptr&) override {
            if (api::console::console_enabled()) {
                const util::nbt& command = api::configuration::get() ^ "scheduled" ^ "on_stop" ^ "command";
                for (auto& commands : command.get_list())
                    api::console::on_command(commands.as_string());
            }
        }
    };
}
