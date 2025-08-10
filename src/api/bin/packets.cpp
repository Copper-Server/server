/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/configuration.hpp>
#include <src/api/packets.hpp>
#include <src/api/permissions.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/base_objects/slot.hpp>
#include <src/registers.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server {
    namespace api::packets {
        bool debugging_enabled = false;

        namespace __internal {
            std::unordered_map<size_t, base_objects::events::sync_event<client_bound_packet&, base_objects::SharedClientData&>> client_viewers[4];
            std::unordered_map<size_t, base_objects::events::sync_event<client_bound_packet&, base_objects::SharedClientData&>> client_post_send_viewers[4];
            std::unordered_map<size_t, base_objects::events::sync_event<server_bound_packet&, base_objects::SharedClientData&>> server_viewers[5];

            base_objects::events::event_register_id register_client_viewer(uint8_t mode, size_t id, base_objects::events::sync_event<client_bound_packet&, base_objects::SharedClientData&>::function&& fn) {
                return client_viewers[mode][id].join(std::move(fn));
            }

            base_objects::events::event_register_id register_server_viewer(uint8_t mode, size_t id, base_objects::events::sync_event<server_bound_packet&, base_objects::SharedClientData&>::function&& fn) {
                return server_viewers[mode][id].join(std::move(fn));
            }

            base_objects::events::event_register_id register_viewer_post_send_client_bound(uint8_t mode, size_t id, std::function<void(client_bound_packet&, base_objects::SharedClientData&)>&& fn) {
                return client_post_send_viewers[mode][id].join([fn = std::move(fn)](auto& it, auto& cl) { fn(it, cl); return false; });
            }

            void unregister_client_viewer(uint8_t mode, size_t id, base_objects::events::event_register_id reg_id) {
                client_viewers[mode][id].leave(reg_id);
            }

            void unregister_server_viewer(uint8_t mode, size_t id, base_objects::events::event_register_id reg_id) {
                server_viewers[mode][id].leave(reg_id);
            }

            void unregister_viewer_post_send_client_bound(uint8_t mode, size_t id, base_objects::events::event_register_id reg_id) {
                client_post_send_viewers[mode][id].leave(reg_id);
            }

            bool visit_packet_viewer(client_bound_packet& packet, base_objects::SharedClientData& context) {
                return std::visit(
                    [&](auto& it) {
                        using T = std::decay_t<decltype(it)>;
                        return std::visit(
                            [&](auto& pack) {
                                using pack_T = std::decay_t<decltype(pack)>;
                                if constexpr (base_objects::is_packet<pack_T>) {
                                    if constexpr (std::is_same_v<T, client_bound::status_packet>) {
                                        return client_viewers[0][pack_T::packet_id::value].notify(packet, context);
                                    } else if constexpr (std::is_same_v<T, client_bound::login_packet>) {
                                        return client_viewers[1][pack_T::packet_id::value].notify(packet, context);
                                    } else if constexpr (std::is_same_v<T, client_bound::configuration_packet>) {
                                        return client_viewers[2][pack_T::packet_id::value].notify(packet, context);
                                    } else
                                        return client_viewers[3][pack_T::packet_id::value].notify(packet, context);
                                } else
                                    return false;
                            },
                            it
                        );
                    },
                    packet
                );
            }

            bool visit_packet_viewer(server_bound_packet& packet, base_objects::SharedClientData& context) {
                return std::visit(
                    [&](auto& it) {
                        using T = std::decay_t<decltype(it)>;
                        return std::visit(
                            [&](auto& pack) {
                                using pack_T = std::decay_t<decltype(pack)>;
                                if constexpr (base_objects::is_packet<pack_T>) {
                                    if constexpr (std::is_same_v<T, server_bound::handshake_packet>) {
                                        return server_viewers[0][pack_T::packet_id::value].notify(packet, context);
                                    } else if constexpr (std::is_same_v<T, server_bound::status_packet>) {
                                        return server_viewers[1][pack_T::packet_id::value].notify(packet, context);
                                    } else if constexpr (std::is_same_v<T, server_bound::login_packet>) {
                                        return server_viewers[2][pack_T::packet_id::value].notify(packet, context);
                                    } else if constexpr (std::is_same_v<T, server_bound::configuration_packet>) {
                                        return server_viewers[3][pack_T::packet_id::value].notify(packet, context);
                                    } else
                                        return server_viewers[4][pack_T::packet_id::value].notify(packet, context);
                                } else
                                    return false;
                            },
                            it
                        );
                    },
                    packet
                );
            }

            void visit_packet_post_send_viewer(client_bound_packet& packet, base_objects::SharedClientData& context) {
                std::visit(
                    [&](auto& it) {
                        using T = std::decay_t<decltype(it)>;
                        std::visit(
                            [&](auto& pack) {
                                using pack_T = std::decay_t<decltype(pack)>;
                                if constexpr (base_objects::is_packet<pack_T>) {
                                    if constexpr (std::is_same_v<T, client_bound::status_packet>) {
                                        client_post_send_viewers[0][pack_T::packet_id::value].notify(packet, context);
                                    } else if constexpr (std::is_same_v<T, client_bound::login_packet>) {
                                        client_post_send_viewers[1][pack_T::packet_id::value].notify(packet, context);
                                    } else if constexpr (std::is_same_v<T, client_bound::configuration_packet>) {
                                        client_post_send_viewers[2][pack_T::packet_id::value].notify(packet, context);
                                    } else
                                        client_post_send_viewers[3][pack_T::packet_id::value].notify(packet, context);
                                }
                            },
                            it
                        );
                    },
                    packet
                );
            }
        }

        slot slot::create(const base_objects::slot& value) {
            return value.to_packet();
        }

        namespace client_bound {
            namespace play {
                commands build_commands(const base_objects::command_manager& manager) {
                    commands commanands{.root_index = 0};
                    bool is_root = true;
                    auto& command_nodes = manager.get_nodes();
                    commanands.nodes.reserve(command_nodes.size());
                    for (auto& command : command_nodes) {
                        commands::node node;
                        node.children = command.childs.convert<var_int32>();
                        if (command.redirect)
                            node.flags_values.set(commands::node::redirect_node{.node = command.redirect->target_command});
                        if (command.executable)
                            node.flags_values.set(commands::node::is_executable{});
                        if (api::permissions::has_action_limits(command.action_name))
                            node.flags_values.set(commands::node::is_restricted{});
                        if (command.has_suggestion()) {
                            node.flags_values.set(
                                commands::node::suggestions_type{
                                    .name = command.is_named_suggestion()
                                                ? command.get_named_suggestion()
                                                : "minecraft:ask_server"
                                }
                            );
                        }
                        if (is_root) {
                            node.flags_values.set(commands::node::root_node{});
                            is_root = false;
                        } else if (command.argument_predicate) {
                            //TODO
                        } else
                            node.flags_values.set(commands::node::literal_node{.name = command.name});
                    }
                    return commanands;
                }

                commands commands::create(const base_objects::command_manager& manager) {
                    static commands res;
                    static size_t changes_id = size_t(-1);
                    if (auto current_changes_id = manager.get_changes_id(); changes_id != current_changes_id) {
                        res = build_commands(manager);
                        changes_id = current_changes_id;
                    }
                    return res;
                }

                chunks_biomes chunks_biomes::create(const storage::chunk_data& chunk) {
                    chunks_biomes result;
                    result.x = (int32_t)chunk.chunk_x;
                    result.z = (int32_t)chunk.chunk_z;
                    for (auto& section : chunk.sub_chunks) {
                        base_objects::pallete_container_biome biomes(registers::biomes.size());
                        for (auto& x : section.biomes)
                            for (auto& y : x)
                                for (auto& z : y)
                                    biomes.add(z);
                        result.sections_of_biomes.value.push_back(std::move(biomes));
                    }
                    return result;
                }

                level_chunk_with_light level_chunk_with_light::create(const storage::chunk_data& chunk, const storage::world_data& world) {
                    level_chunk_with_light result;
                    static auto build_height_map = [](uint8_t type, const uint64_t (&hei_map)[16][16], size_t world_height) {
                        base_objects::pallete_data_height_map data(base_objects::pallete_data::bits_for_max(world_height));
                        for (uint_fast8_t x = 0; x < 16; x++)
                            for (uint_fast8_t z = 0; z < 16; z++)
                                data.add(hei_map[x][z]);
                        return height_map{
                            .type = height_map::type_e(type),
                            .pallete_data = std::move(data)
                        };
                    };
                    size_t world_height = chunk.sub_chunks.size() * 16;
                    result.height_maps = {
                        build_height_map(1, chunk.height_maps.surface, world_height),
                        build_height_map(3, chunk.height_maps.ocean_floor, world_height),
                        build_height_map(4, chunk.height_maps.motion_blocking, world_height),
                        build_height_map(5, chunk.height_maps.motion_blocking_no_leaves, world_height)
                    };


                    result.sections.value.reserve(chunk.sub_chunks.size());
                    for (auto& section_ : chunk.sub_chunks) {
                        uint16_t block_count = 0;
                        base_objects::pallete_container_block blocks(base_objects::block::block_states_size());
                        base_objects::pallete_container_biome biomes(registers::biomes.size());
                        for (auto& x : section_.blocks)
                            for (auto& y : x)
                                for (auto z : y) {
                                    block_count += !z.is_air();
                                    blocks.add(z.id);
                                }
                        for (auto& x : section_.biomes)
                            for (auto& y : x)
                                for (auto& z : y)
                                    biomes.add(z);
                        result.sections.value.push_back(section{block_count, std::move(blocks), std::move(biomes)});
                    }
                    if (api::configuration::get().protocol.send_nbt_data_in_chunk) {
                        auto sub_chunk = world.get_world_y_chunk_offset();
                        for (auto& section : chunk.sub_chunks) {
                            auto sub_chunk_pos = sub_chunk * 16;
                            section.for_each_block_entity(
                                [&result, sub_chunk_pos](uint8_t local_x, uint8_t local_y, uint8_t local_z, base_objects::block block, const enbt::value& entity_data) {
                                    result.block_entities.push_back(
                                        block_entity{
                                            .xz = uint8_t((local_x << 4) | local_z),
                                            .y = int16_t(sub_chunk_pos + local_y),
                                            .type = block.block_entity_id(),
                                            .data = entity_data
                                        }
                                    );
                                }
                            );
                            ++sub_chunk;
                        }
                    }

                    auto [x, z, sky_light_mask, block_light_mask, empty_sky_light_mask, empty_block_light_mask, sky_light, block_light] = light_update::create(chunk);
                    result.x = x;
                    result.z = z;
                    result.sky_light_mask = std::move(sky_light_mask);
                    result.block_light_mask = std::move(block_light_mask);
                    result.empty_sky_light_mask = std::move(empty_sky_light_mask);
                    result.empty_block_light_mask = std::move(empty_block_light_mask);
                    result.sky_light = std::move(sky_light);
                    result.block_light = std::move(block_light);
                    return result;
                }

                light_update light_update::create(const storage::chunk_data& chunk) {
                    bit_list_array<uint64_t> sky_light_mask;
                    bit_list_array<uint64_t> block_light_mask;
                    bit_list_array<uint64_t> empty_sky_light_mask;
                    bit_list_array<uint64_t> empty_block_light_mask;
                    list_array<list_array<uint8_t>> sky_light;
                    list_array<list_array<uint8_t>> block_light;
                    {
                        //light below world is unset
                        sky_light_mask.push_back(false);
                        block_light_mask.push_back(false);
                        empty_sky_light_mask.push_back(true);
                        empty_block_light_mask.push_back(true);
                        for (auto& section : chunk.sub_chunks) {
                            sky_light_mask.push_back(section.sky_lighted);
                            block_light_mask.push_back(section.block_lighted);
                            empty_sky_light_mask.push_back(section.sky_lighted);
                            empty_block_light_mask.push_back(section.block_lighted);

                            if (section.sky_lighted) {
                                bit_list_array<> section_sky_light;
                                for (auto& x : section.sky_light.light_map)
                                    for (auto& y : x)
                                        for (auto z : y) {
                                            section_sky_light.push_back(z.light_point & 1);
                                            section_sky_light.push_back(z.light_point & 2);
                                            section_sky_light.push_back(z.light_point & 4);
                                            section_sky_light.push_back(z.light_point & 8);
                                        }
                                sky_light.push_back(section_sky_light.take());
                            }
                            if (section.block_lighted) {
                                bit_list_array<> section_block_light;
                                for (auto& x : section.block_light.light_map)
                                    for (auto& y : x)
                                        for (auto z : y) {
                                            section_block_light.push_back(z.light_point & 1);
                                            section_block_light.push_back(z.light_point & 2);
                                            section_block_light.push_back(z.light_point & 4);
                                            section_block_light.push_back(z.light_point & 8);
                                        }
                                block_light.push_back(section_block_light.take());
                            }
                        }
                        //light above world is unset
                        sky_light_mask.push_back(false);
                        block_light_mask.push_back(false);
                        empty_sky_light_mask.push_back(true);
                        empty_block_light_mask.push_back(true);
                    }


                    static auto convert_light = [](list_array<list_array<uint8_t>>&& arr) {
                        return arr
                            .take()
                            .convert_fn(
                                [](list_array<uint8_t>&& it) {
                                    return it.to_container<list_array_fixed<uint8_t, 2048>>();
                                }
                            );
                    };

                    light_update update;
                    update.x = (int32_t)chunk.chunk_x;
                    update.z = (int32_t)chunk.chunk_z;
                    update.sky_light_mask = sky_light_mask.data();
                    update.block_light_mask = block_light_mask.data();
                    update.empty_sky_light_mask = empty_sky_light_mask.data();
                    update.empty_block_light_mask = empty_block_light_mask.data();
                    update.sky_light = convert_light(std::move(sky_light));
                    update.block_light = convert_light(std::move(block_light));
                    return update;
                }

                bundle_delimiter::bundle_delimiter() {}

                bundle_delimiter::bundle_delimiter(bundle_delimiter&& mov) : packets(std::move(mov.packets)) {}

                bundle_delimiter::bundle_delimiter(const bundle_delimiter& copy) : packets(copy.packets) {}

                bundle_delimiter::bundle_delimiter(list_array<play_packet>&& mov) : packets(std::move(mov)) {}

                bundle_delimiter::bundle_delimiter(const list_array<play_packet>&& copy) : packets(copy) {}

                uint64_t section_blocks_update::position_t::to_packet() const {
                    union {
                        uint64_t r;
                        position_t v;
                    } tmp;

                    tmp.v = *this;
                    return tmp.r;
                }

                section_blocks_update::position_t section_blocks_update::position_t::from_packet(uint64_t value) {
                    union {
                        uint64_t v;
                        position_t r;
                    } tmp;

                    tmp.v = value;
                    return tmp.r;
                }

                var_int64 section_blocks_update::block_entry::to_packet() const {
                    union {
                        int64_t r;
                        block_entry v;
                    } tmp;

                    tmp.v = *this;
                    return tmp.r;
                }

                section_blocks_update::block_entry section_blocks_update::block_entry::from_packet(var_int64 value) {
                    union {
                        int64_t v;
                        block_entry r;
                    } tmp;

                    tmp.v = value;
                    return tmp.r;
                }

            }
        }

        base_objects::events::sync_event<base_objects::SharedClientData&> client_state_changed;

        void set_debug_mode(bool enabled) {
            debugging_enabled = enabled;
        }
    }

}

