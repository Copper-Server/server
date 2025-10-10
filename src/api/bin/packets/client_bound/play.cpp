/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets/client_bound/play.hpp>

#include <src/util/reflect.hpp>
#include <src/util/reflect/api/packets/chat_type.hpp>
#include <src/util/reflect/api/packets/client_bound/play.hpp>
#include <src/util/reflect/api/packets/debug_sub_scription_type.hpp>
#include <src/util/reflect/api/packets/difficulty.hpp>
#include <src/util/reflect/api/packets/gamemode.hpp>
#include <src/util/reflect/api/packets/ops.hpp>
#include <src/util/reflect/api/packets/slot.hpp>
#include <src/util/reflect/api/packets/teleport_flags.hpp>
#include <src/util/reflect/api/packets/types.hpp>
#include <src/util/reflect/base_objects/component.hpp>
#include <src/util/reflect/base_objects/entity/metadata.hpp>
#include <src/util/reflect/base_objects/parsers.hpp>
#include <src/util/reflect/base_objects/particle_data.hpp>
#include <src/util/reflect/base_objects/dye_color.hpp>
#include <src/util/reflect/calculations.hpp>

#include <src/api/bin/packets/generic_auto.hpp>

#include <src/api/configuration.hpp>
#include <src/api/permissions.hpp>
#include <src/api/entity_proxy.hpp>
#include <src/api/registers.hpp>
#include <src/base_objects/block.hpp>
#include <src/base_objects/commands.hpp>
#include <src/storage/world_data.hpp>
#include <src/base_objects/entity.hpp>

