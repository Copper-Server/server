/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_SERVER_BOUND_HANDSHAKE
#define SRC_API_PACKETS_SERVER_BOUND_HANDSHAKE

#include <src/api/packets/ops.hpp>
#include <src/api/packets/types.hpp>

namespace copper_server::api::packets::server_bound::handshake {
    struct intention : public packet<0x00> {
        enum class intent_e : uint8_t {
            status = 1,
            login = 2,
            transfer = 3
        };
        var_int32 protocol_version;
        string_sized<255> server_address;
        uint16_t server_port;
        enum_as<intent_e, var_int32> intent;
    };
}

namespace copper_server::api::packets {
    extern template packet_ops<server_bound::handshake::intention>;
    using server_bound_handshake_ops = state_ops<server_bound::handshake::intention>;
}

#endif /* SRC_API_PACKETS_SERVER_BOUND_HANDSHAKE */
