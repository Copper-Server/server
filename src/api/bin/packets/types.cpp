/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/configuration.hpp>
#include <src/api/entity.hpp>
#include <src/api/packets/types.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server::api::packets {
    namespace events {
        base_objects::events::sync_event_no_cancel<base_objects::shared_client_data&> client_state_changed;
    }

    size_t get_size_source_value(base_objects::shared_client_data& context, size_source resource) {
        switch (resource) {
        case size_source::get_world_chunks_height: {
            if (context.player_data.assigned_entity) {
                api::entity entity(*context.player_data.assigned_entity);
                if (entity.current_world())
                    return entity.current_world()->get_chunk_y_count();
            }
            return 0;
        }
        case size_source::get_world_blocks_height: {
            if (context.player_data.assigned_entity) {
                api::entity entity(*context.player_data.assigned_entity);
                if (entity.current_world())
                    return entity.current_world()->get_chunk_y_count() * 16;
            }
            return 0;
        }
        default:
            return 0;
        }
    }
}

copper_server::base_objects::shared_client_data& operator<<(copper_server::base_objects::shared_client_data& client, copper_server::api::packets::switches_to::status) {
    client.packets_state.state = copper_server::base_objects::shared_client_data::packets_state_t::protocol_state::status;
    client.packets_state.internal_data.set([](auto& data) {
        data.extra_data = nullptr;
    });
    copper_server::api::packets::events::client_state_changed(client);
    return client;
}

copper_server::base_objects::shared_client_data& operator<<(copper_server::base_objects::shared_client_data& client, copper_server::api::packets::switches_to::login) {
    client.packets_state.state = copper_server::base_objects::shared_client_data::packets_state_t::protocol_state::login;
    client.packets_state.internal_data.set([](auto& data) {
        data.extra_data = nullptr;
    });
    copper_server::api::packets::events::client_state_changed(client);
    return client;
}

copper_server::base_objects::shared_client_data& operator<<(copper_server::base_objects::shared_client_data& client, copper_server::api::packets::switches_to::config) {
    client.packets_state.state = copper_server::base_objects::shared_client_data::packets_state_t::protocol_state::configuration;
    copper_server::api::players::login_complete_to_cfg(client);
    client.packets_state.internal_data.set([](auto& data) {
        data.extra_data = nullptr;
    });
    copper_server::api::packets::events::client_state_changed(client);
    return client;
}

copper_server::base_objects::shared_client_data& operator<<(copper_server::base_objects::shared_client_data& client, copper_server::api::packets::switches_to::play) {
    client.packets_state.state = copper_server::base_objects::shared_client_data::packets_state_t::protocol_state::play;
    client.packets_state.internal_data.set([](auto& data) {
        data.extra_data = nullptr;
    });
    copper_server::api::packets::events::client_state_changed(client);
    return client;
}
