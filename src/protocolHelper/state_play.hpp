#ifndef SRC_PROTOCOLHELPER_STATE_PLAY
#define SRC_PROTOCOLHELPER_STATE_PLAY
#include "../api/protocol.hpp"
#include "client_play_handler/765_release.hpp"
#include "packets.hpp"
#include "util.hpp"
#include <random>

namespace crafted_craft {
    class TCPClientHandlePlay : public TCPClientHandle {
        std::list<PluginRegistration::plugin_response> queriedPackets;
        int64_t keep_alive_packet = 0;
        int32_t excepted_pong = 0;
        std::chrono::time_point<std::chrono::system_clock> pong_timer;

        void check_response(Response& resp) {
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

        Response IdleActions() {
            std::list<PluginRegistration::plugin_response> load_next_packets;
            for (size_t i = 0; !queriedPackets.empty(); i++) {
                load_next_packets.push_back(std::move(queriedPackets.front()));
                queriedPackets.pop_front();
            }
            Response response(Response::Empty());
            for (auto& packet : session->sharedData().getPendingPackets())
                response += std::move(packet);
            response.reserve(load_next_packets.size());
            for (auto& it : load_next_packets) {
                if (std::holds_alternative<PluginRegistration::PluginResponse>(it)) {
                    auto& plugin = std::get<PluginRegistration::PluginResponse>(it);
                    response += packets::play::customPayload(plugin.plugin_chanel, plugin.data);
                } else if (std::holds_alternative<Response>(it)) {
                    Response& resp = std::get<Response>(it);
                    check_response(resp);
                    response += resp;
                }
            }
            response += keep_alive_solution->send_keep_alive();
            return response;
        }

        Response WorkPacket(ArrayStream& packet) override {
            uint8_t packet_id = packet.read();
            switch (packet_id) {
            case 0x00:
                client_play_handler::proto_765_release::teleport_confirm(session, packet);
                break;
            case 0x01:
                client_play_handler::proto_765_release::query_block_nbt(session, packet);
                break;
            case 0x02:
                client_play_handler::proto_765_release::change_difficulty(session, packet);
                break;
            case 0x03:
                client_play_handler::proto_765_release::acknowledge_message(session, packet);
                break;
            case 0x04:
                client_play_handler::proto_765_release::chat_command(session, packet);
                break;
            case 0x05:
                client_play_handler::proto_765_release::chat_message(session, packet);
                break;
            case 0x06:
                client_play_handler::proto_765_release::player_session(session, packet);
                break;
            case 0x07:
                client_play_handler::proto_765_release::chunk_batch_received(session, packet);
                break;
            case 0x08:
                client_play_handler::proto_765_release::client_status(session, packet);
                break;
            case 0x09:
                client_play_handler::proto_765_release::client_information(session, packet);
                break;
            case 0x0A:
                client_play_handler::proto_765_release::command_suggestion(session, packet);
                break;
            case 0x0B:
                client_play_handler::proto_765_release::switch_to_configuration(session, packet, next_handler);
                break;
            case 0x0C:
                client_play_handler::proto_765_release::click_container_button(session, packet);
                break;
            case 0x0D:
                client_play_handler::proto_765_release::click_container(session, packet);
                break;
            case 0x0E:
                client_play_handler::proto_765_release::close_container(session, packet);
                break;
            case 0x0F:
                client_play_handler::proto_765_release::change_container_slot_state(session, packet);
                break;
            case 0x10: {
                auto tmp = client_play_handler::proto_765_release::plugin_message(session, packet, TCPClientHandlePlay::plugins_play, queriedPackets);
                if (tmp.data.empty())
                    break;
                else
                    return tmp;
            }
            case 0x11:
                client_play_handler::proto_765_release::edit_book(session, packet);
                break;
            case 0x12:
                client_play_handler::proto_765_release::query_entity_tag(session, packet);
                break;
            case 0x13:
                client_play_handler::proto_765_release::interact(session, packet);
                break;
            case 0x14:
                client_play_handler::proto_765_release::jigsaw_generate(session, packet);
                break;
            case 0x15: { //keep alive
                log::debug("play", "Keep alive");
                int64_t keep_alive_packet_response = ReadValue<int64_t>(packet);
                if (keep_alive_packet == keep_alive_packet_response)
                    session->sharedData().packets_state.keep_alive_ping_ms = std::min<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(keep_alive_solution->got_valid_keep_alive()).count(), INT32_MAX);
                api::protocol::on_keep_alive.async_notify({keep_alive_packet_response, *session, session->sharedDataRef()});
                break;
            }
            case 0x16:
                client_play_handler::proto_765_release::lock_difficulty(session, packet);
                break;
            case 0x17:
                client_play_handler::proto_765_release::set_player_position(session, packet);
                break;
            case 0x18:
                client_play_handler::proto_765_release::set_player_position_and_rotation(session, packet);
                break;
            case 0x19:
                client_play_handler::proto_765_release::set_player_rotation(session, packet);
                break;
            case 0x1A:
                client_play_handler::proto_765_release::set_player_on_ground(session, packet);
                break;
            case 0x1B:
                client_play_handler::proto_765_release::move_vehicle(session, packet);
                break;
            case 0x1C:
                client_play_handler::proto_765_release::paddle_boat(session, packet);
                break;
            case 0x1D:
                client_play_handler::proto_765_release::pick_item(session, packet);
                break;
            case 0x1E:
                client_play_handler::proto_765_release::ping_request(session, packet);
                break;
            case 0x1F:
                client_play_handler::proto_765_release::place_recipe(session, packet);
                break;
            case 0x20:
                client_play_handler::proto_765_release::player_abilities(session, packet);
                break;
            case 0x21:
                client_play_handler::proto_765_release::player_action(session, packet);
                break;
            case 0x22:
                client_play_handler::proto_765_release::player_command(session, packet);
                break;
            case 0x23:
                client_play_handler::proto_765_release::player_input(session, packet);
                break;
            case 0x24: { //pong
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
            case 0x25:
                client_play_handler::proto_765_release::change_recipe_book_settings(session, packet);
                break;
            case 0x26:
                client_play_handler::proto_765_release::set_seen_recipe(session, packet);
                break;
            case 0x27:
                client_play_handler::proto_765_release::rename_item(session, packet);
                break;
            case 0x28:
                client_play_handler::proto_765_release::resource_pack_response(session, packet);
                break;
            case 0x29:
                client_play_handler::proto_765_release::seen_advancements(session, packet);
                break;
            case 0x2A:
                client_play_handler::proto_765_release::select_trade(session, packet);
                break;
            case 0x2B:
                client_play_handler::proto_765_release::set_beacon_effect(session, packet);
                break;
            case 0x2C:
                client_play_handler::proto_765_release::set_held_item(session, packet);
                break;
            case 0x2D:
                client_play_handler::proto_765_release::program_command_block(session, packet);
                break;
            case 0x2E:
                client_play_handler::proto_765_release::program_command_cart(session, packet);
                break;
            case 0x2F:
                client_play_handler::proto_765_release::set_creative_slot(session, packet);
                break;
            case 0x30:
                client_play_handler::proto_765_release::program_jigsaw_block(session, packet);
                break;
            case 0x31:
                client_play_handler::proto_765_release::program_structure_block(session, packet);
                break;
            case 0x32:
                client_play_handler::proto_765_release::update_sign(session, packet);
                break;
            case 0x33:
                client_play_handler::proto_765_release::swing_arm(session, packet);
                break;
            case 0x34:
                client_play_handler::proto_765_release::spectator_teleport(session, packet);
                break;
            case 0x35:
                client_play_handler::proto_765_release::use_item_on(session, packet);
                break;
            case 0x36:
                client_play_handler::proto_765_release::use_item(session, packet);
                break;
            default: {
                auto error = "Unknown packet id: " + std::to_string(packet_id);
                log::debug_error("play", error);
                return packets::play::kick(error);
            }
            }
            return IdleActions();
        }

        Response TooLargePacket() override {
            return packets::play::kick("Packet too large");
        }

        Response Exception(const std::exception& ex) override {
            return packets::play::kick("Internal server error: " + std::string(ex.what()) + "\nPlease report this to the server owner!");
        }

        Response UnexpectedException() override {
            return packets::play::kick("Internal server error\nPlease report this to the server owner!");
        }

        Response OnSwitching() override {
            for (auto& plugin : base_plugins)
                queriedPackets.push_back(plugin->OnPlay_initialize(session->sharedDataRef()));

            for (auto& plugin : session->sharedData().compatible_plugins)
                queriedPackets.push_back(pluginManagement.getPlugin(plugin)->OnPlay_initialize(session->sharedDataRef()));
            return IdleActions();
        }

    public:
        static std::unordered_map<std::string, PluginRegistrationPtr> plugins_play;
        static list_array<PluginRegistrationPtr> base_plugins;

        TCPClientHandlePlay(TCPsession* session)
            : TCPClientHandle(session) {
            keep_alive_solution->set_callback(
                [this]() {
                    log::debug("play", "Send keep alive");
                    keep_alive_packet = generate_random_int();
                    list_array<uint8_t> response;
                    response.push_back(0x24);
                    WriteValue<int64_t>(keep_alive_packet, response);
                    return Response::Answer({std::move(response)});
                }
            );
        }

        ~TCPClientHandlePlay() override {
            for (auto& plugin : base_plugins)
                queriedPackets.push_back(plugin->OnPlay_uninitialized(session->sharedDataRef()));

            for (auto& plugin : session->sharedData().compatible_plugins)
                queriedPackets.push_back(pluginManagement.getPlugin(plugin)->OnPlay_uninitialized(session->sharedDataRef()));
        }

        TCPclient* DefineOurself(TCPsession* sock) override {
            return nullptr;
        }
    };
}

#endif /* SRC_PROTOCOLHELPER_STATE_PLAY */
