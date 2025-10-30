/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS
#define SRC_API_PACKETS
#include <array>
#include <library/enbt/enbt.hpp>
#include <src/api/packets/client_bound/config.hpp>
#include <src/api/packets/client_bound/login.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/api/packets/client_bound/status.hpp>
#include <src/api/packets/ops.hpp>
#include <src/api/packets/server_bound/config.hpp>
#include <src/api/packets/server_bound/handshake.hpp>
#include <src/api/packets/server_bound/login.hpp>
#include <src/api/packets/server_bound/play.hpp>
#include <src/api/packets/server_bound/status.hpp>

//this api allows users to handle clients and simulate them if needed, also supports serialization to string for debug purposes
// note: because this api uses reflection under the hood, recommended to enable build cache to reduce the build time
// the api implements the latest protocol implementation: 773(1.21.10)
namespace copper_server::api::packets {
    using server_bound_ops = direction_ops<
        server_bound_handshake_ops,
        server_bound_status_ops,
        server_bound_login_ops,
        server_bound_config_ops,
        server_bound_play_ops>;

    using client_bound_ops = direction_ops<
        client_bound_status_ops,
        client_bound_login_ops,
        client_bound_config_ops,
        client_bound_play_ops>;

    using global_ops = global_packets_ops<server_bound_ops, client_bound_ops>;

    inline bool decode(base_objects::shared_client_data& context, ArrayStream& stream) {
        bool res = false;
        global_ops::client_decode_direct(context, stream, [&res]<class P>(base_objects::shared_client_data& context, P&& packet) {
            res = packet_ops<P>::make_process(context, std::move(packet));
        });
        return res;
    }

    inline client_bound_status_ops::packet_variants decode_client_status(ArrayStream& stream) {
        return client_bound_status_ops::decode(stream);
    }

    inline client_bound_login_ops::packet_variants decode_client_login(ArrayStream& stream) {
        return client_bound_login_ops::decode(stream);
    }

    inline client_bound_config_ops::packet_variants decode_client_configuration(ArrayStream& stream) {
        return client_bound_config_ops::decode(stream);
    }

    inline client_bound_play_ops::packet_variants decode_client_play(ArrayStream& stream) {
        return client_bound_play_ops::decode(stream);
    }

    inline server_bound_handshake_ops::packet_variants decode_server_handshake(ArrayStream& stream) {
        return server_bound_handshake_ops::decode(stream);
    }

    inline server_bound_status_ops::packet_variants decode_server_status(ArrayStream& stream) {
        return server_bound_status_ops::decode(stream);
    }

    inline server_bound_login_ops::packet_variants decode_server_login(ArrayStream& stream) {
        return server_bound_login_ops::decode(stream);
    }

    inline server_bound_config_ops::packet_variants decode_server_configuration(ArrayStream& stream) {
        return server_bound_config_ops::decode(stream);
    }

    inline server_bound_play_ops::packet_variants decode_server_play(ArrayStream& stream) {
        return server_bound_play_ops::decode(stream);
    }

    inline client_bound_status_ops::packet_variants decode_client_status(base_objects::shared_client_data& context, ArrayStream& stream) {
        return client_bound_status_ops::client_decode(context, stream);
    }

    inline client_bound_login_ops::packet_variants decode_client_login(base_objects::shared_client_data& context, ArrayStream& stream) {
        return client_bound_login_ops::client_decode(context, stream);
    }

    inline client_bound_config_ops::packet_variants decode_client_configuration(base_objects::shared_client_data& context, ArrayStream& stream) {
        return client_bound_config_ops::client_decode(context, stream);
    }

    inline client_bound_play_ops::packet_variants decode_client_play(base_objects::shared_client_data& context, ArrayStream& stream) {
        return client_bound_play_ops::client_decode(context, stream);
    }

    inline server_bound_handshake_ops::packet_variants decode_server_handshake(base_objects::shared_client_data& context, ArrayStream& stream) {
        return server_bound_handshake_ops::client_decode(context, stream);
    }

    inline server_bound_status_ops::packet_variants decode_server_status(base_objects::shared_client_data& context, ArrayStream& stream) {
        return server_bound_status_ops::client_decode(context, stream);
    }

    inline server_bound_login_ops::packet_variants decode_server_login(base_objects::shared_client_data& context, ArrayStream& stream) {
        return server_bound_login_ops::client_decode(context, stream);
    }

    inline server_bound_config_ops::packet_variants decode_server_configuration(base_objects::shared_client_data& context, ArrayStream& stream) {
        return server_bound_config_ops::client_decode(context, stream);
    }

    inline server_bound_play_ops::packet_variants decode_server_play(base_objects::shared_client_data& context, ArrayStream& stream) {
        return server_bound_play_ops::client_decode(context, stream);
    }

    int32_t java_name_to_protocol(const std::string& name_or_number);
    const char* protocol_to_java_name(int32_t id);

    namespace events {
        template <class packet>
        base_objects::events::sync_event<packet&, base_objects::shared_client_data&>& send_viewer() {
            return packet_ops<packet>::send_viewer();
        }

        template <class packet>
        base_objects::events::sync_event_no_cancel<packet&, base_objects::shared_client_data&>& post_send_viewer() {
            return packet_ops<packet>::post_send_viewer();
        }

        template <class packet>
        base_objects::events::sync_event<packet&, base_objects::shared_client_data&>& receive_viewer() {
            return packet_ops<packet>::receive_viewer();
        }

        template <class packet>
        base_objects::events::sync_event_single<packet&&, base_objects::shared_client_data&>& processor() {
            return packet_ops<packet>::processor();
        }
    }
}

#undef decl_variant
#undef STRUCT__
#endif /* SRC_API_PACKETS */
