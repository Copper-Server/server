/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */ 

#include<src/api/packets/client_bound/play.hpp>

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

#include <src/util/encoding/packet/generic_auto.hpp>

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
}
