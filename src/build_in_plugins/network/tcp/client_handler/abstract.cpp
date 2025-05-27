#include <src/build_in_plugins/network/tcp/client_handler/configuration.hpp>
#include <src/build_in_plugins/network/tcp/client_handler/login.hpp>
#include <src/build_in_plugins/network/tcp/client_handler/play.hpp>

//`std::make_unique<handle_*>(client).release();` construction used to prevent memory leaks, it is same as `new handle_*(client);` but when thrown it will be deleted
namespace copper_server::build_in_plugins::network::tcp::client_handler::abstract {
    base_objects::network::tcp::client* createhandle_login(api::network::tcp::session* client) {
        return std::make_unique<handle_login>(client).release();
    }

    base_objects::network::tcp::client* createhandle_configuration(api::network::tcp::session* client) {
        return std::make_unique<handle_configuration>(client).release();
    }

    base_objects::network::tcp::client* createhandle_play(api::network::tcp::session* client) {
        return std::make_unique<handle_play>(client).release();
    }
}