copper_server::base_objects::SharedClientData& operator<<(copper_server::base_objects::SharedClientData& client, copper_server::base_objects::switches_to::status) {
    client.packets_state.state = copper_server::base_objects::SharedClientData::packets_state_t::protocol_state::status;
    client.packets_state.extra_data = nullptr;
    copper_server::api::packets::client_state_changed(client);
    return client;
}

copper_server::base_objects::SharedClientData& operator<<(copper_server::base_objects::SharedClientData& client, copper_server::base_objects::switches_to::login) {
    client.packets_state.state = copper_server::base_objects::SharedClientData::packets_state_t::protocol_state::login;
    client.packets_state.extra_data = nullptr;
    copper_server::api::packets::client_state_changed(client);
    return client;
}

copper_server::base_objects::SharedClientData& operator<<(copper_server::base_objects::SharedClientData& client, copper_server::base_objects::switches_to::configuration) {
    client.packets_state.state = copper_server::base_objects::SharedClientData::packets_state_t::protocol_state::configuration;
    copper_server::api::players::login_complete_to_cfg(client);
    client.packets_state.extra_data = nullptr;
    copper_server::api::packets::client_state_changed(client);
    return client;
}

copper_server::base_objects::SharedClientData& operator<<(copper_server::base_objects::SharedClientData& client, copper_server::base_objects::switches_to::play) {
    client.packets_state.state = copper_server::base_objects::SharedClientData::packets_state_t::protocol_state::play;
    client.packets_state.extra_data = nullptr;
    copper_server::api::packets::client_state_changed(client);
    return client;
}