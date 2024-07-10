#ifndef SRC_API_COMMAND
#define SRC_API_COMMAND
#include "../base_objects/commands.hpp"

namespace crafted_craft {
    namespace api {
        namespace command {
            void register_manager(base_objects::command_manager&);
            void unregister_manager();

            base_objects::command_manager& get_manager();
        }
    }
}

#endif /* SRC_API_COMMAND */
