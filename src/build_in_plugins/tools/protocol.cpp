/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/client.hpp>
#include <src/base_objects/commands.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    struct protocol : public PluginAutoRegister<"tools/protocol", protocol> {
        void OnCommandsLoad(const PluginRegistrationPtr&, base_objects::command_root_browser& browser) override {
            using predicate = base_objects::parser;

            auto _protocol = browser.add_child("protocol").add_child("debug");
            _protocol.add_child({"enable", "enables protocol logging to debug", "/protocol debug enable"})
                .set_callback("command.protocol.debug.enable", [](const list_array<predicate>&, base_objects::command_context&) {
                    api::packets::set_debug_mode(true);
                });
            _protocol.add_child({"disable", "disables protocol logging to debug", "/protocol debug disable"})
                .set_callback("command.protocol.debug.disable", [](const list_array<predicate>&, base_objects::command_context&) {
                    api::packets::set_debug_mode(false);
                });
        }
    };
}
