#include <src/protocolHelper/client_handler/765/configuration.hpp>
#include <src/protocolHelper/client_handler/765/login.hpp>
#include <src/protocolHelper/client_handler/765/play.hpp>

#include <src/protocolHelper/client_handler/766/configuration.hpp>
#include <src/protocolHelper/client_handler/766/login.hpp>
#include <src/protocolHelper/client_handler/766/play.hpp>

#include <src/protocolHelper/client_handler/767/configuration.hpp>
#include <src/protocolHelper/client_handler/767/login.hpp>
#include <src/protocolHelper/client_handler/767/play.hpp>

#include <src/protocolHelper/client_handler/768/configuration.hpp>
#include <src/protocolHelper/client_handler/768/login.hpp>
#include <src/protocolHelper/client_handler/768/play.hpp>

#include <src/protocolHelper/client_handler/configuration.hpp>
#include <src/protocolHelper/client_handler/login.hpp>
#include <src/protocolHelper/client_handler/play.hpp>

//`std::make_unique<handle_*>(client).release();` construction used to prevent memory leaks, it is same as `new handle_*(client);` but when thrown it will be deleted
namespace copper_server {
    namespace client_handler {
        namespace abstract {
            base_objects::network::tcp_client* createhandle_login(base_objects::network::tcp_session* client) {
                switch (client->protocol_version) {
                case 765:
                    return std::make_unique<release_765::handle_login>(client).release();
                case 766:
                    return std::make_unique<release_766::handle_login>(client).release();
                case 767:
                    return std::make_unique<release_767::handle_login>(client).release();
                case 768:
                    return std::make_unique<release_768::handle_login>(client).release();
                default:
                    return std::make_unique<handle_login>(client).release();
                }
            }

            base_objects::network::tcp_client* createhandle_configuration(base_objects::network::tcp_session* client) {
                switch (client->protocol_version) {
                case 765:
                    return std::make_unique<release_765::handle_configuration>(client).release();
                    return new release_765::handle_configuration(client);
                case 766:
                    return std::make_unique<release_766::handle_configuration>(client).release();
                case 767:
                    return std::make_unique<release_767::handle_configuration>(client).release();
                case 768:
                    return std::make_unique<release_768::handle_configuration>(client).release();
                default:
                    return std::make_unique<handle_configuration>(client).release();
                }
            }

            base_objects::network::tcp_client* createhandle_play(base_objects::network::tcp_session* client) {
                switch (client->protocol_version) {
                case 765:
                    return std::make_unique<release_765::handle_play>(client).release();
                case 766:
                    return std::make_unique<release_766::handle_play>(client).release();
                case 767:
                    return std::make_unique<release_767::handle_play>(client).release();
                case 768:
                    return std::make_unique<release_768::handle_play>(client).release();
                default:
                    return std::make_unique<handle_play>(client).release();
                }
            }
        }
    }
}