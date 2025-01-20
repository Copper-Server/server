#include <src/api/configuration.hpp>
#include <src/api/mojang/session_server.hpp>
#include <src/api/players.hpp>
#include <src/api/protocol.hpp>
#include <src/base_objects/network/accept_packet_registry.hpp>
#include <src/base_objects/network/tcp_server.hpp>
#include <src/base_objects/player.hpp>
#include <src/mojang/api/hash.hpp>
#include <src/plugin/main.hpp>
#include <src/protocolHelper/client_handler/abstract.hpp>
#include <src/protocolHelper/packets/abstract.hpp>
#include <src/protocolHelper/util.hpp>

namespace copper_server::build_in_plugins::protocol {

    namespace login {
        void login_start(base_objects::network::tcp_session* session, ArrayStream& packet) {
            std::string nickname = packet.read_string(16);
            auto player = api::players::get_player(nickname);
            if (api::players::has_player(nickname)) {
                if (api::configuration::get().protocol.connection_conflict == base_objects::ServerConfiguration::Protocol::connection_conflict_t::prevent_join) {
                    session->sharedData().sendPacket(packets::login::kick(session->sharedData(), "Someone already connected with this nickname"));
                    return;
                } else
                    session->sharedData().packets_state.login_data->had_conflict = true;
            }

            session->sharedData().name = nickname;
            session->sharedData().packets_state.protocol_version = session->protocol_version;
        }

        void encryption_response(base_objects::network::tcp_session* session, ArrayStream& packet) {
            int32_t shared_secret_size = packet.read_var<int32_t>();
            list_array<uint8_t> shared_secret = packet.range_read(shared_secret_size).to_vector();
            int32_t verify_token_size = packet.read_var<int32_t>();
            list_array<uint8_t> verify_token = packet.range_read(verify_token_size).to_vector();

            if (!base_objects::network::tcp_server::instance().decrypt_data(verify_token)) {
                session->sharedData().sendPacket(packets::login::kick(session->sharedData(), "Encryption error, invalid verify token"));
                return;
            }
            if (memcmp(verify_token.data(), session->sharedData().packets_state.login_data->verify_token, 4)) {
                session->sharedData().sendPacket(packets::login::kick(session->sharedData(), "Encryption error, invalid verify token"));
                return;
            }
            if (!base_objects::network::tcp_server::instance().decrypt_data(shared_secret)) {
                session->sharedData().sendPacket(packets::login::kick(session->sharedData(), "Encryption error"));
                return;
            }

            mojang::api::hash serverId;
            serverId.update(shared_secret);
            serverId.update(base_objects::network::tcp_server::instance().public_key_buffer().data(), base_objects::network::tcp_server::instance().public_key_buffer().size());

            session->sharedData().data = api::mojang::get_session_server().hasJoined(
                session->sharedData().name,
                serverId.hexdigest(),
                !api::configuration::get().server.offline_mode
            );
            session->start_symmetric_encryption(shared_secret, shared_secret);
        }

        void plugin_response(base_objects::network::tcp_session* session, ArrayStream& packet) {
            int32_t message_id = packet.read_var<int32_t>();
            bool successful = packet.read();
            auto& login_data = *session->sharedData().packets_state.login_data;
            if (message_id != login_data.plugin_sequence_id) {
                session->sharedData().sendPacket(packets::login::kick(session->sharedData(), "Invalid plugin message id"));
                return;
            }
            {
                login_data.excepted_packet = -1;
                while (!login_data.plugins_query.empty()) {
                    auto&& it = login_data.plugins_query.front();
                    auto response = it.second->OnLoginHandle(it.second, it.first, packet.read_left(32767).to_vector(), successful, session->sharedDataRef());
                    std::visit(
                        [&](auto& it) {
                            using T = std::decay_t<decltype(it)>;
                            if constexpr (std::is_same_v<T, PluginRegistration::PluginResponse>) {
                                login_data.plugin_sequence_id++;
                                session->sharedData().sendPacket(packets::login::login(session->sharedData(), login_data.plugin_sequence_id, it.plugin_chanel, it.data));
                            } else if constexpr (std::is_same_v<T, base_objects::network::response>) {
                                session->sharedData().sendPacket(std::move(it));
                            } else if (it) {
                                login_data.plugins_query.pop_front();
                            }
                        },
                        response
                    );
                }
            }
        }

