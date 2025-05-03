#include <src/api/players.hpp>
#include <src/base_objects/network/tcp/client.hpp>
#include <src/base_objects/network/tcp/server.hpp>
#include <src/base_objects/network/tcp/session.hpp>
#include <src/log.hpp>

namespace copper_server::base_objects::network::tcp {
    constexpr bool CONSTEXPR_DEBUG_DATA_TRANSPORT = true;
    std::atomic_uint64_t session::id_gen(0);
    bool session::do_log_connection_errors = true;

    session::session(boost::asio::ip::tcp::socket&& s, client* client_handler, uint64_t& set_timeout)
        : sock(std::move(s)), timeout(set_timeout) {
        chandler = client_handler->define_ourself(this);
        read_data.resize(1024);
    }

    session::~session() {
        if (_sharedData) {
            if (_sharedData->packets_state.state != base_objects::SharedClientData::packets_state_t::protocol_state::initialization)
                api::players::handlers::on_player_leave(sharedDataRef());
            api::players::remove_player(sharedDataRef());
        }
        if (chandler)
            delete chandler;
    }

    base_objects::client_data_holder& session::sharedDataRef() {
        return _sharedData ? _sharedData : _sharedData = api::players::allocate_player();
    }

    base_objects::SharedClientData& session::sharedData() {
        return *sharedDataRef();
    }

    void session::connect() {
        active = true;
        post(sock.get_executor(), [this] { req_loop(); return true; });
    }

    void session::connect(std::vector<uint8_t>& connection_data, boost::system::error_code ec) {
        if (checked(ec, "connect")) {
            active = true;
            read_data = connection_data;
            on_request(ec, read_data.size());
        }
    }

    void session::disconnect() {
        post(sock.get_executor(), [this] {
            if (!sock.is_open()) {
                log::error("protocol", "[error] [" + std::to_string(id) + "] connection aborted by client.");
            } else
                sock.cancel();
            sock.close();
            active = false;
            server::instance().close_session(this);
        });
    }

    bool session::isActive() {
        return active;
    }

    bool session::start_symmetric_encryption(const list_array<uint8_t>& encryption_key, const list_array<uint8_t>& encryption_iv) {
        if (!encryption.initialize(encryption_key, encryption_iv))
            return false;
        encryption_enabled = true;
        return true;
    }

    void session::send(response&& resp) {
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


            if (encryption_enabled)
                encryption.encrypt(response_data, response_data);


            std::shared_ptr<std::vector<uint8_t>> send_data = std::make_shared<std::vector<uint8_t>>(response_data.begin(), response_data.end());
            if (resp.do_disconnect_after_send) {
                sock.async_send(
                    boost::asio::buffer(*send_data),
                    [this, send_data](boost::system::error_code ec, size_t completed) {
                        checked(ec, "response_disconnect");
                        disconnect();
                        return completed;
                    }
                );
            } else
                sock.async_send(
                    boost::asio::buffer(*send_data),
                    [this, send_data](boost::system::error_code ec, size_t completed) {
                        if (checked(ec, "response"))
                            req_loop();
                        return completed;
                    }
                );
        } else
            req_loop();
    }

    bool session::checked(boost::system::error_code ec, std::string const& msg) {
        if (ec || sock.is_open() == false) {
            if (do_log_connection_errors)
                log::error("protocol", "[" + msg + "] [" + std::to_string(id) + "]" + ec.message());
            disconnect();
        }
        return !ec.failed() && sock.is_open();
    }

    void session::req_loop() {

        sock.async_read_some(boost::asio::buffer(read_data), [this](boost::system::error_code ec, size_t size) {
            if (checked(ec, "req_loop"))
                on_request(ec, size);
        });
    }

    void session::consume_data(size_t read_size) {
        list_array<uint8_t> convert_data(read_data.data(), read_size);
        //<for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>
        if constexpr (CONSTEXPR_DEBUG_DATA_TRANSPORT)
            client::log_console("P (" + std::to_string(id) + ")", convert_data, read_size);
        //</for debug, set CONSTEXPR_DEBUG_DATA_TRANSPORT to false to disable this block>

        if (encryption_enabled) {
            encryption.decrypt(convert_data, convert_data);
            read_data_cached.push_back(std::move(convert_data));
        } else
            read_data_cached.push_back(list_array<uint8_t>(convert_data.data(), read_size));
    }

    response session::proceed_data() {
        while (true) {
            response tmp(chandler->work_client(read_data_cached));
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

    void session::on_request(boost::system::error_code ec, size_t read_size) {
        if (checked(ec, "on_request")) {
            consume_data(read_size);
            send(proceed_data());
        }
    }

    client& session::handler() {
        return *chandler;
    }
}