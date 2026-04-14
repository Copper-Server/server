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
#include <src/util/reflect/api/packets/debug_subscription_type.hpp>
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
}
