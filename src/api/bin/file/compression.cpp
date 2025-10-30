/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <library/fast_task/include/task.hpp>
#include <library/list_array.hpp>
#include <mutex>
#include <src / api / file / compression.hpp>

#include <zlib.h>
#include <lz4.h>

namespace copper_server::api::file {
    constexpr size_t STREAM_BUFFER_SIZE = 128 * 1024; // 128 KB

    struct zlib_compressor : public compressor{
        zlib_compressor() = default;
        virtual ~zlib_compressor() = default;

        std::vector<uint8_t> compress(const std::span<uint8_t>& data, compression_level level) const override {
            if (data.empty())
                return {};
            if (data.size() > UINT32_MAX)
                throw std::runtime_error("Data too large to compress");


            z_stream strm = {};
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            int comp_level = 1;

            switch (level) {
            case compression_level::high:
                comp_level = 9;
            case compression_level::medium:
                comp_level = 6;
            case compression_level::low:
                comp_level = 3;

            case compression_level::def:
            default:
                comp_level = Z_DEFAULT_COMPRESSION;
            }

            if (deflateInit2(&strm, comp_level, Z_DEFLATED, 15, 8, Z_DEFAULT_STRATEGY) != Z_OK)
                throw std::runtime_error("Failed to initialize zlib deflate");

            list_array<uint8_t> out_data;
            std::vector<uint8_t> out_buffer(STREAM_BUFFER_SIZE);
            size_t input_consumed = 0;

            do {
                size_t chunk_size = std::min(data.size() - input_consumed, STREAM_BUFFER_SIZE);
                strm.avail_in = (uint32_t)chunk_size;
                strm.next_in = (Bytef*)data.data() + input_consumed;
                input_consumed += chunk_size;

                int flush = (input_consumed == data.size()) ? Z_FINISH : Z_NO_FLUSH;

                do {
                    strm.avail_out = STREAM_BUFFER_SIZE;
                    strm.next_out = out_buffer.data();
                    if (deflate(&strm, flush) == Z_STREAM_ERROR) {
                        deflateEnd(&strm);
                        throw std::runtime_error("Zlib compression failed");
                    }
                    size_t have = out_buffer.size() - strm.avail_out;
                    out_data.push_back(out_buffer.data(), have);
                } while (strm.avail_out == 0);
            } while (input_consumed < data.size());

            deflateEnd(&strm);
            return out_data.to_container<std::vector>();
        }

        std::vector<uint8_t> decompress(const std::span<uint8_t>& data) const override {
            if (data.empty())
                return {};

            if (data.size() > UINT32_MAX)
                throw std::runtime_error("Data too large to decompress");

            z_stream strm = {};
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            strm.avail_in = (uint32_t)data.size() > UINT32_MAX ? UINT32_MAX : (uint32_t)data.size();
            strm.next_in = (Bytef*)data.data();

            if (inflateInit2(&strm, 15) != Z_OK)
                throw std::runtime_error("Failed to initialize zlib inflate");

            list_array<uint8_t> out_data;
            std::vector<uint8_t> buffer(STREAM_BUFFER_SIZE);

            int ret;
            do {
                strm.avail_out = STREAM_BUFFER_SIZE;
                strm.next_out = buffer.data();
                ret = inflate(&strm, Z_NO_FLUSH);
                if (ret != Z_OK && ret != Z_STREAM_END) {
                    inflateEnd(&strm);
                    throw std::runtime_error("Zlib decompression failed with error code: " + std::to_string(ret));
                }
                size_t have = buffer.size() - strm.avail_out;
                out_data.push_back(buffer.data(), have);
            } while (ret != Z_STREAM_END);

            inflateEnd(&strm);
            return out_data.to_container<std::vector>();
        }
    };

    struct gzip_compressor : public compressor {
        gzip_compressor() = default;
        virtual ~gzip_compressor() = default;

