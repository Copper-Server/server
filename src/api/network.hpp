#ifndef SRC_API_NETWORK
#define SRC_API_NETWORK
#if defined(_MSC_VER)
    #include <SDKDDKVer.h>
#endif
#include <boost/asio/ip/address.hpp>
#include <src/base_objects/events/sync_event.hpp>

namespace copper_server::base_objects::network::tcp {
    class client;
}

namespace copper_server::api::network {
    extern base_objects::events::sync_event<const boost::asio::ip::address&> ip_filter;
    base_objects::network::tcp::client* get_first_tcp_handler();
    void register_tcp_handler(base_objects::network::tcp::client*);
}

#endif /* SRC_API_NETWORK */
