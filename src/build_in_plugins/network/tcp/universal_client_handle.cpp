/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/build_in_plugins/network/tcp/universal_client_handle.hpp>

namespace copper_server::build_in_plugins::network::tcp {
    std::string packet_is_to_large_str = Chat("Packet too large").ToStr();
    auto packet_is_to_large = Chat("Packet too large");
    std::string internal_error_str = Chat("Internal server error\nPlease report this to the server owner!").ToStr();
    auto internal_error = Chat("Internal server error\nPlease report this to the server owner!");

    

    base_objects::network::response universal_client_handle::work_packet(ArrayStream& data) {
        api::packets::decode(session->shared_data(), data);
        return {};
    }

    base_objects::network::response universal_client_handle::too_large_packet() {
        switch (session->shared_data().packets_state.state) {
            using enum base_objects::SharedClientData::packets_state_t::protocol_state;
        case handshake:
        case status:
        default:
            return base_objects::network::response::disconnect();
        case login:
            return api::packets::encode(api::packets::client_bound::login::login_disconnect{.reason = {packet_is_to_large_str}});
        case configuration:
            return api::packets::encode(api::packets::client_bound::configuration::disconnect{.reason = packet_is_to_large});
        case play:
            return api::packets::encode(api::packets::client_bound::play::disconnect{.reason = packet_is_to_large});
        }
    }

    base_objects::network::response universal_client_handle::exception(const std::exception& ex) {
        switch (session->shared_data().packets_state.state) {
            using enum base_objects::SharedClientData::packets_state_t::protocol_state;
        case handshake:
        case status:
        default:
            return base_objects::network::response::disconnect();
        case login:
            return api::packets::encode(api::packets::client_bound::login::login_disconnect{.reason = {Chat("Internal server error: " + std::string(ex.what()) + "\nPlease report this to the server owner!").ToStr()}});
        case configuration:
            return api::packets::encode(api::packets::client_bound::configuration::disconnect{.reason = "Internal server error: " + std::string(ex.what()) + "\nPlease report this to the server owner!"});
        case play:
            return api::packets::encode(api::packets::client_bound::play::disconnect{.reason = "Internal server error: " + std::string(ex.what()) + "\nPlease report this to the server owner!"});
        }
    }

    base_objects::network::response universal_client_handle::unexpected_exception() {
        switch (session->shared_data().packets_state.state) {
            using enum base_objects::SharedClientData::packets_state_t::protocol_state;
        case handshake:
        case status:
        default:
            return base_objects::network::response::disconnect();
        case login:
            return api::packets::encode(api::packets::client_bound::login::login_disconnect{.reason = {internal_error_str}});
        case configuration:
            return api::packets::encode(api::packets::client_bound::configuration::disconnect{.reason = internal_error});
        case play:
            return api::packets::encode(api::packets::client_bound::play::disconnect{.reason = internal_error});
        }
    }

    universal_client_handle::universal_client_handle(api::network::tcp::session* sock)
        : tcp_client_handle(sock) {}

    universal_client_handle::universal_client_handle()
        : tcp_client_handle(nullptr) {}

    base_objects::network::tcp::client* universal_client_handle::define_ourself(api::network::tcp::session* sock) {
        return new universal_client_handle(sock);
    }

    base_objects::network::tcp::client* universal_client_handle::redefine_handler() {
        return next_handler;
    }
}