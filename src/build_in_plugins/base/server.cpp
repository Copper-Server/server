/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/server.hpp>
#include <src/base_objects/commands.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    struct ServerPlugin : public PluginAutoRegister<"base/server", ServerPlugin> {
        void OnCommandsLoad(const PluginRegistrationPtr&, base_objects::command_root_browser& browser) override {
            browser.add_child({"stop", "stop server", "/stop"})
                .set_callback("command.stop", [](const list_array<base_objects::parser>&, base_objects::command_context&) {
                    api::server::shutdown();
                    pluginManagement.callUnload();
                });
        }
    };
}
