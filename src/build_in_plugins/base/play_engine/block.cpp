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
#include <src/api/packets/client_bound/play.hpp>
#include <src/api/packets/server_bound/play.hpp>
#include <src/api/players.hpp>
#include <src/api/registers.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::base::play_engine {
    //handles clients with play state, allows players to access world and other things through api

    struct block : public PluginAutoRegister<"base/play_engine/block", block> {
        block() {}

        ~block() noexcept {}

        void OnInitialization(const PluginRegistrationPtr& _) override {
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::set_command_block&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::set_command_minecart&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::set_jigsaw_block&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::set_structure_block&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::set_test_block&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::sign_update&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::test_instance_block_action&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
        }
    };
}