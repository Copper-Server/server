#ifndef SRC_BASE_OBJECTS_NETWORK_TCP_SERVER
#define SRC_BASE_OBJECTS_NETWORK_TCP_SERVER
#if defined(_MSC_VER)
    #include <SDKDDKVer.h>
#endif
#include <atomic>
#include <boost/asio.hpp>
#include <library/list_array.hpp>
#include <list>
#include <mutex>
#include <openssl/rsa.h>
#include <string>
#include <unordered_set>

namespace copper_server::base_objects::network::tcp {
    class session;

    class server {
        boost::asio::ip::tcp::acceptor TCPacceptor;

        std::recursive_mutex close_mutex;
        std::unordered_set<session*> sessions;
        RSA* server_rsa_key;
        list_array<uint8_t> server_private_key; //PEM
        list_array<uint8_t> server_public_key;  //PEM
        size_t ssl_key_length;

        void AsyncWork(session* session);
        void Worker();

        std::atomic_bool disabled = true;
        std::string ip;
        bool local_server;

        auto resolveEndpoint(const std::string& ip, uint16_t port);


        friend class session;
        void close_session(session* session);
        static server* global_instance;

    public:
        static server& instance();
        bool is_local_server();

        std::string get_ip() const;

        bool decrypt_data(list_array<uint8_t>& data) const;

        bool encrypt_data(list_array<uint8_t>& data) const;

        boost::asio::const_buffer private_key_buffer();

        boost::asio::const_buffer public_key_buffer();

        size_t key_length();

        server(const std::string& ip, uint16_t port, size_t ssl_key_length = 1024);

        ~server();

        void start();

        void stop();
    };
}

#endif /* SRC_BASE_OBJECTS_NETWORK_TCP_SERVER */
