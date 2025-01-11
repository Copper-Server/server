
#include <src/api/configuration.hpp>
#include <src/api/mojang/session_server.hpp>
#include <src/api/players.hpp>
#include <src/api/protocol.hpp>
#include <src/base_objects/network/tcp_server.hpp>
#include <src/log.hpp>
#include <src/mojang/api/hash.hpp>
#include <src/plugin/main.hpp>
#include <src/protocolHelper/client_handler/767/login.hpp>
#include <src/protocolHelper/client_handler/abstract.hpp>
#include <src/protocolHelper/packets/767/packets.hpp>

namespace copper_server::client_handler::release_767 {
    base_objects::network::response handle_login::encryptionRequest() {
        excepted_packet = 1;
        auto generate_ui8 = []() -> uint8_t {
            static std::random_device rd;
            static std::mt19937_64 gen;
            static std::uniform_int_distribution<uint16_t> dis;
            uint16_t ui16 = dis(gen);
            return (uint8_t)((ui16 & 0xFF ^ (ui16 >> 8)) & 0xFF);
        };
        verify_token[0] = generate_ui8();
        verify_token[1] = generate_ui8();
        verify_token[2] = generate_ui8();
        verify_token[3] = generate_ui8();

        return packets::release_767::login::encryptionRequest("", verify_token);
    }

    base_objects::network::response handle_login::loginSuccess() {
        auto tmp = packets::release_767::login::loginSuccess(session->sharedData());
        log::debug("login", "request login completion");
        excepted_packet = 3;
        return tmp;
    }

    base_objects::network::response handle_login::setCompression(int32_t threshold) {
        base_objects::network::response response = packets::release_767::login::setCompression(threshold);
        log::debug("login", "set compression");
        if (plugins_query.empty())
            response += loginSuccess();
        return response;
    }

    base_objects::network::response handle_login::proceedPlugin(ArrayStream& data, bool successful) {
        log::debug("login", "handle plugin request");
        excepted_packet = -1;
        while (!plugins_query.empty()) {
            auto&& it = plugins_query.front();
            auto response = it.second->OnLoginHandle(it.second, it.first, data.read_left(32767).to_vector(), successful, session->sharedDataRef());
            if (std::holds_alternative<PluginRegistration::PluginResponse>(response)) {
                auto& plugin = std::get<PluginRegistration::PluginResponse>(response);
                plugin_message_id++;
                return packets::release_767::login::login(plugin_message_id, plugin.plugin_chanel, plugin.data);
            } else if (std::holds_alternative<base_objects::network::response>(response))
                return std::move(std::get<base_objects::network::response>(response));
            else
                plugins_query.pop_front();
        }
        return loginSuccess();
    }

    void handle_login::resolve_join_conflict() {
        if (
            has_conflict && api::configuration::get().protocol.connection_conflict == base_objects::ServerConfiguration::Protocol::connection_conflict_t::kick_connected
        ) {
            api::players::iterate_players_not_state(base_objects::SharedClientData::packets_state_t::protocol_state::initialization, [&](base_objects::SharedClientData& player) {
                if (player.name == session->sharedData().name) {
                    constexpr const char reason[] = "Some player with same nickname joined to this server";
                    switch (player.packets_state.state) {
                        using ps = base_objects::SharedClientData::packets_state_t::protocol_state;
                    case ps::play:
                        player.sendPacket(packets::release_767::play::kick(reason));
                        break;
                    case ps::configuration:
                        player.sendPacket(packets::release_767::configuration::kick(reason));
                    default:
                        break;
                    }
                }
                return true;
            });
        }
    }

