/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_FILE_COMPRESSION
#define SRC_API_FILE_COMPRESSION
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <span>

namespace copper_server::api::file {
    enum class compression_level {
        high,
        medium,
        low,
        def
    };

    struct compressor{
        virtual ~compressor() = default;
        virtual std::vector<uint8_t> compress(const std::span<uint8_t>& data, compression_level level = compression_level::def) const = 0;
        virtual std::vector<uint8_t> decompress(const std::span<uint8_t>& data) const = 0;
    };

    //if the compressor already registered the function will return false
    bool register_compressor(const std::string& name, std::unique_ptr<compressor> compressor);

    //allows override
    void set_compressor(const std::string& name, std::unique_ptr<compressor> compressor);

    //The api automatically provides zlib, gzip, lz4_vanilla and none compressors
    //lz4_vanilla decompress limits up to 2MB, compress 2GB
    //all other compressors is limited to address space
    const compressor* get_compressor(const std::string& name);
}

#endif /* SRC_API_FILE_COMPRESSION */
