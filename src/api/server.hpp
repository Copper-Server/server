/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_SERVER
#define SRC_API_SERVER
#include <src/base_objects/events/event.hpp>

namespace copper_server::api::server {
    extern base_objects::events::event<void> shutdown_event;

    void shutdown();
    bool is_shutting_down();
}

#endif /* SRC_API_SERVER */
