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
#include <src/base_objects/entity.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::base::play_engine {
    struct item : public PluginAutoRegister<"base/play_engine/item", item> {
        item() {}

        ~item() noexcept {}

        void OnInitialization(const PluginRegistrationPtr& _) override {
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::interact&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::jigsaw_generate&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::use_item_on&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::use_item&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
        }
    };
}