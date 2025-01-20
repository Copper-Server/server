#ifndef SRC_PROTOCOLHELPER_CLIENT_HANDLER_767_CONFIGURATION
#define SRC_PROTOCOLHELPER_CLIENT_HANDLER_767_CONFIGURATION
#include <src/plugin/registration.hpp>
#include <src/protocolHelper/util.hpp>

namespace copper_server::client_handler::release_767 {
    class handle_configuration : public tcp_client_handle {
    protected:
        base_objects::network::response IdleActions();
        base_objects::network::response FinishConfiguration();
        base_objects::network::response Ping();
        base_objects::network::response RegistryData();

        //Server bound@0x07 new(known packs)
        //  [var_int] count
        //     {
        //      [String] namespace
        //      [String] ID
        //      [String] version
        //     }
        //  *If the client specifies a pack in this packet, the server should omit its contained data from the Registry Data packet.
        //
        base_objects::network::response work_packet(ArrayStream& packet) override;
        base_objects::network::response too_large_packet() override;
        base_objects::network::response exception(const std::exception& ex) override;
        base_objects::network::response unexpected_exception() override;

        std::list<base_objects::network::plugin_response> queriedPackets;
        std::chrono::time_point<std::chrono::system_clock> pong_timer;
        int32_t excepted_pong = 0;
        bool inited = false;

    public:
        handle_configuration(base_objects::network::tcp_session* sock);
        ~handle_configuration();
        base_objects::network::tcp_client* define_ourself(base_objects::network::tcp_session* sock) override;
        void queryPacket(const PluginRegistration::PluginResponse& packet);
    };
}
#endif /* SRC_PROTOCOLHELPER_CLIENT_HANDLER_767_CONFIGURATION */
