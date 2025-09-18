/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_MOJANG_API_HASH
#define SRC_UTIL_MOJANG_API_HASH
#include <library/list_array.hpp>
#include <string>
#include <vector>

typedef struct evp_md_ctx_st EVP_MD_CTX;

namespace copper_server::util::mojang::api {
    class hash {
        EVP_MD_CTX* ctx;
        static constexpr size_t DIGEST_LENGTH = 20; // SHA1 digest length
        using hashed_array = unsigned char[DIGEST_LENGTH];

        static void two_complement(hashed_array& data);

        static void to_hex(hashed_array data, char* hex);

    public:
        hash();

        ~hash();

        void update(const std::string& data);
        void update(const void* data, size_t size);

        template <typename T>
        void update(const list_array<T>& data) {
            update(data.data(), data.size() * sizeof(T));
        }

        std::string hexdigest();
    };
}
#endif /* SRC_UTIL_MOJANG_API_HASH */
