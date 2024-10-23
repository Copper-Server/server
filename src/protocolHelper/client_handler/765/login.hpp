#ifndef SRC_PROTOCOLHELPER_CLIENT_PLAY_HANDLER_765_LOGIN
#define SRC_PROTOCOLHELPER_CLIENT_PLAY_HANDLER_765_LOGIN
#include "../../api/players.hpp"
#include "../../mojang/api/hash.hpp"
#include "../../packets/765/packets.hpp"
#include "../abstract.hpp"

namespace crafted_craft {
    namespace client_handler {
        namespace release_765 {
            class HandleLogin : public TCPClientHandle {
            protected:
                virtual Chat* AllowPlayersName(std::string nick) {
                    return nullptr;
                    Chat chat = Chat("Server closed");
                    chat.SetColor("red");
                    return new Chat(std::move(chat));
                }

                Response encryptionRequest() {
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

                    return packets::release_765::login::encryptionRequest("", verify_token);
                }

                Response loginSuccess() {
                    auto tmp = packets::release_765::login::loginSuccess(session->sharedData());
                    log::debug("login", "request login completion");
                    excepted_packet = 3;
                    return tmp;
                }

                Response setCompression(int32_t threshold) {
                    Response response = packets::release_765::login::setCompression(threshold);
                    log::debug("login", "set compression");
                    if (!plugins_query.empty())
                        response += loginSuccess();
                    return response;
                }

                Response proceedPlugin(ArrayStream& data, bool successful = true) {
                    log::debug("login", "handle plugin request");
                    excepted_packet = -1;
                    while (plugins_query.size()) {
                        auto&& it = plugins_query.front();
                        auto response = it.second->OnLoginHandle(it.second, it.first, data.to_vector(), successful, session->sharedDataRef());
                        if (std::holds_alternative<PluginRegistration::PluginResponse>(response)) {
                            auto& plugin = std::get<PluginRegistration::PluginResponse>(response);
                            plugin_message_id++;
                            return packets::release_765::login::login(plugin_message_id, plugin.plugin_chanel, plugin.data);
                        } else if (std::holds_alternative<Response>(response))
                            return std::move(std::get<Response>(response));
                        else
                            plugins_query.pop_front();
                    }
                    return loginSuccess();
                }

                void resolve_join_conflict() {

                    if (
                        has_conflict && api::configuration::get().protocol.connection_conflict == base_objects::ServerConfiguration::Protocol::connection_conflict_t::kick_connected
                    ) {
                        Server::instance().online_players.iterate_players_not_state(base_objects::SharedClientData::packets_state_t::protocol_state::initialization, [&](base_objects::SharedClientData& player) {
                            if (player.name == session->sharedData().name) {
                                constexpr const char reason[] = "Some player with same nickname joined to this server";
                                switch (player.packets_state.state) {
                                    using ps = base_objects::SharedClientData::packets_state_t::protocol_state;
                                case ps::play:
                                    player.sendPacket(packets::release_765::play::kick(reason));
                                    break;
                                case ps::configuration:
                                    player.sendPacket(packets::release_765::configuration::kick(reason));
                                default:
                                    break;
                                }
                            }
                            return true;
                        });
                    }
                }

