/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_VIRTUAL_CLIENT
#define SRC_BASE_OBJECTS_VIRTUAL_CLIENT
#include <src/api/packets/ops.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/shared_client_data.hpp>

namespace copper_server::base_objects {
    //by default initialized in play state
    //also by default some packets would not sent to
    // virtual client because it already has access to server memory,
    // to avoid this set client->is_virtual to false
    // all checks for virtual clients should be done using this variable
    //
    // to join world assign entity for virtual client and then register
    // the entity to world
    struct virtual_client {
        client_data_holder client;

        virtual_client(client_data_holder allocated, const std::string& name, const std::string& brand);
        virtual ~virtual_client() = default;

        //sets the callback to receive raw packets, to decode them use the `cliend_bound_*_ops` from api/packets/client_bound/*.hpp
        void set_special_callback(std::function<void(virtual_client&, base_objects::shared_client_data& self, base_objects::network::response&& response)>&& callback) {
            client->special_callback = [this, cc = std::move(callback)](base_objects::shared_client_data& self, base_objects::network::response&& response) {
                cc(*this, self, std::move(response));
            };
        }

        //Send server bound packets to imitate client sending to the server
        template <class Packet>
        void send(Packet&& p) {
            api::packets::make_process(client, std::move(p));
        }
    };

    //Send server bound packets to imitate client sending to the server
    template <class Packet>
    inline virtual_client& operator<<(virtual_client& client, Packet&& p) {
        api::packets::make_process(client, std::move(p));
        return client;
    }
}
#endif /* SRC_BASE_OBJECTS_VIRTUAL_CLIENT */
