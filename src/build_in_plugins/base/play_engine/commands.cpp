/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/command.hpp>
#include <src/api/configuration.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/api/packets/server_bound/play.hpp>
#include <src/api/players.hpp>
#include <src/api/registers.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::base::play_engine {
    struct commands : public PluginAutoRegister<"base/play_engine/commands", commands> {
        commands() {}

        ~commands() noexcept {}

        void OnInitialization(const PluginRegistrationPtr& _) override {
            log::info("Loading");
            api::packets::processor(*this, [](api::packets::server_bound::play::client_command&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                if (packet.action_id == api::packets::server_bound::play::client_command::action_id_e::perform_respawn) {
                    //TODO client <<  api::packets::client_bound::play::respawn{};
                } else if (packet.action_id == api::packets::server_bound::play::client_command::action_id_e::request_stats) {
                    //TODO client <<  api::packets::client_bound::play::award_stats{
                    //    .
                    //};
                }
            });
        }

        void OnCommandsLoadComplete(const std::shared_ptr<PluginRegistration>&, base_objects::command_root_browser& root) override {
            api::players::iterate_online([&manager = root.get_manager()](base_objects::SharedClientData& client) {
                if (!client.is_virtual)
                    client << api::packets::client_bound::play::commands::create(manager);
                return false;
            });
        }
    };
}