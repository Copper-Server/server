#include <src/build_in_plugins/network/tcp/client_handler/configuration.hpp>

#include <src/api/configuration.hpp>
#include <src/api/packets.hpp>
#include <src/api/protocol.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/build_in_plugins/network/tcp/client_handler/abstract.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/resources/registers.hpp>

namespace copper_server::build_in_plugins::network::tcp::client_handler {
    base_objects::network::response handle_configuration::IdleActions() {
        using load_state_t = base_objects::SharedClientData::packets_state_t::configuration_load_state_t;
        auto& shared_data = session->shared_data();
        auto& load_state = shared_data.packets_state.load_state;
        base_objects::network::response response(base_objects::network::response::empty());
        if (load_state == load_state_t::to_init) {
            std::string assigned_version = base_objects::packets::protocol_to_java_name(shared_data.packets_state.protocol_version);
            response += api::packets::configuration::knownPacks(
                shared_data,
                resources::loaded_packs()
                    .transform([&assigned_version](auto&& pack) {
                        pack.version = assigned_version;
                        return std::move(pack);
                    })
            );
            load_state = load_state_t::await_known_packs;
            response += _keep_alive_solution->make_keep_alive_packet();
            return response;
        } else if (auto packets = session->shared_data().getPendingPackets(); !packets.empty()) {
            for (auto& packet : packets)
                response += std::move(packet);
        } else if (response.data.empty() && load_state == load_state_t::await_processing) {
            load_state = load_state_t::done;
        }
        if (load_state == load_state_t::done) {
            response += api::packets::configuration::finish(shared_data);
            _keep_alive_solution = nullptr;
            return response;
        }
        return response;
    }

    base_objects::network::response handle_configuration::work_packet(ArrayStream& packet) {
        auto packet_id = packet.read_var<int32_t>();
        log::debug("configuration", "Work packet: " + registry.get_packet_name(packet_id));
        registry.handle(packet_id, session, packet);
        return IdleActions();
    }

    base_objects::network::response handle_configuration::too_large_packet() {
        return api::packets::configuration::kick(session->shared_data(), "Packet too large");
    }

    base_objects::network::response handle_configuration::exception(const std::exception& ex) {
        return api::packets::configuration::kick(session->shared_data(), "Internal server error: " + std::string(ex.what()) + "\nPlease report this to the server owner!");
    }

    base_objects::network::response handle_configuration::unexpected_exception() {
        return api::packets::configuration::kick(session->shared_data(), "Internal server error\nPlease report this to the server owner!");
    }

    base_objects::network::response handle_configuration::on_switching() {
        pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::configuration, [&](auto&& plugin) {
            if (auto res = plugin->OnConfiguration(session->shared_data_ref()); res)
                session->shared_data().sendPacket(std::move(*res));
        });
        session->shared_data().setSwitchToHandlerCallback([this](base_objects::network::tcp::client* cl) {
            next_handler = cl;
            _keep_alive_solution = nullptr;
        });
        session->request_buffer(api::configuration::get().protocol.buffer);
        _keep_alive_solution->start();
        return base_objects::network::response::empty();
    }

    handle_configuration::handle_configuration(api::network::tcp::session* session)
        : tcp_client_handle(session), registry(base_objects::network::tcp::packet_registry.serverbound.configuration.get(session->shared_data().packets_state.protocol_version)) {
        _keep_alive_solution->set_callback(
            [this](int64_t keep_alive_packet) {
                log::debug("configuration", "Send keep alive");
                return api::packets::configuration::keep_alive(this->session->shared_data(), keep_alive_packet);
            }
        );
        session->shared_data().setKeepAliveCallback([this](int64_t keep_alive_packet) {
            auto delay = _keep_alive_solution->got_valid_keep_alive(keep_alive_packet);
            this->session->shared_data().packets_state.keep_alive_ping_ms = (int32_t)std::min<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(delay).count(), INT32_MAX);
            api::protocol::on_keep_alive.async_notify({keep_alive_packet, *this->session, this->session->shared_data_ref()});
        });
    }

    handle_configuration::~handle_configuration() {
        pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
            if (auto res = plugin->OnPlay_uninitialized(session->shared_data_ref()); res)
                session->shared_data().sendPacket(std::move(*res));
        });

        for (auto& plugin : session->shared_data().compatible_plugins) {
            if (auto res = pluginManagement.getPlugin(plugin)->OnPlay_uninitialized_compatible(session->shared_data_ref()); res)
                session->shared_data().sendPacket(std::move(*res));
        }
    }

    base_objects::network::tcp::client* handle_configuration::define_ourself(api::network::tcp::session* sock) {
        return nullptr;
    }
}