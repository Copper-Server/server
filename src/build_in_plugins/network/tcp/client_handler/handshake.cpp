/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets/server_bound/handshake.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::network::tcp {
    struct tcp_handshake : public plugin_auto_register<"network/tcp_handshake", tcp_handshake> {
        void on_register(const plugin_registration_ptr&) override {
            using intention = api::packets::server_bound::handshake::intention;
            api::packets::processor(*this, [](intention&& packet, base_objects::shared_client_data& client) {
                client.packets_state.protocol_version = packet.protocol_version;
                switch (packet.intent.value) {
                case intention::intent_e::status:
                    client << api::packets::switches_to::status{};
                    break;
                case intention::intent_e::transfer:
                    client.packets_state.is_transferred = true;
                //[[fallthrough]]
                case intention::intent_e::login:
                    client << api::packets::switches_to::login{};
                    break;
                }
            });
        }
    };
}