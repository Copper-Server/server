/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_WORLD_LOADING_POINT_TICKET
#define SRC_BASE_OBJECTS_WORLD_LOADING_POINT_TICKET
#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include <src/base_objects/bounds.hpp>

namespace copper_server::storage {
    class world_data;
}

namespace copper_server::base_objects::world {
    struct loading_point_ticket {
        //returns true if ticket not expired
        using callback = std::function<bool(copper_server::storage::world_data&, size_t, loading_point_ticket&)>;

        struct entity_bound_ticket {
            size_t id;
        };

        std::variant<uint16_t, callback, entity_bound_ticket> expiration;
        base_objects::cubic_bounds_chunk_radius point;
        std::string name;
        int8_t level;

        loading_point_ticket(callback&& callback, const cubic_bounds_chunk_radius& point, const std::string& name, int8_t level) : expiration(std::move(callback)), point(point), name(name), level(level) {}

        loading_point_ticket(entity_bound_ticket bound, const cubic_bounds_chunk_radius& point, const std::string& name, int8_t level) : expiration(bound), point(point), name(name), level(level) {}

        loading_point_ticket(uint16_t ticks, const cubic_bounds_chunk_radius& point, const std::string& name, int8_t level) : expiration(ticks), point(point), name(name), level(level) {}


        //sets the whole cubic point to specified level and propagates to neighbors by default
    };
}
#endif /* SRC_BASE_OBJECTS_WORLD_LOADING_POINT_TICKET */
