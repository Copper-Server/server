#include <src/protocolHelper/client_handler/765/configuration.hpp>
#include <src/protocolHelper/client_handler/765/login.hpp>
#include <src/protocolHelper/client_handler/765/play.hpp>

#include <src/protocolHelper/client_handler/766/configuration.hpp>
#include <src/protocolHelper/client_handler/766/login.hpp>
#include <src/protocolHelper/client_handler/766/play.hpp>

#include <src/protocolHelper/client_handler/767/configuration.hpp>
#include <src/protocolHelper/client_handler/767/login.hpp>
#include <src/protocolHelper/client_handler/767/play.hpp>

namespace copper_server {
    namespace client_handler {
        namespace abstract {
            base_objects::network::tcp_client* createhandle_login(base_objects::network::tcp_session* client) {
                switch (client->protocol_version) {
                case 765:
                    return new release_765::handle_login(client);
                case 766:
                    return new release_766::handle_login(client);
                case 767:
                    return new release_767::handle_login(client);
                default:
                    throw std::runtime_error("Unknown protocol version");
                }
            }

            base_objects::network::tcp_client* createhandle_configuration(base_objects::network::tcp_session* client) {
                switch (client->protocol_version) {
                case 765:
                    return new release_765::handle_configuration(client);
                case 766:
                    return new release_766::handle_configuration(client);
                case 767:
                    return new release_767::handle_configuration(client);
                default:
                    throw std::runtime_error("Unknown protocol version");
                }
            }

            base_objects::network::tcp_client* createhandle_play(base_objects::network::tcp_session* client) {
                switch (client->protocol_version) {
                case 765:
                    return new release_765::handle_play(client);
                case 766:
                    return new release_766::handle_play(client);
                case 767:
                    return new release_767::handle_play(client);
                default:
                    throw std::runtime_error("Unknown protocol version");
                }
            }
        }
    }
}