#ifndef SRC_PROTOCOLHELPER_CLIENT_HANDLER_769_PLAY
#define SRC_PROTOCOLHELPER_CLIENT_HANDLER_769_PLAY
#include <src/protocolHelper/util.hpp>

namespace copper_server::client_handler::release_769 {
    class handle_play : public tcp_client_handle {
        std::list<base_objects::network::plugin_response> queriedPackets;
        int64_t keep_alive_packet = 0;
        int32_t excepted_pong = 0;
        std::chrono::time_point<std::chrono::system_clock> pong_timer;

        void check_response(base_objects::network::response& resp);
        base_objects::network::response IdleActions();
        base_objects::network::response work_packet(ArrayStream& packet) override;
        base_objects::network::response too_large_packet() override;
        base_objects::network::response exception(const std::exception& ex) override;
        base_objects::network::response unexpected_exception() override;
        base_objects::network::response on_switching() override;

    public:
        handle_play(base_objects::network::tcp_session* session);
        ~handle_play() override;
        base_objects::network::tcp_client* define_ourself(base_objects::network::tcp_session* sock) override;
    };
}

#endif /* SRC_PROTOCOLHELPER_CLIENT_HANDLER_769_PLAY */
