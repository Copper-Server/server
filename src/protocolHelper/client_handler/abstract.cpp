#include <src/protocolHelper/client_handler/765/configuration.hpp>

#include <src/protocolHelper/client_handler/configuration.hpp>
#include <src/protocolHelper/client_handler/login.hpp>
#include <src/protocolHelper/client_handler/play.hpp>

//`std::make_unique<handle_*>(client).release();` construction used to prevent memory leaks, it is same as `new handle_*(client);` but when thrown it will be deleted
namespace copper_server {
    namespace client_handler {
        namespace abstract {
            base_objects::network::tcp_client* createhandle_login(base_objects::network::tcp_session* client) {
                return std::make_unique<handle_login>(client).release();
            }

            base_objects::network::tcp_client* createhandle_configuration(base_objects::network::tcp_session* client) {
                switch (client->protocol_version) {
                case 765:
                    return std::make_unique<release_765::handle_configuration>(client).release();
                default:
                    return std::make_unique<handle_configuration>(client).release();
                }
            }

            base_objects::network::tcp_client* createhandle_play(base_objects::network::tcp_session* client) {
                return std::make_unique<handle_play>(client).release();
            }
        }
    }
}