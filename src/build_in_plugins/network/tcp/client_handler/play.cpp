#include <src/build_in_plugins/network/tcp/client_handler/play.hpp>

#include <src/api/packets.hpp>
#include <src/api/protocol.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/build_in_plugins/network/tcp/client_handler/abstract.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::network::tcp::client_handler {

    void handle_play::check_response(base_objects::network::response& resp) {
        if (resp.data.empty())
            return;
        int32_t ping_id = registry.get_packet_state("minecraft:ping");
        bool got_ping = false;
        auto& state = session->shared_data().packets_state;
        resp.data.remove_if(
            [&got_ping, ping_id, &state](auto& entrys) {
                auto& data = entrys.data;
                if (!data.empty()) {
                    ArrayStream packet(data.data(), data.size());
                    if (data[0] == packet.read_var<int32_t>()) {
                        if (state.excepted_pong != -1 || got_ping)
                            return true;
                        int32_t id = packet.read_var<int32_t>();
                        if (id == -1)
                            return true;
                        state.excepted_pong = id;
                        state.pong_timer = std::chrono::system_clock::now();
                        got_ping = true;
                        return true;
                    }
                }
                return false;
            }
        );
    }

    base_objects::network::response handle_play::IdleActions() {
        base_objects::network::response response;
        for (auto& packet : session->shared_data().getPendingPackets())
            response += std::move(packet);
        return response;
    }

    base_objects::network::response handle_play::work_packet(ArrayStream& packet) {
        auto packet_id = packet.read_var<int32_t>();
        log::debug("play", "Work packet: " + registry.get_packet_name(packet_id));
        registry.handle(packet_id, session, packet);
        return IdleActions();
    }

    base_objects::network::response handle_play::too_large_packet() {
        return api::packets::play::kick(session->shared_data(), "Packet too large");
    }

    base_objects::network::response handle_play::exception(const std::exception& ex) {
        return api::packets::play::kick(session->shared_data(), "Internal server error: " + std::string(ex.what()) + "\nPlease report this to the server owner!");
    }

    base_objects::network::response handle_play::unexpected_exception() {
        return api::packets::play::kick(session->shared_data(), "Internal server error\nPlease report this to the server owner!");
    }

    base_objects::network::response handle_play::on_switching() {
        session->shared_data().packets_state.play_data = base_objects::SharedClientData::packets_state_t::state_play();
        pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
            if (auto res = plugin->OnPlay_initialize(session->shared_data_ref()); res)
                session->shared_data().sendPacket(std::move(*res));
        });

        for (auto& plugin : session->shared_data().compatible_plugins) {
            if (auto res = pluginManagement.getPlugin(plugin)->OnPlay_initialize_compatible(session->shared_data_ref()); res)
                session->shared_data().sendPacket(std::move(*res));
        }
        session->shared_data().setSwitchToHandlerCallback([this](base_objects::network::tcp::client* cl) {
            next_handler = cl;
            _keep_alive_solution = nullptr;
        });
        _keep_alive_solution->start();
        return IdleActions();
    }

    handle_play::handle_play(api::network::tcp::session* session)
        : tcp_client_handle(session), registry(base_objects::network::tcp::packet_registry.serverbound.play.get(session->shared_data().packets_state.protocol_version)) {
        handle_tick_sync = registry.has_packet_name("client_tick_end");
        await_for_player_loading = registry.has_packet_name("player_loaded");
        _keep_alive_solution->set_callback(
            [this](int64_t keep_alive_packet) {
                log::debug("play", "Send keep alive");
                return api::packets::play::keepAlive(this->session->shared_data(), keep_alive_packet);
            }
        );
        session->shared_data().setKeepAliveCallback([this](int64_t keep_alive_packet) {
            auto delay = _keep_alive_solution->got_valid_keep_alive(keep_alive_packet);
            this->session->shared_data().packets_state.keep_alive_ping_ms = (int32_t)std::min<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(delay).count(), INT32_MAX);
            api::protocol::on_keep_alive.async_notify({keep_alive_packet, *this->session, this->session->shared_data_ref()});
        });
    }

    handle_play::~handle_play() {
        pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
            if (auto res = plugin->OnPlay_uninitialized(session->shared_data_ref()); res)
                session->shared_data().sendPacket(std::move(*res));
        });

        for (auto& plugin : session->shared_data().compatible_plugins) {
            if (auto res = pluginManagement.getPlugin(plugin)->OnPlay_uninitialized_compatible(session->shared_data_ref()); res)
                session->shared_data().sendPacket(std::move(*res));
        }
    }

    base_objects::network::tcp::client* handle_play::define_ourself(api::network::tcp::session* sock) {
        return nullptr;
    }
}