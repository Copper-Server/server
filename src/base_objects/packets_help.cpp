/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/base_objects/entity.hpp>
#include <src/base_objects/packets_help.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/storage/world_data.hpp>

namespace copper_server::base_objects {

    size_t get_size_source_value(SharedClientData& context, size_source resource) {
        switch (resource) {
        case size_source::get_world_chunks_height: {
            if (context.player_data.assigned_entity)
                if (context.player_data.assigned_entity->current_world())
                    return context.player_data.assigned_entity->current_world()->get_chunk_y_count();
            return 0;
        }
        case size_source::get_world_blocks_height: {
            if (context.player_data.assigned_entity)
                if (context.player_data.assigned_entity->current_world())
                    return context.player_data.assigned_entity->current_world()->get_chunk_y_count() * 16;
            return 0;
        }
        default:
            return 0;
        }
    }
}