/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/bin/ecs/manager.hpp>

namespace copper_server::api::ecs {
    manager& manager::instance() {
        static manager self;
        return self;
    }

    local_entity_id_cache& local_entity_id_cache::get_local() {
        static thread_local local_entity_id_cache self;
        return self;
    }

    local_entity_id_cache::~local_entity_id_cache() {
        if (ids.size())
            manager::instance().bulk_release_ids(ids);
    }

    uint32_t local_entity_id_cache::get_next_id() {
        if (ids.empty()) {
            manager::instance().bulk_acquire_ids(ids, ID_CACHE_SIZE);
            if (ids.empty())
                return UINT32_MAX;
        }
        uint32_t id = ids.back();
        ids.pop_back();
        return id;
    }

    void local_entity_id_cache::recycle_id(uint32_t id) {
        ids.push_back(id);
        if (ids.size() >= ID_CACHE_SIZE * 2) {
            std::vector<uint32_t> to_flush;
            size_t flush_count = ID_CACHE_SIZE;
            to_flush.insert(to_flush.end(), ids.end() - flush_count, ids.end());
            ids.resize(ids.size() - flush_count);

            manager::instance().bulk_release_ids(to_flush);
        }
    }
}