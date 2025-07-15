
#include <library/fast_task/src/networking/networking.hpp>
#include <src/api/configuration.hpp>
#include <src/base_objects/events/sync_event.hpp>
#include <src/log.hpp>

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <span>
#define OPENSSL_CHECK(OPERATION, console_output)  \
    if ((OPERATION) <= 0) {                       \
        log::error("OpenSSL", console_output);    \
        throw std::runtime_error(console_output); \
    }
#define NOT_NULL(X, console_output)               \
    if (!(X)) {                                   \
        log::error("OpenSSL", console_output);    \
        throw std::runtime_error(console_output); \
    }
#define IS_CORRECT(X, console_output)             \
    if (!(X)) {                                   \
        log::error("OpenSSL", console_output);    \
        throw std::runtime_error(console_output); \
    }

namespace copper_server::api::network {
    base_objects::events::sync_event<const fast_task::networking::address&> ip_filter;

    namespace tcp {
        EVP_PKEY* server_key = nullptr;
        list_array<uint8_t> server_private_key; //PEM
        list_array<uint8_t> server_public_key;  //DER

        void init_ssl() {
            auto ssl_key_length = api::configuration::get().server.ssl_key_length;
            IS_CORRECT(ssl_key_length <= INT32_MAX, "Invalid configuration, server.ssl_key_length is too large. int32_t max");
            IS_CORRECT(ssl_key_length && !(ssl_key_length & (ssl_key_length - 1)), "Invalid configuration, server.ssl_key_length is too large. int32_t max");
            IS_CORRECT(ssl_key_length >= 1024, "Invalid configuration, server.ssl_key_length is too large. int32_t max");

            if (!server_key) {
                EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
                NOT_NULL(ctx, "Failed to create EVP_PKEY_CTX");
                OPENSSL_CHECK(EVP_PKEY_keygen_init(ctx), "Failed to init keygen");
                OPENSSL_CHECK(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, (int)ssl_key_length), "Failed to set key bits");
                EVP_PKEY* pkey = nullptr;
                OPENSSL_CHECK(EVP_PKEY_keygen(ctx, &pkey), "Failed to generate key");
                NOT_NULL(pkey, "Failed to generate key");
                server_key = pkey;
                EVP_PKEY_CTX_free(ctx);
                // Write private key (PEM)
                BIO* bio = BIO_new(BIO_s_mem());
                NOT_NULL(bio, "Failed to create BIO");
                OPENSSL_CHECK(PEM_write_bio_PrivateKey(bio, server_key, nullptr, nullptr, 0, nullptr, nullptr), "Failed to write private key");
                server_private_key.resize(BIO_pending(bio));
                OPENSSL_CHECK(BIO_read(bio, server_private_key.data(), (int)server_private_key.size()), "Failed to read private key");
                OPENSSL_CHECK(BIO_free(bio), "Failed to free BIO");
                // Write public key (DER)
                int len = i2d_PUBKEY(server_key, nullptr);
                IS_CORRECT(len > 0, "Failed to get DER public key length");
                server_public_key.resize(len);
                unsigned char* pubkey_ptr = server_public_key.data();
                OPENSSL_CHECK(i2d_PUBKEY(server_key, &pubkey_ptr), "Failed to write DER public key");
            }
        }

        bool decrypt_data(list_array<uint8_t>& data) {
            init_ssl();
            if (data.size() > INT32_MAX)
                return false;
            if (!server_key)
                return false;
            EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(server_key, nullptr);
            if (!ctx)
                return false;
            if (EVP_PKEY_decrypt_init(ctx) <= 0) {
                EVP_PKEY_CTX_free(ctx);
                return false;
            }
            size_t outlen = 0;
            if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, data.data(), data.size()) <= 0) {
                EVP_PKEY_CTX_free(ctx);
                return false;
            }
            list_array<uint8_t> out;
            out.resize(outlen);
            if (EVP_PKEY_decrypt(ctx, out.data(), &outlen, data.data(), data.size()) <= 0) {
                EVP_PKEY_CTX_free(ctx);
                return false;
            }
            out.resize(outlen);
            data = std::move(out);
            EVP_PKEY_CTX_free(ctx);
            return true;
        }

        bool encrypt_data(list_array<uint8_t>& data) {
            init_ssl();
            if (data.size() > INT32_MAX)
                return false;
            if (!server_key)
                return false;
            EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(server_key, nullptr);
            if (!ctx)
                return false;
            if (EVP_PKEY_encrypt_init(ctx) <= 0) {
                EVP_PKEY_CTX_free(ctx);
                return false;
            }
            size_t outlen = 0;
            if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, data.data(), data.size()) <= 0) {
                EVP_PKEY_CTX_free(ctx);
                return false;
            }
            list_array<uint8_t> out;
            out.resize(outlen);
            if (EVP_PKEY_encrypt(ctx, out.data(), &outlen, data.data(), data.size()) <= 0) {
                EVP_PKEY_CTX_free(ctx);
                return false;
            }
            out.resize(outlen);
            data = std::move(out);
            EVP_PKEY_CTX_free(ctx);
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