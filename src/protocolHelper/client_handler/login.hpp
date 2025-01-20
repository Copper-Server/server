#ifndef SRC_PROTOCOLHELPER_CLIENT_HANDLER_LOGIN
#define SRC_PROTOCOLHELPER_CLIENT_HANDLER_LOGIN
#include <src/base_objects/network/accept_packet_registry.hpp>
#include <src/protocolHelper/util.hpp>

namespace copper_server::client_handler {
    class handle_login : public tcp_client_handle {
        const base_objects::network::protocol_packet_registry_t& registry;
        base_objects::network::response IdleActions();
        base_objects::network::response work_packet(ArrayStream& packet) override;
        base_objects::network::response too_large_packet() override;
        base_objects::network::response exception(const std::exception& ex) override;
        base_objects::network::response unexpected_exception() override;
        base_objects::network::response on_switching() override;

    public:
        handle_login(base_objects::network::tcp_session* session);
        ~handle_login() override;
        base_objects::network::tcp_client* define_ourself(base_objects::network::tcp_session* sock) override;
    };
}

#endif /* SRC_PROTOCOLHELPER_CLIENT_HANDLER_LOGIN */
