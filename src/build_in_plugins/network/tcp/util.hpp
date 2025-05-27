#ifndef SRC_BUILD_IN_PLUGINS_NETWORK_TCP_UTIL
#define SRC_BUILD_IN_PLUGINS_NETWORK_TCP_UTIL
#include <exception>
#include <functional>
#include <library/fast_task.hpp>
#include <library/fast_task/src/networking/networking.hpp>
#include <src/api/network/tcp.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/network/tcp/client.hpp>
#include <src/base_objects/packets.hpp>
#include <src/base_objects/ptr_optional.hpp>
#include <src/util/readers.hpp>
#include <string>

namespace copper_server::build_in_plugins::network::tcp {
    class keep_alive_solution {
        std::function<base_objects::network::response(int64_t)> callback;
        fast_task::deadline_timer timeout_timer;
        fast_task::deadline_timer send_keep_alive_timer;
        bool send_keep_alive_requested = false;
        bool need_to_send = false;
        bool got_keep_alive = true;
        api::network::tcp::session* session;
        std::chrono::time_point<std::chrono::system_clock> last_keep_alive;

        void _keep_alive_sended();
        void _keep_alive_request();

    public:
        keep_alive_solution(api::network::tcp::session* session);
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
        static inline size_t max_packet_size = 4096;
        base_objects::network::tcp::client* next_handler = nullptr;
        api::network::tcp::session* session;
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
        tcp_client_handle(api::network::tcp::session* session);
        ~tcp_client_handle() override;
        base_objects::network::tcp::client* redefine_handler() override;
        base_objects::network::response work_client(list_array<uint8_t>& clientData) final;
        base_objects::network::response on_switch() final;
    };
}
#endif /* SRC_BUILD_IN_PLUGINS_NETWORK_TCP_UTIL */
