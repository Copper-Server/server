#include <src/api/packets.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::network::tcp {
    struct tcp_handshake : public PluginAutoRegister<"network/tcp_handshake", tcp_handshake> {
        void OnRegister(const PluginRegistrationPtr&) override {
            using intention = api::packets::server_bound::handshake::intention;
            api::packets::register_server_bound_processor<intention>([](intention&& packet, base_objects::SharedClientData& client) {
                client.packets_state.protocol_version = packet.protocol_version;
                switch (packet.intent.value) {
                case intention::intent_e::status:
                    client << base_objects::switches_to::status{};
                    break;
                case intention::intent_e::transfer:
                    client.packets_state.is_transferred = true;
                //[[fallthrough]]
                case intention::intent_e::login:
                    client << base_objects::switches_to::login{};
                    break;
                }
            });
        }
    };
}