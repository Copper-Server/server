/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_BAN
#define SRC_API_BAN
#include <src/base_objects/events/event.hpp>

namespace copper_server::api::ban {
    struct ban_data {
        std::string who;
        std::string by;
        std::string reason;
    };

    extern base_objects::events::event<ban_data> on_ban;
    extern base_objects::events::event<ban_data> on_pardon;
    extern base_objects::events::event<ban_data> on_ban_ip;
    extern base_objects::events::event<ban_data> on_pardon_ip;
}
#endif /* SRC_API_BAN */
