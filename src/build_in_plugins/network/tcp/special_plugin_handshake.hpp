#ifndef SRC_BUILD_IN_PLUGINS_NETWORK_TCP_SPECIAL_PLUGIN_HANDSHAKE
#define SRC_BUILD_IN_PLUGINS_NETWORK_TCP_SPECIAL_PLUGIN_HANDSHAKE

#include <src/build_in_plugins/network/tcp/util.hpp>
#include <src/plugin/main.hpp>
#include <library/fast_task.hpp>
#include <functional>

namespace copper_server::build_in_plugins::network::tcp {

    class SpecialPluginHandshake {
    public:
        //allows to handle custom packets from the client, like send a packet from server to server or another use case.
        //if tcp_client_handle nullptr then connection will be closed
        virtual std::pair<tcp_client_handle*, list_array<uint8_t>> InvalidPacket(uint8_t packet_id, ArrayStream& data) = 0;
        virtual std::pair<tcp_client_handle*, list_array<uint8_t>> InvalidNextOperation(uint8_t next_operation, ArrayStream& data) = 0;

        //allows to set custom handler for this protocol, packet will be again parsed by this new handler
        virtual tcp_client_handle* UnsupportedProtocolVersion(int protocol_version) {
            return nullptr;
        }

        virtual ~SpecialPluginHandshake() {}
    };

    class SpecialPluginHandshakeRegistration : public PluginAutoRegister<"tcp_special_plugin_handshake_registration", SpecialPluginHandshakeRegistration> {
        fast_task::protected_value<list_array<std::unique_ptr<SpecialPluginHandshake>>> list;

    public:
        SpecialPluginHandshakeRegistration();

        ~SpecialPluginHandshakeRegistration() noexcept {}

        void
        register_handle(std::unique_ptr<SpecialPluginHandshake>&& self);
        void unregister_handle(SpecialPluginHandshake* self);

        //return true to break
        void enum_handles(const std::function<bool(SpecialPluginHandshake&)>& callback);
    };
}

#endif /* SRC_BUILD_IN_PLUGINS_NETWORK_TCP_SPECIAL_PLUGIN_HANDSHAKE */
