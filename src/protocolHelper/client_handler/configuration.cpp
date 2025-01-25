#include <src/protocolHelper/client_handler/configuration.hpp>

#include <src/api/protocol.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/protocolHelper/client_handler/abstract.hpp>
#include <src/protocolHelper/packets/abstract.hpp>
#include <src/resources/registers.hpp>

namespace copper_server::client_handler {
    base_objects::network::response handle_configuration::IdleActions() {
        using load_state_t = base_objects::SharedClientData::packets_state_t::configuration_load_state_t;
        auto& shared_data = session->sharedData();
        auto& load_state = shared_data.packets_state.load_state;
        base_objects::network::response response(base_objects::network::response::empty());
        if (auto packets = session->sharedData().getPendingPackets(); !packets.empty()) {
            for (auto& packet : packets)
                response += std::move(packet);
            if (load_state == load_state_t::done)
                load_state = load_state_t::await_processing;
        } else if (load_state == load_state_t::to_init) {
            std::string assigned_version = base_objects::packets::protocol_to_java_name(shared_data.packets_state.protocol_version);
            response += packets::configuration::knownPacks(
                shared_data,
                resources::loaded_packs()
                    .transform([&assigned_version](auto&& pack) {
                        pack.version = assigned_version;
                        return std::move(pack);
                    })
            );
            load_state = load_state_t::await_known_packs;
            return response;
        } else if (response.data.empty() && load_state == load_state_t::await_processing) {
            load_state = load_state_t::done;
        } else if (load_state == load_state_t::done) {
            response += packets::configuration::finish(shared_data);
            _keep_alive_solution->ignore_keep_alive();
            return response;
        }
        response += _keep_alive_solution->send_keep_alive();
        return response;
    }

    base_objects::network::response handle_configuration::work_packet(ArrayStream& packet) {
        auto packet_id = packet.read_var<int32_t>();
        log::debug("configuration", "Work packet: " + registry.get_packet_name(packet_id));
        registry.handle(packet_id, session, packet);
        return IdleActions();
    }

    base_objects::network::response handle_configuration::too_large_packet() {
        return packets::configuration::kick(session->sharedData(), "Packet too large");
    }

    base_objects::network::response handle_configuration::exception(const std::exception& ex) {
        return packets::configuration::kick(session->sharedData(), "Internal server error: " + std::string(ex.what()) + "\nPlease report this to the server owner!");
    }

    base_objects::network::response handle_configuration::unexpected_exception() {
        return packets::configuration::kick(session->sharedData(), "Internal server error\nPlease report this to the server owner!");
    }

    base_objects::network::response handle_configuration::on_switching() {
        pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::configuration, [&](auto&& plugin) {
            if (auto res = plugin->OnConfiguration(session->sharedDataRef()); res)
                session->sharedData().sendPacket(std::move(*res));
        });
        session->sharedData().setSwitchToHandlerCallback([this](tcp_client* cl) {
            next_handler = cl;
        });
        return base_objects::network::response::empty();
    }

    handle_configuration::handle_configuration(base_objects::network::tcp_session* session)
        : tcp_client_handle(session), registry(base_objects::network::packet_registry.serverbound.configuration.get(session->sharedData().packets_state.protocol_version)) {
        _keep_alive_solution->set_callback(
            [this](int64_t keep_alive_packet) {
                log::debug("configuration", "Send keep alive");
                return packets::configuration::keep_alive(this->session->sharedData(), keep_alive_packet);
            }
        );
        session->sharedData().setKeepAliveCallback([this](int64_t keep_alive_packet) {
            auto delay = _keep_alive_solution->got_valid_keep_alive(keep_alive_packet);
            this->session->sharedData().packets_state.keep_alive_ping_ms = std::min<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(delay).count(), INT32_MAX);
            api::protocol::on_keep_alive.async_notify({keep_alive_packet, *this->session, this->session->sharedDataRef()});
        });
    }

    handle_configuration::~handle_configuration() {
        pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
            if (auto res = plugin->OnPlay_uninitialized(session->sharedDataRef()); res)
                session->sharedData().sendPacket(std::move(*res));
        });

        for (auto& plugin : session->sharedData().compatible_plugins) {
            if (auto res = pluginManagement.getPlugin(plugin)->OnPlay_uninitialized_compatible(session->sharedDataRef()); res)
                session->sharedData().sendPacket(std::move(*res));
        }
    }

    base_objects::network::tcp_client* handle_configuration::define_ourself(base_objects::network::tcp_session* sock) {
        return nullptr;
    }
}