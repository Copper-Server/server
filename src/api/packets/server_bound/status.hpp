/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_SERVER_BOUND_STATUS
#define SRC_API_PACKETS_SERVER_BOUND_STATUS
#include <src/api/packets/types.hpp>
#include <src/api/packets/ops.hpp>

namespace copper_server::api::packets::server_bound::status {
    struct status_request : public packet<0x00> {};

    struct ping_response : public packet<0x01> {
        uint64_t timestamp;
    };
}

namespace copper_server::api::packets {
    extern template packet_ops<server_bound::status::status_request>;
    extern template packet_ops<server_bound::status::ping_response>;

    using server_bound_status_ops = state_ops<
        server_bound::status::status_request,
        server_bound::status::ping_response
    >;
}

#endif /* SRC_API_PACKETS_SERVER_BOUND_STATUS */
