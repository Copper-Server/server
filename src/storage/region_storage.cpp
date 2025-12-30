/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/file/compression.hpp>
#include <src/storage/region_storage.hpp>
#include <src/util/endian.hpp>

namespace copper_server::storage {
    constexpr size_t SECTOR_SIZE = 4096;

    region_storage::region_storage() = default;
    region_storage::~region_storage() = default;

    void region_storage::remove_free_block(uint32_t offset, uint32_t size) {
        free_sectors_by_offset.erase(offset);
        auto& size_list = free_sectors_by_size.at(size);
        size_list.remove(offset);
        if (size_list.empty()) {
            free_sectors_by_size.erase(size);
        }
    }

    void region_storage::free_sectors(uint32_t offset, uint32_t size) {
        if (offset < 2 || size == 0)
            return;
        fast_task::lock_guard lock(allocation_mutex);

        uint32_t current_offset = offset;
        uint32_t current_size = size;

        auto next_it = free_sectors_by_offset.find(offset + size);
        if (next_it != free_sectors_by_offset.end()) {
            current_size += next_it->second;
            remove_free_block(next_it->first, next_it->second);
        }

        auto prev_it = free_sectors_by_offset.lower_bound(offset);
        if (prev_it != free_sectors_by_offset.begin()) {
            --prev_it;
            if (prev_it->first + prev_it->second == offset) {
                current_offset = prev_it->first;
                current_size += prev_it->second;
                remove_free_block(prev_it->first, prev_it->second);
            }
        }

        free_sectors_by_offset[current_offset] = current_size;
        free_sectors_by_size[current_size].push_back(current_offset);
    }

    uint32_t region_storage::allocate_sectors(uint8_t required_sectors) {
        fast_task::lock_guard lock(allocation_mutex);
        auto it = free_sectors_by_size.lower_bound(required_sectors);

        if (it == free_sectors_by_size.end()) {
            uint64_t file_size_sectors = (handle.size() + SECTOR_SIZE - 1) / SECTOR_SIZE;
            return std::max(2u, (uint32_t)file_size_sectors);
        }

        uint32_t block_size = it->first;
        auto& offset_list = it->second;
        uint32_t offset = offset_list.front();

        remove_free_block(offset, block_size);

        uint32_t remainder_size = block_size - required_sectors;
        if (remainder_size > 0) {
            uint32_t remainder_offset = offset + required_sectors;
            free_sectors(remainder_offset, remainder_size);
        }
        return offset;
    }

    void region_storage::build_free_space_cache() {
        fast_task::lock_guard lock(allocation_mutex);
        free_sectors_by_size.clear();
        free_sectors_by_offset.clear();

        std::vector<std::pair<uint32_t, uint32_t>> used_blocks;
        for (uint32_t loc : locations) {
            if (loc == 0)
                continue;
            uint32_t offset = loc >> 8;
            uint8_t size = loc & 0xFF;
            if (offset >= 2 && size > 0) {
                used_blocks.push_back({offset, offset + size});
            }
        }
        std::sort(used_blocks.begin(), used_blocks.end());

        uint32_t last_sector_end = 2;
        for (const auto& block : used_blocks) {
            if (block.first > last_sector_end) {
                free_sectors_by_offset[last_sector_end] = block.first - last_sector_end;
                free_sectors_by_size[block.first - last_sector_end].push_back(last_sector_end);
            }
            last_sector_end = block.second;
        }

        uint32_t file_size_sectors = uint32_t((handle.size() + SECTOR_SIZE - 1) / SECTOR_SIZE);
        if (file_size_sectors > last_sector_end) {
            free_sectors_by_offset[last_sector_end] = file_size_sectors - last_sector_end;
            free_sectors_by_size[file_size_sectors - last_sector_end].push_back(last_sector_end);
        }
    }

    std::shared_ptr<region_storage> region_storage::open(const std::filesystem::path& path) {
        std::shared_ptr<region_storage> instance;
        instance.reset(new region_storage{});
        try {
            instance->handle = fast_task::files::file_handle::open_throws(
                path,
                fast_task::files::open_mode::read_write,
                fast_task::files::on_open_action::open,
                fast_task::files::_async_flags{}
            );
        } catch (...) {
            instance->handle = fast_task::files::file_handle::open_throws(
                path,
                fast_task::files::open_mode::read_write,
                fast_task::files::on_open_action::create_new,
                fast_task::files::_async_flags{}
            );
        }

        if (!instance->handle.is_open())
            return nullptr;

        if (instance->handle.size() < HEADER_LOCATIONS_BYTES + HEADER_TIMESTAMPS_BYTES) {
            std::vector<uint8_t> empty_header(HEADER_LOCATIONS_BYTES + HEADER_TIMESTAMPS_BYTES, 0);
            instance->handle.write_inline_at(0, empty_header.data(), HEADER_LOCATIONS_BYTES + HEADER_TIMESTAMPS_BYTES);
            instance->handle.flush();
            instance->locations.fill(0);
            instance->timestamps.fill(0);
        } else {
            auto locations_data = instance->handle.read_fixed_at(0, HEADER_LOCATIONS_BYTES)->get();
            auto timestamps_data = instance->handle.read_fixed_at(HEADER_LOCATIONS_BYTES, HEADER_TIMESTAMPS_BYTES)->get();

            for (size_t i = 0; i < CHUNKS_PER_REGION; ++i) {
                memcpy(&instance->locations[i], locations_data.data() + i * 4, 4);
                memcpy(&instance->timestamps[i], timestamps_data.data() + i * 4, 4);
            }
            util::convert_endian_arr(std::endian::big, instance->locations.data(), CHUNKS_PER_REGION);
            util::convert_endian_arr(std::endian::big, instance->timestamps.data(), CHUNKS_PER_REGION);
        }

        instance->build_free_space_cache();
        return instance;
    }