    base_objects::network::response handle_login::work_packet(ArrayStream& packet) {
        uint8_t packet_id = packet.read();

        if (packet_id != excepted_packet && excepted_packet != -1) {
            api::players::remove_player(session->sharedDataRef());
            return base_objects::network::response::disconnect();
        }

        switch (packet_id) {
        case 0: { //login start
            log::debug("login", "login start");
            std::string nickname = ReadString(packet, 16);
            auto player = api::players::get_player(nickname);
            if (api::players::has_player(nickname)) {
                if (api::configuration::get().protocol.connection_conflict == base_objects::ServerConfiguration::Protocol::connection_conflict_t::prevent_join)
                    return packets::release_767::login::kick("You someone already connected with this nickname");
                else
                    has_conflict = true;
            }

            session->sharedData().name = nickname;
            session->sharedData().packets_state.protocol_version = session->protocol_version;
            if (
                !api::configuration::get().server.offline_mode
                || api::configuration::get().protocol.enable_encryption
            ) {
                return encryptionRequest();
            } else if (api::configuration::get().protocol.compression_threshold != -1) {
                return setCompression(api::configuration::get().protocol.compression_threshold);
            } else if (!plugins_query.empty()) {
                ArrayStream empty((const uint8_t*)nullptr, 0);
                return proceedPlugin(empty);
            } else {
                return loginSuccess();
            }
            break;
        }
        case 1: { //encryption response
            log::debug("login", "encryption response");
            int32_t shared_secret_size = ReadVar<int32_t>(packet);
            list_array<uint8_t> shared_secret = packet.range_read(shared_secret_size).to_vector();
            int32_t verify_token_size = ReadVar<int32_t>(packet);
            list_array<uint8_t> verify_token = packet.range_read(verify_token_size).to_vector();

            if (!base_objects::network::tcp_server::instance().decrypt_data(verify_token)) {
                log::error("login", "invalid verify token");
                return packets::release_767::login::kick("Invalid verify token");
            }

            if (memcmp(verify_token.data(), this->verify_token, 4)) {
                log::error("login", "invalid verify token");
                return packets::release_767::login::kick("Invalid verify token");
            }


            if (!base_objects::network::tcp_server::instance().decrypt_data(shared_secret)) {
                log::error("login", "encryption error");
                return packets::release_767::login::kick("Encryption error");
            }

            mojang::api::hash serverId;
            serverId.update(shared_secret);
            serverId.update(base_objects::network::tcp_server::instance().public_key_buffer().data(), base_objects::network::tcp_server::instance().public_key_buffer().size());

            log::debug("login", "mojang request");
            session->sharedData().data = api::mojang::get_session_server().hasJoined(
                session->sharedData().name,
                serverId.hexdigest(),
                !api::configuration::get().server.offline_mode
            );
            session->start_symmetric_encryption(shared_secret, shared_secret);

            if (api::configuration::get().protocol.compression_threshold != -1)
                return setCompression(api::configuration::get().protocol.compression_threshold);
            else if (plugins_query.empty())
                return loginSuccess();
            else {
                ArrayStream empty((const uint8_t*)nullptr, 0);
                return proceedPlugin(empty);
            }
        }
        case 2: { //login plugin response
            log::debug("login", "login plugin response");
            int32_t message_id = ReadVar<int32_t>(packet);
            bool successful = packet.read();
            if (message_id != plugin_message_id)
                log::debug("login", "invalid plugin message id");
            return proceedPlugin(packet, successful);
        }
        case 3: //Login Acknowledged
            if (excepted_packet == -1) {
                log::debug("login", "[protocol error] unexpected packet while plugin handle");
                return base_objects::network::response::disconnect();
            }
            resolve_join_conflict();
            api::players::login_complete_to_cfg(session->sharedDataRef());
            api::players::handlers::on_player_join(session->sharedDataRef());

            next_handler = abstract::createhandle_configuration(session);
            log::debug("login", "login ack");
            return base_objects::network::response::empty();
        case 4: { //from 767 Cookie response
            api::protocol::data::cookie_response data;
            data.key = ReadIdentifier(packet);
            if (packet.read())
                data.payload = ReadArray<uint8_t>(packet, 5120);
            api::protocol::on_cookie_response.async_notify({data, *session, session->sharedDataRef()});
            break;
        }
        default:
            log::debug("login", "invalid packet");
        }
        return base_objects::network::response::disconnect();
    }

    base_objects::network::response handle_login::too_large_packet() {
        return packets::release_767::login::kick("Packet too large");
    }

    base_objects::network::response handle_login::exception(const std::exception& ex) {
        return packets::release_767::login::kick("Caught exception: " + std::string(ex.what()));
    }

    base_objects::network::response handle_login::unexpected_exception() {
        return packets::release_767::login::kick("Caught unexpected exception");
    }

    handle_login::handle_login(base_objects::network::tcp_session* sock)
        : tcp_client_handle(sock) {
        pluginManagement.inspect_plugin_bind(PluginManagement::registration_on::login, [&](const std::pair<std::string, PluginRegistrationPtr>& it) {
            plugins_query.push_back(it);
        });
    }

    handle_login::handle_login()
        : tcp_client_handle(nullptr) {}

    base_objects::network::tcp_client* handle_login::define_ourself(base_objects::network::tcp_session* sock) {
        return new handle_login(sock);
    }
};