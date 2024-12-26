#ifndef SRC_PROTOCOLHELPER_STATE_STATUS
#define SRC_PROTOCOLHELPER_STATE_STATUS
#include <src/protocolHelper/util.hpp>

namespace copper_server {
    class tcp_client_handle_status : public tcp_client_handle {
    protected:
        //response status
        virtual std::string build_response();
        base_objects::network::response work_packet(ArrayStream& packet) override;
        base_objects::network::response too_large_packet() override;
        base_objects::network::response exception(const std::exception& ex) override;
        base_objects::network::response unexpected_exception() override;

    public:
        tcp_client_handle_status(base_objects::network::tcp_session* session);

        base_objects::network::tcp_client* define_ourself(base_objects::network::tcp_session* sock) override;
    };
}
#endif /* SRC_PROTOCOLHELPER_STATE_STATUS */
