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
#include <src/api/entity.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/entity_proxy.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/api/packets/server_bound/play.hpp>
#include <src/api/players.hpp>
#include <src/api/registers.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::base::play_engine {
    struct movement : public PluginAutoRegister<"base/play_engine/movement", movement> {
        movement() {}

        ~movement() noexcept {}

        void OnInitialization(const PluginRegistrationPtr& _) override {
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::player_command&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::player_input&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::player_abilities&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::move_player_pos&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                if (!client.player_data.assigned_entity)
                    return;

                api::entity entity = {*client.player_data.assigned_entity};
                bool moved = false;
                client.packets_state.get_play_data([&](auto& data) {
                    ++data.shadow_movement.last_movement_packet;
                    if (client.player_data.gamemode == 1 || client.player_data.gamemode == 3) { //creative or spectator
                        moved = true;
                        return;
                    }
                    auto pos = entity.get_position();
                    auto delta_x = (pos.x - packet.x);
                    auto delta_y = (pos.y - packet.y);
                    auto delta_z = (pos.z - packet.z);
                    auto delta = delta_x + delta_y + delta_z;
                    auto squared_delta = delta * delta;

                    auto max_speed = data.shadow_movement.using_elytra ? 300 : 100;
                    if (squared_delta <= max_speed * data.shadow_movement.last_movement_packet) {
                        moved = true;
                    } else {
                        log::info(client.name + "\" moved too quickly! " + std::to_string(delta_x) + " " + std::to_string(delta_y) + " " + std::to_string(delta_z));
                        entity.teleport(pos);
                    }
                });

                if (moved) {
                    entity.moved({packet.x, packet.y, packet.z});
                    entity.set_on_ground(packet.flags | api::packets::server_bound::play::move_player_pos::flags_f::on_ground);
                }
            });

            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::move_player_pos_rot&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                if (!client.player_data.assigned_entity)
                    return;

                api::entity entity = {*client.player_data.assigned_entity};
                bool moved = false;
                client.packets_state.get_play_data([&](auto& data) {
                    ++data.shadow_movement.last_movement_packet;
                    if (client.player_data.gamemode == 1) { //creative or spectator
                        moved = true;
                        return;
                    }
                    auto pos = entity.get_position();
                    auto delta_x = (pos.x - packet.x);
                    auto delta_y = (pos.y - packet.y);
                    auto delta_z = (pos.z - packet.z);
                    auto delta = delta_x + delta_y + delta_z;
                    auto squared_delta = delta * delta;

                    auto max_speed = data.shadow_movement.using_elytra ? 300 : 100;
                    if (squared_delta <= max_speed * data.shadow_movement.last_movement_packet) {
                        moved = true;
                    } else {
                        log::info(client.name + "\" moved too quickly! " + std::to_string(delta_x) + " " + std::to_string(delta_y) + " " + std::to_string(delta_z));
                        entity.teleport(pos, packet.yaw, packet.pitch);
                    }
                });

                if (moved)
                    entity.moved({packet.x, packet.y, packet.z}, packet.yaw, packet.pitch, packet.flags | api::packets::server_bound::play::move_player_pos_rot::flags_f::on_ground);
            });

            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::move_player_rot&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                if (client.player_data.assigned_entity)
                    api::entity(*client.player_data.assigned_entity).rotated(packet.yaw, packet.pitch, packet.flags | api::packets::server_bound::play::move_player_rot::flags_f::on_ground);
            });

            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::move_player_status_only&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                if (client.player_data.assigned_entity)
                    client.player_data.assigned_entity->modify<api::ecs::com::on_ground>()->value = packet.flags | api::packets::server_bound::play::move_player_status_only::flags_f::on_ground;
            });

            api::packets::processor(*this, []([[maybe_unused]] api::packets::server_bound::play::move_vehicle&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::processor(*this, [](api::packets::server_bound::play::paddle_boat&& packet, base_objects::SharedClientData& client) {
                if (!client.player_data.assigned_entity)
                    return;

                auto& entity = *client.player_data.assigned_entity;
                if (entity.has<api::ecs::com::ride_entity>()) {
                    api::entity_proxy::oak_boat boat(entity.get<api::ecs::com::ride_entity>().other);
                    boat.set_left_paddle_moving(packet.left_paddle_turning);
                    boat.set_right_paddle_moving(packet.left_paddle_turning);
                }
            });
        }
    };
}