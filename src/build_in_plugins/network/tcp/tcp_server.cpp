#include <src/api/configuration.hpp>
#include <src/api/network.hpp>
#include <src/api/server.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/plugin/special.hpp>

#include <library/fast_task/include/networking.hpp>
#include <src/build_in_plugins/network/tcp/session.hpp>
#include <src/build_in_plugins/network/tcp/universal_client_handle.hpp>

#include <stacktrace>

namespace copper_server::build_in_plugins::network::tcp {
    using fast_task::networking::TcpError;
    base_objects::network::tcp::client* tcp_handler = new universal_client_handle();

    void handler(fast_task::networking::TcpNetworkStream& stream) {
        if (api::network::ip_filter(stream.remote_address()))
            return;

        auto session = std::make_shared<tcp::session>(stream, tcp_handler, api::configuration::get().protocol.all_connections_timeout_seconds);
        try {
            while (!stream.is_closed()) {
                auto input = stream.read_available_ref();
                if (stream.error() == TcpError::none)
                    session->received(input);
            }
        } catch (const std::exception& ex) {
            std::stringstream stack_trace;
            stack_trace << std::stacktrace::current();
            log::error("Network", "unhandled exception while processing client. Id: " + std::to_string(session->id) + ". Address: " + stream.remote_address().to_string());
            log::debug_error("Network", "client id " + std::to_string(session->id) + " stack trace:\n" + stack_trace.str());
            log::debug_error("Network", "client id " + std::to_string(session->id) + " exceptions data:\n" + ex.what());
        } catch (...) {
            std::stringstream stack_trace;
            stack_trace << std::stacktrace::current();
            log::error("Network", "unhandled undefined exception while processing client. Id: " + std::to_string(session->id) + ". Address: " + stream.remote_address().to_string());
            log::debug_error("Network", "client id " + std::to_string(session->id) + " stack trace:\n" + stack_trace.str());
        }
        session->disconnect();
    }

    class TCPServerPlugin : public PluginAutoRegister<"network/tcp_server", TCPServerPlugin> {
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
            register_event(api::server::shutdown_event, base_objects::events::priority::low, [this]() { if (tcp_server) stop(); return false; });
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