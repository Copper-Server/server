/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_EVENTS_PRIORITY
#define SRC_BASE_OBJECTS_EVENTS_PRIORITY

namespace copper_server::base_objects::events {
    enum class priority {
        high,
        upper_avg,
        avg,
        lower_avg,
        low
    };
}

#endif /* SRC_BASE_OBJECTS_EVENTS_PRIORITY */
