#include <openssl/pem.h>
#include <src/api/asio.hpp>
#include <src/api/configuration.hpp>
#include <src/api/network.hpp>
#include <src/base_objects/network/tcp/client.hpp>
#include <src/base_objects/network/tcp/server.hpp>
#include <src/base_objects/network/tcp/session.hpp>
#include <src/log.hpp>
#define OPENSSL_CHECK(OPERATION, console_output) \
    if ((OPERATION) <= 0) {                      \
        log::fatal("OpenSSL", console_output);   \
    }
#define NOT_NULL(X, console_output)            \
    if (!(X)) {                                \
        log::fatal("OpenSSL", console_output); \
    }

namespace copper_server::base_objects::network::tcp {
    server* server::global_instance = nullptr;

    server& server::instance() {
        return global_instance ? *global_instance : throw std::runtime_error("tcp::server not initialized");
    }

    void server::close_session(session* session) {
        std::unique_lock lock(close_mutex);
        if (!sessions.erase(session))
            log::error("protocol", "Failed to erase session from list");
        lock.unlock();
        delete session;
    }

    void server::AsyncWork(session* session_) {
        boost::system::error_code err;
        if (api::network::ip_filter(session_->sock.remote_endpoint().address()))
            session_->sock.close();
        else {
            std::unique_lock lock(close_mutex);
            if (sessions.insert(session_).second)
                session_->connect();
        }
    }

    void server::Worker() {
        session* session_ = new session(boost::asio::ip::tcp::socket(make_strand(api::asio::get_threads())), api::network::get_first_tcp_handler(), api::configuration::get().server.all_connections_timeout);
        TCPacceptor.async_accept(session_->sock, [this, session_](const boost::system::error_code& error) {
            if (error == boost::asio::error::operation_aborted || disabled)
                return;

            boost::asio::ip::tcp::no_delay option(true);
            session_->sock.set_option(option);
            Worker();
            AsyncWork(session_);
        });
    }

    auto server::resolveEndpoint(const std::string& ip, uint16_t port) {
        boost::asio::ip::tcp::resolver resolver(api::asio::get_service());
        auto list = resolver.resolve(ip, std::to_string(port));
        auto endpoint = list.begin()->endpoint();

        local_server = false;
        //iterate all endpoints
        for (auto& endpoints : list) {
            auto address = endpoints.endpoint().address();
            if (address.is_loopback())
                local_server = true;
            log::debug("server", "tcp::server address: " + address.to_string());
        }

        return endpoint;
    }

    bool server::is_local_server() {
        return local_server;
    }

    std::string server::get_ip() const {
        return ip;
    }

    bool server::decrypt_data(list_array<uint8_t>& data) const {
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

    bool server::encrypt_data(list_array<uint8_t>& data) const {
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

    boost::asio::const_buffer server::private_key_buffer() {
        return boost::asio::const_buffer(server_private_key.data(), server_private_key.size());
    }

    boost::asio::const_buffer server::public_key_buffer() {
        return boost::asio::const_buffer(server_public_key.data(), server_public_key.size());
    }

    size_t server::key_length() {
        return ssl_key_length;
    }

    server::server(const std::string& ip, uint16_t port, size_t ssl_key_length)
        : TCPacceptor(api::asio::get_service(), resolveEndpoint(ip, port)),
          ssl_key_length(ssl_key_length),
          ip(ip) {
        if (global_instance)
            throw std::runtime_error("tcp::server already initialized");
        global_instance = this;
        if (ssl_key_length) {
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
        if (local_server)
            api::configuration::get().server.offline_mode = true;
    }

    server::~server() {
        if (!disabled)
            stop();
        if (server_rsa_key)
            RSA_free(server_rsa_key);
    }

    void server::start() {
        if (disabled) {
            Worker();
            disabled = false;
            api::asio::get_service().run();
        } else
            throw std::exception("tcp::server already run");
    }

    void server::stop() {
        if (!disabled) {
            std::lock_guard lock(close_mutex);
            TCPacceptor.close();
            for (auto it : sessions)
                it->disconnect();
            disabled = true;
            api::asio::get_service().stop();
        } else
            throw std::exception("tcp::server already stoped");
    }
}