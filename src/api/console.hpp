#ifndef SRC_API_CONSOLE
#define SRC_API_CONSOLE
#include <string>

namespace crafted_craft::api::console {
    void execute_as_console(const std::string& command);
    bool console_enabled();
}

#endif /* SRC_API_CONSOLE */
