#include <boost/asio/ip/address.hpp>
#include <src/base_objects/events/sync_event.hpp>

namespace copper_server::base_objects::network::tcp {
    class client;
}

namespace copper_server::api::network {
    base_objects::events::sync_event<const boost::asio::ip::address&> ip_filter;
    base_objects::network::tcp::client* tcp_handler = nullptr;

    base_objects::network::tcp::client* get_first_tcp_handler() {
        return tcp_handler;
    }

    void register_tcp_handler(base_objects::network::tcp::client* _handler) {
        if (tcp_handler)
            throw std::runtime_error("tcp::client handler already registered");
        tcp_handler = _handler;
    }
}