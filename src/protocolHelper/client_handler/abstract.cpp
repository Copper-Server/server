#include "765/configuration.hpp"
#include "765/login.hpp"
#include "765/play.hpp"

#include "766/configuration.hpp"
#include "766/login.hpp"
#include "766/play.hpp"

#include "767/configuration.hpp"
#include "767/login.hpp"
#include "767/play.hpp"

namespace crafted_craft {
    namespace client_handler {
        namespace abstract {
            TCPclient* createHandleLogin(TCPsession* client) {
                switch (client->protocol_version) {
                case 765:
                    return new release_765::HandleLogin(client);
                case 766:
                    return new release_766::HandleLogin(client);
                case 767:
                    return new release_767::HandleLogin(client);
                default:
                    throw std::runtime_error("Unknown protocol version");
                }
            }

            TCPclient* createHandleConfiguration(TCPsession* client) {
                switch (client->protocol_version) {
                case 765:
                    return new release_765::HandleConfiguration(client);
                case 766:
                    return new release_766::HandleConfiguration(client);
                case 767:
                    return new release_767::HandleConfiguration(client);
                default:
                    throw std::runtime_error("Unknown protocol version");
                }
            }

            TCPclient* createHandlePlay(TCPsession* client) {
                switch (client->protocol_version) {
                case 765:
                    return new release_765::HandlePlay(client);
                case 766:
                    return new release_766::HandlePlay(client);
                case 767:
                    return new release_767::HandlePlay(client);
                default:
                    throw std::runtime_error("Unknown protocol version");
                }
            }
        }
    }
}