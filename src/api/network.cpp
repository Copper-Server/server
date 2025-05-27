
#include <library/fast_task/src/networking/networking.hpp>
#include <src/api/configuration.hpp>
#include <src/base_objects/events/sync_event.hpp>
#include <src/log.hpp>

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <span>
#define OPENSSL_CHECK(OPERATION, console_output) \
    if ((OPERATION) <= 0) {                      \
        log::fatal("OpenSSL", console_output);   \
    }
#define NOT_NULL(X, console_output)            \
    if (!(X)) {                                \
        log::fatal("OpenSSL", console_output); \
    }

namespace copper_server::api::network {
    base_objects::events::sync_event<const fast_task::networking::address&> ip_filter;

    namespace tcp {
        RSA* server_rsa_key = nullptr;
        list_array<uint8_t> server_private_key; //PEM
        list_array<uint8_t> server_public_key;  //PEM

        void init_ssl() {
            auto ssl_key_length = api::configuration::get().server.ssl_key_length;
            if (!server_rsa_key) {
                server_rsa_key = RSA_generate_key(ssl_key_length, RSA_F4, nullptr, nullptr);
                NOT_NULL(server_rsa_key, "Failed to generate RSA key");
                BIO* bio = BIO_new(BIO_s_mem());
                NOT_NULL(bio, "Failed to create BIO");
                OPENSSL_CHECK(PEM_write_bio_RSAPrivateKey(bio, server_rsa_key, nullptr, nullptr, 0, nullptr, nullptr), "Failed to write RSA private key");
                server_private_key.resize(BIO_pending(bio));
                OPENSSL_CHECK(BIO_read(bio, server_private_key.data(), server_private_key.size()), "Failed to read RSA private key");
                OPENSSL_CHECK(BIO_free(bio), "Failed to free BIO");
                { //public key in DEM format
                    uint8_t* der = nullptr;
                    int32_t len = i2d_RSA_PUBKEY(server_rsa_key, &der);
                    if (len < 0)
                        throw std::runtime_error("Failed to convert RSA key to DER format");
                    NOT_NULL(der, "Failed to convert RSA key to DER format");
                    server_public_key = list_array<uint8_t>(der, len);
                    OPENSSL_free(der);
                }
            }
        }

        bool decrypt_data(list_array<uint8_t>& data) {
            init_ssl();
            if (data.size() > INT32_MAX)
                return false;
            if (!server_rsa_key)
                return false;
            int result = RSA_private_decrypt(data.size(), data.data(), data.data(), server_rsa_key, RSA_PKCS1_PADDING);
            if (result < 0)
                return false;
            data.resize(result);
            return true;
        }

        bool encrypt_data(list_array<uint8_t>& data) {
            if (data.size() > INT32_MAX)
                return false;
            if (!server_rsa_key)
                return false;
            int result = RSA_public_encrypt(data.size(), data.data(), data.data(), server_rsa_key, RSA_PKCS1_PADDING);
            if (result < 0)
                return false;
            data.resize(result);
            return true;
        }

        std::span<uint8_t> private_key_buffer() {
            return {server_private_key.data(), server_private_key.size()};
        }

        std::span<uint8_t> public_key_buffer() {
            return {server_public_key.data(), server_public_key.size()};
        }
    }
}