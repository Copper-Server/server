#include <src/api/command.hpp>
#include <src/api/packets.hpp>
#include <src/api/players.hpp>
#include <src/api/protocol.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    //handles clients with play state, allows players to access world and other things through api
    class PlayEngine : public PluginAutoRegister<"play_engine", PlayEngine> {
        fast_task::task_mutex messages_order;
        list_array<std::array<uint8_t, 256>> lastset_messages;

        static list_array<base_objects::chunk::chunk_biomes> build_biomes_chunk(storage::chunk_data& chunk) {
            base_objects::chunk::chunk_biomes biomes;
            biomes.x = chunk.chunk_x;
            biomes.z = chunk.chunk_z;
            biomes.biomes.reserve(chunk.sub_chunks.size());
            for (auto& it : chunk.sub_chunks) {
                base_objects::chunk::pelleted_container_direct_biomes current;
                for (auto& x : it.biomes)
                    for (auto& y : x)
                        for (auto& z : y)
                            current.data.add(z);
                biomes.biomes.push_back(current);
            }
            return {std::move(biomes)};
        }

        static std::shared_ptr<base_objects::entity_data::world_processor> make_processor() {
            base_objects::entity_data::world_processor proc;
            proc.entity_add_effect = [](base_objects::entity& self, base_objects::entity& target, uint32_t id, uint32_t duration, uint8_t amplifier, bool ambient, bool show_particles, bool show_icon, bool use_blend) {
                if (self.assigned_player) {
                    base_objects::packets::effect_flags flags{.raw = 0};
                    flags.is_ambient = ambient;
                    flags.show_icon = show_icon;
                    flags.show_particles = show_particles;
                    flags.use_blend = use_blend;
                    api::packets::play::entityEffect(*self.assigned_player, target.protocol_id, id, duration, amplifier, flags);
                }
            };
            proc.entity_animation = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_animation animation) {
                if (self.assigned_player)
                    api::packets::play::entityAnimation(*self.assigned_player, target, animation);
            };
            proc.entity_attach = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other_entity_id) {
                if (self.assigned_player && other_entity_id)
                    api::packets::play::linkEntities(*self.assigned_player, other_entity_id->protocol_id, target.protocol_id);
            };
            proc.entity_attack = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other_entity_id) {
                if (self.assigned_player)
                    api::packets::play::entityAnimation(*self.assigned_player, target, base_objects::entity_animation::swing_main_arm); //damage event passed from other entity
            };
            proc.entity_break = [](base_objects::entity& self, base_objects::entity& target, int64_t x, int64_t y, int64_t z, uint8_t state) {
                if (self.assigned_player)
                    api::packets::play::setBlockDestroyStage(*self.assigned_player, target, {(int)x, (int)y, (int)z}, state);
            };
            proc.entity_cancel_break = [](base_objects::entity& self, base_objects::entity& target, int64_t x, int64_t y, int64_t z) {
                if (self.assigned_player)
                    api::packets::play::setBlockDestroyStage(*self.assigned_player, target, {(int)x, (int)y, (int)z}, 10);
            };
            proc.entity_damage = [](base_objects::entity& self, base_objects::entity& target, float health, int32_t type_id, const std::optional<util::VECTOR>& pos) {
                if (self.assigned_player)
                    api::packets::play::damageEvent(*self.assigned_player, target.protocol_id, type_id, 0, 0, pos);
            };
            proc.entity_damage_with_source = [](base_objects::entity& self, base_objects::entity& target, float health, int32_t type_id, base_objects::entity_ref& source, const std::optional<util::VECTOR>& pos) {
                if (self.assigned_player && source)
                    api::packets::play::damageEvent(*self.assigned_player, target.protocol_id, type_id, source->protocol_id, source->protocol_id, pos);
            };
            proc.entity_damage_with_sources = [](base_objects::entity& self, base_objects::entity& target, float health, int32_t type_id, base_objects::entity_ref& source, base_objects::entity_ref& source_direct, const std::optional<util::VECTOR>& pos) {
                if (self.assigned_player && source && source_direct)
                    api::packets::play::damageEvent(*self.assigned_player, target.protocol_id, type_id, source->protocol_id, source_direct->protocol_id, pos);
            };
            proc.entity_death = [](base_objects::entity& self, base_objects::entity& target) {
                if (self.assigned_player) {
                    api::packets::play::entityEvent(*self.assigned_player, target.protocol_id, base_objects::entity_event::entity_died);
                    //TODO add delay
                    api::packets::play::entityEvent(*self.assigned_player, target.protocol_id, base_objects::entity_event::death_smoke);
                }
            };
            proc.entity_deinit = [](base_objects::entity& self, base_objects::entity& target) {
                if (self.assigned_player)
                    api::packets::play::removeEntities(*self.assigned_player, {target.protocol_id});
            };
            proc.entity_detach = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other) {
                if (self.assigned_player)
                    api::packets::play::linkEntities(*self.assigned_player, target.protocol_id, -1);
            };
            proc.entity_event = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_event status) {
                if (self.assigned_player)
                    api::packets::play::entityEvent(*self.assigned_player, target.protocol_id, status);
            };
            proc.entity_finish_break = [](base_objects::entity& self, base_objects::entity& target, int64_t x, int64_t y, int64_t z) {
                if (self.assigned_player)
                    api::packets::play::setBlockDestroyStage(*self.assigned_player, target, {(int)x, (int)y, (int)z}, 11);
            };
            proc.entity_init = [](base_objects::entity& self, base_objects::entity& target) {
                if (self.assigned_player)
                    api::packets::play::spawnEntity(*self.assigned_player, target, target.protocol_id);
            };
            proc.entity_iteract = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other) {
                if (self.assigned_player)
                    api::packets::play::entityAnimation(*self.assigned_player, target, base_objects::entity_animation::swing_main_arm);
            };
            proc.entity_iteract_block = [](base_objects::entity& self, base_objects::entity& target, auto, auto, auto) {
                if (self.assigned_player)
                    api::packets::play::entityAnimation(*self.assigned_player, target, base_objects::entity_animation::swing_main_arm);
            };
            proc.entity_leaves_ride = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other) {
                if (self.assigned_player && other)
                    api::packets::play::setPassengers(*self.assigned_player, other->protocol_id, other->ride_by_entity.convert_fn([](auto& entity) { return entity->protocol_id; }));
            };
            proc.entity_look_changes = [](base_objects::entity& self, base_objects::entity& target, util::ANGLE_DEG rot) {
                if (self.assigned_player)
                    api::packets::play::setHeadRotation(*self.assigned_player, target.protocol_id, rot);
            };
            proc.entity_motion_changes = [](base_objects::entity& self, base_objects::entity& target, util::VECTOR mot) {
                if (self.assigned_player)
                    api::packets::play::setEntityMotion(*self.assigned_player, target.protocol_id, mot);
            };
            proc.entity_move = [](base_objects::entity& self, base_objects::entity& target, util::VECTOR dif) {
                if (self.assigned_player)
                    api::packets::play::updateEntityPosition(*self.assigned_player, target.protocol_id, {(float)dif.x, (float)dif.y, (float)dif.z}, target.is_on_ground());
            };
            proc.entity_place_block = [](base_objects::entity& self, base_objects::entity& target, bool is_main_hand, int64_t x, int64_t y, int64_t z, const base_objects::block& block) {
                if (self.assigned_player)
                    api::packets::play::entityAnimation(*self.assigned_player, target, is_main_hand ? base_objects::entity_animation::swing_main_arm : base_objects::entity_animation::swing_offhand);
            };
            proc.entity_place_block_entity = [](base_objects::entity& self, base_objects::entity& target, bool is_main_hand, int64_t x, int64_t y, int64_t z, auto) {
                if (self.assigned_player)
                    api::packets::play::entityAnimation(*self.assigned_player, target, is_main_hand ? base_objects::entity_animation::swing_main_arm : base_objects::entity_animation::swing_offhand);
            };
            proc.entity_remove_effect = [](base_objects::entity& self, base_objects::entity& target, uint32_t id) {
                if (self.assigned_player)
                    api::packets::play::removeEntityEffect(*self.assigned_player, target.protocol_id, id);
            };
            proc.entity_rides = [](base_objects::entity& self, base_objects::entity& target, base_objects::entity_ref& other_entity) {
                if (self.assigned_player && other_entity)
                    api::packets::play::setPassengers(*self.assigned_player, other_entity->protocol_id, other_entity->ride_by_entity.convert_fn([](auto& entity) { return entity->protocol_id; }));
            };
            proc.entity_rotation_changes = [](base_objects::entity& self, base_objects::entity& target, util::ANGLE_DEG rot) {
                if (self.assigned_player)
                    api::packets::play::updateEntityRotation(*self.assigned_player, target.protocol_id, rot, target.is_on_ground());
            };
            proc.entity_teleport = [](base_objects::entity& self, base_objects::entity& target, util::VECTOR pos) {
                if (self.assigned_player)
                    api::packets::play::teleportEntity(*self.assigned_player, target.protocol_id, pos, target.rotation.x, target.rotation.y, target.is_on_ground());
            };


            proc.notify_biome_change = [](base_objects::entity& self, int64_t x, int64_t y, int64_t z, uint32_t biome_id) {
                if (self.assigned_player) {
                    if (self.current_world()) {
                        if (self.assigned_player->chunk_loaded_at(x, z))
                            self.current_world()->get_chunk_at(x, z, [&self](storage::chunk_data& chunk) {
                                auto res = build_biomes_chunk(chunk);
                                api::packets::play::chunkBiomes(*self.assigned_player, res);
                            });
                    }
                }
            };
            proc.notify_block_change = [](base_objects::entity& self, int64_t x, int64_t y, int64_t z, const base_objects::block& block) {
                if (self.assigned_player) {
                    if (self.assigned_player->chunk_loaded_at(x, z))
                        api::packets::play::blockUpdate(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, block);
                }
            };
            proc.notify_block_destroy_change = [](base_objects::entity& self, int64_t x, int64_t y, int64_t z, const base_objects::block& block) {
                if (self.assigned_player)
                    if (self.current_world()) {
                        if (self.assigned_player->chunk_loaded_at(x, z)) {
                            self.current_world()->get_block(
                                x,
                                y,
                                z,
                                [&self, x, y, z](base_objects::block& block) {
                                    api::packets::play::worldEvent(*self.assigned_player, base_objects::packets::world_event_id::block_break_and_sound, {(int32_t)x, (int32_t)y, (int32_t)z}, block.id, false);
                                },
                                [&self, x, y, z](base_objects::block& block, auto) {
                                    api::packets::play::worldEvent(*self.assigned_player, base_objects::packets::world_event_id::block_break_and_sound, {(int32_t)x, (int32_t)y, (int32_t)z}, block.id, false);
                                }
                            );
                            api::packets::play::blockUpdate(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, block);
                        }
                    }
            };
            proc.notify_block_entity_change = [](base_objects::entity& self, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block_entity) {
                if (self.assigned_player) {
                    if (self.assigned_player->chunk_loaded_at(x, z)) {
                        api::packets::play::blockUpdate(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, block_entity.block);
                        api::packets::play::blockEntityData(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, block_entity.block, block_entity.data);
                    }
                }
            };
            proc.notify_block_entity_destroy_change = [](base_objects::entity& self, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block_entity) {
                if (self.assigned_player)
                    if (self.current_world()) {
                        if (self.assigned_player->chunk_loaded_at(x, z)) {
                            self.current_world()->get_block(
                                x,
                                y,
                                z,
                                [&self, x, y, z](base_objects::block& block) {
                                    api::packets::play::worldEvent(*self.assigned_player, base_objects::packets::world_event_id::block_break_and_sound, {(int32_t)x, (int32_t)y, (int32_t)z}, block.id, false);
                                },
                                [&self, x, y, z](base_objects::block& block, auto) {
                                    api::packets::play::worldEvent(*self.assigned_player, base_objects::packets::world_event_id::block_break_and_sound, {(int32_t)x, (int32_t)y, (int32_t)z}, block.id, false);
                                }
                            );
                            api::packets::play::blockUpdate(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, block_entity.block);
                            api::packets::play::blockEntityData(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, block_entity.block, block_entity.data);
                        }
                    }
            };
            proc.notify_block_event = [](base_objects::entity& self, const base_objects::world::block_action& action, int64_t x, int64_t y, int64_t z) {
                if (self.assigned_player) {
                    if (self.assigned_player->chunk_loaded_at(x, z)) {
                        std::visit(
                            [x, y, z, &self](auto& it) mutable {
                                using T = std::decay_t<decltype(it)>;
                                if constexpr (std::is_same_v<T, base_objects::world::block_action::noteblock_activated>) {
                                    api::packets::play::blockAction(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, 0, 0, base_objects::block::make_block("minecraft:note_block"));
                                } else if constexpr (std::is_same_v<T, base_objects::world::block_action::piston_extend>) {
                                    api::packets::play::blockAction(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, 0, (uint8_t)it.dir, base_objects::block::make_block("minecraft:piston"));
                                } else if constexpr (std::is_same_v<T, base_objects::world::block_action::piston_retract>) {
                                    api::packets::play::blockAction(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, 1, (uint8_t)it.dir, base_objects::block::make_block("minecraft:piston"));
                                } else if constexpr (std::is_same_v<T, base_objects::world::block_action::piston_canceled>) {
                                    api::packets::play::blockAction(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, 2, (uint8_t)it.dir, base_objects::block::make_block("minecraft:piston"));
                                } else if constexpr (std::is_same_v<T, base_objects::world::block_action::chest_opened>) {
                                    api::packets::play::blockAction(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, 1, it.count, base_objects::block::make_block("minecraft:chest"));
                                } else if constexpr (std::is_same_v<T, base_objects::world::block_action::reset_spawner>) {
                                    api::packets::play::blockAction(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, 1, 0, base_objects::block::make_block("minecraft:spawner"));
                                } else if constexpr (std::is_same_v<T, base_objects::world::block_action::end_gateway_activated>) {
                                    api::packets::play::blockAction(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, 1, 0, base_objects::block::make_block("minecraft:end_gateway"));
                                } else if constexpr (std::is_same_v<T, base_objects::world::block_action::shulker_box_closed>) {
                                    api::packets::play::blockAction(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, 0, 0, base_objects::block::make_block("minecraft:shulker_box"));
                                } else if constexpr (std::is_same_v<T, base_objects::world::block_action::shulker_box_opened>) {
                                    api::packets::play::blockAction(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, 0, 1, base_objects::block::make_block("minecraft:shulker_box"));
                                } else if constexpr (std::is_same_v<T, base_objects::world::block_action::shulker_box_opened_count>) {
                                    api::packets::play::blockAction(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, 1, it.count, base_objects::block::make_block("minecraft:shulker_box"));
                                } else if constexpr (std::is_same_v<T, base_objects::world::block_action::bell_ring>) {
                                    api::packets::play::blockAction(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, 1, (uint8_t)it.dir, base_objects::block::make_block("minecraft:bell"));
                                } else if constexpr (std::is_same_v<T, base_objects::world::block_action::decorated_block_woble>) {
                                    api::packets::play::blockAction(*self.assigned_player, {(int32_t)x, (int32_t)y, (int32_t)z}, 1, !it.successful, base_objects::block::make_block("minecraft:decorated_pot"));
                                }
                            },
                            action.action
                        );
                    }
                }
            };
            proc.notify_chunk = [](base_objects::entity& self, int64_t x, int64_t z, const storage::chunk_data& chunk) {
                if (self.assigned_player) {
                    if (self.assigned_player->chunk_in_bounds(x, z)) {
                    }
                }
            };

            return std::make_shared<base_objects::entity_data::world_processor>(std::move(proc));
        }

    public:
        PlayEngine() {}

        void OnLoad(const PluginRegistrationPtr& self) override {
            register_event(api::protocol::on_chat_command, [this](const auto& event) {
                base_objects::command_context context(event.client_data, true);
                try {
                    api::command::get_manager().execute_command(event.data, context);
                } catch (base_objects::command_exception& ex) {
                    try {
                        std::rethrow_exception(ex.exception);
                    } catch (const std::exception& inner_ex) {
                        std::string error_message = event.data;
                        std::string error_place(event.data.size() + 4, ' ');
                        error_place[0] = '\n';
                        error_place[error_place.size() - 2] = '\n';
                        error_place[error_place.size() - 1] = '\t';
                        if (ex.pos != -1)
                            error_place[ex.pos] = '^';
                        api::players::calls::on_system_message({event.client_data, error_message + error_place + inner_ex.what()});
                    }
                }
                return false;
            });
            base_objects::entity_data::register_entity_world_processor(make_processor(), "minecraft:player");
        }

        void OnUnload(const PluginRegistrationPtr& self) override {}

        void OnCommandsLoadComplete(const std::shared_ptr<PluginRegistration>&, base_objects::command_root_browser& root) override {
            api::players::iterate_online([&manager = root.get_manager()](base_objects::SharedClientData& client) {
                if (!client.is_virtual)
                    client.sendPacket(api::packets::play::commands(client, manager));
                return false;
            });
        }

        plugin_response PlayerJoined(base_objects::client_data_holder& client_ref) override {
            base_objects::network::response response = base_objects::network::response::empty();
            list_array<base_objects::packets::player_actions_add> all_players;
            list_array<base_objects::packets::player_actions_add> new_player;
            new_player.push_back(
                base_objects::packets::player_actions_add{
                    .player_id = client_ref->data->uuid,
                    .name = client_ref->name,
                    .properties = to_list_array(client_ref->data->properties).convert_fn([](auto&& mojang) {
                        return base_objects::packets::player_actions_add::property{
                            .name = std::move(mojang.name),
                            .value = std::move(mojang.value),
                            .signature = std::move(mojang.signature)
                        };
                    })
                }
            );
            api::players::iterate_online([&new_player, &all_players, &client_ref](base_objects::SharedClientData& client) {
                if (&client != &*client_ref && !client.is_virtual) {
                    all_players.push_back(
                        base_objects::packets::player_actions_add{
                            .player_id = client.data->uuid,
                            .name = client.name,
                            .properties = to_list_array(client.data->properties).convert_fn([](auto&& mojang) {
                                return base_objects::packets::player_actions_add::property{
                                    .name = std::move(mojang.name),
                                    .value = std::move(mojang.value),
                                    .signature = std::move(mojang.signature)
                                };
                            })
                        }
                    );
                    client.sendPacket(api::packets::play::playerInfoAdd(client, new_player));
                }
                return false;
            });
            response += api::packets::play::playerInfoAdd(*client_ref, all_players);
            response += api::packets::play::playerInfoAdd(*client_ref, new_player);
            return response;
        }

        plugin_response PlayerLeave(base_objects::client_data_holder& client_ref) override {
            base_objects::network::response response = base_objects::network::response::empty();
            list_array<base_objects::packets::player_actions_add> all_players;
            api::players::iterate_online([&all_players, &client_ref](base_objects::SharedClientData& client) {
                if (&client != &*client_ref && !client.is_virtual) {
                    all_players.push_back(
                        base_objects::packets::player_actions_add{
                            .player_id = client.data->uuid,
                            .name = client.name,
                            .properties = to_list_array(client.data->properties).convert_fn([](auto&& mojang) {
                                return base_objects::packets::player_actions_add::property{
                                    .name = std::move(mojang.name),
                                    .value = std::move(mojang.value),
                                    .signature = std::move(mojang.signature)
                                };
                            })
                        }
                    );
                }
                return false;
            });
            response += api::packets::play::playerInfoAdd(*client_ref, all_players);
            return response;
        }
    };
}
