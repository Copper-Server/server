#ifndef SRC_BASE_OBJECTS_NETWORK_TCP_SESSION
#define SRC_BASE_OBJECTS_NETWORK_TCP_SESSION
#if defined(_MSC_VER)
    #include <SDKDDKVer.h>
#endif
#include <atomic>
#include <boost/asio/ip/tcp.hpp>
#include <src/base_objects/encryption/aes.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <vector>

namespace copper_server::base_objects::network::tcp {
    class client;

    class session {
    public:
        static bool do_log_connection_errors;
        static std::atomic_uint64_t id_gen;
        encryption::aes encryption;
        uint64_t id = id_gen++;
        boost::asio::ip::tcp::socket sock;
        int32_t protocol_version = -1;

        session(boost::asio::ip::tcp::socket&& s, client* client_handler, uint64_t& set_timeout);
        ~session();
        base_objects::client_data_holder& sharedDataRef();
        base_objects::SharedClientData& sharedData();
        void connect();
        void connect(std::vector<uint8_t>& connection_data, boost::system::error_code ec);
        void disconnect();
        bool isActive();
        bool start_symmetric_encryption(const list_array<uint8_t>& encryption_key, const list_array<uint8_t>& encryption_iv);

        client& handler();

    private:
        void send(response&& resp);
        bool checked(boost::system::error_code ec, std::string const& msg = "error");
        void req_loop();
        void consume_data(size_t read_size);
        response proceed_data();
        void on_request(boost::system::error_code ec, size_t read_size);

        uint64_t& timeout;
        std::vector<uint8_t> read_data;
        list_array<uint8_t> read_data_cached;
        base_objects::client_data_holder _sharedData;
        tcp::client* chandler = nullptr;
        std::atomic_bool active{false};
        bool encryption_enabled : 1 = false;

    public:
        bool is_not_legacy : 1 = false;
        int32_t compression_threshold = -1;
    };
}


#endif /* SRC_BASE_OBJECTS_NETWORK_TCP_SESSION */
