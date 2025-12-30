/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets/server_bound/play.hpp>

#include <src/util/reflect.hpp>
#include <src/util/reflect/api/packets/difficulty.hpp>
#include <src/util/reflect/api/packets/gamemode.hpp>
#include <src/util/reflect/api/packets/server_bound/play.hpp>
#include <src/util/reflect/api/packets/slot.hpp>
#include <src/util/reflect/api/packets/types.hpp>

#include <src/util/encoding/packet/generic_auto.hpp>

namespace copper_server::api::packets {
    auto_define_packet_ops(server_bound::play::accept_teleportation);
    auto_define_packet_ops(server_bound::play::block_entity_tag_query);
    auto_define_packet_ops(server_bound::play::bundle_item_selected);
    auto_define_packet_ops(server_bound::play::change_difficulty);
    auto_define_packet_ops(server_bound::play::change_gamemode);
    auto_define_packet_ops(server_bound::play::chat_ack);
    auto_define_packet_ops(server_bound::play::chat_command);
    auto_define_packet_ops(server_bound::play::chat_command_signed);
    auto_define_packet_ops(server_bound::play::chat);
    auto_define_packet_ops(server_bound::play::chat_session_update);
    auto_define_packet_ops(server_bound::play::chunk_batch_received);
    auto_define_packet_ops(server_bound::play::client_command);
    auto_define_packet_ops(server_bound::play::client_tick_end);
    auto_define_packet_ops(server_bound::play::client_information);
    auto_define_packet_ops(server_bound::play::command_suggestion);
    auto_define_packet_ops(server_bound::play::configuration_acknowledged);
    auto_define_packet_ops(server_bound::play::container_button_click);
    auto_define_packet_ops(server_bound::play::container_click);
    auto_define_packet_ops(server_bound::play::container_close);
    auto_define_packet_ops(server_bound::play::container_slot_state_changed);
    auto_define_packet_ops(server_bound::play::cookie_response);
    auto_define_packet_ops(server_bound::play::custom_payload);
    auto_define_packet_ops(server_bound::play::debug_subscription_request);
    auto_define_packet_ops(server_bound::play::edit_book);
    auto_define_packet_ops(server_bound::play::entity_tag_query);
    auto_define_packet_ops(server_bound::play::interact);
    auto_define_packet_ops(server_bound::play::jigsaw_generate);
    auto_define_packet_ops(server_bound::play::keep_alive);
    auto_define_packet_ops(server_bound::play::lock_difficulty);
    auto_define_packet_ops(server_bound::play::move_player_pos);
    auto_define_packet_ops(server_bound::play::move_player_pos_rot);
    auto_define_packet_ops(server_bound::play::move_player_rot);
    auto_define_packet_ops(server_bound::play::move_player_status_only);
    auto_define_packet_ops(server_bound::play::move_vehicle);
    auto_define_packet_ops(server_bound::play::paddle_boat);
    auto_define_packet_ops(server_bound::play::pick_item_from_block);
    auto_define_packet_ops(server_bound::play::pick_item_from_entity);
    auto_define_packet_ops(server_bound::play::ping_request);
    auto_define_packet_ops(server_bound::play::place_recipe);
    auto_define_packet_ops(server_bound::play::player_abilities);
    auto_define_packet_ops(server_bound::play::player_action);
    auto_define_packet_ops(server_bound::play::player_command);
    auto_define_packet_ops(server_bound::play::player_input);
    auto_define_packet_ops(server_bound::play::player_loaded);
    auto_define_packet_ops(server_bound::play::pong);
    auto_define_packet_ops(server_bound::play::recipe_book_change_settings);
    auto_define_packet_ops(server_bound::play::recipe_book_seen_recipe);
    auto_define_packet_ops(server_bound::play::rename_item);
    auto_define_packet_ops(server_bound::play::resource_pack);
    auto_define_packet_ops(server_bound::play::seen_advancements);
    auto_define_packet_ops(server_bound::play::select_trade);
    auto_define_packet_ops(server_bound::play::set_beacon);
    auto_define_packet_ops(server_bound::play::set_carried_item);
    auto_define_packet_ops(server_bound::play::set_command_block);
    auto_define_packet_ops(server_bound::play::set_command_minecart);
    auto_define_packet_ops(server_bound::play::set_creative_mode_slot);
    auto_define_packet_ops(server_bound::play::set_jigsaw_block);
    auto_define_packet_ops(server_bound::play::set_structure_block);
    auto_define_packet_ops(server_bound::play::set_test_block);
    auto_define_packet_ops(server_bound::play::sign_update);
    auto_define_packet_ops(server_bound::play::swing);
    auto_define_packet_ops(server_bound::play::teleport_to_entity);
    auto_define_packet_ops(server_bound::play::test_instance_block_action);
    auto_define_packet_ops(server_bound::play::use_item_on);
    auto_define_packet_ops(server_bound::play::use_item);
    auto_define_packet_ops(server_bound::play::custom_click_action);
}