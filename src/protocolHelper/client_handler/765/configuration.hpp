#ifndef SRC_PROTOCOLHELPER_CLIENT_HANDLER_765_CONFIGURATION
#define SRC_PROTOCOLHELPER_CLIENT_HANDLER_765_CONFIGURATION
#include "../../packets/765/packets.hpp"
#include "../abstract.hpp"

namespace crafted_craft {
    namespace client_handler {
        namespace release_765 {
            class HandleConfiguration : public TCPClientHandle {
            protected:
                Response IdleActions() {
                    list_array<PluginRegistration::plugin_response> load_next_packets;
                    Response response(Response::Empty());
                    if (!inited) {
                        response += RegistryData();
                        inited = true;
                        return response;
                    } else if (queriedPackets.empty()) {
                        response += FinishConfiguration();
                        keep_alive_solution->got_valid_keep_alive();
                        return response;
                    }

                    for (auto& packet : session->sharedData().getPendingPackets())
                        response += std::move(packet);

                    response.reserve(load_next_packets.size());
                    for (size_t i = 0; !queriedPackets.empty(); i++) {
                        load_next_packets.push_back(std::move(queriedPackets.front()));
                        queriedPackets.pop_front();
                    }
                    for (auto& it : load_next_packets) {
                        if (std::holds_alternative<PluginRegistration::PluginResponse>(it)) {
                            auto& plugin = std::get<PluginRegistration::PluginResponse>(it);
                            response += packets::release_765::configuration::configuration(plugin.plugin_chanel, plugin.data);
                        } else if (std::holds_alternative<Response>(it))
                            response += std::get<Response>(it);
                    }
                    response += keep_alive_solution->send_keep_alive();
                    return response;
                }

                Response FinishConfiguration() {
                    log::debug("configuration", "Finish configuration");
                    return packets::release_765::configuration::finish();
                }

                Response Ping() {
                    log::debug("configuration", "Ping");
                    pong_timer = std::chrono::system_clock::now();
                    return packets::release_765::configuration::ping(excepted_pong = generate_random_int());
                }

                Response RegistryData() {
                    log::debug("configuration", "Registry data");
                    return packets::release_765::configuration::registry_data();
                }

                Response WorkPacket(ArrayStream& packet) override {
                    uint8_t packet_id = packet.read();
                    switch (packet_id) {
                    case 0x00: { //client settings
                        log::debug("configuration", "Client settings");
                        auto& shared_data = session->sharedData();
                        shared_data.locale = ReadString(packet, 16);
                        shared_data.view_distance = packet.read();
                        shared_data.chat_mode = (SharedClientData::ChatMode)ReadVar<int32_t>(packet);
                        shared_data.enable_chat_colors = packet.read();
                        shared_data.skin_parts.mask = packet.read();
                        shared_data.main_hand = (SharedClientData::MainHand)ReadVar<int32_t>(packet);
                        shared_data.enable_filtering = packet.read();
                        shared_data.allow_server_listings = packet.read();
                        break;
                    }
                    case 0x01: { //plugin message
                        log::debug("configuration", "Plugin message");
                        std::string channel = ReadString(packet, 32767);
                        auto it = pluginManagement.get_bind_plugin(PluginManagement::registration_on::configuration, channel);
                        if (it != nullptr) {
                            auto result = it->OnConfigurationHandle(it, channel, packet.read_left().to_vector(), session->sharedDataRef());
                            if (std::holds_alternative<PluginRegistration::PluginResponse>(result)) {
                                auto& plugin = std::get<PluginRegistration::PluginResponse>(result);
                                log::debug("configuration", "Plugin message");
                                return packets::release_765::configuration::configuration(plugin.plugin_chanel, plugin.data);
                            } else if (std::holds_alternative<Response>(result)) {
                                log::debug("configuration", "Plugin native packet");
                                return std::get<Response>(result);
                            }
                        }
                        break;
                    }
                    case 0x02: //configuration complete
                        if (!session->sharedData().packets_state.pending_resource_packs.empty())
                            return packets::release_765::configuration::kick("You are not downloaded all requested packs.");
                        session->sharedData().packets_state.state = SharedClientData::packets_state_t::protocol_state::play;
                        log::debug("configuration", "Start game");
                        next_handler = abstract::createHandlePlay(session);
                        return Response::Empty();
                    case 0x03: { //keep alive
                        log::debug("configuration", "Keep alive");
                        int64_t keep_alive_packet_response = ReadValue<int64_t>(packet);
                        if (keep_alive_packet == keep_alive_packet_response)
                            session->sharedData().packets_state.keep_alive_ping_ms = std::min<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(keep_alive_solution->got_valid_keep_alive()).count(), INT32_MAX);
                        break;
                    }
                    case 0x04: { //pong
                        log::debug("configuration", "Pong");
                        int32_t pong = ReadValue<int32_t>(packet);
                        if (pong == excepted_pong)
                            excepted_pong = 0;
                        else
                            return Response::Disconnect();
                        session->sharedData().ping = std::chrono::duration_cast<std::chrono::milliseconds>(pong_timer - std::chrono::system_clock::now());
                        break;
                    }
                    case 0x05: { //registry resource pack
                        log::debug("configuration", "Registry resource pack");
                        ENBT::UUID id = ReadUUID(packet);
                        int32_t result = ReadVar<int32_t>(packet);
                        auto res = session->sharedData().packets_state.pending_resource_packs.find(id);
                        if (res != session->sharedData().packets_state.pending_resource_packs.end()) {
                            switch (result) {
                            case 0:
                            case 3:
                                session->sharedData().packets_state.active_resource_packs.insert(id);
                                session->sharedData().packets_state.pending_resource_packs.erase(res);
                                break;
                            default:
                                if (res->second.required)
                                    return Response::Disconnect();
                                else
                                    session->sharedData().packets_state.pending_resource_packs.erase(res);
                            }
                        }
                        break;
                    }
                    default:
                        break;
                    }
                    return IdleActions();
                }

                Response TooLargePacket() override {
                    return Response::Disconnect();
                }

                Response Exception(const std::exception& ex) override {
                    return packets::release_765::configuration::kick(ex.what());
                }

                Response UnexpectedException() override {
                    return packets::release_765::configuration::kick("Unexpected exception");
                }

                std::list<PluginRegistration::plugin_response> queriedPackets;
                std::chrono::time_point<std::chrono::system_clock> pong_timer;
                int64_t keep_alive_packet = 0;
                int32_t excepted_pong = 0;
                bool inited = false;

            public:
                HandleConfiguration(TCPsession* sock)
                    : TCPClientHandle(sock) {
                    keep_alive_solution->set_callback([this]() {
                        log::debug("configuration", "Send keep alive");
                        return packets::release_765::configuration::keep_alive(keep_alive_packet);
                    });

                    pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::configuration, [this](PluginRegistrationPtr plugin) {
                        queriedPackets.push_back(plugin->OnConfiguration(session->sharedDataRef()));
                    });
                    for (auto& plugin : session->sharedData().compatible_plugins)
                        queriedPackets.push_back(pluginManagement.getPlugin(plugin)->OnConfiguration(session->sharedDataRef()));
                }

                TCPclient* DefineOurself(TCPsession* sock) override {
                    return new HandleConfiguration(sock);
                }

                void queryPacket(PluginRegistration::PluginResponse&& packet) {
                    queriedPackets.push_back(std::move(packet));
                }
            };
        }
    }
}

#endif /* SRC_PROTOCOLHELPER_CLIENT_HANDLER_765_CONFIGURATION */
