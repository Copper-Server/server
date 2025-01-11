
#include <src/api/protocol.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/protocolHelper/client_handler/768/768_release.hpp>
#include <src/protocolHelper/client_handler/768/play.hpp>
#include <src/protocolHelper/client_handler/abstract.hpp>
#include <src/protocolHelper/packets/768/packets.hpp>

namespace copper_server::client_handler::release_768 {
    void handle_play::check_response(base_objects::network::response& resp) {
        if (resp.data.empty())
            return;
        size_t index;
    re_check:
        index = 0;
        for (auto& entrys : resp.data) {
            auto& data = entrys.data;
            switch (data[0]) {
            case 0x33: { //client bound ping
                ArrayStream packet(data.data(), data.size());
                int32_t id = ReadVar<int32_t>(packet);
                if (id == -1 && excepted_pong != -1) {
                    resp.data.erase(index);
                    goto re_check;
                }
                excepted_pong = id;
                pong_timer = std::chrono::system_clock::now();
                break;
            }
            default:
                break;
            }
            index++;
        }
    }

    base_objects::network::response handle_play::IdleActions() {
        list_array<base_objects::network::plugin_response> load_next_packets;
        for (size_t i = 0; !queriedPackets.empty(); i++) {
            load_next_packets.push_back(std::move(queriedPackets.front()));
            queriedPackets.pop_front();
        }
        base_objects::network::response response(base_objects::network::response::empty());
        for (auto& packet : session->sharedData().getPendingPackets())
            response += std::move(packet);
        response.reserve(load_next_packets.size());
        for (auto& it : load_next_packets)
            if (it)
                response += *it;
        response += _keep_alive_solution->send_keep_alive();
        return response;
    }

