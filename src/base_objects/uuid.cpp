#include <chrono>
#include <library/enbt/enbt.hpp>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <random>
#include <src/base_objects/uuid.hpp>

namespace copper_server::base_objects {
    uuid uuid::from_string(std::string_view view){
        return from_string_md5(view);
    }


    void process_open_ssl_error(){
        uint32_t errCode;
        char errBuf[ERR_MAX_DATA_SIZE]{0};
        errCode = ERR_get_error();
        ERR_error_string_n(errCode, errBuf, ERR_MAX_DATA_SIZE);
        throw std::runtime_error(errBuf);
    }

    void make_digest(uuid& res, unsigned char* buffer, size_t max_size, std::string_view view, const char* mode) {
        EVP_MD_CTX* context = EVP_MD_CTX_new();
        if (context == nullptr)
            process_open_ssl_error();
        EVP_MD* md = EVP_MD_fetch(nullptr, mode, nullptr);
        if (md == nullptr) {
            EVP_MD_CTX_free(context);
            process_open_ssl_error();
        }
        try {
            if (1 != EVP_DigestInit_ex(context, md, nullptr))
                process_open_ssl_error();
            if (1 != EVP_DigestUpdate(context, view.data(), view.size()))
                process_open_ssl_error();
            uint32_t size = 0;
            if (1 != EVP_DigestFinal_ex(context, buffer, &size))
                process_open_ssl_error();
            if (size != max_size)
                throw std::runtime_error(std::string(mode) + " digest length is not 16 bytes");
        } catch (...) {
            EVP_MD_free(md);
            EVP_MD_CTX_free(context);
            throw;
        }
        EVP_MD_free(md);
        EVP_MD_CTX_free(context);
        memcpy(res.data, buffer, 16);
    }


    uuid uuid::from_string_sha1(std::string_view view) {
        uuid res;
        unsigned char buf[20];
        make_digest(res, buf, 20, view, "SHA1");
        res.data[6] = uint8_t((res.data[6] & 0x0F) | 0x50);
        res.data[8] = uint8_t((res.data[8] & 0x3F) | 0x80);
        return res;
    }

    uuid uuid::from_string_md5(std::string_view view) {
        uuid res;
        unsigned char buf[16];
        make_digest(res, buf, 16, view, "MD5");
        res.data[6] = uint8_t((res.data[6] & 0x0F) | 0x30);
        res.data[8] = uint8_t((res.data[8] & 0x3F) | 0x80);
        return res;
    }

    uuid uuid::generate_v4() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        uuid uuid;
        for (std::size_t i = 0; i < 16; i++)
            uuid.data[i] = (uint8_t)dis(gen);
        uuid.data[6] = uint8_t((uuid.data[6] & 0x0F) | 0x40);
        uuid.data[8] = uint8_t((uuid.data[8] & 0x3F) | 0x80);
        return uuid;
    }

    uuid uuid::generate_v7() {
        auto current_time = std::chrono::system_clock::now().time_since_epoch().count() / 1000;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        uuid uuid;
        for (std::size_t i = 4; i < 16; i++)
            uuid.data[i] = (uint8_t)dis(gen);
        uuid.data[6] = uint8_t((uuid.data[6] & 0x0F) | 0x70);
        uuid.data[8] = uint8_t((uuid.data[8] & 0x3F) | 0x80);
        uuid.data[3] = uint8_t((current_time >> 24) & 0xFF);
        uuid.data[2] = uint8_t((current_time >> 16) & 0xFF);
        uuid.data[1] = uint8_t((current_time >> 8) & 0xFF);
        uuid.data[0] = uint8_t(current_time & 0xFF);
        return uuid;
    }

     uuid uuid::generate_npc(){
         uuid res = generate_v7();
         res.data[6] = uint8_t((res.data[6] & 0x0F) | 0x20);
         return res;
     }

     uuid uuid::generate_offline() {
         uuid res = generate_v7();
         res.data[6] = uint8_t((res.data[6] & 0x0F) | 0x30);
         return res;
     }

     uuid uuid::create_offline(std::string_view view) {
         std::string tmp = "OfflinePlayer:" + std::string(view);
         return from_string_md5(tmp);
     }

    uuid uuid::as_null() {
        return {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    }

    uuid uuid::to_uuid(const enbt::raw_uuid& e) noexcept {
        uuid res;
        for (std::size_t i = 0; i < 16; i++)
            res.data[i] = e.data[i];
        return res;
    }

    uuid::operator enbt::raw_uuid() const noexcept {
        enbt::raw_uuid res;
        for (std::size_t i = 0; i < 16; i++)
            res.data[i] = data[i];
        return res;
    }
}