#ifndef SRC_BUILD_IN_PLUGINS_NETWORK_TCP_UTIL
#define SRC_BUILD_IN_PLUGINS_NETWORK_TCP_UTIL
#include <exception>
#include <functional>
#include <library/fast_task.hpp>
#include <library/fast_task/include/networking.hpp>
#include <src/api/network/tcp.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/network/tcp/client.hpp>
#include <src/base_objects/packets.hpp>
#include <src/base_objects/ptr_optional.hpp>
#include <src/util/readers.hpp>
#include <string>

namespace copper_server::build_in_plugins::network::tcp {
    class session;

    class keep_alive_solution {
        struct handle_t;
        std::shared_ptr<handle_t> handle;

    public:
        keep_alive_solution(api::network::tcp::session* session);
        ~keep_alive_solution();

        //this function should be called before using keep_alive_solution, this callback will be called
        //when keep alive packet is would be sent
        void set_callback(const std::function<void(int64_t, base_objects::SharedClientData&)>& fun);
        std::chrono::system_clock::duration got_valid_keep_alive(int64_t check);
        void make_keep_alive_packet(); //could return empty packet if keep alive already requested

        void start();
    };

    class tcp_client_handle : public base_objects::network::tcp::client {
        friend class session;
        list_array<uint8_t> legacy_motd_helper(const std::u8string& motd);

    protected:
        base_objects::network::tcp::client* next_handler = nullptr;
        api::network::tcp::session* session;

        static uint64_t generate_random_int();
        list_array<uint8_t> prepare_incoming(ArrayStream& packet);
        static list_array<uint8_t> prepare_send(base_objects::network::response::item&& packet_item, api::network::tcp::session* session);
        static list_array<list_array<uint8_t>> prepare_send(base_objects::network::response&& packet, api::network::tcp::session* session);
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
