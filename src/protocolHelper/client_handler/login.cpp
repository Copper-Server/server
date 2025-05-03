#include <src/protocolHelper/client_handler/login.hpp>

#include <src/api/configuration.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/protocolHelper/client_handler/abstract.hpp>
#include <src/protocolHelper/packets/abstract.hpp>

namespace copper_server::client_handler {
    base_objects::network::response handle_login::IdleActions() {
        base_objects::network::response response;
        if (auto packets = session->sharedData().getPendingPackets(); packets.size()) {
            for (auto& packet : packets)
                response += std::move(packet);
        } else {
            auto& login_data = *session->sharedData().packets_state.login_data;
            if (
                (!api::configuration::get().server.offline_mode || api::configuration::get().protocol.enable_encryption) && login_data.login_check == 0
            ) {
                login_data.excepted_packet = 1;
                auto generate_ui8 = []() -> uint8_t {
                    static std::random_device rd;
                    static std::mt19937_64 gen;
                    static std::uniform_int_distribution<uint16_t> dis;
                    uint16_t ui16 = dis(gen);
                    return (uint8_t)((ui16 & 0xFF ^ (ui16 >> 8)) & 0xFF);
                };
                login_data.verify_token[0] = generate_ui8();
                login_data.verify_token[1] = generate_ui8();
                login_data.verify_token[2] = generate_ui8();
                login_data.verify_token[3] = generate_ui8();

                return packets::login::encryptionRequest(session->sharedData(), "", login_data.verify_token);
            }
            login_data.login_check = 1;
            if (auto threshold = api::configuration::get().protocol.compression_threshold; threshold != -1 && login_data.login_check == 1) {
                base_objects::network::response response = packets::login::setCompression(session->sharedData(), threshold);
                log::debug("login", "set compression");
                if (login_data.plugins_query.empty()) {
                    response += packets::login::loginSuccess(session->sharedData());
                    login_data.excepted_packet = 3;
                }
                return response;
            }
            login_data.login_check = 2;
            while (!login_data.plugins_query.empty()) {
                auto& self = login_data.plugins_query.front();
                auto plugin_response = self.second->OnLoginStart(self.second, self.first, session->sharedDataRef());
                auto res = std::visit(
                    [&](auto& it) {
                        using T = std::decay_t<decltype(it)>;
                        if constexpr (std::is_same_v<T, PluginRegistration::PluginResponse>) {
                            login_data.plugin_sequence_id++;
                            return packets::login::login(session->sharedData(), login_data.plugin_sequence_id, it.plugin_chanel, it.data);
                        } else if constexpr (std::is_same_v<T, base_objects::network::response>) {
                            return std::move(it);
                        } else if (!it) {
                            login_data.plugins_query.pop_front();
                        }
                        return base_objects::network::response::empty();
                    },
                    plugin_response
                );
                if (res.is_disconnect() || res.data.size())
                    return res;
            }

            login_data.login_check = 3;
            response += packets::login::loginSuccess(session->sharedData());
            login_data.excepted_packet = 3;
        }

        return response;
    }

    base_objects::network::response handle_login::work_packet(ArrayStream& packet) {
        auto packet_id = packet.read_var<int32_t>();
        log::debug("play", "Work packet: " + registry.get_packet_name(packet_id));
        registry.handle(packet_id, session, packet);
        return IdleActions();
    }

    base_objects::network::response handle_login::too_large_packet() {
        return packets::login::kick(session->sharedData(), "Packet too large");
    }

    base_objects::network::response handle_login::exception(const std::exception& ex) {
        return packets::login::kick(session->sharedData(), "Internal server error: " + std::string(ex.what()) + "\nPlease report this to the server owner!");
    }

    base_objects::network::response handle_login::unexpected_exception() {
        return packets::login::kick(session->sharedData(), "Internal server error\nPlease report this to the server owner!");
    }

    base_objects::network::response handle_login::on_switching() {
        session->sharedData().setSwitchToHandlerCallback([this](base_objects::network::tcp::client* cl) {
            next_handler = cl;
        });
        return base_objects::network::response::empty();
    }

    handle_login::handle_login(base_objects::network::tcp::session* session)
        : tcp_client_handle(session), registry(base_objects::network::tcp::packet_registry.serverbound.login.get(session->protocol_version)) {
        auto& login_data = *(session->sharedData().packets_state.login_data = base_objects::SharedClientData::packets_state_t::state_login{});
        session->sharedData().packets_state.protocol_version = session->protocol_version;

        pluginManagement.inspect_plugin_bind(PluginManagement::registration_on::login, [&login_data](const std::pair<std::string, PluginRegistrationPtr>& it) {
            login_data.plugins_query.push_back(it);
        });
    }

    handle_login::~handle_login() {
        session->sharedData().packets_state.login_data = nullptr;
    }

    base_objects::network::tcp::client* handle_login::define_ourself(base_objects::network::tcp::session* sock) {
        return nullptr;
    }
}