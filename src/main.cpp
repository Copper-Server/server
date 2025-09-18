/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <library/fast_task.hpp>
#include <library/fast_task/include/allocator.hpp>
#include <src/api/configuration.hpp>
#include <src/api/log.hpp>
#include <src/api/server.hpp>
#include <src/plugin/main.hpp>
#include <src/resources/registers.hpp>

using namespace copper_server;

int main() {
    atexit([]() {
        api::log::info("Initializer thread", "Shutting down...");
        try {
            pluginManagement.callUnload();
        } catch (const std::exception& e) {
            api::log::error("Initializer thread", "An error occurred while unloading plugins\n" + std::string(e.what()));
        } catch (...) {
            api::log::error("Initializer thread", "An error occurred while unloading plugins");
        }
        try {
            pluginManagement.unregisterAll();
        } catch (...) {
            api::log::error("Initializer thread", "An error occurred while unregistering plugins");
        }
        api::log::commands::deinit();
        fast_task::scheduler::await_end_tasks(false);
        fast_task::scheduler::shut_down();
    });
    try {
        size_t working_threads = api::configuration::get().server.working_threads;
        fast_task::scheduler::reduce_executor(fast_task::scheduler::total_executors());
        fast_task::scheduler::create_executor(working_threads);
        fast_task::task::task::enable_task_naming = false;


        api::log::commands::init();
        api::log::set_log_folder(api::configuration::get().server.get_storage_path() / "logs");
        api::log::info("Initializer thread", "Initializing server...");
        pluginManagement.autoRegister();
        pluginManagement.callInitialization();

        resources::initialize();
    } catch (const std::exception& e) {
        api::log::fatal("Initializer thread", "An error occurred while initializing the server, shutting down...\n" + std::string(e.what()));
        return 1;
    } catch (...) {
        api::log::fatal("Initializer thread", "An error occurred while initializing the server, shutting down...");
        pluginManagement.callFaultUnload();
        return 1;
    }
    api::log::info("Initializer thread", "Initialization complete.");

    if (api::server::is_shutting_down())
        return 0;

    api::log::info("Initializer thread", "Loading plugins");
    try {
        pluginManagement.callLoad();
    } catch (const std::exception& e) {
        api::log::fatal("Initializer thread", "An error occurred while loading plugins, shutting down...\n" + std::string(e.what()));
        pluginManagement.callFaultUnload();
        return 1;
    } catch (...) {
        api::log::fatal("Initializer thread", "An error occurred while loading plugins, shutting down...");
        pluginManagement.callFaultUnload();
        return 1;
    }
    api::log::info("Initializer thread", "Loading complete.");
    fast_task::scheduler::await_end_tasks(false);
    return 0;
}

//Allocation safety(only when preemptive scheduler enabled in fast_task)

void* operator new(std::size_t n) noexcept(false) {
    return fast_task::allocate(n);
}

void operator delete(void* p) noexcept {
    return fast_task::free(p);
}

void* operator new[](std::size_t s) noexcept(false) {
    return fast_task::allocate(s);
}

void operator delete[](void* p) noexcept {
    return fast_task::free(p);
}