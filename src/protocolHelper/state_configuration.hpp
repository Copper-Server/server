#ifndef SRC_PROTOCOLHELPER_STATE_CONFIGURATION
#define SRC_PROTOCOLHELPER_STATE_CONFIGURATION
#include "state_play.hpp"
#include "util.hpp"

namespace crafted_craft {
    class TCPClientHandleConfiguration : public TCPClientHandle {
    protected:
        Response IdleActions() {
            std::list<PluginRegistration::plugin_response> load_next_packets;
            for (size_t i = 0; i < 20 && !queriedPackets.empty(); i++) {
                load_next_packets.push_back(std::move(queriedPackets.front()));
                queriedPackets.pop_front();
            }
            Response response(Response::Empty());
            response.reserve(load_next_packets.size() + 1);
            if (!inited) {
                response += RegistryData();
                inited = true;
            } else if (load_next_packets.empty()) {
                response += FinishConfiguration();
                timer.cancel();
                return response;
            }
            for (auto& it : load_next_packets) {
                if (std::holds_alternative<PluginRegistration::PluginResponse>(it)) {
                    auto& plugin = std::get<PluginRegistration::PluginResponse>(it);
                    response += packets::configuration::configuration(plugin.plugin_chanel, plugin.data);
                } else if (std::holds_alternative<Response>(it))
                    response += std::get<Response>(it);
            }
            if (!keep_alive_wait)
                response += SendKeepAlive();
            return response;
        }

        Response FinishConfiguration() {
            log::debug("configuration", "Finish configuration");
            return Response::Answer({{2}});
        }

        Response Ping() {
            log::debug("configuration", "Ping");
            pong_timer = std::chrono::system_clock::now();
            excepted_pong = std::chrono::duration_cast<std::chrono::milliseconds>(pong_timer.time_since_epoch()).count();
            list_array<uint8_t> response;
            response.push_back(0x04);
            WriteVar<int32_t>(excepted_pong, response);

            return Response::Answer({std::move(response)});
        }

        Response SendKeepAlive() {
            keep_alive_packet = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            list_array<uint8_t> response;
            response.push_back(0x03);
            WriteValue<int64_t>(keep_alive_packet, response);
            timer.expires_from_now(boost::posix_time::seconds(session->serverData().timeout_seconds));
            timer.async_wait([this](const boost::system::error_code& ec) {
                if (ec == boost::asio::error::operation_aborted)
                    return;
                session->disconnect();
            });
            keep_alive_wait = true;
            log::debug("configuration", "Send keep alive");
            return Response::Answer({std::move(response)});
        }

        Response RegistryData() {
            log::debug("configuration", "Registry data");
            return Response::Answer({registers::registryDataPacket()});
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
                auto it = plugins_configuration.find(channel);
                if (it != plugins_configuration.end()) {
                    auto result = it->second->OnConfigurationHandle(it->second, channel, packet.read_left().to_vector(), session->sharedDataRef());
                    if (std::holds_alternative<PluginRegistration::PluginResponse>(result)) {
                        auto& plugin = std::get<PluginRegistration::PluginResponse>(result);
                        log::debug("configuration", "Plugin message");
                        return packets::configuration::configuration(plugin.plugin_chanel, plugin.data);
                    } else if (std::holds_alternative<Response>(result)) {
                        log::debug("configuration", "Plugin native packet");
                        return std::get<Response>(result);
                    }
                }
                break;
            }
            case 0x02:
                session->sharedData().packets_state.state = SharedClientData::packets_state_t::protocol_state::play;
                log::debug("configuration", "Start game");
                next_handler = new TCPClientHandlePlay(session);
                return Response::Empty();
            case 0x03: { //keep alive
                log::debug("configuration", "Keep alive");
                int64_t keep_alive_packet_response = ReadValue<int64_t>(packet);
                if (keep_alive_packet == keep_alive_packet_response) {
                    timer.cancel();
                    keep_alive_wait = false;
                }
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
            return packets::configuration::kick(ex.what());
        }

        Response UnexpectedException() override {
            return packets::configuration::kick("Unexpected exception");
        }

        //Response OnSwitch() override {
        //    return IdleActions();
        //}

        bool inited = false;
        bool keep_alive_wait = false;
        boost::asio::deadline_timer timer;
        std::list<PluginRegistration::plugin_response> queriedPackets;
        int64_t keep_alive_packet = 0;
        int32_t excepted_pong = 0;
        std::chrono::time_point<std::chrono::system_clock> pong_timer;

    public:
        static std::unordered_map<std::string, PluginRegistrationPtr> plugins_configuration;
        static list_array<PluginRegistrationPtr> base_plugins;

        TCPClientHandleConfiguration(TCPsession* sock)
            : TCPClientHandle(sock), timer(sock->sock.get_executor()) {
            for (auto& plugin : base_plugins)
                queriedPackets.push_back(plugin->OnConfiguration(session->sharedDataRef()));
            for (auto& plugin : session->sharedData().compatible_plugins)
                queriedPackets.push_back(pluginManagement.getPlugin(plugin)->OnConfiguration(session->sharedDataRef()));
        }

        TCPclient* DefineOurself(TCPsession* sock) override {
            return new TCPClientHandleConfiguration(sock);
        }

        void queryPacket(PluginRegistration::PluginResponse&& packet) {
            queriedPackets.push_back(std::move(packet));
        }
    };
}

#endif /* SRC_PROTOCOLHELPER_STATE_CONFIGURATION */
