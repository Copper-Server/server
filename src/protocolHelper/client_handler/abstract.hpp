#ifndef SRC_PROTOCOLHELPER_CLIENT_HANDLER_ABSTRACT
#define SRC_PROTOCOLHELPER_CLIENT_HANDLER_ABSTRACT
#include <src/protocolHelper/util.hpp>

namespace copper_server {
    namespace client_handler {
        namespace abstract {
            base_objects::network::tcp_client* createhandle_login(base_objects::network::tcp_session*);
            base_objects::network::tcp_client* createhandle_configuration(base_objects::network::tcp_session*);
            base_objects::network::tcp_client* createhandle_play(base_objects::network::tcp_session*);
        }
    }
}

#endif /* SRC_PROTOCOLHELPER_CLIENT_HANDLER_ABSTRACT */
