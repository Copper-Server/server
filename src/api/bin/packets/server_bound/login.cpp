/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets/server_bound/login.hpp>

#include <src/util/reflect.hpp>
#include <src/util/reflect/api/packets/server_bound/login.hpp>
#include <src/util/reflect/api/packets/types.hpp>

#include <src/util/encoding/packet/generic_auto.hpp>

namespace copper_server::api::packets {
    auto_define_packet_ops(server_bound::login::hello);
    auto_define_packet_ops(server_bound::login::key);
    auto_define_packet_ops(server_bound::login::custom_query_answer);
    auto_define_packet_ops(server_bound::login::login_acknowledged);
    auto_define_packet_ops(server_bound::login::cookie_response);
}