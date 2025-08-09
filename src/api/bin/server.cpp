
#include <src/base_objects/events/event.hpp>

namespace copper_server::api::server {
    bool shutdown_command = false;
    base_objects::events::event<void> shutdown_event;

    void shutdown(){
        shutdown_command = true;
        shutdown_event();
    }

    bool is_shutting_down(){
        return shutdown_command;
    }
}