/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_CLIENT_BOUND_STATUS
#define SRC_API_PACKETS_CLIENT_BOUND_STATUS
#include <src/api/packets/ops.hpp>
#include <src/api/packets/types.hpp>

namespace copper_server::api::packets::client_bound::status {
    struct status_response : public packet<0x00> {
        string_sized<32767> json_response;
    };

    struct pong_response : public packet<0x01> {
        uint64_t timestamp;
    };
}

namespace copper_server::api::packets {
    extern template packet_ops<client_bound::status::status_response>;
    extern template packet_ops<client_bound::status::pong_response>;
    using client_bound_status_ops = state_ops<
        client_bound::status::status_response,
        client_bound::status::pong_response>;
}

#endif /* SRC_API_PACKETS_CLIENT_BOUND_STATUS */