    base_objects::network::response handle_play::work_packet(ArrayStream& packet) {
        uint8_t packet_id = packet.read();
        switch (packet_id) {
        case 0x00:
            play_768_release::teleport_confirm(session, packet);
            break;
        case 0x01:
            play_768_release::query_block_nbt(session, packet);
            break;
        case 0x02:
            play_768_release::bundle_item_selected(session, packet);
            break;
        case 0x03:
            play_768_release::change_difficulty(session, packet);
            break;
        case 0x04:
            play_768_release::acknowledge_message(session, packet);
            break;
        case 0x05:
            play_768_release::chat_command(session, packet);
            break;
        case 0x06:
            play_768_release::signed_chat_command(session, packet);
            break;
        case 0x07:
            play_768_release::chat_message(session, packet);
            break;
        case 0x08:
            play_768_release::player_session(session, packet);
            break;
        case 0x09:
            play_768_release::chunk_batch_received(session, packet);
            break;
        case 0x0A:
            play_768_release::client_status(session, packet);
            break;
        case 0x0B:
            play_768_release::client_tick_end(session, packet);
            break;
        case 0x0C:
            play_768_release::client_information(session, packet);
            break;
        case 0x0D:
            play_768_release::command_suggestion(session, packet);
            break;
        case 0x0E:
            play_768_release::switch_to_configuration(session, packet, next_handler);
            break;
        case 0x0F:
            play_768_release::click_container_button(session, packet);
            break;
        case 0x10:
            play_768_release::click_container(session, packet);
            break;
        case 0x11:
            play_768_release::close_container(session, packet);
            break;
        case 0x12:
            play_768_release::change_container_slot_state(session, packet);
            break;
        case 0x13:
            play_768_release::cookie_response(session, packet);
            break;
        case 0x14: {
            auto tmp = play_768_release::plugin_message(session, packet, queriedPackets);
            if (tmp.data.empty())
                break;
            else
                return tmp;
        }
        case 0x15:
            play_768_release::subscribe_to_debug_sample(session, packet);
            break;
        case 0x16:
            play_768_release::edit_book(session, packet);
            break;
        case 0x17:
            play_768_release::query_entity_tag(session, packet);
            break;
        case 0x18:
            play_768_release::interact(session, packet);
            break;
        case 0x19:
            play_768_release::jigsaw_generate(session, packet);
            break;
        case 0x1A: { //keep alive
            log::debug("play", "Keep alive");
            int64_t keep_alive_packet_response = ReadValue<int64_t>(packet);
            if (keep_alive_packet == keep_alive_packet_response)
                session->sharedData().packets_state.keep_alive_ping_ms = std::min<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(_keep_alive_solution->got_valid_keep_alive()).count(), INT32_MAX);
            api::protocol::on_keep_alive.async_notify({keep_alive_packet_response, *session, session->sharedDataRef()});
            break;
        }
        case 0x1B:
            play_768_release::lock_difficulty(session, packet);
            break;
        case 0x1C:
            play_768_release::set_player_position(session, packet);
            break;
        case 0x1D:
            play_768_release::set_player_position_and_rotation(session, packet);
            break;
        case 0x1E:
            play_768_release::set_player_rotation(session, packet);
            break;
        case 0x1F:
            play_768_release::set_player_movement_flags(session, packet);
            break;
        case 0x20:
            play_768_release::move_vehicle(session, packet);
            break;
        case 0x21:
            play_768_release::paddle_boat(session, packet);
            break;
        case 0x22:
            play_768_release::pick_item_from_block(session, packet);
            break;
        case 0x23:
            play_768_release::pick_item_from_entity(session, packet);
            break;
        case 0x24:
            play_768_release::ping_request(session, packet);
            break;
        case 0x25:
            play_768_release::place_recipe(session, packet);
            break;
        case 0x26:
            play_768_release::player_abilities(session, packet);
            break;
        case 0x27:
            play_768_release::player_action(session, packet);
            break;
        case 0x28:
            play_768_release::player_command(session, packet);
            break;
        case 0x29:
            play_768_release::player_input(session, packet);
            break;
        case 0x2A:
            play_768_release::player_loaded(session, packet);
            break;
        case 0x2B: { //pong
            log::debug("play", "Pong");
            api::protocol::data::pong data;
            data.id = ReadValue<int32_t>(packet);
            data.elapsed = std::chrono::system_clock::now() - pong_timer;
            if (data.id == excepted_pong) {
                api::protocol::on_pong.async_notify({data, *session, session->sharedDataRef()});
                excepted_pong = -1;
            }
            break;
        }
        case 0x2C:
            play_768_release::change_recipe_book_settings(session, packet);
            break;
        case 0x2D:
            play_768_release::set_seen_recipe(session, packet);
            break;
        case 0x2E:
            play_768_release::rename_item(session, packet);
            break;
        case 0x2F:
            play_768_release::resource_pack_response(session, packet);
            break;
        case 0x30:
            play_768_release::seen_advancements(session, packet);
            break;
        case 0x31:
            play_768_release::select_trade(session, packet);
            break;
        case 0x32:
            play_768_release::set_beacon_effect(session, packet);
            break;
        case 0x33:
            play_768_release::set_held_item(session, packet);
            break;
        case 0x34:
            play_768_release::program_command_block(session, packet);
            break;
        case 0x35:
            play_768_release::program_command_cart(session, packet);
            break;
        case 0x36:
            play_768_release::set_creative_slot(session, packet);
            break;
        case 0x37:
            play_768_release::program_jigsaw_block(session, packet);
            break;
        case 0x38:
            play_768_release::program_structure_block(session, packet);
            break;
        case 0x39:
            play_768_release::update_sign(session, packet);
            break;
        case 0x3A:
            play_768_release::swing_arm(session, packet);
            break;
        case 0x3B:
            play_768_release::spectator_teleport(session, packet);
            break;
        case 0x3C:
            play_768_release::use_item_on(session, packet);
            break;
        case 0x3D:
            play_768_release::use_item(session, packet);
            break;
        default: {
            auto error = "Unknown packet id: " + std::to_string(packet_id);
            log::debug_error("play", error);
            return packets::release_768::play::kick(error);
        }
        }
        return IdleActions();
    }

    base_objects::network::response handle_play::too_large_packet() {
        return packets::release_768::play::kick("Packet too large");
    }

    base_objects::network::response handle_play::exception(const std::exception& ex) {
        return packets::release_768::play::kick("Internal server error: " + std::string(ex.what()) + "\nPlease report this to the server owner!");
    }

    base_objects::network::response handle_play::unexpected_exception() {
        return packets::release_768::play::kick("Internal server error\nPlease report this to the server owner!");
    }

    base_objects::network::response handle_play::on_switching() {
        pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
            queriedPackets.push_back(plugin->OnPlay_initialize(session->sharedDataRef()));
        });

        for (auto& plugin : session->sharedData().compatible_plugins)
            queriedPackets.push_back(pluginManagement.getPlugin(plugin)->OnPlay_initialize(session->sharedDataRef()));
        return IdleActions();
    }

    handle_play::handle_play(base_objects::network::tcp_session* session)
        : tcp_client_handle(session) {
        _keep_alive_solution->set_callback(
            [this]() {
                log::debug("play", "Send keep alive");
                keep_alive_packet = generate_random_int();
                return packets::release_768::play::keepAlive(keep_alive_packet);
            }
        );
    }

    handle_play::~handle_play() {
        pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
            queriedPackets.push_back(plugin->OnPlay_uninitialized(session->sharedDataRef()));
        });

        for (auto& plugin : session->sharedData().compatible_plugins)
            queriedPackets.push_back(pluginManagement.getPlugin(plugin)->OnPlay_uninitialized(session->sharedDataRef()));
    }

    base_objects::network::tcp_client* handle_play::define_ourself(base_objects::network::tcp_session* sock) {
        return nullptr;
    }
}