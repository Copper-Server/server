#ifndef SRC_API_SERVER
#define SRC_API_SERVER
#include <src/base_objects/events/event.hpp>

namespace copper_server::api::server {
    extern base_objects::events::event<void> shutdown_event;

    void shutdown();
    bool is_shutting_down();
}

#endif /* SRC_API_SERVER */
