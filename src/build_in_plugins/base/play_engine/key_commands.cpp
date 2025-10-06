/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/client.hpp>
#include <src/api/command.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    struct key_commands : public PluginAutoRegister<"base/play_engine/key_commands", key_commands> {
        key_commands() {}

        ~key_commands() noexcept {}

        void OnInitialization(const PluginRegistrationPtr& _) override {
            register_packet_processor([]([[maybe_unused]] api::packets::server_bound::play::lock_difficulty&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                base_objects::command_context context(client);
                context.apply_executor_data();
                api::command::get_manager().execute_command("lock_difficulty " + std::string(packet.is_locked ? "true" : "false"), context);
            });

            register_packet_processor([]([[maybe_unused]] api::packets::server_bound::play::change_difficulty&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                base_objects::command_context context(client);
                context.apply_executor_data();
                std::string diff;
                switch (packet.difficulty.value) {
                case api::packets::difficulty_e::peaceful:
                    diff = "peaceful";
                case api::packets::difficulty_e::easy:
                    diff = "easy";
                case api::packets::difficulty_e::normal:
                    diff = "normal";
                    break;
                default:
                case api::packets::difficulty_e::hard:
                    diff = "hard";
                    break;
                }

                api::command::get_manager().execute_command("difficulty " + diff, context);
            });

            register_packet_processor([]([[maybe_unused]] api::packets::server_bound::play::change_gamemode&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                base_objects::command_context context(client);
                context.apply_executor_data();
                std::string gam;
                switch (packet.gamemode.value) {
                case api::packets::gamemode_e::survival:
                    gam = "survival";
                case api::packets::gamemode_e::creative:
                    gam = "creative";
                case api::packets::gamemode_e::adventure:
                    gam = "adventure";
                    break;
                default:
                case api::packets::gamemode_e::spectator:
                    gam = "spectator";
                    break;
                }

                api::command::get_manager().execute_command("gamemode " + gam, context);
            });
            register_packet_processor([]([[maybe_unused]] api::packets::server_bound::play::teleport_to_entity&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                if (client.player_data.gamemode == (uint8_t)api::packets::gamemode_e::spectator) {
                    auto entity = api::entity_id_map::get_entity(packet.uuid);
                    auto& client_entity = client.player_data.assigned_entity;
                    if (entity && client_entity) {
                        if (entity->current_world() == client_entity->current_world())
                            client.player_data.assigned_entity->teleport(entity->position, entity->rotation.x, entity->rotation.y);
                        else if (entity->current_world())
                            api::world::transfer(client_entity, entity->current_world()->world_id, entity->position, entity->rotation);
                    }
                }
            });
        }
    };
}