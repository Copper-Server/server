#ifndef SRC_API_CONSOLE
#define SRC_API_CONSOLE
#include <string>

namespace copper_server::api::console {
    //this event should be handled by plugin
    extern base_objects::events::event<std::string> on_command;

    //uses console's virtual client and executes command
    // that's means, command executes with console's rights
    // on errors throws base_objects::command_exception or std::exception
    // plugin should use this
    void execute_as_console(const std::string& command);
    bool console_enabled();
}

#endif /* SRC_API_CONSOLE */