    fast_task::future_ptr<std::vector<uint8_t>> region_storage::get_chunk_data(int32_t region_chunk_x, int32_t region_chunk_z) {
        int local_chunk_x = region_chunk_x & 31;
        int local_chunk_z = region_chunk_z & 31;
        size_t index = size_t(local_chunk_x + local_chunk_z * 32);
        if (index >= CHUNKS_PER_REGION)
            throw std::out_of_range("Local chunk coordinates out of range.");

        return fast_task::future<std::vector<uint8_t>>::start([self = shared_from_this(), index, region_chunk_x, region_chunk_z]() -> std::vector<uint8_t> {
            fast_task::read_lock lock(self->chunk_mutexes[index]);
            uint32_t location;
            {
                fast_task::lock_guard alloc_lock(self->allocation_mutex);
                location = self->locations[index];
            }

            uint32_t offset_sectors = location >> 8;
            uint8_t size_sectors = location & 0xFF;

            if (offset_sectors == 0 || size_sectors == 0)
                return {};

            uint64_t byte_offset = static_cast<uint64_t>(offset_sectors) * SECTOR_SIZE;

            auto chunk_header = self->handle.read_fixed_at(byte_offset, 5)->get();
            if (chunk_header.size() < 5)
                return {};

            uint32_t exact_length;
            memcpy(&exact_length, chunk_header.data(), 4);
            exact_length = util::convert_endian(std::endian::big, exact_length);

            if (exact_length == 0)
                return {};

            auto compression = static_cast<compression_type>(chunk_header[4] & 0x7F);

            std::vector<uint8_t> raw;
            bool is_external = (chunk_header[4] & 0x80) != 0;
            if (is_external) {
                auto mcc_path = self->file_path.parent_path() / ("c." + std::to_string(region_chunk_x) + "." + std::to_string(region_chunk_z) + ".mcc");
                if (!std::filesystem::exists(mcc_path))
                    return {};
                fast_task::files::async_iofstream mcc_handle(
                    mcc_path,
                    fast_task::files::open_mode::read,
                    fast_task::files::on_open_action::open_exists,
                    fast_task::files::_sync_flags{.sequential_scan = true}
                );
                if (!mcc_handle.is_open())
                    throw std::runtime_error("Failed to open the external file");
                mcc_handle.seekg(0, std::ios::end);
                size_t file_size = mcc_handle.tellg();
                mcc_handle.seekg(0, std::ios::beg);
                if (mcc_handle.bad())
                    throw std::runtime_error("Failed to read the external file");
                raw.resize(file_size);
                mcc_handle.read((char*)raw.data(), file_size);
            } else
                raw = self->handle.read_fixed_at(byte_offset + 5, exact_length - 1)->get();


            switch (compression) {
            case compression_type::gzip:
                return api::file::get_compressor("gzip")->decompress(raw);
            case compression_type::zlib:
                return api::file::get_compressor("zlib")->decompress(raw);
            case compression_type::none:
                return raw;
            case compression_type::lz4:
                return api::file::get_compressor("lz4_vanilla")->decompress(raw);
            case compression_type::custom: {
                if (raw.size() < 2)
                    throw std::runtime_error("Custom compression missing ID");
                uint16_t id_len = (uint16_t)raw[0] << 8 | raw[1];
                if (raw.size() < 2 + id_len)
                    throw std::runtime_error("Custom compression ID corrupt");
                std::string id(raw.begin() + 2, raw.begin() + 2 + id_len);
                auto compressor = api::file::get_compressor(id);
                if (!compressor)
                    throw std::runtime_error("Unknown custom compressor: " + id);
                return compressor->decompress({raw.begin() + 2 + id_len, raw.end()});
            }
            default:
                throw std::runtime_error("Unsupported chunk compression type: " + std::to_string(static_cast<uint8_t>(compression)));
            }
        });
    }

