/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets/client_bound/config.hpp>

#include <src/util/reflect.hpp>
#include <src/util/reflect/api/packets/client_bound/config.hpp>
#include <src/util/reflect/api/packets/types.hpp>

#include <src/util/encoding/packet/generic_auto.hpp>

namespace copper_server::api::packets {
    auto_define_packet_ops(client_bound::config::cookie_request);
    auto_define_packet_ops(client_bound::config::custom_payload);
    auto_define_packet_ops(client_bound::config::disconnect);
    auto_define_packet_ops(client_bound::config::finish_configuration);
    auto_define_packet_ops(client_bound::config::keep_alive);
    auto_define_packet_ops(client_bound::config::ping);
    auto_define_packet_ops(client_bound::config::reset_chat);
    auto_define_packet_ops(client_bound::config::registry_data);
    auto_define_packet_ops(client_bound::config::resource_pack_pop);
    auto_define_packet_ops(client_bound::config::resource_pack_push);
    auto_define_packet_ops(client_bound::config::store_cookie);
    auto_define_packet_ops(client_bound::config::transfer);
    auto_define_packet_ops(client_bound::config::update_enabled_features);
    auto_define_packet_ops(client_bound::config::update_tags);
    auto_define_packet_ops(client_bound::config::select_known_packs);
    auto_define_packet_ops(client_bound::config::custom_report_details);
    auto_define_packet_ops(client_bound::config::server_links);
    auto_define_packet_ops(client_bound::config::clear_dialog);
    auto_define_packet_ops(client_bound::config::show_dialog);
    auto_define_packet_ops(client_bound::config::code_of_conduct);
}