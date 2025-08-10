/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/command.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/client.hpp>
#include <src/api/players.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>
#include <src/registers.hpp>

namespace copper_server::build_in_plugins {
    //handles clients with play state, allows players to access world and other things through api
    class PlayEngine : public PluginAutoRegister<"base/play_engine", PlayEngine> {
        base_objects::events::event_register_id process_chat_command_id;
        fast_task::task_mutex messages_order;
        list_array<std::array<uint8_t, 256>> lastset_messages;

        static std::shared_ptr<base_objects::entity_data::world_processor> make_processor() {
            base_objects::entity_data::world_processor proc;
            proc.entity_add_effect = [](base_objects::entity& self, base_objects::entity& target, uint32_t id, uint32_t duration, uint8_t amplifier, bool ambient, bool show_particles, bool show_icon, bool use_blend) {
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
                        .entity_id = target.protocol_id,
                        .effect = id,
                        .amplifier = amplifier,
                        .duration = (int32_t)duration,
                        .flags = f
                    };
                }
            };
            proc.entity_animation = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_animation animation) {
                if (self.assigned_player) {
                    *self.assigned_player << api::client::play::animate{
                        .entity_id = target.protocol_id,
                        .animation = static_cast<api::client::play::animate::animation_e>(animation)
                    };
                }
            };
            proc.entity_attach = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other_entity_id) {
                if (self.assigned_player && other_entity_id)
                    *self.assigned_player << api::client::play::set_entity_link{
                        .attached_entity_id = other_entity_id->protocol_id,
                        .holding_entity_id = target.protocol_id
                    };
            };
            proc.entity_attack = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other_entity_id) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::animate{
                        .entity_id = target.protocol_id,
                        .animation = api::client::play::animate::swing_main_arm
                    };
            };
            proc.entity_break = [](base_objects::entity& self, base_objects::entity& target, int64_t x, int64_t y, int64_t z, uint8_t state) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::block_destruction{
                        .entity_id = target.protocol_id,
                        .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                        .destroy_stage = state
                    };
            };
            proc.entity_cancel_break = [](base_objects::entity& self, base_objects::entity& target, int64_t x, int64_t y, int64_t z) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::block_destruction{
                        .entity_id = target.protocol_id,
                        .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                        .destroy_stage = 10
                    };
            };
            proc.entity_damage = [](base_objects::entity& self, base_objects::entity& target, float health, int32_t type_id, const std::optional<util::VECTOR>& pos) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::damage_event{
                        .entity_id = target.protocol_id,
                        .source_damage_type_id = type_id,
                        .source_pos = pos
                    };
            };
            proc.entity_damage_with_source = [](base_objects::entity& self, base_objects::entity& target, float health, int32_t type_id, base_objects::entity_ref& source, const std::optional<util::VECTOR>& pos) {
                if (self.assigned_player && source)
                    *self.assigned_player << api::client::play::damage_event{
                        .entity_id = target.protocol_id,
                        .source_damage_type_id = type_id,
                        .source_entity_id = source->protocol_id,
                        .source_direct_entity_id = source->protocol_id,
                        .source_pos = pos
                    };
            };
            proc.entity_damage_with_sources = [](base_objects::entity& self, base_objects::entity& target, float health, int32_t type_id, base_objects::entity_ref& source, base_objects::entity_ref& source_direct, const std::optional<util::VECTOR>& pos) {
                if (self.assigned_player && source && source_direct)
                    *self.assigned_player << api::client::play::damage_event{
                        .entity_id = target.protocol_id,
                        .source_damage_type_id = type_id,
                        .source_entity_id = source->protocol_id,
                        .source_direct_entity_id = source_direct->protocol_id,
                        .source_pos = pos
                    };
            };
            proc.entity_death = [](base_objects::entity& self, base_objects::entity& target) {
                if (self.assigned_player) {
                    *self.assigned_player << api::client::play::entity_event{
                        .entity_id = target.protocol_id,
                        .status = (int8_t)base_objects::entity_event::entity_died
                    };
                    //TODO add delay
                    *self.assigned_player << api::client::play::entity_event{
                        .entity_id = target.protocol_id,
                        .status = (int8_t)base_objects::entity_event::death_smoke
                    };
                }
            };
            proc.entity_deinit = [](base_objects::entity& self, base_objects::entity& target) {
                if (self.assigned_player) {
                    *self.assigned_player << api::client::play::remove_entities{
                        .entity_ids{target.protocol_id}
                    };
                }
            };
            proc.entity_detach = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::set_entity_link{
                        .attached_entity_id = (int32_t)target.protocol_id,
                        .holding_entity_id = -1
                    };
            };
            proc.entity_event = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_event status) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::entity_event{
                        .entity_id = target.protocol_id,
                        .status = (int8_t)status
                    };
            };
            proc.entity_finish_break = [](base_objects::entity& self, base_objects::entity& target, int64_t x, int64_t y, int64_t z) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::block_destruction{
                        .entity_id = target.protocol_id,
                        .location = {(int)x, (int)y, (int)z},
                        .destroy_stage = 11
                    };
            };
            proc.entity_init = [](base_objects::entity& self, base_objects::entity& target) {
                if (self.assigned_player) {
                    auto velocity = util::minecraft::packets::velocity(target.motion);
                    api::client::play::add_entity{
                        .entity_id = target.protocol_id,
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
            proc.entity_iteract = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::animate{
                        .entity_id = target.protocol_id,
                        .animation = api::client::play::animate::swing_main_arm
                    };
            };
            proc.entity_iteract_block = [](base_objects::entity& self, base_objects::entity& target, auto, auto, auto) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::animate{
                        .entity_id = target.protocol_id,
                        .animation = api::client::play::animate::swing_main_arm
                    };
            };
            proc.entity_leaves_ride = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other) {
                if (self.assigned_player && other)
                    *self.assigned_player << api::client::play::set_passengers{
                        .entity_id = target.protocol_id,
                        .passengers = other->ride_by_entity.convert_fn([](auto& entity) { return (base_objects::var_int32)entity->protocol_id; })
                    };
            };
            proc.entity_look_changes = [](base_objects::entity& self, base_objects::entity& target, util::ANGLE_DEG rot) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::rotate_head{
                        .entity_id = target.protocol_id,
                        .head_yaw = rot.x
                    };
            };
            proc.entity_motion_changes = [](base_objects::entity& self, base_objects::entity& target, util::VECTOR mot) {
                if (self.assigned_player) {
                    auto velocity = util::minecraft::packets::velocity(target.motion);
                    *self.assigned_player << api::client::play::set_entity_motion{
                        .entity_id = target.protocol_id,
                        .velocity_x = velocity.x,
                        .velocity_y = velocity.y,
                        .velocity_z = velocity.z
                    };
                }
            };
            proc.entity_move = [](base_objects::entity& self, base_objects::entity& target, util::VECTOR dif) {
                if (self.assigned_player) {
                    auto delta = util::minecraft::packets::delta_move({(float)dif.x, (float)dif.y, (float)dif.z});
                    *self.assigned_player << api::client::play::move_entity_pos{
                        .entity_id = target.protocol_id,
                        .delta_x = delta.x,
                        .delta_y = delta.y,
                        .delta_z = delta.z,
                        .on_ground = target.is_on_ground()
                    };
                }
            };
            proc.entity_place_block = [](base_objects::entity& self, base_objects::entity& target, bool is_main_hand, int64_t x, int64_t y, int64_t z, const base_objects::block& block) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::animate{
                        .entity_id = target.protocol_id,
                        .animation = is_main_hand ? api::client::play::animate::swing_main_arm : api::client::play::animate::swing_offhand
                    };
            };
            proc.entity_place_block_entity = [](base_objects::entity& self, base_objects::entity& target, bool is_main_hand, int64_t x, int64_t y, int64_t z, auto) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::animate{
                        .entity_id = target.protocol_id,
                        .animation = is_main_hand ? api::client::play::animate::swing_main_arm : api::client::play::animate::swing_offhand
                    };
            };
            proc.entity_remove_effect = [](base_objects::entity& self, base_objects::entity& target, uint32_t id) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::remove_mob_effect{
                        .entity_id = target.protocol_id,
                        .effect_id = id
                    };
            };
            proc.entity_rides = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other_entity) {
                if (self.assigned_player && other_entity)
                    *self.assigned_player << api::client::play::set_passengers{
                        .entity_id = other_entity->protocol_id,
                        .passengers = other_entity->ride_by_entity.convert_fn([](auto& entity) { return (base_objects::var_int32)entity->protocol_id; })
                    };
            };
            proc.entity_rotation_changes = [](base_objects::entity& self, base_objects::entity& target, util::ANGLE_DEG rot) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::move_entity_rot{
                        .entity_id = target.protocol_id,
                        .yaw = rot.x,
                        .pitch = rot.y,
                        .on_ground = target.is_on_ground()
                    };
            };
            proc.entity_teleport = [](base_objects::entity& self, base_objects::entity& target, util::VECTOR pos) {
                if (self.assigned_player)
                    *self.assigned_player << api::client::play::entity_position_sync{
                        .entity_id = target.protocol_id,
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


            proc.notify_biome_change = [](base_objects::entity& self, int64_t x, int64_t y, int64_t z, uint32_t biome_id) {
                if (self.assigned_player) {
                    if (self.current_world()) {
                        if (self.get_syncing_data().chunk_processed(x, z))
                            self.current_world()->get_chunk_at(x, z, [&self](storage::chunk_data& chunk) {
                                *self.assigned_player << api::client::play::chunks_biomes::create(chunk);
                            });
                    }
                }
            };
            proc.notify_block_change = [](base_objects::entity& self, int64_t x, int64_t y, int64_t z, const base_objects::block& block) {
                if (self.assigned_player) {
                    if (self.get_syncing_data().chunk_processed(x, z))
                        *self.assigned_player << api::client::play::block_update{
                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                            .block = block.id
                        };
                }
            };
            proc.notify_block_destroy_change = [](base_objects::entity& self, int64_t x, int64_t y, int64_t z, const base_objects::block& block) {
                if (self.assigned_player)
                    if (self.current_world()) {
                        if (self.get_syncing_data().chunk_processed(x, z)) {
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
            proc.notify_block_entity_change = [](base_objects::entity& self, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block_entity) {
                if (self.assigned_player) {
                    if (self.get_syncing_data().chunk_processed(x, z)) {
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
            proc.notify_block_entity_destroy_change = [](base_objects::entity& self, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block_entity) {
                if (self.assigned_player)
                    if (self.current_world()) {
                        if (self.get_syncing_data().chunk_processed(x, z)) {
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
            proc.notify_block_event = [](base_objects::entity& self, const base_objects::world::block_action& action, int64_t x, int64_t y, int64_t z) {
                if (self.assigned_player) {
                    if (self.get_syncing_data().chunk_processed(x, z)) {
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
            proc.notify_chunk = [](base_objects::entity& self, int64_t x, int64_t z, const storage::chunk_data& chunk) {
                if (self.assigned_player) {
                    if (self.get_syncing_data().chunk_in_bounds(x, z)) {
                        //TODO implement batching
                        *self.assigned_player << api::client::play::level_chunk_with_light::create(chunk, *self.current_world());
                        self.get_syncing_data().mark_chunk(x, z, true);
                    }
                }
            };
            proc.notify_chunk_blocks = [](base_objects::entity& self, int64_t x, int64_t z, const storage::chunk_data& chunk) {
                if (self.assigned_player) {
                    if (self.get_syncing_data().chunk_in_bounds(x, z)) {
                        //TODO implement batching
                        *self.assigned_player << api::client::play::level_chunk_with_light::create(chunk, *self.current_world());
                        self.get_syncing_data().mark_chunk(x, z, true);
                    }
                }
            };
            proc.notify_chunk_light = [](base_objects::entity& self, int64_t x, int64_t z, const storage::chunk_data& chunk) {
                if (self.assigned_player)
                    if (self.get_syncing_data().chunk_in_bounds(x, z))
                        *self.assigned_player << api::client::play::light_update::create(chunk);
            };
            proc.notify_sub_chunk = [](base_objects::entity& self, int64_t x, int64_t y, int64_t z, const base_objects::world::sub_chunk_data& chunk) {
                if (self.assigned_player) {
                    if (self.get_syncing_data().chunk_in_bounds(x, z)) {
                        if (self.current_world()) {
                            self.current_world()->get_chunk_at(x, z, [&](auto& chunk) {
                                //TODO implement batching
                                *self.assigned_player << api::client::play::level_chunk_with_light::create(chunk, *self.current_world());
                                self.get_syncing_data().mark_chunk(x, z, true);
                            });
                        }
                    }
                }
            };
            proc.notify_sub_chunk_blocks = [](base_objects::entity& self, int64_t x, int64_t y, int64_t z, const base_objects::world::sub_chunk_data& chunk) { //TODO use api::client::play::section_blocks_update
                if (self.assigned_player) {
                    if (self.get_syncing_data().chunk_in_bounds(x, z)) {
                        if (self.current_world()) {
                            self.current_world()->get_chunk_at(x, z, [&](auto& chunk) {
                                //TODO implement batching
                                *self.assigned_player << api::client::play::level_chunk_with_light::create(chunk, *self.current_world());
                                self.get_syncing_data().mark_chunk(x, z, true);
                            });
                        }
                    }
                }
            };
            proc.notify_sub_chunk_light = [](base_objects::entity& self, int64_t x, int64_t y, int64_t z, const base_objects::world::sub_chunk_data& chunk) {
                if (self.assigned_player) {
                    if (self.get_syncing_data().chunk_in_bounds(x, z)) {
                        if (self.current_world()) {
                            self.current_world()->get_chunk_at(x, z, [&](auto& chunk) {
                                *self.assigned_player << api::client::play::light_update::create(chunk);
                            });
                        }
                    }
                }
            };
            proc.on_change_world = [](base_objects::entity& self, storage::world_data& new_world) {
                if (self.assigned_player) {
                    if (self.current_world()) {
                        auto& player_data = self.assigned_player->player_data;
                        *self.assigned_player << api::client::play::respawn{
                            .dimension_type = registers::dimensionTypes.at(new_world.world_type).id,
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
                        self.get_syncing_data().flush_processing();
                    }
                }
            };

            proc.on_tick = [](base_objects::entity& self) {
                if (self.assigned_player) {
                    if (self.current_world()) {
                        //TODO implement batching
                        self.get_syncing_data().for_each_processing([&](int64_t chunk_x, int64_t chunk_z, bool loaded) {
                            if (!loaded) {
                                auto chunk = self.current_world()->request_chunk_data_weak(chunk_x, chunk_z);
                                if (chunk) {
                                    if ((*chunk)->generator_stage == 0xFF) {
                                        *self.assigned_player << api::client::play::level_chunk_with_light::create(**chunk, *self.current_world());
                                        self.get_syncing_data().mark_chunk(chunk_x, chunk_z, true);
                                    }
                                }
                            }
                        });
                    }
                }
            };
            return std::make_shared<base_objects::entity_data::world_processor>(std::move(proc));
        }

    public:
        PlayEngine() {}

        ~PlayEngine() noexcept {}

        void OnLoad(const PluginRegistrationPtr& self) override {
            process_chat_command_id = api::packets::register_server_bound_processor<api::packets::server_bound::play::chat_command>([](api::packets::server_bound::play::chat_command&& packet, base_objects::SharedClientData& client) {
                base_objects::command_context context(client, true);
                try {
                    api::command::get_manager().execute_command(packet.command, context);
                } catch (base_objects::command_exception& ex) {
                    try {
                        std::rethrow_exception(ex.exception);
                    } catch (const std::exception& inner_ex) {
                        std::string error_message = (std::string)packet.command;
                        std::string error_place(error_message.size() + 4, ' ');
                        error_place[0] = '\n';
                        error_place[error_place.size() - 2] = '\n';
                        error_place[error_place.size() - 1] = '\t';
                        if (ex.pos != -1)
                            error_place[ex.pos] = '^';
                        Chat res = error_message + error_place + inner_ex.what();
                        res.SetColor("red");
                        client << api::client::play::system_chat{
                            .content = std::move(res),
                            .is_overlay = false
                        };
                    }
                }
            });
            base_objects::entity_data::register_entity_world_processor(make_processor(), "minecraft:player");
        }

        void OnUnload(const PluginRegistrationPtr& self) override {
            api::packets::unregister_server_bound_processor(process_chat_command_id);
        }

        void OnCommandsLoadComplete(const std::shared_ptr<PluginRegistration>&, base_objects::command_root_browser& root) override {
            api::players::iterate_online([&manager = root.get_manager()](base_objects::SharedClientData& client) {
                if (!client.is_virtual)
                    client << api::client::play::commands::create(manager);
                return false;
            });
        }

        void PlayerJoined(base_objects::SharedClientData& client_ref) override {
            using piu = api::client::play::player_info_update;
            base_objects::network::response response = base_objects::network::response::empty();

            list_array<piu::action> all_players;
            piu::action new_player;
            new_player.set(
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
            all_players.push_back(new_player);

            api::players::iterate_online([&new_player, &all_players, &client_ref](base_objects::SharedClientData& client) {
                if (&client != &client_ref && !client.is_virtual) {
                    piu::action act;
                    act.set(
                        piu::add_player{
                            .name = client.name,
                            .properties
                            = to_list_array(client.data->properties)
                                  .convert_fn([](auto&& mojang) {
                                      return piu::add_player::property{
                                          .name = std::move(mojang.name),
                                          .value = std::move(mojang.value),
                                          .signature = std::move(mojang.signature)
                                      };
                                  })
                        }
                    );
                    all_players.push_back(std::move(act));
                    client << piu{.actions{new_player}};
                }
                return false;
            });
            client_ref << piu{.actions{all_players.take()}};
            client_ref << piu{.actions{std::move(new_player)}};
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
