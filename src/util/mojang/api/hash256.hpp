/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTIL_MOJANG_API_HASH256
#define SRC_UTIL_MOJANG_API_HASH256
#include <library/list_array.hpp>
#include <string>
#include <vector>

typedef struct evp_md_ctx_st EVP_MD_CTX;

namespace copper_server::util::mojang::api {
    class hash256 {
        EVP_MD_CTX* ctx;
        static constexpr size_t DIGEST_LENGTH = 32; // SHA1 digest length
        using hashed_array = unsigned char[DIGEST_LENGTH];

        static void two_complement(hashed_array& data);
        static void to_hex(hashed_array data, char* hex);

    public:
        hash256();
        ~hash256();
        void update(const void* data, size_t size);
        uint64_t to_part_hash();
    };
}

#endif /* SRC_UTIL_MOJANG_API_HASH256 */