        void login_ack(base_objects::network::tcp_session* session, ArrayStream& packet) {
            if (session->sharedData().packets_state.login_data->excepted_packet == -1) {
                session->sharedData().sendPacket(packets::login::kick(session->sharedData(), "Unexpected packet"));
                return;
            }
            if (
                session->sharedData().packets_state.login_data->had_conflict && api::configuration::get().protocol.connection_conflict == base_objects::ServerConfiguration::Protocol::connection_conflict_t::kick_connected
            ) {
                api::players::iterate_players_not_state(base_objects::SharedClientData::packets_state_t::protocol_state::initialization, [&](base_objects::SharedClientData& player) {
                    if (player.name == session->sharedData().name) {
                        constexpr const char reason[] = "Some player with same nickname joined to this server";
                        switch (player.packets_state.state) {
                            using ps = base_objects::SharedClientData::packets_state_t::protocol_state;
                        case ps::play:
                            player.sendPacket(packets::play::kick(player, reason));
                            break;
                        case ps::configuration:
                            player.sendPacket(packets::configuration::kick(player, reason));
                        default:
                            break;
                        }
                    }
                    return true;
                });
            }
            api::players::login_complete_to_cfg(session->sharedDataRef());
            api::players::handlers::on_player_join(session->sharedDataRef());
            session->sharedData().switchToHandler(client_handler::abstract::createhandle_configuration(session));
        }

        void cookie_response(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::cookie_response data;
            data.key = packet.read_identifier();
            if (packet.read())
                data.payload = packet.read_array<uint8_t>(5120);
            api::protocol::on_cookie_response.async_notify({data, *session, session->sharedDataRef()});
        }
    }

    class ProtocolSupport_login : public PluginAutoRegister<"protocol_support_for_login_state_universal", ProtocolSupport_login> {
    public:
        void OnRegister(const PluginRegistrationPtr& self) override {
            base_objects::network::packet_registry.serverbound.login.register_seq(
                765,
                {
                    {"login_start", login::login_start},
                    {"encryption_response", login::encryption_response},
                    {"plugin_response", login::plugin_response},
                    {"login_ack", login::login_ack},
                }
            );
            base_objects::network::packet_registry.serverbound.login.register_seq(
                766,
                {
                    {"login_start", login::login_start},
                    {"encryption_response", login::encryption_response},
                    {"plugin_response", login::plugin_response},
                    {"login_ack", login::login_ack},
                    {"cookie_response", login::cookie_response},
                }
            );
            base_objects::network::packet_registry.serverbound.login.register_seq(
                767,
                {
                    {"login_start", login::login_start},
                    {"encryption_response", login::encryption_response},
                    {"plugin_response", login::plugin_response},
                    {"login_ack", login::login_ack},
                    {"cookie_response", login::cookie_response},
                }
            );
            base_objects::network::packet_registry.serverbound.login.register_seq(
                768,
                {
                    {"login_start", login::login_start},
                    {"encryption_response", login::encryption_response},
                    {"plugin_response", login::plugin_response},
                    {"login_ack", login::login_ack},
                    {"cookie_response", login::cookie_response},
                }
            );
            base_objects::network::packet_registry.serverbound.login.register_seq(
                769,
                {
                    {"login_start", login::login_start},
                    {"encryption_response", login::encryption_response},
                    {"plugin_response", login::plugin_response},
                    {"login_ack", login::login_ack},
                    {"cookie_response", login::cookie_response},
                }
            );
        }
    };
}
