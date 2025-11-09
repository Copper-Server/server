/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_SERVER_BOUND_LOGIN
#define SRC_API_PACKETS_SERVER_BOUND_LOGIN

#include <library/enbt/enbt.hpp>
#include <optional>
#include <src/api/packets/ops.hpp>
#include <src/api/packets/types.hpp>

namespace copper_server::api::packets::server_bound::login {
    struct hello : public packet<0x00> {
        string_sized<16> name;
        base_objects::uuid uuid;
    };

    struct key : public packet<0x01> {
        list_array<uint8_t> shared_secret;
        list_array<uint8_t> verify_token;
    };

    struct custom_query_answer : public packet<0x02> {
        var_int32 query_message_id;
        list_array_sized_siz_from_packet<uint8_t, 32767> payload;
    };

    struct login_acknowledged : public packet<0x03>, switches_to::config {};

    struct cookie_response : public packet<0x04> {
        identifier key;
        std::optional<list_array_sized<uint8_t, 5120>> payload = std::nullopt;
    };
}

namespace copper_server::api::packets {
    extern template packet_ops<server_bound::login::hello>;
    extern template packet_ops<server_bound::login::key>;
    extern template packet_ops<server_bound::login::custom_query_answer>;
    extern template packet_ops<server_bound::login::login_acknowledged>;
    extern template packet_ops<server_bound::login::cookie_response>;
    using server_bound_login_ops = state_ops<
        server_bound::login::hello,
        server_bound::login::key,
        server_bound::login::custom_query_answer,
        server_bound::login::login_acknowledged,
        server_bound::login::cookie_response>;
}

#endif /* SRC_API_PACKETS_SERVER_BOUND_LOGIN */