        std::vector<uint8_t> compress(const std::span<uint8_t>& data, compression_level level) const override {
            if (data.empty())
                return {};

            z_stream strm = {};
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            int comp_level = 1;

            switch (level) {
            case compression_level::high:
                comp_level = 9;
            case compression_level::medium:
                comp_level = 6;
            case compression_level::low:
                comp_level = 3;

            case compression_level::def:
            default:
                comp_level = Z_DEFAULT_COMPRESSION;
            }

            if (deflateInit2(&strm, comp_level, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
                throw std::runtime_error("Failed to initialize zlib deflate");

            list_array<uint8_t> out_data;
            std::vector<uint8_t> out_buffer(STREAM_BUFFER_SIZE);
            size_t input_consumed = 0;

            do {
                size_t chunk_size = std::min(data.size() - input_consumed, STREAM_BUFFER_SIZE);
                strm.avail_in = (uint32_t)chunk_size;
                strm.next_in = (Bytef*)data.data() + input_consumed;
                input_consumed += chunk_size;

                int flush = (input_consumed == data.size()) ? Z_FINISH : Z_NO_FLUSH;

                do {
                    strm.avail_out = STREAM_BUFFER_SIZE;
                    strm.next_out = out_buffer.data();
                    if (deflate(&strm, flush) == Z_STREAM_ERROR) {
                        deflateEnd(&strm);
                        throw std::runtime_error("Zlib compression failed");
                    }
                    size_t have = out_buffer.size() - strm.avail_out;
                    out_data.push_back(out_buffer.data(), have);
                } while (strm.avail_out == 0);
            } while (input_consumed < data.size());

            deflateEnd(&strm);
            return out_data.to_container<std::vector>();

        }

        std::vector<uint8_t> decompress(const std::span<uint8_t>& data) const override {
            if (data.empty())
                return {};

            z_stream strm = {};
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            strm.avail_in = (uint32_t)data.size() > UINT32_MAX ? UINT32_MAX : (uint32_t)data.size();
            strm.next_in = (Bytef*)data.data();

            if (inflateInit2(&strm, 15 + 16) != Z_OK) 
                throw std::runtime_error("Failed to initialize zlib inflate");

            list_array<uint8_t> out_data;
            std::vector<uint8_t> buffer(STREAM_BUFFER_SIZE);

            int ret;
            do {
                strm.avail_out = STREAM_BUFFER_SIZE;
                strm.next_out = buffer.data();
                ret = inflate(&strm, Z_NO_FLUSH);
                if (ret != Z_OK && ret != Z_STREAM_END) {
                    inflateEnd(&strm);
                    throw std::runtime_error("Zlib decompression failed with error code: " + std::to_string(ret));
                }
                size_t have = buffer.size() - strm.avail_out;
                out_data.push_back(buffer.data(), have);
            } while (ret != Z_STREAM_END);

            inflateEnd(&strm);
            return out_data.to_container<std::vector>();
        }
    };

    constexpr int VANILLA_MAX_CHUNK_SIZE = 2 * 1024 * 1024; // 2 MiB
    struct lz4_vanilla_compressor : public compressor {
        lz4_vanilla_compressor() = default;
        virtual ~lz4_vanilla_compressor() = default;

        std::vector<uint8_t> compress(const std::span<uint8_t>& data, compression_level level) const override {
            if (data.empty())
                return {};
            if (data.size() > INT32_MAX)
                throw std::runtime_error("Data too large to compress");

            int acceleration = 1;

            switch (level) {
            case compression_level::high:
                acceleration = 1;
            case compression_level::medium:
                acceleration = /*LZ4_ACCELERATION_MAX*/ 65537 / 8;
            case compression_level::low:
                acceleration = /*LZ4_ACCELERATION_MAX*/ 65537 / 4;

            case compression_level::def:
            default:
                acceleration = 1;
                break;
            }
            int max_compressed_size = LZ4_compressBound((int32_t)data.size());
            std::vector<uint8_t> out_data(max_compressed_size);

            int compressed_size = LZ4_compress_fast(
                (const char*)data.data(),
                (char*)out_data.data(),
                (int32_t)data.size(),
                max_compressed_size,
                acceleration
            );

            if (compressed_size <= 0)
                throw std::runtime_error("LZ4 compression failed");

            out_data.resize(compressed_size);
            return out_data;

        }

        std::vector<uint8_t> decompress(const std::span<uint8_t>& data) const override {
            if (data.size() > INT32_MAX)
                throw std::runtime_error("Data too large to compress");

            std::vector<uint8_t> out_data(VANILLA_MAX_CHUNK_SIZE);

            int decompressed_size = LZ4_decompress_safe(
                (const char*)data.data() + 4,
                (char*)out_data.data(),
                int32_t(data.size() - 4),
                VANILLA_MAX_CHUNK_SIZE
            );

            if (decompressed_size < 0)
                throw std::runtime_error("LZ4 decompression failed");


            out_data.resize(decompressed_size);
            return out_data;
        }
    };

    struct none_compressor : public compressor {
        none_compressor() = default;
        virtual ~none_compressor() = default;

        std::vector<uint8_t> compress(const std::span<uint8_t>& data, compression_level level) const override {
            return std::vector<uint8_t>(data.begin(), data.end());
        }

        std::vector<uint8_t> decompress(const std::span<uint8_t>& data) const override {
            return std::vector<uint8_t>(data.begin(), data.end());
        }
    };

    fast_task::protected_value<std::unordered_map<std::string, std::unique_ptr<compressor>>>& get_registry() {
        static std::once_flag once;
        static fast_task::protected_value<std::unordered_map<std::string, std::unique_ptr<compressor>>> registry;
        std::call_once(once, []() {
            registry.set([](auto& reg) {
                reg.emplace("zlib", std::make_unique<zlib_compressor>());
                reg.emplace("gzip", std::make_unique<gzip_compressor>());
                reg.emplace("lz4_vanilla", std::make_unique<lz4_vanilla_compressor>());
                reg.emplace("none", std::make_unique<none_compressor>());
            });
        });

        return registry;
    }

    bool register_compressor(const std::string& name, std::unique_ptr<compressor> compressor) {
        return get_registry().set([&](auto& reg) {
            return reg.try_emplace(name, std::move(compressor)).second;
        });
    }

    void set_compressor(const std::string& name, std::unique_ptr<compressor> compressor) {
        return get_registry().set([&](auto& reg) {
            reg[name] = std::move(compressor);
        });
    }

    const compressor* get_compressor(const std::string& name) {
        return get_registry().get([&](auto& reg) -> const compressor* {
            auto it = reg.find(name);
            if (it == reg.end())
                return nullptr;
            return it->second.get();
        });
    }
}
