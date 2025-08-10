/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_CONSOLE
#define SRC_API_CONSOLE
#include <src/base_objects/events/event.hpp>
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
