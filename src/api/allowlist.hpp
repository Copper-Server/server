/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_ALLOWLIST
#define SRC_API_ALLOWLIST
#include <src/base_objects/atomic_holder.hpp>
#include <src/base_objects/events/event.hpp>

namespace copper_server::base_objects {
    struct SharedClientData;
    using client_data_holder = atomic_holder<SharedClientData>;
}
namespace copper_server::api::allowlist {
    enum class allowlist_mode {
        allow,
        block,
        off
    };
    extern base_objects::events::event<allowlist_mode> on_mode_change;
    // std::string is the player name
    extern base_objects::events::event<base_objects::client_data_holder> on_kick;
    extern base_objects::events::event<std::string> on_add;
    extern base_objects::events::event<std::string> on_remove;
}
#endif /* SRC_API_ALLOWLIST */
