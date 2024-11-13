#ifndef SRC_API_COMMAND
#define SRC_API_COMMAND
#include <src/base_objects/commands.hpp>

namespace copper_server::api::command {
    base_objects::command_manager& get_manager();
}

#endif /* SRC_API_COMMAND */
