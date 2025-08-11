/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_NETWORK_TCP_CLIENT
#define SRC_BASE_OBJECTS_NETWORK_TCP_CLIENT
#include <src/base_objects/network/response.hpp>
#include <string>

namespace copper_server::api::network::tcp {
    class session;
}

namespace copper_server::base_objects::network::tcp {

    class client {
    protected:
    public:
        virtual response work_client(list_array<uint8_t>&) = 0;

        virtual response on_switch();

        //will return nullptr if Redefine not needed
        virtual client* redefine_handler() = 0;
        virtual client* define_ourself(api::network::tcp::session* session) = 0;

        virtual ~client();

        static void log_console(const std::string& prefix, const list_array<uint8_t>& data, size_t size);
    };
}

#endif /* SRC_BASE_OBJECTS_NETWORK_TCP_CLIENT */
