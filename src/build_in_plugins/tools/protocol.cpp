/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <library/fast_task/include/files.hpp>
#include <src/api/configuration.hpp>
#include <src/api/network/tcp.hpp>
#include <src/api/packets.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/commands.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::tools {
    struct protocol : public plugin_auto_register<"tools/protocol", protocol> {
        bool debug_mode = false;
        std::string now = std::format("{:%Y_%m_%d__%H_%M_%OS}", std::chrono::current_zone()->to_local(std::chrono::system_clock::now()));

        void app_item(std::string_view dat, uint64_t client_id) {
            fast_task::files::async_iofstream out(api::configuration::get().server.get_storage_path() / "protocol" / now / (std::to_string(client_id) + ".rs"), std::ios::app); //idc why, but rust highlighter works nice with current format
            out << dat;
        }

        void on_register(const plugin_registration_ptr& _) override {
            register_event(api::players::handlers::on_disconnect, base_objects::events::priority::high, [this](const base_objects::client_data_holder& client) {
                if (client)
                    if (debug_mode && client->get_session())
                        app_item("client_disconnected {}\n", client->get_session()->id);
                return false;
            });

            api::packets::client_bound_ops::post_send_viewer(*this, [this](auto& packet, base_objects::shared_client_data& client) {
                if (debug_mode && client.get_session())
                    app_item("client_bound " + api::packets::stringize(packet) + '\n', client.get_session()->id);
            });

            api::packets::server_bound_ops::receive_viewer(*this, [this](auto& packet, base_objects::shared_client_data& client) {
                if (debug_mode && client.get_session())
                    app_item("server_bound " + api::packets::stringize(packet) + '\n', client.get_session()->id);
                return false;
            });
        }

        void on_commands_load(const plugin_registration_ptr&, base_objects::command_root_browser& browser) override {
            using predicate = base_objects::parser;

            auto _protocol = browser.add_child("protocol").add_child("debug");
            _protocol.add_child({"enable", "enables protocol logging to debug", "/protocol debug enable"})
                .set_callback("command.protocol.debug.enable", [this](const list_array<predicate>&, base_objects::command_context&) {
                    std::filesystem::create_directories(api::configuration::get().server.get_storage_path() / "protocol" / now);
                    bool changed = debug_mode != true;
                    debug_mode = true;
                    return changed;
                });
            _protocol.add_child({"disable", "disables protocol logging to debug", "/protocol debug disable"})
                .set_callback("command.protocol.debug.disable", [this](const list_array<predicate>&, base_objects::command_context&) {
                    bool changed = debug_mode != false;
                    debug_mode = false;
                    return changed;
                });
        }
    };
}
