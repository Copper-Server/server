#include <library/fast_task.hpp>
#include <src/api/asio.hpp>
#include <src/api/configuration.hpp>
#include <src/api/network.hpp>
#include <src/base_objects/network/tcp_server.hpp>
#include <src/build_in_plugins/special/status.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/protocolHelper/state_handshaking.hpp>
#include <src/resources/registers.hpp>

using namespace copper_server;


int main() {
    atexit([]() {
        log::info("Initializer thread", "Shutting down...");
        try {
            pluginManagement.callUnload();
        } catch (const std::exception& e) {
            log::error("Initializer thread", "An error occurred while unloading plugins");
            log::error("Initializer thread", e.what());
        } catch (...) {
            log::error("Initializer thread", "An error occurred while unloading plugins");
        }
        try {
            pluginManagement.unregisterAll();
        } catch (...) {
            log::error("Initializer thread", "An error occurred while unregistering plugins");
        }
        fast_task::task::shutDown();
        log::commands::deinit();
    });
    try {
        size_t executors = api::configuration::get().server.working_threads;
        fast_task::task::create_executor(executors ? executors : std::thread::hardware_concurrency());
        fast_task::task::task::enable_task_naming = false;
        api::asio::init(api::configuration::get().server.accepting_threads);


        log::commands::init();
        log::info("Initializer thread", "Initializing server...");
        pluginManagement.autoRegister();
        pluginManagement.callInitialization();

        resources::initialize();

    } catch (const std::exception& e) {
        log::fatal("Initializer thread", "An error occurred while initializing the server, shutting down...");
        log::fatal("Initializer thread", e.what());
        return 1;
    } catch (...) {
        log::fatal("Initializer thread", "An error occurred while initializing the server, shutting down...");
        pluginManagement.callFaultUnload();
        return 1;
    }
    try {
        base_objects::network::tcp_server server(
            api::configuration::get().server.ip,
            api::configuration::get().server.port,
            api::configuration::get().server.ssl_key_length
        );

        log::disable_log_level(log::level::debug);
        special_handshake = new SpecialPluginHandshake();
        special_status = new build_in_plugins::special::Status();
        api::network::register_tcp_handler(new tcp_client_handle_handshaking());

        log::info("Initializer thread", "Loading plugins");
        try {
            pluginManagement.callLoad();
        } catch (const std::exception& e) {
            log::fatal("Initializer thread", "An error occurred while loading plugins, shutting down...");
            log::fatal("Initializer thread", e.what());
            pluginManagement.callFaultUnload();
            return 1;
        } catch (...) {
            log::fatal("Initializer thread", "An error occurred while loading plugins, shutting down...");
            pluginManagement.callFaultUnload();
            return 1;
        }
        log::info("Initializer thread", "Start handling to clients");
        try {
            server.start();
        } catch (...) {
            log::fatal("Initializer thread", "An error occurred while starting the tcp server, shutting down...");
            pluginManagement.callFaultUnload();
            return 1;
        }
    } catch (const std::exception& e) {
        log::fatal("Initializer thread", "An error occurred while initializing the server, shutting down...");
        log::fatal("Initializer thread", e.what());
        pluginManagement.callFaultUnload();
        return 1;
    } catch (...) {
        log::fatal("Initializer thread", "An error occurred while initializing the server, shutting down...");
        pluginManagement.callFaultUnload();
        return 1;
    }
    return 0;
}
