#ifndef SRC_CLIENTHANDLEHELPER
#define SRC_CLIENTHANDLEHELPER
#if defined(_MSC_VER)
    #include <SDKDDKVer.h>
#endif

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/aes.h>
#include <string>
#include <unordered_set>
#include <utf8.h>
#include <string>


#include "library/list_array.hpp"

#include "base_objects/player.hpp"
#include "mojang_api/session_server.hpp"


#include "base_objects/response.hpp"
#include "base_objects/server_configuaration.hpp"
#include "base_objects/shared_client_data.hpp"

#include "storage/memory/entity_ids_map.hpp"
#include "storage/memory/online_player.hpp"
#include "storage/permissions_manager.hpp"

namespace crafted_craft {
    constexpr bool CONSTEXPR_DEBUG_DATA_TRANSPORT = true;
    struct TCPsession;
    class TCPserver;

    class AESencryption {
        AESencryption(const AESencryption&) = delete;
        AESencryption& operator=(const AESencryption&) = delete;

    public:
        AESencryption() = default;

        bool initialize(const list_array<uint8_t>& key, const list_array<uint8_t>& iv) {
            if (key.size() != 16 || iv.size() != 16)
                return false;
            if (AES_set_encrypt_key(key.data(), 128, &enc_key) != 0)
                return false;
            if (AES_set_decrypt_key(key.data(), 128, &dec_key) != 0)
                return false;
            memcpy(iv_data, iv.data(), 8);
            return true;
        }

        void encrypt(const list_array<uint8_t>& data, list_array<uint8_t>& out) {
            out.resize(data.size());
            AES_cfb8_encrypt(data.data(), out.data(), data.size(), &enc_key, iv_data, &num, AES_ENCRYPT);
        }

        void decrypt(const list_array<uint8_t>& data, list_array<uint8_t>& out) {
            out.resize(data.size());
            AES_cfb8_encrypt(data.data(), out.data(), data.size(), &dec_key, iv_data, &num, AES_DECRYPT);
        }

    private:
        AES_KEY enc_key;
        AES_KEY dec_key;
        uint8_t iv_data[8];
        int num = 0;
    };

    class TCPclient {
    protected:
    public:
        virtual Response WorkClient(list_array<uint8_t>&) = 0;

        virtual Response OnSwitch();

        //will return nullptr if Redefine not needed
        virtual TCPclient* RedefineHandler() = 0;
        virtual bool DoDisconnect(boost::asio::ip::address) = 0;
        virtual TCPclient* DefineOurself(TCPsession* session) = 0;

        virtual ~TCPclient();

        static void logConsole(const std::string& prefix, const list_array<uint8_t>& data, size_t size);
    };

    extern TCPclient* first_client_holder;

    struct TCPsession {
        static bool do_log_connection_errors;
        static std::atomic_uint64_t id_gen;
        AESencryption encryption;
        uint64_t id = id_gen++;
        boost::asio::ip::tcp::socket sock;
        int32_t protocol_version = -1;

        TCPsession(boost::asio::ip::tcp::socket&& s, TCPclient* client_handler, uint64_t& set_timeout, TCPserver* server = nullptr);
        ~TCPsession();
        base_objects::client_data_holder& sharedDataRef();
        SharedClientData& sharedData();
        TCPserver& serverData();
        void connect();
        void connect(std::vector<uint8_t>& connection_data, boost::system::error_code ec);
        void disconnect();
        bool isActive();
        bool start_symmetric_encryption(const list_array<uint8_t>& encryption_key, const list_array<uint8_t>& encryption_iv);
        boost::asio::const_buffer public_key();

    private:
        void send(Response&& resp);
        bool checked(boost::system::error_code ec, std::string const& msg = "error");
        void req_loop();
        void consume_data(size_t read_size);
        Response proceed_data();
        void on_request(boost::system::error_code ec, size_t read_size);


    private:
        uint64_t& timeout;
        std::vector<uint8_t> read_data;
        list_array<uint8_t> read_data_cached;
        base_objects::client_data_holder _sharedData;
        TCPclient* chandler = nullptr;
        std::atomic_bool active{false};
        TCPserver* server;
        bool encryption_enabled : 1 = false;

    public:
        bool is_not_legacy : 1 = false;
        int32_t compression_threshold = -1;
    };

    class TCPserver {
        boost::asio::io_service* service;
        boost::asio::ip::tcp::acceptor TCPacceptor;
        boost::asio::thread_pool threads;

        std::mutex close_mutex;
        std::list<TCPsession*> queried_close;
        std::unordered_set<TCPsession*> sessions;
        RSA* server_rsa_key;
        list_array<uint8_t> server_private_key; //PEM
        list_array<uint8_t> server_public_key;  //PEM
        size_t ssl_key_length;

        void make_clean_up();

        void AsyncWork(TCPsession* session);

        void Worker();

        std::atomic_bool disabled = true;
        std::string ip;
        bool local_server;

        auto resolveEndpoint(const std::string& ip, uint16_t port);

        mojang::api::session_server session_server;


        friend class TCPsession;
        void close_session(TCPsession* session);
        static TCPserver* global_instance;

    public:
        static TCPserver& get_global_instance();
        static void register_global_instance(TCPserver& instance);

        storage::memory::online_player_storage online_players;
        storage::memory::entity_ids_map_storage entity_ids_map;
        storage::permissions_manager permissions_manager;

        base_objects::ServerConfiguration server_config;

        boost::asio::io_service& getService();
        bool is_local_server();

        mojang::api::session_server& getSessionServer();

        std::string get_ip() const;

        bool decrypt_data(list_array<uint8_t>& data) const;

        bool encrypt_data(list_array<uint8_t>& data) const;

        boost::asio::const_buffer private_key_buffer();

        boost::asio::const_buffer public_key_buffer();

        size_t key_length();

        bool handle_legacy = false;
        uint16_t timeout_seconds = 30;
        uint16_t max_accept_buffer = 100;
        //30 sec
        uint64_t all_connections_timeout = 30000;

        TCPserver(const std::filesystem::path& base_path, boost::asio::io_service* io_service, const std::string& ip, uint16_t port, size_t threads = 0, size_t ssl_key_length = 1024);

        ~TCPserver();

        void start();

        void stop();
    };
}


#endif /* SRC_CLIENTHANDLEHELPER */
