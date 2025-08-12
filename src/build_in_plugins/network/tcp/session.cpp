/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/players.hpp>
#include <src/base_objects/network/tcp/client.hpp>
#include <src/build_in_plugins/network/tcp/session.hpp>
#include <src/build_in_plugins/network/tcp/util.hpp>
#include <src/log.hpp>

namespace copper_server::build_in_plugins::network::tcp {
    using base_objects::network::tcp::client;

    constexpr bool CONSTEXPR_DEBUG_DATA_TRANSPORT = true;
    std::atomic_uint64_t id_gen(0);
    bool session::do_log_connection_errors = true;

    session::session(fast_task::networking::TcpNetworkStream& s, client* client_handler, float& set_timeout)
        : api::network::tcp::session(id_gen++), stream(&s), timeout(set_timeout) {
        chandler = client_handler->define_ourself(this);
        read_data.resize(1024);
    }

    session::~session() noexcept {
        if (chandler)
            delete chandler;
        if (_sharedData) {
            try {
                api::players::handlers::on_disconnect.await_notify(shared_data_ref());
            } catch (...) {
            }
            api::players::remove_player(shared_data_ref());
        }
    }

    base_objects::client_data_holder& session::shared_data_ref() {
        return _sharedData ? _sharedData : _sharedData = api::players::allocate_player(this);
    }

    base_objects::SharedClientData& session::shared_data() {
        return *shared_data_ref();
    }

    void session::disconnect() {
        std::lock_guard guard(tc);
        if (stream) {
            stream->close();
            stream = nullptr;
        }
    }

    bool session::is_active() {
        return stream;
    }

    bool session::start_symmetric_encryption(const list_array<uint8_t>& encryption_key, const list_array<uint8_t>& encryption_iv) {
        if (!encryption.initialize(encryption_key, encryption_iv))
            return false;
        encryption_enabled = true;
        return true;
    }

    void session::request_buffer(size_t new_size) {
        std::lock_guard guard(tc);
        if (stream) {
            if (INT32_MAX < new_size)
                new_size = INT32_MAX;
            stream->rebuffer(static_cast<int32_t>(new_size));
        }
    }

    void session::send_indirect(base_objects::network::response&& resp) {
        if ((resp.do_disconnect || resp.do_disconnect_after_send) && resp.data.size())
            send(base_objects::network::response::disconnect(tcp_client_handle::prepare_send(std::move(resp), this)));
        if (resp.do_disconnect)
            send(base_objects::network::response::disconnect());
        send(base_objects::network::response::answer(tcp_client_handle::prepare_send(std::move(resp), this)));
    }

    void session::send(base_objects::network::response&& resp) {
        //<for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
        if constexpr (CONSTEXPR_DEBUG_DATA_TRANSPORT)
            for (auto& it : resp.data)
                client::log_console("S (" + std::to_string(id) + ")", it.data, it.data.size());
        //</for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
        if (resp.do_disconnect)
            disconnect();
        else if (resp.data.size()) {
            list_array<uint8_t> response_data;
            for (auto& it : resp.data)
                response_data.push_back(std::move(it.data));


            std::lock_guard guard(tc);
            if (encryption_enabled)
                encryption.encrypt(response_data, response_data);
            if (stream) {
                std::shared_ptr<std::vector<uint8_t>> send_data = std::make_shared<std::vector<uint8_t>>(response_data.begin(), response_data.end());
                stream->write((char*)response_data.data(), response_data.size());
                if (resp.do_disconnect_after_send) {
                    stream->force_write();
                    stream->close();
                }
            }
        } else if (resp.do_disconnect_after_send)
            disconnect();
    }

    void session::received(std::span<char> readed_data) {
        list_array<uint8_t> convert_data((uint8_t*)readed_data.data(), readed_data.size());
        //<for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
        if constexpr (CONSTEXPR_DEBUG_DATA_TRANSPORT)
            client::log_console("P (" + std::to_string(id) + ")", convert_data, convert_data.size());
        //</for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
        if (encryption_enabled) {
            encryption.decrypt(convert_data, convert_data);
            //<for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
            if constexpr (CONSTEXPR_DEBUG_DATA_TRANSPORT)
                client::log_console("PD (" + std::to_string(id) + ")", convert_data, convert_data.size());
            //</for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
            read_data_cached.push_back(std::move(convert_data));
        } else
            read_data_cached.push_back(list_array<uint8_t>(convert_data.data(), convert_data.size()));

        send(proceed_data());
    }

    base_objects::network::response session::proceed_data() {
        while (true) {
            base_objects::network::response tmp(chandler->work_client(read_data_cached));
            read_data_cached.erase(0, tmp.valid_till);
            if (auto redefHandler = chandler->redefine_handler(); redefHandler && !tmp.is_disconnect()) {
                tmp.valid_till = 0;
                delete chandler;
                chandler = redefHandler;
                tmp = chandler->on_switch();
                if (tmp.data.empty() && !tmp.do_disconnect && !tmp.do_disconnect_after_send)
                    continue;
            }
            return tmp;
        }
    }

    client& session::handler() {
        return *chandler;
    }
}