                Response WorkPacket(ArrayStream& data) override {
                    uint8_t packet_id = data.read();

                    if (packet_id != excepted_packet && excepted_packet != -1) {
                        Server::instance().online_players.remove_player(session->sharedDataRef());
                        return Response::Disconnect();
                    }

                    switch (packet_id) {
                    case 0: { //login start
                        auto& online_players = Server::instance().online_players;

                        log::debug("login", "login start");
                        std::string nickname = ReadString(data, 16);
                        auto player = online_players.get_player(nickname);
                        if (online_players.has_player(nickname)) {
                            if (api::configuration::get().protocol.connection_conflict == base_objects::ServerConfiguration::Protocol::connection_conflict_t::prevent_join)
                                return packets::release_765::login::kick("You someone already connected with this nickname");
                            else
                                has_conflict = true;
                        }

                        session->sharedData().name = nickname;
                        session->sharedData().packets_state.protocol_version = session->protocol_version;
                        Chat* kick_reason_chat = AllowPlayersName(nickname);
                        if (kick_reason_chat) {
                            Chat str = *kick_reason_chat;
                            delete kick_reason_chat;
                            log::debug("login", "kick...");
                            return packets::release_765::login::kick(str);
                        }
                        if (
                            !api::configuration::get().protocol.offline_mode
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
                        int32_t shared_secret_size = ReadVar<int32_t>(data);
                        list_array<uint8_t> shared_secret = data.range_read(shared_secret_size).to_vector();
                        int32_t verify_token_size = ReadVar<int32_t>(data);
                        list_array<uint8_t> verify_token = data.range_read(verify_token_size).to_vector();

                        if (!Server::instance().decrypt_data(verify_token)) {
                            log::error("login", "invalid verify token");
                            return packets::release_765::login::kick("Invalid verify token");
                        }

                        if (memcmp(verify_token.data(), this->verify_token, 4)) {
                            log::error("login", "invalid verify token");
                            return packets::release_765::login::kick("Invalid verify token");
                        }


                        if (!Server::instance().decrypt_data(shared_secret)) {
                            log::error("login", "encryption error");
                            return packets::release_765::login::kick("Encryption error");
                        }
                        mojang::api::hash serverId;
                        serverId.update(shared_secret);
                        serverId.update(Server::instance().public_key_buffer().data(), Server::instance().public_key_buffer().size());

                        log::debug("login", "mojang request");
                        session->sharedData().data = Server::instance().getSessionServer().hasJoined(
                            session->sharedData().name,
                            serverId.hexdigest(),
                            !api::configuration::get().protocol.offline_mode
                        );
                        session->start_symmetric_encryption(shared_secret, shared_secret);
                        return loginSuccess();
                    }
                    case 2: { //login plugin response
                        log::debug("login", "login plugin response");
                        int32_t message_id = ReadVar<int32_t>(data);
                        bool successful = data.read();
                        if (message_id != plugin_message_id)
                            log::debug("login", "invalid plugin message id");
                        return proceedPlugin(data, successful);
                    }
                    case 3: //Login Acknowledged
                        if (excepted_packet == -1) {
                            log::debug("login", "[protocol error] unexpected packet while plugin handle");
                            return Response::Disconnect();
                        }
                        resolve_join_conflict();
                        Server::instance().online_players.login_complete_to_cfg(session->sharedDataRef());
                        api::players::handlers::on_player_join(session->sharedDataRef());

                        next_handler = abstract::createHandleConfiguration(session);
                        log::debug("login", "login ack");
                        return Response::Empty();
                    default:
                        log::debug("login", "invalid packet");
                    }
                    return Response::Disconnect();
                }

                Response TooLargePacket() override {
                    return packets::release_765::login::kick("Packet too large");
                }

                Response Exception(const std::exception& ex) override {
                    return packets::release_765::login::kick("Caught exception: " + std::string(ex.what()));
                }

                Response UnexpectedException() override {
                    return packets::release_765::login::kick("Caught unexpected exception");
                }

                std::list<std::pair<std::string, PluginRegistrationPtr>> plugins_query;
                uint8_t verify_token[4];
                int plugin_message_id = 0;
                bool is_authed = false;
                uint8_t excepted_packet = 0;
                bool has_conflict = false;

            public:
                HandleLogin(TCPsession* sock)
                    : TCPClientHandle(sock) {
                    pluginManagement.inspect_plugin_bind(PluginManagement::registration_on::login, [&](const std::pair<std::string, PluginRegistrationPtr>& it) {
                        plugins_query.push_back(it);
                    });
                }

                HandleLogin()
                    : TCPClientHandle(nullptr) {}

                TCPclient* DefineOurself(TCPsession* sock) override {
                    return new HandleLogin(sock);
                }
            };
        }
    }
}

#endif /* SRC_PROTOCOLHELPER_CLIENT_PLAY_HANDLER_765_LOGIN */
