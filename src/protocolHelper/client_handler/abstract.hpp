#ifndef SRC_PROTOCOLHELPER_CLIENT_PLAY_HANDLER_ABSTRACT
#define SRC_PROTOCOLHELPER_CLIENT_PLAY_HANDLER_ABSTRACT
#include <src/protocolHelper/util.hpp>

namespace copper_server {
    namespace client_handler {
        namespace abstract {
            TCPclient* createHandleLogin(TCPsession*);
            TCPclient* createHandleConfiguration(TCPsession*);
            TCPclient* createHandlePlay(TCPsession*);
        }
    }
}

#endif /* SRC_PROTOCOLHELPER_CLIENT_PLAY_HANDLER_ABSTRACT */
