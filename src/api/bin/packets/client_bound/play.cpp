/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets/client_bound/play.hpp>

#include <src/api/configuration.hpp>
#include <src/api/ecs/base_components.hpp>
#include <src/api/ecs/block_entity_components.hpp>
#include <src/api/ecs/entity_definition.hpp>
#include <src/api/entity.hpp>
#include <src/api/entity_proxy.hpp>
#include <src/api/permissions.hpp>
#include <src/api/registers.hpp>
#include <src/base_objects/block.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/world/chunk.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server::api::packets::client_bound::play {
    commands build_commands(const base_objects::command_manager& manager) {
        commands commanands{.nodes = {}, .root_index = 0};
        auto& command_nodes = manager.get_nodes();
        commanands.nodes.reserve(command_nodes.size());
        size_t i = 0;
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
                if (command.is_named_suggestion()) {
                    if (command.get_named_suggestion().size())
                        node.flags_values.set(commands::node::suggestions_type{.name = command.get_named_suggestion()});
                } else
                    node.flags_values.set(commands::node::suggestions_type{.name = "minecraft:ask_server"});
            }
            if (0 == i++) {
                node.flags_values.set(commands::node::root_node{});
            } else if (command.argument_predicate) {
                node.flags_values.set(commands::node::argument_node{.name = command.name, .type = *command.argument_predicate});
            } else
                node.flags_values.set(commands::node::literal_node{.name = command.name});
            commanands.nodes.push_back(std::move(node));
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

    chunks_biomes chunks_biomes::create(const base_objects::world::chunk_data& chunk) {
        chunks_biomes result;
        result.x = (int32_t)chunk.chunk_x;
        result.z = (int32_t)chunk.chunk_z;
        for (auto& section : chunk.sub_chunks)
            result.sections_of_biomes.value.push_back(section.biomes);

        return result;
    }

    level_chunk_with_light level_chunk_with_light::create(const base_objects::world::chunk_data& chunk, const storage::world_data& world) {
        level_chunk_with_light result;
        static auto build_height_map = [](uint8_t type, const base_objects::palette_data_height_map& hei_map, size_t world_height) {
            base_objects::palette_data_height_map data = hei_map;
            data.add(0); //TODO check if bug fixed MC-247438, currently at 1.21.9 still not fixed
            return height_map{
                .type = height_map::type_e(type),
                .palette_data = std::move(data)
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
        for (auto& section_ : chunk.sub_chunks)
            result.sections.value.push_back(section{section_.active_blocks, section_.blocks, section_.biomes});

        if (api::configuration::get().protocol.send_nbt_data_in_chunk) {
            auto sub_chunk = world.get_world_y_chunk_offset();
            for (auto& section : chunk.sub_chunks) {
                auto sub_chunk_pos = sub_chunk * 16;
                section.for_each_block_entity(
                    [&result, sub_chunk_pos](uint8_t local_x, uint8_t local_y, uint8_t local_z, api::ecs::entity block_e) {
                        std::stringstream ss;
                        util::nbt_write_stream ws(ss);
                        block_e.get<api::ecs::com::type_definition>().type->to_nbt(ws, block_e);
                        auto type = block_e.get<api::ecs::com::block_entity::type>().id;
                        size_t res_size = 0;
                        auto data = util::nbt_convert::readNBT((uint8_t*)ss.view().data(), ss.view().size(), res_size);

                        result.block_entities.push_back(
                            block_entity{
                                .xz = uint8_t((local_x << 4) | local_z),
                                .y = int16_t(sub_chunk_pos + local_y),
                                .type = type,
                                .data = std::move(data)
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

    light_update light_update::create(const base_objects::world::chunk_data& chunk) {
        bit_list_array<uint64_t> sky_light_mask;
        bit_list_array<uint64_t> block_light_mask;
        bit_list_array<uint64_t> empty_sky_light_mask;
        bit_list_array<uint64_t> empty_block_light_mask;
        list_array<list_array_fixed<uint8_t, 2048>> sky_light;
        list_array<list_array_fixed<uint8_t, 2048>> block_light;
        {
            //light below world is unset
            sky_light_mask.push_back(false);
            block_light_mask.push_back(false);
            empty_sky_light_mask.push_back(true);
            empty_block_light_mask.push_back(true);
            for (auto& section : chunk.sub_chunks) {
                sky_light_mask.push_back(section.sky_lighted);
                block_light_mask.push_back(section.block_lighted);
                empty_sky_light_mask.push_back(!section.sky_lighted);
                empty_block_light_mask.push_back(!section.block_lighted);

                if (section.sky_lighted) {
                    auto proxy = reinterpret_cast<const uint8_t*>(section.sky_light.light_map);
                    list_array_fixed<uint8_t, 2048> buf;
                    buf.resize(sizeof(section.sky_light.light_map));
                    memcpy(buf.data(), proxy, sizeof(section.sky_light.light_map));
                    sky_light.push_back(std::move(buf));
                }
                if (section.block_lighted) {
                    auto proxy = reinterpret_cast<const uint8_t*>(section.block_light.light_map);
                    list_array_fixed<uint8_t, 2048> buf;
                    buf.resize(sizeof(section.block_light.light_map));
                    memcpy(buf.data(), proxy, sizeof(section.block_light.light_map));
                    block_light.push_back(std::move(buf));
                }
            }
            //light above world is unset
            sky_light_mask.push_back(false);
            block_light_mask.push_back(false);
            empty_sky_light_mask.push_back(true);
            empty_block_light_mask.push_back(true);
        }

        light_update update;
        update.x = (int32_t)chunk.chunk_x;
        update.z = (int32_t)chunk.chunk_z;
        update.sky_light_mask = std::move(sky_light_mask.data());
        update.block_light_mask = std::move(block_light_mask.data());
        update.empty_sky_light_mask = std::move(empty_sky_light_mask.data());
        update.empty_block_light_mask = std::move(empty_block_light_mask.data());
        update.sky_light = std::move(sky_light);
        update.block_light = std::move(block_light);
        return update;
    }

    bundle_delimiter::bundle_delimiter() {}

    bundle_delimiter::bundle_delimiter(bundle_delimiter&& mov) : packets(std::move(mov.packets)) {}

    bundle_delimiter::bundle_delimiter(const bundle_delimiter& copy) : packets(copy.packets) {}

    bundle_delimiter::bundle_delimiter(list_array<play_packet>&& mov) : packets(std::move(mov)) {}

    bundle_delimiter::bundle_delimiter(const list_array<play_packet>&& copy) : packets(copy) {}

        bundle_delimiter& bundle_delimiter::operator=(bundle_delimiter&& mov){
            packets = std::move(mov.packets);
            return *this;
        }
        bundle_delimiter& bundle_delimiter::operator=(const bundle_delimiter& copy){
            packets = copy.packets;
            return *this;
        }

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