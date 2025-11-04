/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_STORAGE_ANVIL
#define SRC_STORAGE_ANVIL
#include <src/storage/region_manager.hpp>
#include <src/base_objects/world/chunk.hpp>

namespace copper_server::storage {
    class anvil {
        region_manager manager;
    public:
        explicit anvil(const std::filesystem::path& region_path);

        fast_task::future_ptr<std::shared_ptr<base_objects::world::chunk_data>> get_chunk(int32_t chunk_x, int32_t chunk_z);

        fast_task::future_ptr<void> write_chunk(int32_t chunk_x, int32_t chunk_z, std::shared_ptr<base_objects::world::chunk_data> chunk, region_storage::compression_type type = region_storage::compression_type::zlib, bool use_external_file = false);
        fast_task::future_ptr<void> write_chunk(int32_t chunk_x, int32_t chunk_z, std::shared_ptr<base_objects::world::chunk_data> chunk, const std::string& type, bool use_external_file = false);
    };
}

#endif /* SRC_STORAGE_ANVIL */
