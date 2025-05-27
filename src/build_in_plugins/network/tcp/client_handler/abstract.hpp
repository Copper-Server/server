#ifndef SRC_BUILD_IN_PLUGINS_NETWORK_TCP_CLIENT_HANDLER_ABSTRACT
#define SRC_BUILD_IN_PLUGINS_NETWORK_TCP_CLIENT_HANDLER_ABSTRACT
#include <src/build_in_plugins/network/tcp/util.hpp>

namespace copper_server::build_in_plugins::network::tcp::client_handler::abstract {
    base_objects::network::tcp::client* createhandle_login(api::network::tcp::session*);
    base_objects::network::tcp::client* createhandle_configuration(api::network::tcp::session*);
    base_objects::network::tcp::client* createhandle_play(api::network::tcp::session*);
}

#endif /* SRC_BUILD_IN_PLUGINS_NETWORK_TCP_CLIENT_HANDLER_ABSTRACT */
