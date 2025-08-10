/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_MOJANG_API_HASH256
#define SRC_MOJANG_API_HASH256
#include <library/list_array.hpp>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string>
#include <vector>

namespace mojang::api {
    class hash256 {
        EVP_MD_CTX* ctx;
        static constexpr size_t DIGEST_LENGTH = 32; // SHA1 digest length
        using hashed_array = unsigned char[DIGEST_LENGTH];

        static void two_complement(hashed_array& data) {
            bool carry = true;
            for (int i = DIGEST_LENGTH - 1; i >= 0; i--) {
                data[i] = ~data[i];
                if (carry) {
                    carry = data[i] == 0xff;
                    data[i]++;
                }
            }
        }

        static void to_hex(hashed_array data, char* hex) {
            constexpr char hex_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
            for (int i = 0; i < DIGEST_LENGTH; i++) {
                hex[i * 2] = hex_map[(data[i] & 0xf0) >> 4];
                hex[i * 2 + 1] = hex_map[data[i] & 0x0f];
            }
        }

    public:
        hash256() {
            ctx = EVP_MD_CTX_new();
            EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
        }

        ~hash256() {
            if (ctx)
                EVP_MD_CTX_free(ctx);
        }

        void update(const void* data, size_t size) {
            EVP_DigestUpdate(ctx, data, size);
        }

        uint64_t to_part_hash() {
            hashed_array md;
            unsigned int md_len = 0;
            EVP_DigestFinal_ex(ctx, md, &md_len);
            EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
            bool negative = (md[0] & 0x80) == 0x80;
            if (negative)
                two_complement(md);
            return reinterpret_cast<uint64_t&>(md);
        }
    };
}
#endif /* SRC_MOJANG_API_HASH256 */
