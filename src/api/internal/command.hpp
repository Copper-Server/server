#ifndef SRC_API_INTERNAL_REGISTRATIONS
#define SRC_API_INTERNAL_REGISTRATIONS
#include <src/base_objects/commands.hpp>

namespace copper_server::api::command {
    void register_manager(base_objects::command_manager&);
    void unregister_manager();
}


#endif /* SRC_API_INTERNAL_REGISTRATIONS */
