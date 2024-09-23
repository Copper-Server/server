#include "plugin/main.hpp"
#include "ClientHandleHelper.hpp"
#include "build_in_plugins/special/status.hpp"
#include "library/fast_task.hpp"
#include "log.hpp"
#include "protocolHelper.hpp"
#include "resources/registers.hpp"

using namespace crafted_craft;


int main() {
    using type = std::add_lvalue_reference_t<std::add_const_t<void>>;
    log::commands::init();
    log::info("Initializer thread", "Initializing server...");
    resources::load_blocks();
    resources::initialize_registers();
    resources::initialize_entities();

    fast_task::task::create_executor(16);
    fast_task::task::task::enable_task_naming = false;
    try {
        boost::asio::io_service service;
        Server server(&service, "localhost", 1234);

        log::disable_log_level(log::level::debug);
        pluginManagement.autoRegister();
        special_handshake = new SpecialPluginHandshake();
        special_status = new build_in_plugins::special::Status(server.config, server.online_players);

        first_client_holder = new TCPClientHandleHandshaking();

        try {
            pluginManagement.callLoad();
        } catch (...) {
            log::fatal("Initializer thread", "An error occurred while loading plugins, shutting down...");
            fast_task::task::shutDown();
            return 1;
        }
        try {
            server.start();
        } catch (...) {
            log::fatal("Initializer thread", "An error occurred while starting the tcp server, shutting down...");
            pluginManagement.callFaultUnload();
            fast_task::task::shutDown();
            return 1;
        }
    } catch (...) {
        log::fatal("Initializer thread", "An error occurred while initializing the server, shutting down...");
        pluginManagement.callFaultUnload();
        fast_task::task::shutDown();
        return 1;
    }
    pluginManagement.callUnload();
    fast_task::task::shutDown();
    log::commands::deinit();
    return 0;
}
