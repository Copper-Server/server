/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_MOJANG_API_HASH
#define SRC_MOJANG_API_HASH
#include <library/list_array.hpp>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string>
#include <vector>

namespace mojang::api {
    class hash {
        EVP_MD_CTX* ctx;
        static constexpr size_t DIGEST_LENGTH = 20; // SHA1 digest length
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
        hash() {
            ctx = EVP_MD_CTX_new();
            EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr);
        }

        ~hash() {
            if (ctx)
                EVP_MD_CTX_free(ctx);
        }
        void update(const std::string& data) {
            EVP_DigestUpdate(ctx, data.c_str(), data.size());
        }
        void update(const void* data, size_t size) {
            EVP_DigestUpdate(ctx, data, size);
        }
        template <typename T>
        void update(const list_array<T>& data) {
            EVP_DigestUpdate(ctx, data.data(), data.size() * sizeof(T));
        }
        std::string hexdigest() {
            hashed_array md;
            unsigned int md_len = 0;
            EVP_DigestFinal_ex(ctx, md, &md_len);
            EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr);
            bool negative = (md[0] & 0x80) == 0x80;
            if (negative)
                two_complement(md);
            constexpr size_t result_size = DIGEST_LENGTH * 2 + 1;
            char hex[result_size + 1];
            to_hex(md, hex + 1);
            std::string_view view(hex, result_size);
            if (negative) {
                hex[0] = '-';
            } else
                view.remove_prefix(1);
            while (view[0] == '0') {
                view.remove_prefix(1);
                if (view.empty())
                    return "0";
            }
            return std::string(view);
        }
    };
}
#endif /* SRC_MOJANG_API_HASH */
