#include "plugin/main.hpp"
#include "ClientHandleHelper.hpp"
#include "api/configuration.hpp"
#include "build_in_plugins/special/status.hpp"
#include "library/fast_task.hpp"
#include "log.hpp"
#include "protocolHelper/state_handshaking.hpp"
#include "resources/registers.hpp"

using namespace crafted_craft;


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
        log::commands::init();
        log::info("Initializer thread", "Initializing server...");
        pluginManagement.autoRegister();
        pluginManagement.callInitialization();


        resources::initialize_registers();
        resources::prepare_versions();
        resources::load_items();
        resources::load_blocks();
        resources::initialize_entities();
        size_t executors = api::configuration::get().server.working_threads;
        fast_task::task::create_executor(executors ? executors : std::thread::hardware_concurrency());
        fast_task::task::task::enable_task_naming = false;
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
        boost::asio::io_service service;
        Server server(
            &service,
            api::configuration::get().server.ip,
            api::configuration::get().server.port,
            api::configuration::get().server.accepting_threads,
            api::configuration::get().server.ssl_key_length
        );

        log::disable_log_level(log::level::debug);
        special_handshake = new SpecialPluginHandshake();
        special_status = new build_in_plugins::special::Status(api::configuration::get(), server.online_players);

        first_client_holder = new TCPClientHandleHandshaking();

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
