/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_EVENTS_BASE_EVENT
#define SRC_BASE_OBJECTS_EVENTS_BASE_EVENT
#include <src/base_objects/events/priority.hpp>
#include <cstdint>
namespace copper_server::base_objects::events {
    struct event_register_id {
        std::uint64_t id;
    };

    class base_event {
    public:
        virtual bool leave(event_register_id func, priority priority = priority::avg, bool async_mode = false) = 0;
    };
}
#endif /* SRC_BASE_OBJECTS_EVENTS_BASE_EVENT */
