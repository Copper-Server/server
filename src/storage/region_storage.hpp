/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_STORAGE_REGION_STORAGE
#define SRC_STORAGE_REGION_STORAGE
#include <array>
#include <boost/container/flat_map.hpp>
#include <library/fast_task/include/files.hpp>
#include <memory>

namespace copper_server::storage {
    class region_storage : public std::enable_shared_from_this<region_storage> {
    private:
        static constexpr inline size_t HEADER_LOCATIONS_BYTES = 4096;
        static constexpr inline size_t HEADER_TIMESTAMPS_BYTES = 4096;
        static constexpr inline size_t CHUNKS_PER_REGION = 32 * 32;
        fast_task::files::file_handle handle;
        std::filesystem::path file_path;
        std::array<fast_task::task_rw_mutex, CHUNKS_PER_REGION> chunk_mutexes;
        fast_task::task_mutex allocation_mutex;

        std::array<uint32_t, CHUNKS_PER_REGION> locations;
        std::array<uint32_t, CHUNKS_PER_REGION> timestamps;

        boost::container::flat_map<uint32_t, std::list<uint32_t>> free_sectors_by_size;
        boost::container::flat_map<uint32_t, uint32_t> free_sectors_by_offset;

        explicit region_storage();

        void remove_free_block(uint32_t offset, uint32_t size);
        void free_sectors(uint32_t offset, uint32_t size);
        uint32_t allocate_sectors(uint8_t required_sectors);
        void build_free_space_cache();

    public:
        static std::shared_ptr<region_storage> open(const std::filesystem::path& path);
        ~region_storage();

        enum class compression_type {
            gzip = 1,
            zlib = 2,
            none = 3,
            lz4 = 4,

            custom = 127
        };

        fast_task::future_ptr<std::vector<uint8_t>> get_chunk_data(int32_t region_chunk_x, int32_t region_chunk_z);

        fast_task::future_ptr<void> write_chunk_data(int32_t region_chunk_x, int32_t region_chunk_z, std::vector<uint8_t>&& data, compression_type compression = compression_type::zlib, bool use_external_file = false , const std::string& custom_id = "");
        fast_task::future_ptr<void> write_chunk_data(int32_t region_chunk_x, int32_t region_chunk_z, std::vector<uint8_t>&& data, const std::string& custom_id, bool use_external_file = false);
    };
}
#endif /* SRC_STORAGE_REGION_STORAGE */
