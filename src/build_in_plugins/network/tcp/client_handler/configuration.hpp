#ifndef SRC_BUILD_IN_PLUGINS_NETWORK_TCP_CLIENT_HANDLER_CONFIGURATION
#define SRC_BUILD_IN_PLUGINS_NETWORK_TCP_CLIENT_HANDLER_CONFIGURATION
#include <src/base_objects/network/tcp/accept_packet_registry.hpp>
#include <src/build_in_plugins/network/tcp/util.hpp>

namespace copper_server::build_in_plugins::network::tcp::client_handler {
    class handle_configuration : public tcp_client_handle {
        const base_objects::network::tcp::protocol_packet_registry_t& registry;
        base_objects::network::response IdleActions();
        base_objects::network::response work_packet(ArrayStream& packet) override;
        base_objects::network::response too_large_packet() override;
        base_objects::network::response exception(const std::exception& ex) override;
        base_objects::network::response unexpected_exception() override;
        base_objects::network::response on_switching() override;

    public:
        handle_configuration(api::network::tcp::session* session);
        ~handle_configuration() override;
        base_objects::network::tcp::client* define_ourself(api::network::tcp::session* sock) override;
    };
}

#endif /* SRC_BUILD_IN_PLUGINS_NETWORK_TCP_CLIENT_HANDLER_CONFIGURATION */
