#include <src/api/protocol.hpp>
#include <src/base_objects/data_packs/known_pack.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/protocolHelper/client_handler/768/configuration.hpp>
#include <src/protocolHelper/client_handler/abstract.hpp>
#include <src/protocolHelper/packets/768/packets.hpp>

namespace copper_server::client_handler::release_768 {
    base_objects::network::response handle_configuration::IdleActions() {
        list_array<base_objects::network::plugin_response> load_next_packets;
        base_objects::network::response response(base_objects::network::response::empty());
        if (!inited) {
            response += RegistryData();
            inited = true;
            return response;
        } else if (queriedPackets.empty()) {
            response += FinishConfiguration();
            _keep_alive_solution->ignore_keep_alive();
            return response;
        }

        for (auto& packet : session->sharedData().getPendingPackets())
            response += std::move(packet);

        response.reserve(queriedPackets.size());
        while (!queriedPackets.empty()) {
            load_next_packets.push_back(std::move(queriedPackets.front()));
            queriedPackets.pop_front();
        }
        for (auto& it : load_next_packets)
            if (it)
                response += *it;

        response += _keep_alive_solution->send_keep_alive();
        return response;
    }

    base_objects::network::response handle_configuration::FinishConfiguration() {
        log::debug("configuration", "Finish configuration");
        return packets::release_768::configuration::finish();
    }

    base_objects::network::response handle_configuration::Ping() {
        log::debug("configuration", "Ping");
        pong_timer = std::chrono::system_clock::now();
        return packets::release_768::configuration::ping(excepted_pong = generate_random_int());
    }

    base_objects::network::response handle_configuration::RegistryData() {
        log::debug("configuration", "Registry data");
        return packets::release_768::configuration::registry_data();
    }

    //Server bound@0x07 new(known packs)
    //  [var_int] count
    //     {
    //      [String] namespace
    //      [String] ID
    //      [String] version
    //     }
    //  *If the client specifies a pack in this packet, the server should omit its contained data from the Registry Data packet.
    //
    base_objects::network::response handle_configuration::work_packet(ArrayStream& packet) {
        uint8_t packet_id = packet.read();
        switch (packet_id) {
        case 0x00: { //client settings
            log::debug("configuration", "Client settings");
            auto& shared_data = session->sharedData();
            shared_data.locale = packet.read_string(16);
            shared_data.view_distance = packet.read();
            shared_data.chat_mode = (base_objects::SharedClientData::ChatMode)packet.read_var<int32_t>();
            shared_data.enable_chat_colors = packet.read();
            shared_data.skin_parts.mask = packet.read();
            shared_data.main_hand = (base_objects::SharedClientData::MainHand)packet.read_var<int32_t>();
            shared_data.enable_filtering = packet.read();
            shared_data.allow_server_listings = packet.read();
            shared_data.particle_status = (base_objects::SharedClientData::ParticleStatus)packet.read_var<int32_t>();
            break;
        }
        case 0x01: { //cookie response
            api::protocol::data::cookie_response data;
            data.key = packet.read_identifier();
            if (packet.read())
                data.payload = packet.read_array<uint8_t>(5120);
            api::protocol::on_cookie_response.async_notify({data, *session, session->sharedDataRef()});
            break;
        }
        case 0x02: { //plugin message
            log::debug("configuration", "Plugin message");
            std::string channel = packet.read_string(32768);
            auto it = pluginManagement.get_bind_plugin(PluginManagement::registration_on::configuration, channel);
            if (it != nullptr) {
                auto result = it->OnConfigurationHandle(it, channel, packet.read_left(32767).to_vector(), session->sharedDataRef());
                if (result)
                    return *result;
            }
            break;
        }
        case 0x03: //configuration complete
            if (!session->sharedData().packets_state.pending_resource_packs.empty())
                return packets::release_768::configuration::kick("You are not downloaded all requested packs.");

            session->sharedData().packets_state.state = base_objects::SharedClientData::packets_state_t::protocol_state::play;
            log::debug("configuration", "Start game");
            next_handler = abstract::createhandle_play(session);
            return base_objects::network::response::empty();
        case 0x04: { //keep alive
            log::debug("configuration", "Keep alive");
            int64_t keep_alive_packet_response = packet.read_value<int64_t>();
            auto delay = _keep_alive_solution->got_valid_keep_alive(keep_alive_packet_response);
            session->sharedData().packets_state.keep_alive_ping_ms = std::min<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(delay).count(), INT32_MAX);
            api::protocol::on_keep_alive.async_notify({keep_alive_packet_response, *session, session->sharedDataRef()});
            break;
        }
        case 0x05: { //pong
            log::debug("configuration", "Pong");
            int32_t pong = packet.read_value<int32_t>();
            if (pong == excepted_pong)
                excepted_pong = 0;
            else
                return base_objects::network::response::disconnect();
            session->sharedData().ping = std::chrono::duration_cast<std::chrono::milliseconds>(pong_timer - std::chrono::system_clock::now());
            break;
        }
        case 0x06: { //registry resource pack
            log::debug("configuration", "Registry resource pack");
            enbt::raw_uuid id = packet.read_uuid();
            int32_t result = packet.read_var<int32_t>();
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
                        return base_objects::network::response::disconnect();
                    else
                        session->sharedData().packets_state.pending_resource_packs.erase(res);
                }
            }
            break;
        }
        case 0x07: {
            int32_t len = packet.read_var<int32_t>();
            list_array<base_objects::data_packs::known_pack> packs;
            for (int32_t i = 0; i < len; i++) {
                std::string name = packet.read_string(32768);
                std::string id = packet.read_string(32768);
                std::string version = packet.read_string(32768);

                packs.emplace_back(std::move(name), std::move(id), std::move(version));
            }
            pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::configuration, [this, &packs](PluginRegistrationPtr plugin) {
                queriedPackets.push_back(plugin->OnConfiguration_gotKnownPacks(session->sharedDataRef(), packs));
            });
            break;
        }
        default:
            break;
        }
        return IdleActions();
    }

    base_objects::network::response handle_configuration::too_large_packet() {
        return base_objects::network::response::disconnect();
    }

    base_objects::network::response handle_configuration::exception(const std::exception& ex) {
        return packets::release_768::configuration::kick(ex.what());
    }

    base_objects::network::response handle_configuration::unexpected_exception() {
        return packets::release_768::configuration::kick("Unexpected exception");
    }

    handle_configuration::handle_configuration(base_objects::network::tcp_session* sock)
        : tcp_client_handle(sock) {
        _keep_alive_solution->set_callback([this](int64_t keep_alive_packet) {
            log::debug("configuration", "Send keep alive");
            return packets::release_768::configuration::keep_alive(keep_alive_packet);
        });

        pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::configuration, [this](PluginRegistrationPtr plugin) {
            queriedPackets.push_back(plugin->OnConfiguration(session->sharedDataRef()));
        });
        for (auto& plugin : session->sharedData().compatible_plugins)
            queriedPackets.push_back(pluginManagement.getPlugin(plugin)->OnConfiguration(session->sharedDataRef()));
    }

    handle_configuration::~handle_configuration() = default;

    base_objects::network::tcp_client*
    handle_configuration::define_ourself(base_objects::network::tcp_session* sock) {
        return new handle_configuration(sock);
    }

    void handle_configuration::queryPacket(const PluginRegistration::PluginResponse& packet) {
        queriedPackets.push_back(
            packets::release_768::configuration::configuration(packet.plugin_chanel, packet.data)
        );
    }
}