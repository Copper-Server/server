
#include <src/api/configuration.hpp>
#include <src/api/network.hpp>
#include <src/api/server.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/plugin/special.hpp>

#include <library/fast_task/src/networking/networking.hpp>
#include <src/build_in_plugins/network/tcp/session.hpp>
#include <src/build_in_plugins/network/tcp/state_handshaking.hpp>
#include <src/build_in_plugins/network/tcp/state_status.hpp>
#include <src/build_in_plugins/network/tcp/status.hpp>

namespace copper_server::build_in_plugins::network::tcp {
    using fast_task::networking::TcpError;
    base_objects::network::tcp::client* tcp_handler = nullptr;

    base_objects::network::tcp::client* get_first_handler() {
        if (!tcp_handler)
            throw std::runtime_error("tcp::client handler not registered");
        return tcp_handler;
    }

    void register_handler(base_objects::network::tcp::client* _handler) {
        if (tcp_handler) {
            std::swap(tcp_handler, _handler);
            delete _handler;
        } else
            tcp_handler = _handler;
    }

    void handler(fast_task::networking::TcpNetworkStream& stream) {
        if (api::network::ip_filter(stream.remote_address()))
            return;

        auto session = std::make_shared<tcp::session>(stream, get_first_handler(), api::configuration::get().protocol.all_connections_timeout_seconds);
        while (!stream.is_closed()) {
            auto input = stream.read_available_ref();
            if (stream.error() == TcpError::none)
                session->received(input);
        }
        session->disconnect();
    }

    class TCPServerPlugin : public PluginAutoRegister<"tcp_server", TCPServerPlugin> {
        std::shared_ptr<fast_task::networking::TcpNetworkServer> tcp_server;

        void start() {
            tcp_server->start();
            if (tcp_server->is_running()) {
                auto address = tcp_server->server_address();
                log::info("Network", "TCP server started on " + address.to_string());
                if (address.is_loopback())
                    api::configuration::get().server.offline_mode = true;
            } else {
                log::error("Network", "Failed to start TCP server on " + api::configuration::get().server.ip + ":" + std::to_string(api::configuration::get().server.port));
                throw std::runtime_error("TCP server failed to start");
            }
        }

        void stop() {
            tcp_server->stop();
            log::info("Network", "TCP server stopped on " + tcp_server->server_address().to_string());
        }

    public:
        TCPServerPlugin() {
            register_event(api::server::shutdown_event, [this]() { if (tcp_server) stop(); return false; });
            special_status = new Status();
            register_handler(new tcp_client_handle_handshaking());
        }

        void OnPostLoad(const PluginRegistrationPtr&) override {
            if (!tcp_server) {
                tcp_server = std::make_shared<fast_task::networking::TcpNetworkServer>(handler, api::configuration::get().server.ip + ":" + std::to_string(api::configuration::get().server.port));
                tcp_server->set_configuration(fast_task::networking::TcpConfiguration{.buffer_size = api::configuration::get().protocol.new_client_buffer, .allow_ip4 = true});
            }
            if (!tcp_server->is_running())
                start();
        }

        void OnConfigReload(const PluginRegistrationPtr&) override {
            if (tcp_server)
                tcp_server->set_configuration(fast_task::networking::TcpConfiguration{.buffer_size = api::configuration::get().protocol.new_client_buffer, .allow_ip4 = true});
        }

        void OnUnregister(const PluginRegistrationPtr&) override {
            if (tcp_server)
                stop();
        }
    };
}