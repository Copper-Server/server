#ifndef SRC_API_CONSOLE
#define SRC_API_CONSOLE
#include "../base_objects/virtual_client.hpp"

namespace crafted_craft {
    namespace api {
        namespace console {
            void register_virtual_client(base_objects::virtual_client& client);
            void unregister_virtual_client();

            void execute_as_console(const std::string& command);
            bool console_enabled();
        }
    }
}

#endif /* SRC_API_CONSOLE */
