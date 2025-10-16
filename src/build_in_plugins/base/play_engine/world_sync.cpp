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
#include <src/api/ecs/base_components.hpp>
#include <src/api/entity.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/api/packets/server_bound/play.hpp>
#include <src/api/players.hpp>
#include <src/api/registers.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::base::play_engine {
    class world_sync : public PluginAutoRegister<"base/play_engine/world_sync", world_sync> {
        static fast_task::future_ptr<void> send_async(auto& client, auto&& packet) {
            return fast_task::future<void>::start([client, packet = std::move(packet)]() mutable { *client << std::move(packet); });
        }

        static void entity_add_effect(api::ecs::entity self, api::ecs::entity target, uint32_t id, uint32_t duration, uint8_t amplifier, bool ambient, bool show_particles, bool show_icon, bool use_blend) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                api::packets::client_bound::play::update_mob_effect::flags_f f{};
                if (ambient)
                    f = f | api::packets::client_bound::play::update_mob_effect::flags_f::is_ambient;
                if (show_particles)
                    f = f | api::packets::client_bound::play::update_mob_effect::flags_f::show_particles;
                if (show_icon)
                    f = f | api::packets::client_bound::play::update_mob_effect::flags_f::show_icon;
                if (use_blend)
                    f = f | api::packets::client_bound::play::update_mob_effect::flags_f::blend;

                *assigned_player << api::packets::client_bound::play::update_mob_effect{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .effect = id,
                    .amplifier = amplifier,
                    .duration = (int32_t)duration,
                    .flags = f
                };
            }
        };

        static void entity_animation(api::ecs::entity self, api::ecs::entity target, base_objects::entity_animation animation) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;

            if (assigned_player) {
                *assigned_player << api::packets::client_bound::play::animate{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .animation = static_cast<api::packets::client_bound::play::animate::animation_e>(animation)
                };
            }
        };

        static void entity_attach(api::ecs::entity self, api::ecs::entity target, api::ecs::entity other_id) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::set_entity_link{
                    .attached_id = other_id.get<api::ecs::com::protocol_id>().value,
                    .holding_id = target.get<api::ecs::com::protocol_id>().value
                };
        };

        static void entity_attack(api::ecs::entity self, api::ecs::entity target, [[maybe_unused]] api::ecs::entity other_id) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::animate{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .animation = api::packets::client_bound::play::animate::swing_main_arm
                };
        };

        static void entity_break(api::ecs::entity self, api::ecs::entity target, int64_t x, int64_t y, int64_t z, uint8_t state) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::block_destruction{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                    .destroy_stage = state
                };
        };

        static void entity_cancel_break(api::ecs::entity self, api::ecs::entity target, int64_t x, int64_t y, int64_t z) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::block_destruction{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                    .destroy_stage = 10
                };
        };

        static void entity_damage(api::ecs::entity self, api::ecs::entity target, [[maybe_unused]] float health, int32_t type_id, const std::optional<util::VECTOR>& pos) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::damage_event{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .source_damage_type_id = type_id,
                    .source_pos = pos
                };
        };

        static void entity_damage_with_source(api::ecs::entity self, api::ecs::entity target, [[maybe_unused]] float health, int32_t type_id, std::optional<api::ecs::entity> source, const std::optional<util::VECTOR>& pos) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::damage_event{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .source_damage_type_id = type_id,
                    .source_id = source.transform([](auto entity) { return (api::id::entity_id)entity.get<api::ecs::com::protocol_id>().value; }),
                    .source_direct_id = source.transform([](auto entity) { return (api::id::entity_id)entity.get<api::ecs::com::protocol_id>().value; }),
                    .source_pos = pos
                };
        };

        static void entity_damage_with_sources(api::ecs::entity self, api::ecs::entity target, [[maybe_unused]] float health, int32_t type_id, std::optional<api::ecs::entity> source, std::optional<api::ecs::entity> source_direct, const std::optional<util::VECTOR>& pos) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::damage_event{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .source_damage_type_id = type_id,
                    .source_id = source.transform([](auto entity) { return (api::id::entity_id)entity.get<api::ecs::com::protocol_id>().value; }),
                    .source_direct_id = source_direct.transform([](auto entity) { return (api::id::entity_id)entity.get<api::ecs::com::protocol_id>().value; }),
                    .source_pos = pos
                };
        };

        static void entity_death(api::ecs::entity self, api::ecs::entity target) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                *assigned_player << api::packets::client_bound::play::entity_event{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .status = (int8_t)base_objects::entity_event::entity_died
                };
                //TODO add delay
                *assigned_player << api::packets::client_bound::play::entity_event{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .status = (int8_t)base_objects::entity_event::death_smoke
                };
            }
        };

        static void entity_deinit(api::ecs::entity self, api::ecs::entity target) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                *assigned_player << api::packets::client_bound::play::remove_entities{
                    .ids{target.get<api::ecs::com::protocol_id>().value}
                };
            }
        };

        static void entity_detach(api::ecs::entity self, api::ecs::entity target, [[maybe_unused]] api::ecs::entity other) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::set_entity_link{
                    .attached_id = (int32_t)target.get<api::ecs::com::protocol_id>().value,
                    .holding_id = -1
                };
        };

        static void entity_event(api::ecs::entity self, api::ecs::entity target, base_objects::entity_event status) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::entity_event{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .status = (int8_t)status
                };
        }

        static void entity_metadata(api::ecs::entity self, api::ecs::entity target) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::set_entity_data::create(target);
        }

        static void entity_finish_break(api::ecs::entity self, api::ecs::entity target, int64_t x, int64_t y, int64_t z) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::block_destruction{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .location = {(int)x, (int)y, (int)z},
                    .destroy_stage = 11
                };
        }

        static void entity_init(api::ecs::entity self, api::ecs::entity target) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                auto velocity = target.get<api::ecs::com::motion>();
                auto pos = target.get<api::ecs::com::position>();
                auto rot = target.get<api::ecs::com::rotation>();
                auto head_rot = target.get<api::ecs::com::head_rotation>();
                *assigned_player << api::packets::client_bound::play::add_entity{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .uuid = api::entity_id_map::get_uuid(target.get<api::ecs::com::protocol_id>().value),
                    .type = target.get<api::ecs::com::entity_type>().type,
                    .x = pos.x,
                    .y = pos.y,
                    .z = pos.z,
                    .pitch = rot.pitch,
                    .yaw = rot.yaw,
                    .head_yaw = head_rot.yaw,
                    .data = api::entity(target).get_object_field().or_else([]() { return std::optional<int>(0); }).value(),
                    .velocity = {velocity.x, velocity.y, velocity.z}
                };
            }
        }

        static void entity_iteract(api::ecs::entity self, api::ecs::entity target, [[maybe_unused]] api::ecs::entity other) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::animate{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .animation = api::packets::client_bound::play::animate::swing_main_arm
                };
        }

        static void entity_iteract_block(api::ecs::entity self, api::ecs::entity target, auto, auto, auto) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::animate{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .animation = api::packets::client_bound::play::animate::swing_main_arm
                };
        };

        static void entity_leaves_ride(api::ecs::entity self, api::ecs::entity target, api::ecs::entity other) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::set_passengers{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .passengers = other.get<api::ecs::com::ride_by_entity>().ride_by.convert_fn([](auto& entity) { return (base_objects::var_int32)entity.get<api::ecs::com::protocol_id>().value; })
                };
        };

        static void entity_look_changes(api::ecs::entity self, api::ecs::entity target, util::ANGLE_DEG rot) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::rotate_head{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .head_yaw = rot.yaw
                };
        };

        static void entity_motion_changes(api::ecs::entity self, api::ecs::entity target, [[maybe_unused]] util::VECTOR mot) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                *assigned_player << api::packets::client_bound::play::set_entity_motion{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .velocity = {mot.x, mot.y, mot.z}
                };
            }
        };

        static void entity_move(api::ecs::entity self, api::ecs::entity target, util::VECTOR dif) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                auto delta = util::minecraft::packets::delta_move({(float)dif.x, (float)dif.y, (float)dif.z});
                *assigned_player << api::packets::client_bound::play::move_entity_pos{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .delta_x = delta.x,
                    .delta_y = delta.y,
                    .delta_z = delta.z,
                    .on_ground = target.get<api::ecs::com::on_ground>().value
                };
            }
        };

        static void entity_place_block(api::ecs::entity self, api::ecs::entity target, bool is_main_hand, [[maybe_unused]] int64_t x, [[maybe_unused]] int64_t y, [[maybe_unused]] int64_t z, [[maybe_unused]] const base_objects::block& block) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::animate{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .animation = is_main_hand ? api::packets::client_bound::play::animate::swing_main_arm : api::packets::client_bound::play::animate::swing_offhand
                };
        };

        static void entity_place_block_entity(api::ecs::entity self, api::ecs::entity target, bool is_main_hand, [[maybe_unused]] int64_t x, [[maybe_unused]] int64_t y, [[maybe_unused]] int64_t z, auto) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::animate{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .animation = is_main_hand ? api::packets::client_bound::play::animate::swing_main_arm : api::packets::client_bound::play::animate::swing_offhand
                };
        };

        static void entity_remove_effect(api::ecs::entity self, api::ecs::entity target, uint32_t id) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::remove_mob_effect{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .effect_id = id
                };
        };

        static void entity_rides(api::ecs::entity self, [[maybe_unused]] api::ecs::entity target, api::ecs::entity other_entity) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::set_passengers{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .passengers = other_entity.get<api::ecs::com::ride_by_entity>().ride_by.convert_fn([](auto& entity) { return (base_objects::var_int32)entity.get<api::ecs::com::protocol_id>().value; })
                };
        };

        static void entity_rotation_changes(api::ecs::entity self, api::ecs::entity target, util::ANGLE_DEG rot) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::move_entity_rot{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .yaw = rot.yaw,
                    .pitch = rot.pitch,
                    .on_ground = target.get<api::ecs::com::on_ground>().value
                };
        };

        static void entity_teleport(api::ecs::entity self, api::ecs::entity target, util::VECTOR pos) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                auto mot = target.get<api::ecs::com::motion>();
                auto rot = target.get<api::ecs::com::rotation>();
                *assigned_player << api::packets::client_bound::play::entity_position_sync{
                    .id = target.get<api::ecs::com::protocol_id>().value,
                    .x = pos.x,
                    .y = pos.y,
                    .z = pos.z,
                    .velocity_x = mot.x,
                    .velocity_y = mot.y,
                    .velocity_z = mot.z,
                    .yaw = (float)rot.yaw,
                    .pitch = (float)rot.pitch,
                    .on_ground = target.get<api::ecs::com::on_ground>().value
                };
            }
        };

        static void notify_biome_change(api::ecs::entity self, int64_t x, [[maybe_unused]] int64_t y, int64_t z, [[maybe_unused]] uint32_t biome_id) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {

                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world) {
                        if (ws.chunk_processed(x, z))
                            ws.world->get_chunk_at(x, z, [&assigned_player](storage::chunk_data& chunk) {
                                *assigned_player << api::packets::client_bound::play::chunks_biomes::create(chunk);
                            });
                    }
                }
            }
        };

        static void notify_block_change(api::ecs::entity self, int64_t x, int64_t y, int64_t z, const base_objects::block& block) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world) {
                        if (ws.chunk_processed(x, z)) {
                            *assigned_player << api::packets::client_bound::play::block_update{
                                .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                .block = block.id
                            };
                        }
                    }
                }
            }
        };

        static void notify_block_destroy_change(api::ecs::entity self, int64_t x, int64_t y, int64_t z, const base_objects::block& block) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world) {
                        if (ws.chunk_processed(x, z)) {
                            *assigned_player << api::packets::client_bound::play::level_event{
                                .event = api::packets::client_bound::play::level_event::event_id::block_break_and_sound,
                                .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                .data = (int32_t)block.id,
                                .disable_volume = false
                            };
                            *assigned_player << api::packets::client_bound::play::block_update{
                                .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                .block = block.id
                            };
                        }
                    }
                }
            }
        };

        static void notify_block_entity_change(api::ecs::entity self, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block_entity) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world) {
                        if (ws.chunk_processed(x, z)) {
                            *assigned_player << api::packets::client_bound::play::block_update{
                                .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                .block = block_entity.block.id
                            };
                            *assigned_player << api::packets::client_bound::play::block_entity_data{
                                .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                .type = block_entity.block.block_entity_id(),
                                .data = block_entity.data
                            };
                        }
                    }
                }
            }
        }

        static void notify_block_entity_destroy_change(api::ecs::entity self, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block_entity) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world) {
                        if (ws.chunk_processed(x, z)) {
                            *assigned_player << api::packets::client_bound::play::level_event{
                                .event = api::packets::client_bound::play::level_event::event_id::block_break_and_sound,
                                .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                .data = (int32_t)block_entity.block.id,
                                .disable_volume = false
                            };
                            *assigned_player << api::packets::client_bound::play::block_update{
                                .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                .block = block_entity.block.id
                            };
                        }
                    }
                }
            }
        };

        static void notify_block_event(api::ecs::entity self, const base_objects::world::block_action& action, int64_t x, int64_t y, int64_t z) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world) {
                        if (ws.chunk_processed(x, z)) {
                            std::visit(
                                [x, y, z, &assigned_player](auto& it) mutable {
                                    using T = std::decay_t<decltype(it)>;
                                    if constexpr (std::is_same_v<T, base_objects::world::block_action::noteblock_activated>) {
                                        *assigned_player << api::packets::client_bound::play::block_event{
                                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                            .action_id = 0,
                                            .action_param = 0,
                                            .block = "minecraft:note_block"
                                        };
                                    } else if constexpr (std::is_same_v<T, base_objects::world::block_action::piston_extend>) {
                                        *assigned_player << api::packets::client_bound::play::block_event{
                                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                            .action_id = 0,
                                            .action_param = (uint8_t)it.dir,
                                            .block = "minecraft:piston"
                                        };
                                    } else if constexpr (std::is_same_v<T, base_objects::world::block_action::piston_retract>) {
                                        *assigned_player << api::packets::client_bound::play::block_event{
                                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                            .action_id = 1,
                                            .action_param = (uint8_t)it.dir,
                                            .block = "minecraft:piston"
                                        };
                                    } else if constexpr (std::is_same_v<T, base_objects::world::block_action::piston_canceled>) {
                                        *assigned_player << api::packets::client_bound::play::block_event{
                                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                            .action_id = 2,
                                            .action_param = (uint8_t)it.dir,
                                            .block = "minecraft:piston"
                                        };
                                    } else if constexpr (std::is_same_v<T, base_objects::world::block_action::chest_opened>) {
                                        *assigned_player << api::packets::client_bound::play::block_event{
                                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                            .action_id = 1,
                                            .action_param = (uint8_t)it.count,
                                            .block = "minecraft:chest"
                                        };
                                    } else if constexpr (std::is_same_v<T, base_objects::world::block_action::reset_spawner>) {
                                        *assigned_player << api::packets::client_bound::play::block_event{
                                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                            .action_id = 1,
                                            .action_param = 0,
                                            .block = "minecraft:spawner"
                                        };
                                    } else if constexpr (std::is_same_v<T, base_objects::world::block_action::end_gateway_activated>) {
                                        *assigned_player << api::packets::client_bound::play::block_event{
                                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                            .action_id = 1,
                                            .action_param = 0,
                                            .block = "minecraft:end_gateway"
                                        };
                                    } else if constexpr (std::is_same_v<T, base_objects::world::block_action::shulker_box_closed>) {
                                        *assigned_player << api::packets::client_bound::play::block_event{
                                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                            .action_id = 0,
                                            .action_param = 0,
                                            .block = "minecraft:shulker_box"
                                        };
                                    } else if constexpr (std::is_same_v<T, base_objects::world::block_action::shulker_box_opened>) {
                                        *assigned_player << api::packets::client_bound::play::block_event{
                                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                            .action_id = 0,
                                            .action_param = 1,
                                            .block = "minecraft:shulker_box"
                                        };
                                    } else if constexpr (std::is_same_v<T, base_objects::world::block_action::shulker_box_opened_count>) {
                                        *assigned_player << api::packets::client_bound::play::block_event{
                                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                            .action_id = 1,
                                            .action_param = (uint8_t)it.count,
                                            .block = "minecraft:shulker_box"
                                        };
                                    } else if constexpr (std::is_same_v<T, base_objects::world::block_action::bell_ring>) {
                                        *assigned_player << api::packets::client_bound::play::block_event{
                                            .location = {(int32_t)x, (int32_t)y, (int32_t)z},
                                            .action_id = 1,
                                            .action_param = (uint8_t)it.dir,
                                            .block = "minecraft:bell"
                                        };
                                    } else if constexpr (std::is_same_v<T, base_objects::world::block_action::decorated_block_woble>) {
                                        *assigned_player << api::packets::client_bound::play::block_event{
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
                }
            }
        }

        static void notify_chunk(api::ecs::entity self, int64_t x, int64_t z, const storage::chunk_data& chunk) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world) {
                        if (ws.chunk_in_bounds(x, z)) {
                            if (assigned_player->packets_state.chunk_batch_size > assigned_player->packets_state.chunks_sent) {
                                ++assigned_player->packets_state.chunks_sent;
                                send_async(assigned_player, api::packets::client_bound::play::level_chunk_with_light::create(chunk, *ws.world));
                                self.modify<api::ecs::com::world_syncing>()->mark_chunk(x, z, true);
                            }
                        }
                    }
                }
            }
        }

        static void notify_chunk_blocks(api::ecs::entity self, int64_t x, int64_t z, const storage::chunk_data& chunk) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world) {
                        if (ws.chunk_processed(x, z)) {
                            if (assigned_player->is_active()) {
                                if (assigned_player->packets_state.chunk_batch_size > assigned_player->packets_state.chunks_sent) {
                                    ++assigned_player->packets_state.chunks_sent;
                                    send_async(assigned_player, api::packets::client_bound::play::level_chunk_with_light::create(chunk, *ws.world));
                                    self.modify<api::ecs::com::world_syncing>()->mark_chunk(x, z, true);
                                }
                            }
                        }
                    }
                }
            }
        }

        static void notify_chunk_light(api::ecs::entity self, int64_t x, int64_t z, const storage::chunk_data& chunk) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world)
                        if (ws.chunk_in_bounds(x, z))
                            if (ws.chunk_processed(x, z))
                                *assigned_player << api::packets::client_bound::play::light_update::create(chunk);
                }
            }
        }

        static void notify_sub_chunk(api::ecs::entity self, int64_t x, [[maybe_unused]] int64_t y, int64_t z, [[maybe_unused]] const base_objects::world::sub_chunk_data& chunk) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world) {
                        if (ws.chunk_in_bounds(x, z)) {
                            if (assigned_player->packets_state.chunk_batch_size > assigned_player->packets_state.chunks_sent) {
                                ++assigned_player->packets_state.chunks_sent;
                                ws.world->get_chunk_at(x, z, [&](auto& chunk) {
                                    if (assigned_player->is_active()) {
                                        send_async(assigned_player, api::packets::client_bound::play::level_chunk_with_light::create(chunk, *ws.world));
                                        self.modify<api::ecs::com::world_syncing>()->mark_chunk(x, z, true);
                                    }
                                });
                            }
                        }
                    }
                }
            }
        }

        static void notify_sub_chunk_blocks(api::ecs::entity self, int64_t x, [[maybe_unused]] int64_t y, int64_t z, [[maybe_unused]] const base_objects::world::sub_chunk_data& chunk) { //TODO use  api::packets::client_bound::play::section_blocks_update
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world) {
                        if (ws.chunk_in_bounds(x, z)) {
                            if (assigned_player->packets_state.chunk_batch_size > assigned_player->packets_state.chunks_sent) {
                                ++assigned_player->packets_state.chunks_sent;
                                ws.world
                                    ->get_chunk_at(x, z, [&](auto& chunk) {
                                        if (assigned_player->is_active()) {
                                            send_async(assigned_player, api::packets::client_bound::play::level_chunk_with_light::create(chunk, *ws.world));
                                            self.modify<api::ecs::com::world_syncing>()->mark_chunk(x, z, true);
                                        }
                                    });
                            }
                        }
                    }
                }
            }
        }

        static void notify_sub_chunk_light(api::ecs::entity self, int64_t x, [[maybe_unused]] int64_t y, int64_t z, [[maybe_unused]] const base_objects::world::sub_chunk_data& chunk) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world)
                        if (ws.chunk_in_bounds(x, z))
                            ws.world->get_chunk_at(x, z, [&](auto& chunk) {
                                *assigned_player << api::packets::client_bound::play::light_update::create(chunk);
                            });
                }
            }
        }

        static void on_change_world(api::ecs::entity self, storage::world_data& new_world) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world) {
                        auto& player_data = assigned_player->player_data;
                        *assigned_player << api::packets::client_bound::play::respawn{
                            .dimension_type = api::registers::dimensionTypes.at(new_world.get_world_type()).id,
                            .dimension_name = new_world.world_name,
                            .seed_hashed = new_world.get_hashed_seed(),
                            .gamemode = player_data.gamemode,
                            .previous_gamemode = (api::packets::optional_gamemode_e)player_data.gamemode,
                            .is_debug = new_world.world_generator_data.contains("debug") ? (bool)new_world.world_generator_data["debug"] : false,
                            .is_flat = new_world.world_generator_data.contains("flat") ? (bool)new_world.world_generator_data["flat"] : false,
                            .death_location = player_data.last_death_location ? std::make_optional(api::packets::client_bound::play::respawn::death_location_t(player_data.last_death_location->world_id, {(int32_t)player_data.last_death_location->x, (int32_t)player_data.last_death_location->y, (int32_t)player_data.last_death_location->z})) : std::nullopt,
                            .portal_cooldown = 0,
                            .sea_level = new_world.world_generator_data.contains("sea_level") ? (bool)new_world.world_generator_data["sea_level"] : 60,
                            .flags = api::packets::client_bound::play::respawn::keep_attributes | api::packets::client_bound::play::respawn::keep_metadata
                        };
                        self.modify<api::ecs::com::world_syncing>()->flush_processing();
                    }
                }
            }
        }

        static void on_tick(api::ecs::entity self) {
            auto assigned_player = self.get<api::ecs::com::assigned_player>().player;
            if (assigned_player) {
                auto& player = *assigned_player;
                if (self.has<api::ecs::com::world_syncing>()) {
                    auto& ws = self.get<api::ecs::com::world_syncing>();
                    if (ws.world) {
                        if (!player.packets_state.is_play_initialized) {
                            player.packets_state.is_play_initialized = true;
                            *assigned_player << api::packets::client_bound::play::game_event{
                                .event = {api::packets::client_bound::play::game_event::wait_for_level_chunks{}},
                            };
                            *assigned_player << api::packets::client_bound::play::set_chunk_cache_center{
                                .x = (int32_t)ws.processing_region.center_x,
                                .z = (int32_t)ws.processing_region.center_z,
                            };
                            *assigned_player << api::packets::client_bound::play::set_chunk_cache_radius{
                                .distance = (int32_t)ws.processing_region.radius
                            };
                        }
                        fast_task::spin_lock packet_futs_lock;
                        std::vector<fast_task::future_ptr<void>> packet_futs;
                        bool make_tick = false;
                        bool make_batch = false;
                        auto max_unbc = api::configuration::get().protocol.max_unacknowledged_chunk_batches;
                        if (max_unbc > player.packets_state.await_ack_chunk_batches) {
                            std::vector<fast_task::future_ptr<void>> serialization_futs;
                            ws.for_each_processing([&](int64_t chunk_x, int64_t chunk_z, bool loaded) {
                                if (!loaded) {
                                    if (player.is_active()) {
                                        if (player.packets_state.chunk_batch_size > player.packets_state.chunks_sent) {
                                            ++player.packets_state.chunks_sent;
                                            auto chunk = ws.world->request_chunk_data_weak(chunk_x, chunk_z);
                                            if (chunk) {
                                                if (!make_batch) {
                                                    *assigned_player << api::packets::client_bound::play::chunk_batch_start{};
                                                    make_batch = true;
                                                }
                                                if ((*chunk)->generator_stage == 0xFF) {
                                                    serialization_futs.push_back(
                                                        fast_task::future<void>::start(
                                                            [&, chunk]() {
                                                                auto packet = send_async(assigned_player, api::packets::client_bound::play::level_chunk_with_light::create(**chunk, *ws.world));
                                                                fast_task::lock_guard guard(packet_futs_lock);
                                                                packet_futs.push_back(std::move(packet));
                                                            }
                                                        )
                                                    );
                                                    self.modify<api::ecs::com::world_syncing>()->mark_chunk(chunk_x, chunk_z, true);
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
                            fast_task::future_tool::combine_all(packet_futs)->when_ready([batch_size = player.packets_state.chunks_sent, player = assigned_player]() {
                                *player << api::packets::client_bound::play::chunk_batch_finished{
                                    .batch_size = batch_size
                                };
                            });
                            player.packets_state.chunks_sent = 0;
                        }
                        if (make_tick) {
                            if (!player.packets_state.is_play_fully_initialized) {
                                player.packets_state.is_play_fully_initialized = true;
                                auto pos = self.get<api::ecs::com::position>();
                                auto mot = self.get<api::ecs::com::motion>();
                                auto rot = self.get<api::ecs::com::rotation>();
                                *assigned_player << api::packets::client_bound::play::player_position{
                                    .x = pos.x,
                                    .y = pos.y,
                                    .z = pos.z,
                                    .velocity_x = mot.x,
                                    .velocity_y = mot.y,
                                    .velocity_z = mot.z,
                                    .yaw = (float)rot.yaw,
                                    .pitch = (float)rot.pitch,
                                    .flags = api::packets::teleport_flags{}
                                };
                            }
                            if (!ws.world->ticking_frozen)
                                *assigned_player << api::packets::client_bound::play::ticking_step{.steps = 1};
                        }
                    }
                }
            }
        };

        static std::shared_ptr<api::entity_data::world_processor> make_processor() {
            api::entity_data::world_processor proc;
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
            proc.entity_metadata = entity_metadata;
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
            return std::make_shared<api::entity_data::world_processor>(std::move(proc));
        }

    public:
        world_sync() {}

        ~world_sync() noexcept {}

        void OnInitialization(const PluginRegistrationPtr& _) override {
            api::entity_data::register_entity_world_processor(make_processor(), "minecraft:player");
            api::packets::processor(*this, [](api::packets::server_bound::play::chunk_batch_received&& packet, base_objects::SharedClientData& client) {
                client.packets_state.chunk_batch_size = (int32_t)std::ceil(packet.chunks_per_tick);

                uint32_t expected = client.packets_state.await_ack_chunk_batches.load(); // Read the current value
                while (expected > 0)
                    if (client.packets_state.await_ack_chunk_batches.compare_exchange_weak(expected, expected - 1))
                        return;
                throw std::invalid_argument("There's no batches to acknowledge");
            });

            api::packets::processor(*this, [](api::packets::server_bound::play::client_tick_end&&, base_objects::SharedClientData& client) {
            });
        }
    };
}