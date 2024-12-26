#include <boost/asio/ip/address.hpp>
#include <src/base_objects/events/sync_event.hpp>

namespace copper_server::base_objects::network {
    class tcp_client;
}

namespace copper_server::api::network {
    base_objects::events::sync_event<const boost::asio::ip::address&> ip_filter;
    base_objects::network::tcp_client* handler = nullptr;

    base_objects::network::tcp_client* get_first_tcp_handler() {
        return handler;
    }

    void register_tcp_handler(base_objects::network::tcp_client* _handler) {
        if (handler)
            throw std::runtime_error("tcp_client handler already registered");
        handler = _handler;
    }
}