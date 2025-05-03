#ifndef SRC_PROTOCOLHELPER_UTIL
#define SRC_PROTOCOLHELPER_UTIL
#if defined(_MSC_VER)
    #include <SDKDDKVer.h>
#endif
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <exception>
#include <functional>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/network/tcp/client.hpp>
#include <src/base_objects/network/tcp/session.hpp>
#include <src/base_objects/packets.hpp>
#include <src/base_objects/ptr_optional.hpp>
#include <src/util/readers.hpp>
#include <string>

namespace copper_server {
    class keep_alive_solution {
        std::function<base_objects::network::response(int64_t)> callback;
        boost::asio::deadline_timer timeout_timer;
        boost::asio::deadline_timer send_keep_alive_timer;
        bool send_keep_alive_requested = false;
        bool need_to_send = false;
        bool got_keep_alive = true;
        base_objects::network::tcp::session* session;
        std::chrono::time_point<std::chrono::system_clock> last_keep_alive;

        void _keep_alive_sended();
        void _keep_alive_request();

    public:
        keep_alive_solution(base_objects::network::tcp::session* session);
        ~keep_alive_solution();
        void set_callback(const std::function<base_objects::network::response(int64_t)>& fun);
        void keep_alive_sended();

        base_objects::network::response send_keep_alive();
        //returns elapsed time from last keep_alive
        std::chrono::system_clock::duration got_valid_keep_alive(int64_t check);
        void ignore_keep_alive(); //used to shut down the timer
        base_objects::network::response no_response();
    };

    class tcp_client_handle : public base_objects::network::tcp::client {

        list_array<uint8_t> legacy_motd_helper(const std::u8string& motd);

    protected:
        static inline std::unordered_set<boost::asio::ip::address> banned_clients;
        static inline size_t max_packet_size = 4096;
        base_objects::network::tcp::client* next_handler = nullptr;
        base_objects::network::tcp::session* session;
        base_objects::ptr_optional<keep_alive_solution> _keep_alive_solution;

        static uint64_t generate_random_int();
        list_array<uint8_t> prepare_incoming(ArrayStream& packet);
        list_array<uint8_t> prepare_send(base_objects::network::response::item&& packet_item);
        list_array<list_array<uint8_t>> prepare_send(auto&& packet);
        virtual base_objects::network::response work_packet(ArrayStream& packet) = 0;
        virtual base_objects::network::response too_large_packet() = 0;
        virtual base_objects::network::response exception(const std::exception& ex) = 0;
        virtual base_objects::network::response unexpected_exception() = 0;
        virtual base_objects::network::response on_switching();
        base_objects::network::response work_packets(list_array<uint8_t>& combined);


    public:
        tcp_client_handle(base_objects::network::tcp::session* session);
        ~tcp_client_handle() override;
        base_objects::network::tcp::client* redefine_handler() override;
        base_objects::network::response work_client(list_array<uint8_t>& clientData) final;
        base_objects::network::response on_switch() final;
    };
}
#endif /* SRC_PROTOCOLHELPER_UTIL */
