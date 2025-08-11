/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/allowlist.hpp>
#include <src/api/ban.hpp>
#include <src/api/statistics.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/virtual_client.hpp>
#include <src/log.hpp>

namespace copper_server::api {
    namespace allowlist {
        base_objects::events::event<allowlist_mode> on_mode_change;
        base_objects::events::event<base_objects::client_data_holder> on_kick;
        base_objects::events::event<std::string> on_add;
        base_objects::events::event<std::string> on_remove;
    }

    namespace ban {
        base_objects::events::event<ban_data> on_ban;
        base_objects::events::event<ban_data> on_pardon;
        base_objects::events::event<ban_data> on_ban_ip;
        base_objects::events::event<ban_data> on_pardon_ip;
    }

    namespace command {
        base_objects::command_manager* global_manager = nullptr;

        void register_manager(base_objects::command_manager& manager) {
            if (!global_manager)
                global_manager = &manager;
            else
                throw std::runtime_error("Command manager already registered");
        }

        void unregister_manager() {
            if (global_manager)
                global_manager = nullptr;
            else
                throw std::runtime_error("Command manager not yet registered");
        }

        base_objects::command_manager& get_manager() {
            if (global_manager)
                return *global_manager;
            else
                throw std::runtime_error("Command manager not yet registered");
        }
    }

    namespace console {
        base_objects::virtual_client* console_data;
        base_objects::events::event<std::string> on_command;

        void register_virtual_client(base_objects::virtual_client& client) {
            if (!console_data)
                console_data = &client;
            else
                throw std::runtime_error("Console's virtual client already registered");
        }

        void unregister_virtual_client() {
            if (console_data)
                console_data = nullptr;
        }

        void execute_as_console(const std::string& command) {
            if (!console_data)
                throw std::runtime_error("Console's virtual client not yet registered");

            base_objects::command_context context(console_data->client);
            api::command::get_manager().execute_command(command, context);
        }

        bool console_enabled() {
            if (!console_data)
                return false;
            return log::commands::is_inited();
        }
    }

    namespace statistics {
        namespace minecraft {
            base_objects::events::event<statistic_event> custom;
            base_objects::events::event<statistic_event> mined;
            base_objects::events::event<statistic_event> broken;
            base_objects::events::event<statistic_event> crafted;
            base_objects::events::event<statistic_event> used;
            base_objects::events::event<statistic_event> picked_up;
            base_objects::events::event<statistic_event> dropped;
            base_objects::events::event<statistic_event> killed;
            base_objects::events::event<statistic_event> killed_by;
        }
    }
}
