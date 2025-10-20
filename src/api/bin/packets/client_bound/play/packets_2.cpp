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
#include <src/util/reflect/base_objects/dye_color.hpp>
#include <src/util/reflect/base_objects/entity/metadata.hpp>
#include <src/util/reflect/base_objects/parsers.hpp>
#include <src/util/reflect/base_objects/particle_data.hpp>
#include <src/util/reflect/calculations.hpp>

#include <src/api/bin/packets/generic_auto.hpp>

#include <src/api/configuration.hpp>
#include <src/api/ecs/base_components.hpp>
#include <src/api/entity.hpp>
#include <src/api/entity_proxy.hpp>
#include <src/api/permissions.hpp>
#include <src/api/registers.hpp>
#include <src/base_objects/block.hpp>
#include <src/base_objects/commands.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server::api::packets {
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
