/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets.hpp>
#include <src/api/registers.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/virtual_client.hpp>

namespace copper_server::base_objects {

    virtual_client::virtual_client(client_data_holder allocated, const std::string& name, const std::string& brand)
        : client(allocated) {

        client->name = name;
        client->ip = "";
        client->client_brand = brand;
        client->locale = "en_US";
        client->data = {};

        client->player_data.local_data["virtual_client"] = name;
        client->player_data.gamemode = (uint8_t)-1;
        client->player_data.op_level = 4;
        client->player_data.world_id = "virtual_client astral space";
        client->is_virtual = true;
        client->packets_state.protocol_version = api::registers::current_protocol_id;
        client->packets_state.state = base_objects::shared_client_data::packets_state_t::protocol_state::play;
    }
}
