/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets/ops.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/util/readers.hpp>

namespace copper_server::api::packets {
    namespace __internal {
        current_state get_state(base_objects::SharedClientData& client) {
            switch (client.packets_state.state) {
            case base_objects::SharedClientData::packets_state_t::protocol_state::handshake:
                return current_state::handshake;
            case base_objects::SharedClientData::packets_state_t::protocol_state::status:
                return current_state::status;
            case base_objects::SharedClientData::packets_state_t::protocol_state::login:
                return current_state::login;
            case base_objects::SharedClientData::packets_state_t::protocol_state::configuration:
                return current_state::configuration;
            case base_objects::SharedClientData::packets_state_t::protocol_state::play:
                return current_state::play;
            default:
                return current_state::handshake;
            }
        }

        size_t get_packet_id(ArrayStream& stream) {
            return stream.read_var<int32_t>();
        }
    }
}