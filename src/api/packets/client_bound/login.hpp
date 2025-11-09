/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_CLIENT_BOUND_LOGIN
#define SRC_API_PACKETS_CLIENT_BOUND_LOGIN

#include <library/enbt/enbt.hpp>
#include <optional>
#include <src/api/packets/ops.hpp>
#include <src/api/packets/types.hpp>

namespace copper_server::api::packets::client_bound::login {
    struct login_disconnect : public packet<0x00>, disconnect_after {
        json_text_component reason;
    };

    struct hello : public packet<0x01> {
        string_sized<20> server_id;
        list_array<uint8_t> public_key;
        list_array<uint8_t> verify_token;
        bool should_authenticate;
    };

    struct login_finished : public packet<0x02> {
        struct property {
            string_sized<64> name;
            string_sized<32767> value;
            std::optional<string_sized<1024>> signature = std::nullopt;
        };

        base_objects::uuid uuid;
        string_sized<16> user_name;
        list_array_sized<property, 16> properties;
    };

    struct login_compression : public packet<0x03> {
        packet_compress<var_int32> threshold;
    };

    struct custom_query : public packet<0x04> {
        var_int32 query_message_id;
        identifier channel;
        list_array_sized_siz_from_packet<uint8_t, 1048576> payload;
    };

    struct cookie_request : public packet<0x05> {
        identifier key;
    };
}

namespace copper_server::api::packets {
    extern template packet_ops<client_bound::login::login_disconnect>;
    extern template packet_ops<client_bound::login::hello>;
    extern template packet_ops<client_bound::login::login_finished>;
    extern template packet_ops<client_bound::login::login_compression>;
    extern template packet_ops<client_bound::login::custom_query>;
    extern template packet_ops<client_bound::login::cookie_request>;

    using client_bound_login_ops = state_ops<
        client_bound::login::login_disconnect,
        client_bound::login::hello,
        client_bound::login::login_finished,
        client_bound::login::login_compression,
        client_bound::login::custom_query,
        client_bound::login::cookie_request>;
}

#endif /* SRC_API_PACKETS_CLIENT_BOUND_LOGIN */
