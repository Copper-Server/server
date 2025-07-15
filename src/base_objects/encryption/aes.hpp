#ifndef SRC_BASE_OBJECTS_ENCRYPTION_AES
#define SRC_BASE_OBJECTS_ENCRYPTION_AES
#include <cstdint>
#include <library/list_array.hpp>
#include <openssl/aes.h>
#include <openssl/evp.h>

namespace copper_server::encryption {
    class aes {
        aes(const aes&) = delete;
        aes& operator=(const aes&) = delete;

    public:
        aes() = default;

        bool initialize(const list_array<uint8_t>& key, const list_array<uint8_t>& iv);
        void encrypt(const list_array<uint8_t>& data, list_array<uint8_t>& out);
        void decrypt(const list_array<uint8_t>& data, list_array<uint8_t>& out);

    private:
        EVP_CIPHER_CTX* enc_ctx = nullptr;
        EVP_CIPHER_CTX* dec_ctx = nullptr;
    };
}

#endif /* SRC_BASE_OBJECTS_ENCRYPTION_AES */
