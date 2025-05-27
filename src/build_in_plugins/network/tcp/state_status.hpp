#ifndef SRC_BUILD_IN_PLUGINS_NETWORK_TCP_STATE_STATUS
#define SRC_BUILD_IN_PLUGINS_NETWORK_TCP_STATE_STATUS
#include <src/build_in_plugins/network/tcp/util.hpp>

namespace copper_server::build_in_plugins::network::tcp  {
    class tcp_client_handle_status : public tcp_client_handle {
    protected:
        //response status
        virtual std::string build_response();
        base_objects::network::response work_packet(ArrayStream& packet) override;
        base_objects::network::response too_large_packet() override;
        base_objects::network::response exception(const std::exception& ex) override;
        base_objects::network::response unexpected_exception() override;

    public:
        tcp_client_handle_status(api::network::tcp::session* session);

        base_objects::network::tcp::client* define_ourself(api::network::tcp::session* sock) override;
    };
}
#endif /* SRC_BUILD_IN_PLUGINS_NETWORK_TCP_STATE_STATUS */
