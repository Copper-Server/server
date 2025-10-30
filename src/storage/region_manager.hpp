/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_STORAGE_REGION_MANAGER
#define SRC_STORAGE_REGION_MANAGER
#include <src/storage/region_storage.hpp>

namespace copper_server::storage {
    class region_manager {
    private:
        std::filesystem::path region_dir_path;
        fast_task::task_mutex cache_mutex;

        struct region_coords {
            int32_t x, z;

            bool operator==(const region_coords& other) const;
        };

        struct region_coords_hash {
            std::size_t operator()(const region_coords& c) const noexcept;
        };

        std::unordered_map<region_coords, std::shared_ptr<region_storage>, region_coords_hash> region_cache;

        std::shared_ptr<region_storage> get_region_file(int32_t region_x, int32_t region_z);

    public:
        explicit region_manager(const std::filesystem::path& region_path);

        fast_task::future_ptr<std::vector<uint8_t>> get_chunk(int32_t chunk_x, int32_t chunk_z);

        fast_task::future_ptr<void> write_chunk(int32_t chunk_x, int32_t chunk_z, std::vector<uint8_t>&& chunk, region_storage::compression_type type = region_storage::compression_type::zlib, bool use_external_file = false);
        fast_task::future_ptr<void> write_chunk(int32_t chunk_x, int32_t chunk_z, std::vector<uint8_t>&& chunk, const std::string& type, bool use_external_file = false);
    };
}
#endif /* SRC_STORAGE_REGION_MANAGER */
