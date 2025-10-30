/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/storage/region_manager.hpp>

namespace copper_server::storage {

    bool region_manager::region_coords::operator==(const region_manager::region_coords& other) const {
        return x == other.x && z == other.z;
    }

    std::size_t region_manager::region_coords_hash::operator()(const region_manager::region_coords& c) const noexcept {
        return std::hash<int>()(c.x) ^ (std::hash<int>()(c.z) << 1);
    }

    std::shared_ptr<region_storage> region_manager::get_region_file(int32_t region_x, int32_t region_z) {
        region_coords coords = {region_x, region_z};
        fast_task::lock_guard lock(this->cache_mutex);
        if (auto it = this->region_cache.find(coords); it != this->region_cache.end())
            return it->second;

        auto file_path = region_dir_path / ("r." + std::to_string(region_x) + "." + std::to_string(region_z) + ".mca");
        auto region_file = region_storage::open(file_path);
        this->region_cache[coords] = region_file;
        return region_file;
    }

    region_manager::region_manager(const std::filesystem::path& region_path) {
        region_dir_path = region_path;
        if (!std::filesystem::exists(region_dir_path))
            std::filesystem::create_directories(region_dir_path);
    }

    fast_task::future_ptr<std::vector<uint8_t>> region_manager::get_chunk(int32_t chunk_x, int32_t chunk_z) {
        int32_t region_x = chunk_x >> 5;
        int32_t region_z = chunk_z >> 5;

        return fast_task::future<std::vector<uint8_t>>::start([=]() -> std::vector<uint8_t> {
            auto region_file = get_region_file(region_x, region_z);
            if (!region_file)
                return {};

            return region_file->get_chunk_data(chunk_x, chunk_z)->get();
        });
    }

    fast_task::future_ptr<void> region_manager::write_chunk(int32_t chunk_x, int32_t chunk_z, std::vector<uint8_t>&& chunk, region_storage::compression_type type, bool use_external_file) {
        int32_t region_x = chunk_x >> 5;
        int32_t region_z = chunk_z >> 5;

        return fast_task::future<void>::start([=, chunk = std::move(chunk)]() mutable {
            auto region_file = get_region_file(region_x, region_z);
            if (!region_file)
                throw std::runtime_error("Failed to open region file for writing.");

            region_file->write_chunk_data(chunk_x, chunk_z, std::move(chunk), type, use_external_file)->wait();
        });
    }

    fast_task::future_ptr<void> region_manager::write_chunk(int32_t chunk_x, int32_t chunk_z, std::vector<uint8_t>&& chunk, const std::string& type, bool use_external_file) {
        int32_t region_x = chunk_x >> 5;
        int32_t region_z = chunk_z >> 5;

        return fast_task::future<void>::start([=, chunk = std::move(chunk)]() mutable {
            auto region_file = get_region_file(region_x, region_z);
            if (!region_file)
                throw std::runtime_error("Failed to open region file for writing.");

            region_file->write_chunk_data(chunk_x, chunk_z, std::move(chunk), type, use_external_file)->wait();
        });
    }
}