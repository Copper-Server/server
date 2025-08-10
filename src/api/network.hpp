/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_NETWORK
#define SRC_API_NETWORK
#include <library/fast_task/include/networking.hpp>
#include <src/base_objects/atomic_holder.hpp>
#include <src/base_objects/events/sync_event.hpp>

namespace copper_server::base_objects {
    namespace network::tcp {
        class client;
    }
    struct SharedClientData;
    using client_data_holder = atomic_holder<SharedClientData>;
}

namespace copper_server::api::network {
    extern base_objects::events::sync_event<const fast_task::networking::address&> ip_filter;
}

#endif /* SRC_API_NETWORK */
