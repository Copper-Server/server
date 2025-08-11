/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/base_objects/encryption/aes.hpp>

namespace copper_server::encryption {
    bool aes::initialize(const list_array<uint8_t>& key, const list_array<uint8_t>& iv) {
        if (key.size() != 16 || iv.size() != 16)
            return false;
        if (enc_ctx)
            EVP_CIPHER_CTX_free(enc_ctx);
        if (dec_ctx)
            EVP_CIPHER_CTX_free(dec_ctx);
        enc_ctx = EVP_CIPHER_CTX_new();
        dec_ctx = EVP_CIPHER_CTX_new();
        if (!enc_ctx || !dec_ctx)
            return false;
        if (EVP_EncryptInit_ex(enc_ctx, EVP_aes_128_cfb8(), nullptr, key.data(), iv.data()) != 1)
            return false;
        if (EVP_DecryptInit_ex(dec_ctx, EVP_aes_128_cfb8(), nullptr, key.data(), iv.data()) != 1)
            return false;
        return true;
    }

    void aes::encrypt(const list_array<uint8_t>& data, list_array<uint8_t>& out) {
        if (!enc_ctx)
            return;
        out.resize(data.size());
        int outlen = 0;
        if (EVP_EncryptUpdate(enc_ctx, out.data(), &outlen, data.data(), (int)data.size()) != 1) {
            out.clear();
            return;
        }
        out.resize(outlen);
    }

    void aes::decrypt(const list_array<uint8_t>& data, list_array<uint8_t>& out) {
        if (!dec_ctx)
            return;
        out.resize(data.size());
        int outlen = 0;
        if (EVP_DecryptUpdate(dec_ctx, out.data(), &outlen, data.data(), (int)data.size()) != 1) {
            out.clear();
            return;
        }
        out.resize(outlen);
    }
}