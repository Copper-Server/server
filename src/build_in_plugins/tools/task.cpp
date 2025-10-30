/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <library/fast_task/include/debug.hpp>
#include <src/api/configuration.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/api/packets/server_bound/play.hpp>
#include <src/base_objects/commands.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::tools {
    struct task : public plugin_auto_register<"tools/task", task> {
        void on_initialization(const plugin_registration_ptr&) override {
            if (fast_task::debug::is_debug_enabled()) {
                api::configuration::get() ^ "task" ^ "enable_creation_stack_traces" |= false;
                fast_task::debug::enable_init_stack_trace(api::configuration::get() ^ "task" ^ "enable_creation_stack_traces" ^ get_conf);
            }
        }

        void on_commands_load(const plugin_registration_ptr&, base_objects::command_root_browser& browser) override {
            if (!fast_task::debug::is_debug_enabled())
                return;
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;

            auto task = browser.add_child("task");
            task.add_child("make_snapshot")
                .set_callback("command.task.make_snapshot", [](const list_array<predicate>& args, base_objects::command_context& context) {
                    if (fast_task::debug::is_debug_enabled()){
                        context.executor << api::packets::client_bound::play::system_chat{.content = "The introspection api is disabled for tasking library."};
                        return false;
                    }
                    fast_task::thread([]() {
                        std::string now = std::format("{:%Y_%m_%d__%H_%M_%OS}", std::chrono::current_zone()->to_local(std::chrono::system_clock::now()));
                        auto res = (api::configuration::get().server.get_storage_path() / "debug" / "task" / (now + ".txt")).string();
                        fast_task::debug::save_program_state_dump(res.c_str());
                    });
                    return true;
                });
        }

        void on_config_reload(const plugin_registration_ptr& _) override {
            if (fast_task::debug::is_debug_enabled())
                fast_task::debug::enable_init_stack_trace(api::configuration::get() ^ "task" ^ "enable_creation_stack_traces" ^ get_conf);
        }
    };
}