namespace copper_server::api::packets {
    auto_define_packet_ops(client_bound::play::bundle_delimiter);
    auto_define_packet_ops(client_bound::play::add_entity);
    auto_define_packet_ops(client_bound::play::animate);
    auto_define_packet_ops(client_bound::play::award_stats);
    auto_define_packet_ops(client_bound::play::block_changed_ack);
    auto_define_packet_ops(client_bound::play::block_destruction);
    auto_define_packet_ops(client_bound::play::block_entity_data);
    auto_define_packet_ops(client_bound::play::block_event);
    auto_define_packet_ops(client_bound::play::block_update);
    auto_define_packet_ops(client_bound::play::boss_event);
    auto_define_packet_ops(client_bound::play::change_difficulty);
    auto_define_packet_ops(client_bound::play::chunk_batch_finished);
    auto_define_packet_ops(client_bound::play::chunk_batch_start);
    auto_define_packet_ops(client_bound::play::chunks_biomes);
    auto_define_packet_ops(client_bound::play::clear_titles);
    auto_define_packet_ops(client_bound::play::command_suggestions);
    auto_define_packet_ops(client_bound::play::commands);
    auto_define_packet_ops(client_bound::play::container_close);
    auto_define_packet_ops(client_bound::play::container_set_content);
    auto_define_packet_ops(client_bound::play::container_set_data);
    auto_define_packet_ops(client_bound::play::container_set_slot);
    auto_define_packet_ops(client_bound::play::cookie_request);
    auto_define_packet_ops(client_bound::play::cooldown);
    auto_define_packet_ops(client_bound::play::custom_chat_completions);
    auto_define_packet_ops(client_bound::play::custom_payload);
    auto_define_packet_ops(client_bound::play::damage_event);
    auto_define_packet_ops(client_bound::play::debug__block_value);
    auto_define_packet_ops(client_bound::play::debug__chunk_value);
    auto_define_packet_ops(client_bound::play::debug__entity_value);
    auto_define_packet_ops(client_bound::play::debug__event);
    auto_define_packet_ops(client_bound::play::debug_sample);
    auto_define_packet_ops(client_bound::play::delete_chat);
    auto_define_packet_ops(client_bound::play::disconnect);
    auto_define_packet_ops(client_bound::play::disguised_chat);
    auto_define_packet_ops(client_bound::play::entity_event);
    auto_define_packet_ops(client_bound::play::entity_position_sync);
    auto_define_packet_ops(client_bound::play::explode);
    auto_define_packet_ops(client_bound::play::forget_level_chunk);
    auto_define_packet_ops(client_bound::play::game_event);
    auto_define_packet_ops(client_bound::play::game_test_highlight_pos);
    auto_define_packet_ops(client_bound::play::horse_screen_open);
    auto_define_packet_ops(client_bound::play::hurt_animation);
    auto_define_packet_ops(client_bound::play::initialize_border);
    auto_define_packet_ops(client_bound::play::keep_alive);
    auto_define_packet_ops(client_bound::play::level_chunk_with_light);
    auto_define_packet_ops(client_bound::play::level_event);
    auto_define_packet_ops(client_bound::play::level_particles);
    auto_define_packet_ops(client_bound::play::light_update);
    auto_define_packet_ops(client_bound::play::login);
    auto_define_packet_ops(client_bound::play::map_item_data);
    auto_define_packet_ops(client_bound::play::merchant_offers);
    auto_define_packet_ops(client_bound::play::move_entity_pos);
    auto_define_packet_ops(client_bound::play::move_entity_pos_rot);
    auto_define_packet_ops(client_bound::play::move_minecart_along_track);
    auto_define_packet_ops(client_bound::play::move_entity_rot);
    auto_define_packet_ops(client_bound::play::move_vehicle);
    auto_define_packet_ops(client_bound::play::open_book);
    auto_define_packet_ops(client_bound::play::open_screen);
    auto_define_packet_ops(client_bound::play::open_sign_editor);
    auto_define_packet_ops(client_bound::play::ping);
    auto_define_packet_ops(client_bound::play::pong_response);
    auto_define_packet_ops(client_bound::play::place_ghost_recipe);
    auto_define_packet_ops(client_bound::play::player_abilities);
    auto_define_packet_ops(client_bound::play::player_chat);
    auto_define_packet_ops(client_bound::play::player_combat_end);
    auto_define_packet_ops(client_bound::play::player_combat_enter);
    auto_define_packet_ops(client_bound::play::player_combat_kill);
    auto_define_packet_ops(client_bound::play::player_info_remove);
    auto_define_packet_ops(client_bound::play::player_info_update);
    auto_define_packet_ops(client_bound::play::player_look_at);
    auto_define_packet_ops(client_bound::play::player_position);
    auto_define_packet_ops(client_bound::play::player_rotation);
    auto_define_packet_ops(client_bound::play::recipe_book_add);
    auto_define_packet_ops(client_bound::play::recipe_book_remove);
    auto_define_packet_ops(client_bound::play::recipe_book_settings);
    auto_define_packet_ops(client_bound::play::remove_entities);
    auto_define_packet_ops(client_bound::play::remove_mob_effect);
    auto_define_packet_ops(client_bound::play::reset_score);
    auto_define_packet_ops(client_bound::play::resource_pack_pop);
    auto_define_packet_ops(client_bound::play::resource_pack_push);
    auto_define_packet_ops(client_bound::play::respawn);
    auto_define_packet_ops(client_bound::play::rotate_head);
    auto_define_packet_ops(client_bound::play::section_blocks_update);
    auto_define_packet_ops(client_bound::play::select_advancements_tab);
    auto_define_packet_ops(client_bound::play::server_data);
    auto_define_packet_ops(client_bound::play::set_action_bar_text);
    auto_define_packet_ops(client_bound::play::set_border_center);
    auto_define_packet_ops(client_bound::play::set_border_lerp_size);
    auto_define_packet_ops(client_bound::play::set_border_size);
    auto_define_packet_ops(client_bound::play::set_border_warning_delay);
    auto_define_packet_ops(client_bound::play::set_border_warning_distance);
    auto_define_packet_ops(client_bound::play::set_camera);
    auto_define_packet_ops(client_bound::play::set_chunk_cache_center);
    auto_define_packet_ops(client_bound::play::set_chunk_cache_radius);
    auto_define_packet_ops(client_bound::play::set_cursor_item);
    auto_define_packet_ops(client_bound::play::set_default_spawn_position);
    auto_define_packet_ops(client_bound::play::set_display_objective);
    auto_define_packet_ops(client_bound::play::set_entity_data);
    auto_define_packet_ops(client_bound::play::set_entity_link);
    auto_define_packet_ops(client_bound::play::set_entity_motion);
    auto_define_packet_ops(client_bound::play::set_equipment);
    auto_define_packet_ops(client_bound::play::set_experience);
    auto_define_packet_ops(client_bound::play::set_health);
    auto_define_packet_ops(client_bound::play::set_held_slot);
    auto_define_packet_ops(client_bound::play::set_objective);
    auto_define_packet_ops(client_bound::play::set_passengers);
    auto_define_packet_ops(client_bound::play::set_player_inventory);
    auto_define_packet_ops(client_bound::play::set_player_team);
    auto_define_packet_ops(client_bound::play::set_score);
    auto_define_packet_ops(client_bound::play::set_simulation_distance);
    auto_define_packet_ops(client_bound::play::set_subtitle_text);
    auto_define_packet_ops(client_bound::play::set_time);
    auto_define_packet_ops(client_bound::play::set_title_text);
    auto_define_packet_ops(client_bound::play::set_titles_animation);
    auto_define_packet_ops(client_bound::play::sound_entity);
    auto_define_packet_ops(client_bound::play::sound);
    auto_define_packet_ops(client_bound::play::start_configuration);
    auto_define_packet_ops(client_bound::play::stop_sound);
    auto_define_packet_ops(client_bound::play::store_cookie);
    auto_define_packet_ops(client_bound::play::system_chat);
    auto_define_packet_ops(client_bound::play::tab_list);
    auto_define_packet_ops(client_bound::play::tag_query);
    auto_define_packet_ops(client_bound::play::take_item_entity);
    auto_define_packet_ops(client_bound::play::teleport_entity);
    auto_define_packet_ops(client_bound::play::test_instance_block_status);
    auto_define_packet_ops(client_bound::play::ticking_state);
    auto_define_packet_ops(client_bound::play::ticking_step);
    auto_define_packet_ops(client_bound::play::transfer);
    auto_define_packet_ops(client_bound::play::update_advancements);
    auto_define_packet_ops(client_bound::play::update_attributes);
    auto_define_packet_ops(client_bound::play::update_mob_effect);
    auto_define_packet_ops(client_bound::play::update_recipes);
    auto_define_packet_ops(client_bound::play::update_tags);
    auto_define_packet_ops(client_bound::play::projectile_power);
    auto_define_packet_ops(client_bound::play::custom_report_details);
    auto_define_packet_ops(client_bound::play::server_links);
    auto_define_packet_ops(client_bound::play::waypoint);
    auto_define_packet_ops(client_bound::play::clear_dialog);
    auto_define_packet_ops(client_bound::play::show_dialog);
}

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

    chunks_biomes chunks_biomes::create(const storage::chunk_data& chunk) {
        chunks_biomes result;
        result.x = (int32_t)chunk.chunk_x;
        result.z = (int32_t)chunk.chunk_z;
        for (auto& section : chunk.sub_chunks) {
            base_objects::palette_container_biome biomes(api::registers::biomes.size());
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
            base_objects::palette_data_height_map data(base_objects::palette_data::bits_for_max(world_height));
            for (uint_fast8_t x = 0; x < 16; x++)
                for (uint_fast8_t z = 0; z < 16; z++)
                    data.add(hei_map[x][z]);
            data.add(0); //TODO check if bug fixed MC-247438, currently at 1.21.8 still not fixed
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
        for (auto& section_ : chunk.sub_chunks) {
            base_objects::palette_container_block blocks(base_objects::block::block_states_size());
            base_objects::palette_container_biome biomes(api::registers::biomes.size());
            blocks.reserve(4096);
            biomes.reserve(64);
            for (int i = 0; i < 4096; ++i) // y = i >> 8; x = (i >> 4) & 15; z = i & 15;
                blocks.add(section_.blocks[(i >> 4) & 15][i >> 8][i & 15].id);

            for (int i = 0; i < 64; ++i) // y = i >> 4; z = (i >> 2) & 3; x = i & 3;
                biomes.add(section_.biomes[i & 3][(i >> 2) & 3][i >> 4]);

            result.sections.value.push_back(section{section_.active_blocks, std::move(blocks), std::move(biomes)});
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
                empty_sky_light_mask.push_back(!section.sky_lighted);
                empty_block_light_mask.push_back(!section.block_lighted);

                if (section.sky_lighted) {
                    auto proxy = reinterpret_cast<const uint8_t*>(section.sky_light.light_map);
                    sky_light.push_back(list_array<uint8_t>(proxy, sizeof(section.sky_light.light_map)));
                }
                if (section.block_lighted) {
                    auto proxy = reinterpret_cast<const uint8_t*>(section.block_light.light_map);
                    block_light.push_back(list_array<uint8_t>(proxy, sizeof(section.block_light.light_map)));
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
        update.sky_light_mask = std::move(sky_light_mask.data());
        update.block_light_mask = std::move(block_light_mask.data());
        update.empty_sky_light_mask = std::move(empty_sky_light_mask.data());
        update.empty_block_light_mask = std::move(empty_block_light_mask.data());
        update.sky_light = convert_light(std::move(sky_light));
        update.block_light = convert_light(std::move(block_light));
        return update;
    }

    set_entity_data set_entity_data::create(base_objects::entity& entity) {
        set_entity_data result;
        result.id = entity.protocol_id;
        api::entity_proxy::iterate_all(entity, [&result](auto id, auto& metadata) {
            result.metadata.push_back({id, metadata});
        });
        return result;
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