#include <src/base_objects/encryption/aes.hpp>

namespace copper_server::encryption {
    bool aes::initialize(const list_array<uint8_t>& key, const list_array<uint8_t>& iv) {
        if (key.size() != 16 || iv.size() != 16)
            return false;
        if (AES_set_encrypt_key(key.data(), 128, &enc_key) != 0)
            return false;
        if (AES_set_decrypt_key(key.data(), 128, &dec_key) != 0)
            return false;
        memcpy(iv_data, iv.data(), 8);
        return true;
    }

    void aes::encrypt(const list_array<uint8_t>& data, list_array<uint8_t>& out) {
        out.resize(data.size());
        AES_cfb8_encrypt(data.data(), out.data(), data.size(), &enc_key, iv_data, &num, AES_ENCRYPT);
    }

    void aes::decrypt(const list_array<uint8_t>& data, list_array<uint8_t>& out) {
        out.resize(data.size());
        AES_cfb8_encrypt(data.data(), out.data(), data.size(), &dec_key, iv_data, &num, AES_DECRYPT);
    }
}