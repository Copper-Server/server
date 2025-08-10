/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BUILD_IN_PLUGINS_NETWORK_TCP_UNIVERSAL_CLIENT_HANDLE
#define SRC_BUILD_IN_PLUGINS_NETWORK_TCP_UNIVERSAL_CLIENT_HANDLE
#include <src/build_in_plugins/network/tcp/util.hpp>

namespace copper_server::build_in_plugins::network::tcp {
    class universal_client_handle final : public tcp_client_handle {
        base_objects::network::response work_packet(ArrayStream& data) override;
        base_objects::network::response too_large_packet() override;
        base_objects::network::response exception(const std::exception& ex) override;
        base_objects::network::response unexpected_exception() override;
    public:
        universal_client_handle(api::network::tcp::session* sock);
        universal_client_handle();
        base_objects::network::tcp::client* define_ourself(api::network::tcp::session* sock) override;
        base_objects::network::tcp::client* redefine_handler() override;
    };
}


#endif /* SRC_BUILD_IN_PLUGINS_NETWORK_TCP_UNIVERSAL_CLIENT_HANDLE */
