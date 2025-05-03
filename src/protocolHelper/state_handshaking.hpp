#ifndef SRC_PROTOCOLHELPER_STATE_HANDSHAKING
#define SRC_PROTOCOLHELPER_STATE_HANDSHAKING
#include <src/protocolHelper/util.hpp>

namespace copper_server {
    class tcp_client_handle_handshaking : public tcp_client_handle {
    protected:
        virtual bool AllowServerAddressAndPort(std::string& str, uint16_t port);
        base_objects::network::response work_packet(ArrayStream& data) override;
        base_objects::network::response too_large_packet() override;
        base_objects::network::response exception(const std::exception& ex) override;
        base_objects::network::response unexpected_exception() override;

    public:
        tcp_client_handle_handshaking(base_objects::network::tcp::session* sock);
        tcp_client_handle_handshaking();
        base_objects::network::tcp::client* define_ourself(base_objects::network::tcp::session* sock) override;
        base_objects::network::tcp::client* redefine_handler() override;
    };
}
#endif /* SRC_PROTOCOLHELPER_STATE_HANDSHAKING */
