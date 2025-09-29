/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/client.hpp>
#include <src/api/configuration.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/players.hpp>
#include <src/api/registers.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    //handles clients with play state, allows players to access world and other things through api

    class PlayEngine : public PluginAutoRegister<"base/play_engine", PlayEngine> {

        static fast_task::future_ptr<void> send_async(auto& client, auto&& packet) {
            return fast_task::future<void>::start([client, packet = std::move(packet)]() mutable { *client << std::move(packet); });
        }

        static void entity_add_effect(base_objects::entity& self, base_objects::entity& target, uint32_t id, uint32_t duration, uint8_t amplifier, bool ambient, bool show_particles, bool show_icon, bool use_blend) {
            if (self.assigned_player) {
                api::client::play::update_mob_effect::flags_f f{};
                if (ambient)
                    f = f | api::client::play::update_mob_effect::flags_f::is_ambient;
                if (show_particles)
                    f = f | api::client::play::update_mob_effect::flags_f::show_particles;
                if (show_icon)
                    f = f | api::client::play::update_mob_effect::flags_f::show_icon;
                if (use_blend)
                    f = f | api::client::play::update_mob_effect::flags_f::blend;

                *self.assigned_player << api::client::play::update_mob_effect{
                    .id = target.protocol_id,
                    .effect = id,
                    .amplifier = amplifier,
                    .duration = (int32_t)duration,
                    .flags = f
                };
            }
        };

        static void entity_animation(base_objects::entity& self, base_objects::entity& target, base_objects::entity_animation animation) {
            if (self.assigned_player) {
                *self.assigned_player << api::client::play::animate{
                    .id = target.protocol_id,
                    .animation = static_cast<api::client::play::animate::animation_e>(animation)
                };
            }
        };

        static void entity_attach(base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other_id) {
            if (self.assigned_player && other_id)
                *self.assigned_player << api::client::play::set_entity_link{
                    .attached_id = other_id->protocol_id,
                    .holding_id = target.protocol_id
                };
        };

        static void entity_attack(base_objects::entity& self, base_objects::entity& target, [[maybe_unused]] base_objects::entity_ref& other_id) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::animate{
                    .id = target.protocol_id,
                    .animation = api::client::play::animate::swing_main_arm
                };
        };

        static void entity_break(base_objects::entity& self, base_objects::entity& target, int64_t x, int64_t y, int64_t z, uint8_t state) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::block_destruction{
                    .id = target.protocol_id,
                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                    .destroy_stage = state
                };
        };

        static void entity_cancel_break(base_objects::entity& self, base_objects::entity& target, int64_t x, int64_t y, int64_t z) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::block_destruction{
                    .id = target.protocol_id,
                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                    .destroy_stage = 10
                };
        };

        static void entity_damage(base_objects::entity& self, base_objects::entity& target, [[maybe_unused]] float health, int32_t type_id, const std::optional<util::VECTOR>& pos) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::damage_event{
                    .id = target.protocol_id,
                    .source_damage_type_id = type_id,
                    .source_pos = pos
                };
        };

        static void entity_damage_with_source(base_objects::entity& self, base_objects::entity& target, [[maybe_unused]] float health, int32_t type_id, base_objects::entity_ref& source, const std::optional<util::VECTOR>& pos) {
            if (self.assigned_player && source)
                *self.assigned_player << api::client::play::damage_event{
                    .id = target.protocol_id,
                    .source_damage_type_id = type_id,
                    .source_id = source->protocol_id,
                    .source_direct_id = source->protocol_id,
                    .source_pos = pos
                };
        };

        static void entity_damage_with_sources(base_objects::entity& self, base_objects::entity& target, [[maybe_unused]] float health, int32_t type_id, base_objects::entity_ref& source, base_objects::entity_ref& source_direct, const std::optional<util::VECTOR>& pos) {
            if (self.assigned_player && source && source_direct)
                *self.assigned_player << api::client::play::damage_event{
                    .id = target.protocol_id,
                    .source_damage_type_id = type_id,
                    .source_id = source->protocol_id,
                    .source_direct_id = source_direct->protocol_id,
                    .source_pos = pos
                };
        };

        static void entity_death(base_objects::entity& self, base_objects::entity& target) {
            if (self.assigned_player) {
                *self.assigned_player << api::client::play::entity_event{
                    .id = target.protocol_id,
                    .status = (int8_t)base_objects::entity_event::entity_died
                };
                //TODO add delay
                *self.assigned_player << api::client::play::entity_event{
                    .id = target.protocol_id,
                    .status = (int8_t)base_objects::entity_event::death_smoke
                };
            }
        };

        static void entity_deinit(base_objects::entity& self, base_objects::entity& target) {
            if (self.assigned_player) {
                *self.assigned_player << api::client::play::remove_entities{
                    .ids{target.protocol_id}
                };
            }
        };

        static void entity_detach(base_objects::entity& self, base_objects::entity& target, [[maybe_unused]] base_objects::entity_ref& other) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::set_entity_link{
                    .attached_id = (int32_t)target.protocol_id,
                    .holding_id = -1
                };
        };

        static void entity_event(base_objects::entity& self, base_objects::entity& target, base_objects::entity_event status) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::entity_event{
                    .id = target.protocol_id,
                    .status = (int8_t)status
                };
        };

        static void entity_finish_break(base_objects::entity& self, base_objects::entity& target, int64_t x, int64_t y, int64_t z) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::block_destruction{
                    .id = target.protocol_id,
                    .location = {(int)x, (int)y, (int)z},
                    .destroy_stage = 11
                };
        };

        static void entity_init(base_objects::entity& self, base_objects::entity& target) {
            if (self.assigned_player) {
                auto velocity = util::minecraft::packets::velocity(target.motion);
                *self.assigned_player << api::client::play::add_entity{
                    .id = target.protocol_id,
                    .uuid = api::entity_id_map::get_uuid(target.protocol_id),
                    .type = target.get_entity_type_id(),
                    .x = target.position.x,
                    .y = target.position.y,
                    .z = target.position.z,
                    .pitch = target.rotation.y,
                    .yaw = target.rotation.x,
                    .head_yaw = target.head_rotation.x,
                    .data = *target.get_object_field().or_else([]() { return std::optional<int>(0); }),
                    .velocity_x = velocity.x,
                    .velocity_y = velocity.y,
                    .velocity_z = velocity.z
                };
            }
        };

        static void entity_iteract(base_objects::entity& self, base_objects::entity& target, [[maybe_unused]] base_objects::entity_ref& other) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::animate{
                    .id = target.protocol_id,
                    .animation = api::client::play::animate::swing_main_arm
                };
        };

        static void entity_iteract_block(base_objects::entity& self, base_objects::entity& target, auto, auto, auto) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::animate{
                    .id = target.protocol_id,
                    .animation = api::client::play::animate::swing_main_arm
                };
        };

        static void entity_leaves_ride(base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other) {
            if (self.assigned_player && other)
                *self.assigned_player << api::client::play::set_passengers{
                    .id = target.protocol_id,
                    .passengers = other->ride_by_entity.convert_fn([](auto& entity) { return (base_objects::var_int32)entity->protocol_id; })
                };
        };

        static void entity_look_changes(base_objects::entity& self, base_objects::entity& target, util::ANGLE_DEG rot) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::rotate_head{
                    .id = target.protocol_id,
                    .head_yaw = rot.x
                };
        };

        static void entity_motion_changes(base_objects::entity& self, base_objects::entity& target, [[maybe_unused]] util::VECTOR mot) {
            if (self.assigned_player) {
                auto velocity = util::minecraft::packets::velocity(mot);
                *self.assigned_player << api::client::play::set_entity_motion{
                    .id = target.protocol_id,
                    .velocity_x = velocity.x,
                    .velocity_y = velocity.y,
                    .velocity_z = velocity.z
                };
            }
        };

        static void entity_move(base_objects::entity& self, base_objects::entity& target, util::VECTOR dif) {
            if (self.assigned_player) {
                auto delta = util::minecraft::packets::delta_move({(float)dif.x, (float)dif.y, (float)dif.z});
                *self.assigned_player << api::client::play::move_entity_pos{
                    .id = target.protocol_id,
                    .delta_x = delta.x,
                    .delta_y = delta.y,
                    .delta_z = delta.z,
                    .on_ground = target.is_on_ground()
                };
            }
        };

        static void entity_place_block(base_objects::entity& self, base_objects::entity& target, bool is_main_hand, [[maybe_unused]] int64_t x, [[maybe_unused]] int64_t y, [[maybe_unused]] int64_t z, [[maybe_unused]] const base_objects::block& block) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::animate{
                    .id = target.protocol_id,
                    .animation = is_main_hand ? api::client::play::animate::swing_main_arm : api::client::play::animate::swing_offhand
                };
        };

        static void entity_place_block_entity(base_objects::entity& self, base_objects::entity& target, bool is_main_hand, [[maybe_unused]] int64_t x, [[maybe_unused]] int64_t y, [[maybe_unused]] int64_t z, auto) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::animate{
                    .id = target.protocol_id,
                    .animation = is_main_hand ? api::client::play::animate::swing_main_arm : api::client::play::animate::swing_offhand
                };
        };

        static void entity_remove_effect(base_objects::entity& self, base_objects::entity& target, uint32_t id) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::remove_mob_effect{
                    .id = target.protocol_id,
                    .effect_id = id
                };
        };

        static void entity_rides(base_objects::entity& self, [[maybe_unused]] base_objects::entity& target, base_objects::entity_ref& other_entity) {
            if (self.assigned_player && other_entity)
                *self.assigned_player << api::client::play::set_passengers{
                    .id = target.protocol_id,
                    .passengers = other_entity->ride_by_entity.convert_fn([](auto& entity) { return (base_objects::var_int32)entity->protocol_id; })
                };
        };

        static void entity_rotation_changes(base_objects::entity& self, base_objects::entity& target, util::ANGLE_DEG rot) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::move_entity_rot{
                    .id = target.protocol_id,
                    .yaw = rot.x,
                    .pitch = rot.y,
                    .on_ground = target.is_on_ground()
                };
        };

        static void entity_teleport(base_objects::entity& self, base_objects::entity& target, util::VECTOR pos) {
            if (self.assigned_player)
                *self.assigned_player << api::client::play::entity_position_sync{
                    .id = target.protocol_id,
                    .x = pos.x,
                    .y = pos.y,
                    .z = pos.z,
                    .velocity_x = target.motion.x,
                    .velocity_y = target.motion.y,
                    .velocity_z = target.motion.z,
                    .yaw = (float)target.rotation.x,
                    .pitch = (float)target.rotation.y,
                    .on_ground = target.is_on_ground()
                };
        };

        static void notify_biome_change(base_objects::entity& self, int64_t x, [[maybe_unused]] int64_t y, int64_t z, [[maybe_unused]] uint32_t biome_id) {
            if (self.assigned_player) {
                if (self.current_world()) {
                    if (self.world_syncing_data.chunk_processed(x, z))
                        self.current_world()->get_chunk_at(x, z, [&self](storage::chunk_data& chunk) {
                            *self.assigned_player << api::client::play::chunks_biomes::create(chunk);
                        });
                }
            }
        };

        static void notify_block_change(base_objects::entity& self, int64_t x, int64_t y, int64_t z, const base_objects::block& block) {
            if (self.assigned_player) {
                if (self.world_syncing_data.chunk_processed(x, z))
                    *self.assigned_player << api::client::play::block_update{
                        .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                        .block = block.id
                    };
            }
        };

        static void notify_block_destroy_change(base_objects::entity& self, int64_t x, int64_t y, int64_t z, const base_objects::block& block) {
            if (self.assigned_player)
                if (self.current_world()) {
                    if (self.world_syncing_data.chunk_processed(x, z)) {
                        int32_t block_id = 0;
                        self.current_world()->get_block(
                            x,
                            y,
                            z,
                            [&block_id](base_objects::block& block) {
                                block_id = block.id;
                            },
                            [&block_id](base_objects::block& block, auto) {
                                block_id = block.id;
                            }
                        );
                        *self.assigned_player << api::client::play::level_event{
                            .event = api::client::play::level_event::event_id::block_break_and_sound,
                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                            .data = block_id,
                            .disable_volume = false
                        };
                        *self.assigned_player << api::client::play::block_update{
                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                            .block = block.id
                        };
                    }
                }
        };

        static void notify_block_entity_change(base_objects::entity& self, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block_entity) {
            if (self.assigned_player) {
                if (self.world_syncing_data.chunk_processed(x, z)) {
                    *self.assigned_player << api::client::play::block_update{
                        .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                        .block = block_entity.block.id
                    };
                    *self.assigned_player << api::client::play::block_entity_data{
                        .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                        .type = block_entity.block.block_entity_id(),
                        .data = block_entity.data
                    };
                }
            }
        };

        static void notify_block_entity_destroy_change(base_objects::entity& self, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block_entity) {
            if (self.assigned_player)
                if (self.current_world()) {
                    if (self.world_syncing_data.chunk_processed(x, z)) {
                        int32_t block_id = 0;
                        self.current_world()->get_block(
                            x,
                            y,
                            z,
                            [&block_id](base_objects::block& block) {
                                block_id = block.id;
                            },
                            [&block_id](base_objects::block& block, auto) {
                                block_id = block.id;
                            }
                        );
                        *self.assigned_player << api::client::play::level_event{
                            .event = api::client::play::level_event::event_id::block_break_and_sound,
                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                            .data = block_id,
                            .disable_volume = false
                        };
                        *self.assigned_player << api::client::play::block_update{
                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                            .block = block_entity.block.id
                        };
                    }
                }
        };

        static void notify_block_event(base_objects::entity& self, const base_objects::world::block_action& action, int64_t x, int64_t y, int64_t z) {
            if (self.assigned_player) {
                if (self.world_syncing_data.chunk_processed(x, z)) {
                    std::visit(
                        [x, y, z, &self](auto& it) mutable {
                            using T = std::decay_t<decltype(it)>;
                            if constexpr (std::is_same_v<T, base_objects::world::block_action::noteblock_activated>) {
                                *self.assigned_player << api::client::play::block_event{
                                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                    .action_id = 0,
                                    .action_param = 0,
                                    .block = "minecraft:note_block"
                                };
                            } else if constexpr (std::is_same_v<T, base_objects::world::block_action::piston_extend>) {
                                *self.assigned_player << api::client::play::block_event{
                                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                    .action_id = 0,
                                    .action_param = (uint8_t)it.dir,
                                    .block = "minecraft:piston"
                                };
                            } else if constexpr (std::is_same_v<T, base_objects::world::block_action::piston_retract>) {
                                *self.assigned_player << api::client::play::block_event{
                                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                    .action_id = 1,
                                    .action_param = (uint8_t)it.dir,
                                    .block = "minecraft:piston"
                                };
                            } else if constexpr (std::is_same_v<T, base_objects::world::block_action::piston_canceled>) {
                                *self.assigned_player << api::client::play::block_event{
                                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                    .action_id = 2,
                                    .action_param = (uint8_t)it.dir,
                                    .block = "minecraft:piston"
                                };
                            } else if constexpr (std::is_same_v<T, base_objects::world::block_action::chest_opened>) {
                                *self.assigned_player << api::client::play::block_event{
                                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                    .action_id = 1,
                                    .action_param = (uint8_t)it.count,
                                    .block = "minecraft:chest"
                                };
                            } else if constexpr (std::is_same_v<T, base_objects::world::block_action::reset_spawner>) {
                                *self.assigned_player << api::client::play::block_event{
                                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                    .action_id = 1,
                                    .action_param = 0,
                                    .block = "minecraft:spawner"
                                };
                            } else if constexpr (std::is_same_v<T, base_objects::world::block_action::end_gateway_activated>) {
                                *self.assigned_player << api::client::play::block_event{
                                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                    .action_id = 1,
                                    .action_param = 0,
                                    .block = "minecraft:end_gateway"
                                };
                            } else if constexpr (std::is_same_v<T, base_objects::world::block_action::shulker_box_closed>) {
                                *self.assigned_player << api::client::play::block_event{
                                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                    .action_id = 0,
                                    .action_param = 0,
                                    .block = "minecraft:shulker_box"
                                };
                            } else if constexpr (std::is_same_v<T, base_objects::world::block_action::shulker_box_opened>) {
                                *self.assigned_player << api::client::play::block_event{
                                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                    .action_id = 0,
                                    .action_param = 1,
                                    .block = "minecraft:shulker_box"
                                };
                            } else if constexpr (std::is_same_v<T, base_objects::world::block_action::shulker_box_opened_count>) {
                                *self.assigned_player << api::client::play::block_event{
                                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                    .action_id = 1,
                                    .action_param = (uint8_t)it.count,
                                    .block = "minecraft:shulker_box"
                                };
                            } else if constexpr (std::is_same_v<T, base_objects::world::block_action::bell_ring>) {
                                *self.assigned_player << api::client::play::block_event{
                                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                    .action_id = 1,
                                    .action_param = (uint8_t)it.dir,
                                    .block = "minecraft:bell"
                                };
                            } else if constexpr (std::is_same_v<T, base_objects::world::block_action::decorated_block_woble>) {
                                *self.assigned_player << api::client::play::block_event{
                                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                    .action_id = 1,
                                    .action_param = !it.successful,
                                    .block = "minecraft:decorated_pot"
                                };
                            }
                        },
                        action.action
                    );
                }
            }
        };

        static void notify_chunk(base_objects::entity& self, int64_t x, int64_t z, const storage::chunk_data& chunk) {
            //if (self.assigned_player) {
            //    if (self.world_syncing_data.chunk_in_bounds(x, z)) {
            //if (self.assigned_player->packets_state.chunk_batch_size > self.assigned_player->packets_state.chunks_sent) {
            //    ++self.assigned_player->packets_state.chunks_sent;
            //        send_async(self.assigned_player, api::client::play::level_chunk_with_light::create(chunk, *self.current_world()));
            //        self.world_syncing_data.mark_chunk(x, z, true);
            //}
            //    }
            //}
        };

        static void notify_chunk_blocks(base_objects::entity& self, int64_t x, int64_t z, const storage::chunk_data& chunk) {
            if (self.assigned_player) {
                if (self.world_syncing_data.chunk_in_bounds(x, z)) {
                    if (self.assigned_player->is_active()) {
                        if (self.assigned_player->packets_state.chunk_batch_size > self.assigned_player->packets_state.chunks_sent) {
                            ++self.assigned_player->packets_state.chunks_sent;
                            send_async(self.assigned_player, api::client::play::level_chunk_with_light::create(chunk, *self.current_world()));
                            self.world_syncing_data.mark_chunk(x, z, true);
                        }
                    }
                }
            }
        };

        static void notify_chunk_light(base_objects::entity& self, int64_t x, int64_t z, const storage::chunk_data& chunk) {
            if (self.assigned_player)
                if (self.world_syncing_data.chunk_in_bounds(x, z))
                    *self.assigned_player << api::client::play::light_update::create(chunk);
        };

        static void notify_sub_chunk(base_objects::entity& self, int64_t x, [[maybe_unused]] int64_t y, int64_t z, [[maybe_unused]] const base_objects::world::sub_chunk_data& chunk) {
            if (self.assigned_player) {
                if (self.world_syncing_data.chunk_in_bounds(x, z)) {
                    if (self.current_world()) {
                        if (self.assigned_player->packets_state.chunk_batch_size > self.assigned_player->packets_state.chunks_sent) {
                            ++self.assigned_player->packets_state.chunks_sent;
                            self.current_world()->get_chunk_at(x, z, [&](auto& chunk) {
                                if (self.assigned_player->is_active()) {
                                    send_async(self.assigned_player, api::client::play::level_chunk_with_light::create(chunk, *self.current_world()));
                                    self.world_syncing_data.mark_chunk(x, z, true);
                                }
                            });
                        }
                    }
                }
            }
        };

        static void notify_sub_chunk_blocks(base_objects::entity& self, int64_t x, [[maybe_unused]] int64_t y, int64_t z, [[maybe_unused]] const base_objects::world::sub_chunk_data& chunk) { //TODO use api::client::play::section_blocks_update
            if (self.assigned_player) {
                if (self.world_syncing_data.chunk_in_bounds(x, z)) {
                    if (self.current_world()) {
                        if (self.assigned_player->packets_state.chunk_batch_size > self.assigned_player->packets_state.chunks_sent) {
                            ++self.assigned_player->packets_state.chunks_sent;
                            self.current_world()
                                ->get_chunk_at(x, z, [&](auto& chunk) {
                                    if (self.assigned_player->is_active()) {
                                        send_async(self.assigned_player, api::client::play::level_chunk_with_light::create(chunk, *self.current_world()));
                                        self.world_syncing_data.mark_chunk(x, z, true);
                                    }
                                });
                        }
                    }
                }
            }
        };

        static void notify_sub_chunk_light(base_objects::entity& self, int64_t x, [[maybe_unused]] int64_t y, int64_t z, [[maybe_unused]] const base_objects::world::sub_chunk_data& chunk) {
            if (self.assigned_player) {
                if (self.world_syncing_data.chunk_in_bounds(x, z)) {
                    if (self.current_world()) {
                        self.current_world()->get_chunk_at(x, z, [&](auto& chunk) {
                            *self.assigned_player << api::client::play::light_update::create(chunk);
                        });
                    }
                }
            }
        };

        static void on_change_world(base_objects::entity& self, storage::world_data& new_world) {
            if (self.assigned_player) {
                if (self.current_world()) {
                    auto& player_data = self.assigned_player->player_data;
                    *self.assigned_player << api::client::play::respawn{
                        .dimension_type = api::registers::dimensionTypes.at(new_world.get_world_type()).id,
                        .dimension_name = new_world.world_name,
                        .seed_hashed = new_world.get_hashed_seed(),
                        .gamemode = player_data.gamemode,
                        .previous_gamemode = (api::packets::optional_gamemode_e)player_data.gamemode,
                        .is_debug = new_world.world_generator_data.contains("debug") ? (bool)new_world.world_generator_data["debug"] : false,
                        .is_flat = new_world.world_generator_data.contains("flat") ? (bool)new_world.world_generator_data["flat"] : false,
                        .death_location = player_data.last_death_location ? std::make_optional(api::client::play::respawn::death_location_t(player_data.last_death_location->world_id, {(int32_t)player_data.last_death_location->x, (int32_t)player_data.last_death_location->y, (int32_t)player_data.last_death_location->z})) : std::nullopt,
                        .portal_cooldown = 0,
                        .sea_level = new_world.world_generator_data.contains("sea_level") ? (bool)new_world.world_generator_data["sea_level"] : 60,
                        .flags = api::client::play::respawn::keep_attributes | api::client::play::respawn::keep_metadata
                    };
                    self.world_syncing_data.flush_processing();
                }
            }
        };

        static void on_tick(base_objects::entity& self) {
            if (self.assigned_player) {
                auto& player = *self.assigned_player;
                if (self.current_world()) {
                    if (!player.packets_state.is_play_initialized) {
                        player.packets_state.is_play_initialized = true;
                        *self.assigned_player << api::packets::client_bound::play::game_event{
                            .event = {api::packets::client_bound::play::game_event::wait_for_level_chunks{}},
                        };
                        *self.assigned_player << api::packets::client_bound::play::set_chunk_cache_center{
                            .x = (int32_t)self.world_syncing_data.processing_region.center_x,
                            .z = (int32_t)self.world_syncing_data.processing_region.center_z,
                        };
                        *self.assigned_player << api::packets::client_bound::play::set_chunk_cache_radius{
                            .distance = (int32_t)self.world_syncing_data.processing_region.radius
                        };
                    }
                    std::vector<fast_task::future_ptr<void>> packet_futs;
                    bool make_tick = false;
                    bool make_batch = false;
                    auto max_unbc = api::configuration::get().protocol.max_unacknowledged_chunk_batches;
                    if (max_unbc > player.packets_state.await_ack_chunk_batches) {
                        std::vector<fast_task::future_ptr<void>> serialization_futs;
                        self.world_syncing_data.for_each_processing([&](int64_t chunk_x, int64_t chunk_z, bool loaded) {
                            if (!loaded && self.current_world()) {
                                if (player.is_active()) {
                                    if (player.packets_state.chunk_batch_size > player.packets_state.chunks_sent) {
                                        ++player.packets_state.chunks_sent;
                                        auto chunk = self.current_world()->request_chunk_data_weak(chunk_x, chunk_z);
                                        if (chunk) {
                                            if (!make_batch) {
                                                *self.assigned_player << api::packets::client_bound::play::chunk_batch_start{};
                                                make_batch = true;
                                            }
                                            if ((*chunk)->generator_stage == 0xFF) {
                                                serialization_futs.push_back(
                                                    fast_task::future<void>::start(
                                                        [&, chunk]() {
                                                            packet_futs.push_back(
                                                                send_async(self.assigned_player, api::client::play::level_chunk_with_light::create(**chunk, *self.current_world()))
                                                            );
                                                        }
                                                    )
                                                );
                                                self.world_syncing_data.mark_chunk(chunk_x, chunk_z, true);
                                            }
                                        }
                                    }
                                }
                            } else
                                make_tick = true;
                        });

                        fast_task::future_tool::wait_all(serialization_futs);
                    } else
                        make_tick = true;
                    if (make_batch) {
                        ++player.packets_state.await_ack_chunk_batches;
                        fast_task::future_tool::combine_all(std::move(packet_futs))->when_ready([batch_size = player.packets_state.chunks_sent, player = self.assigned_player]() {
                            *player << api::packets::client_bound::play::chunk_batch_finished{
                                .batch_size = batch_size
                            };
                        });
                        player.packets_state.chunks_sent = 0;
                    }
                    if (make_tick) {
                        if (!player.packets_state.is_play_fully_initialized) {
                            player.packets_state.is_play_fully_initialized = true;
                            auto [yaw, pitch] = util::to_yaw_pitch(self.rotation);
                            *self.assigned_player << api::client::play::player_position{
                                .x = self.position.x,
                                .y = self.position.y,
                                .z = self.position.z,
                                .velocity_x = self.motion.x,
                                .velocity_y = self.motion.y,
                                .velocity_z = self.motion.z,
                                .yaw = (float)yaw,
                                .pitch = (float)pitch,
                                .flags = api::packets::teleport_flags{}
                            };
                        }
                        if (self.current_world())
                            if (!self.current_world()->ticking_frozen)
                                *self.assigned_player << api::client::play::ticking_step{.steps = 1};
                    }
                }
            }
        };

        static std::shared_ptr<base_objects::entity_data::world_processor> make_processor() {
            base_objects::entity_data::world_processor proc;
            proc.entity_add_effect = entity_add_effect;
            proc.entity_animation = entity_animation;
            proc.entity_attach = entity_attach;
            proc.entity_attack = entity_attack;
            proc.entity_break = entity_break;
            proc.entity_cancel_break = entity_cancel_break;
            proc.entity_damage = entity_damage;
            proc.entity_damage_with_source = entity_damage_with_source;
            proc.entity_damage_with_sources = entity_damage_with_sources;
            proc.entity_death = entity_death;
            proc.entity_deinit = entity_deinit;
            proc.entity_detach = entity_detach;
            proc.entity_event = entity_event;
            proc.entity_finish_break = entity_finish_break;
            proc.entity_init = entity_init;
            proc.entity_iteract = entity_iteract;
            proc.entity_iteract_block = entity_iteract_block;
            proc.entity_leaves_ride = entity_leaves_ride;
            proc.entity_look_changes = entity_look_changes;
            proc.entity_motion_changes = entity_motion_changes;
            proc.entity_move = entity_move;
            proc.entity_place_block = entity_place_block;
            proc.entity_place_block_entity = entity_place_block_entity;
            proc.entity_remove_effect = entity_remove_effect;
            proc.entity_rides = entity_rides;
            proc.entity_rotation_changes = entity_rotation_changes;
            proc.entity_teleport = entity_teleport;
            proc.notify_biome_change = notify_biome_change;
            proc.notify_block_change = notify_block_change;
            proc.notify_block_destroy_change = notify_block_destroy_change;
            proc.notify_block_entity_change = notify_block_entity_change;
            proc.notify_block_entity_destroy_change = notify_block_entity_destroy_change;
            proc.notify_block_event = notify_block_event;
            proc.notify_chunk = notify_chunk;
            proc.notify_chunk_blocks = notify_chunk_blocks;
            proc.notify_chunk_light = notify_chunk_light;
            proc.notify_sub_chunk = notify_sub_chunk;
            proc.notify_sub_chunk_blocks = notify_sub_chunk_blocks;
            proc.notify_sub_chunk_light = notify_sub_chunk_light;
            proc.on_change_world = on_change_world;
            proc.on_tick = on_tick;
            return std::make_shared<base_objects::entity_data::world_processor>(std::move(proc));
        }

    public:
        PlayEngine() {}

        ~PlayEngine() noexcept {}

        void OnInitialization(const PluginRegistrationPtr& _) override {
            base_objects::entity_data::register_entity_world_processor(make_processor(), "minecraft:player");
            register_packet_processor([]([[maybe_unused]] api::packets::server_bound::play::chunk_batch_received&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                client.packets_state.chunk_batch_size = int32_t(packet.chunks_per_tick / 2) + 1; //TODO calculate correctly using ticks
                uint32_t expected = client.packets_state.await_ack_chunk_batches.load(); // Read the current value
                while (expected > 0)
                    if (client.packets_state.await_ack_chunk_batches.compare_exchange_weak(expected, expected - 1))
                        return;
                throw std::invalid_argument("There's no batches to acknowledge");
            });
            register_packet_processor([](api::packets::server_bound::play::client_tick_end&&, base_objects::SharedClientData& client) {
            });
        }

        void OnCommandsLoadComplete(const std::shared_ptr<PluginRegistration>&, base_objects::command_root_browser& root) override {
            api::players::iterate_online([&manager = root.get_manager()](base_objects::SharedClientData& client) {
                if (!client.is_virtual)
                    client << api::client::play::commands::create(manager);
                return false;
            });
        }

        static void push_player_info_action(api::client::play::player_info_update& res, base_objects::SharedClientData& client_ref) {
            using piu = api::client::play::player_info_update;
            res.actions.push(piu::header{client_ref.data->uuid});
            res.actions.push(
                piu::add_player{
                    .name = client_ref.name,
                    .properties
                    = to_list_array(client_ref.data->properties)
                          .convert_fn([](auto&& mojang) {
                              return piu::add_player::property{
                                  .name = std::move(mojang.name),
                                  .value = std::move(mojang.value),
                                  .signature = std::move(mojang.signature)
                              };
                          })
                }
            );
            res.actions.push(piu::listed{.should = client_ref.allow_server_listings});
            res.actions.push(piu::set_gamemode{.gamemode = client_ref.player_data.gamemode});
            res.actions.push(piu::set_hat_visible{.visible = client_ref.skin_parts.data.hat_enabled});
            res.actions.push(piu::set_ping{.milliseconds = (int32_t)client_ref.ping.count()});
        }

        void OnPlay_pre_initialize(base_objects::SharedClientData& client_ref) override {
            using piu = api::client::play::player_info_update;
            base_objects::network::response response = base_objects::network::response::empty();

            piu all_players;
            piu new_player;
            push_player_info_action(new_player, client_ref);
            api::players::iterate_online([&new_player, &all_players, &client_ref](base_objects::SharedClientData& client) {
                if (&client != &client_ref && !client.is_virtual) {
                    push_player_info_action(all_players, client);
                    client << piu{new_player};
                }
                return false;
            });
            client_ref << std::move(all_players);
            client_ref << std::move(new_player);
        }

        void PlayerLeave(base_objects::SharedClientData& client_ref) override {
            api::players::iterate_online([&client_ref](base_objects::SharedClientData& client) {
                if (&client != &client_ref && !client.is_virtual)
                    client << api::client::play::player_info_remove{
                        .uuids = {client_ref.data->uuid}
                    };
                return false;
            });
        }
    };
}