    fast_task::future_ptr<void> region_storage::write_chunk_data(int32_t region_chunk_x, int32_t region_chunk_z, std::vector<uint8_t>&& data, compression_type compression, bool use_external_file, const std::string& custom_id) {
        int32_t local_chunk_x = region_chunk_x & 31;
        int32_t local_chunk_z = region_chunk_z & 31;
        size_t index = size_t(local_chunk_x + local_chunk_z * 32);
        if (index >= CHUNKS_PER_REGION)
            throw std::out_of_range("Local chunk coordinates out of range.");
        if (!custom_id.empty())
            compression = compression_type::custom;

        return fast_task::future<void>::start([self = shared_from_this(), index, data = std::move(data), compression, region_chunk_x, region_chunk_z, use_external_file, custom_id]() mutable {
            fast_task::write_lock lock(self->chunk_mutexes[index]);

            switch (compression) {
            case compression_type::gzip:
                data = api::file::get_compressor("gzip")->compress(data);
                break;
            case compression_type::zlib:
                data = api::file::get_compressor("zlib")->compress(data);
                break;
            case compression_type::lz4:
                data = api::file::get_compressor("lz4_vanilla")->compress(data);
                break;
            case compression_type::custom: {
                auto compressor = api::file::get_compressor(custom_id);
                if (!compressor)
                    throw std::runtime_error("Unknown custom compressor: " + custom_id);
                data = compressor->compress(data);
                break;
            }
            case compression_type::none:
            default:
                compression = compression_type::none;
                break;
            }


            uint8_t final_compression_byte = static_cast<uint8_t>(compression) | (use_external_file ? 0x80 : 0);

            if (use_external_file) {
                auto mcc_path = self->file_path.parent_path() / ("c." + std::to_string(region_chunk_x) + "." + std::to_string(region_chunk_z) + ".mcc");
                fast_task::files::async_iofstream mcc_handle(
                    mcc_path,
                    fast_task::files::open_mode::write,
                    fast_task::files::on_open_action::always_new,
                    fast_task::files::_sync_flags{.write_through = true}
                );
                if (!mcc_handle.is_open())
                    throw std::runtime_error("Failed to open the external file");

                mcc_handle.write((const char*)data.data(), (uint32_t)data.size());
                mcc_handle.flush();
                if (mcc_handle.bad())
                    throw std::runtime_error("Failed to write the external file");
            } else if (data.size() > UINT32_MAX)
                throw std::runtime_error("Data too large to store in region file");

            std::vector<uint8_t> file_buffer;
            uint32_t mca_length = 1 + uint32_t(use_external_file ? 0 : data.size());
            uint32_t length_nbo = util::convert_endian(std::endian::big, mca_length);

            file_buffer.resize(5 + (use_external_file ? 0 : data.size()));
            memcpy(file_buffer.data(), &length_nbo, 4);
            file_buffer[4] = final_compression_byte;
            if (!use_external_file)
                memcpy(file_buffer.data() + 5, data.data(), data.size());

            uint8_t required_sectors = uint8_t((file_buffer.size() + SECTOR_SIZE - 1) / SECTOR_SIZE);

            uint32_t write_offset_sectors;
            {
                fast_task::lock_guard alloc_lock(self->allocation_mutex);
                uint32_t old_location = self->locations[index];
                uint8_t old_size_sectors = old_location & 0xFF;

                if (required_sectors == old_size_sectors) {
                    write_offset_sectors = old_location >> 8;
                } else {
                    if (old_size_sectors > 0)
                        self->free_sectors(old_location >> 8, old_size_sectors);
                    write_offset_sectors = self->allocate_sectors(required_sectors);
                }
            }

            self->handle.write_inline_at(static_cast<uint64_t>(write_offset_sectors) * SECTOR_SIZE, file_buffer.data(), (uint32_t)file_buffer.size());

            self->handle.flush();

            {
                fast_task::lock_guard alloc_lock(self->allocation_mutex);
                uint32_t new_location = (write_offset_sectors << 8) | required_sectors;
                uint32_t current_timestamp = static_cast<uint32_t>(std::time(nullptr));

                self->locations[index] = new_location;
                self->timestamps[index] = current_timestamp;

                uint32_t location_nbo = util::convert_endian(std::endian::big, new_location);
                self->handle.write_inline_at(index * 4, reinterpret_cast<uint8_t*>(&location_nbo), 4);
                uint32_t timestamp_nbo = util::convert_endian(std::endian::big, current_timestamp);
                self->handle.write_inline_at(HEADER_LOCATIONS_BYTES + index * 4, reinterpret_cast<uint8_t*>(&timestamp_nbo), 4);
            }
            self->handle.flush();
        });
    }

    fast_task::future_ptr<void> region_storage::write_chunk_data(int32_t region_chunk_x, int32_t region_chunk_z, std::vector<uint8_t>&& data, const std::string& custom_id, bool use_external_file) {
        return write_chunk_data(region_chunk_x, region_chunk_z, std::move(data), compression_type::custom, use_external_file, custom_id);
    }
}