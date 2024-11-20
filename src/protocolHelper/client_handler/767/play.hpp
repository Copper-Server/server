#ifndef SRC_PROTOCOLHELPER_CLIENT_HANDLER_767_PLAY
#define SRC_PROTOCOLHELPER_CLIENT_HANDLER_767_PLAY
#include <src/api/protocol.hpp>
#include <src/protocolHelper/client_handler/767/767_release.hpp>
#include <src/protocolHelper/client_handler/abstract.hpp>
#include <src/protocolHelper/packets/767/packets.hpp>

namespace copper_server {
    namespace client_handler {
        namespace release_767 {
            class HandlePlay : public TCPClientHandle {
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
                    list_array<PluginRegistration::plugin_response> load_next_packets;
                    for (size_t i = 0; !queriedPackets.empty(); i++) {
                        load_next_packets.push_back(std::move(queriedPackets.front()));
                        queriedPackets.pop_front();
                    }
                    Response response(Response::Empty());
                    for (auto& packet : session->sharedData().getPendingPackets())
                        response += std::move(packet);
                    response.reserve(load_next_packets.size());
                    for (auto& it : load_next_packets)
                        if (it)
                            response += *it;
                    response += keep_alive_solution->send_keep_alive();
                    return response;
                }

                Response WorkPacket(ArrayStream& packet) override {
                    uint8_t packet_id = packet.read();
                    switch (packet_id) {
                    case 0x00:
                        play_767_release::teleport_confirm(session, packet);
                        break;
                    case 0x01:
                        play_767_release::query_block_nbt(session, packet);
                        break;
                    case 0x02:
                        play_767_release::change_difficulty(session, packet);
                        break;
                    case 0x03:

                        play_767_release::acknowledge_message(session, packet);
                        break;
                    case 0x04:
                        play_767_release::chat_command(session, packet);
                        break;
                    case 0x05:
                        play_767_release::signed_chat_command(session, packet);
                        break;
                    case 0x06:
                        play_767_release::chat_message(session, packet);
                        break;
                    case 0x07:
                        play_767_release::player_session(session, packet);
                        break;
                    case 0x08:
                        play_767_release::chunk_batch_received(session, packet);
                        break;
                    case 0x09:
                        play_767_release::client_status(session, packet);
                        break;
                    case 0x0A:
                        play_767_release::client_information(session, packet);
                        break;
                    case 0x0B:
                        play_767_release::command_suggestion(session, packet);
                        break;
                    case 0x0C:
                        play_767_release::switch_to_configuration(session, packet, next_handler);
                        break;
                    case 0x0D:
                        play_767_release::click_container_button(session, packet);
                        break;
                    case 0x0E:
                        play_767_release::click_container(session, packet);
                        break;
                    case 0x0F:
                        play_767_release::close_container(session, packet);
                        break;
                    case 0x10:
                        play_767_release::change_container_slot_state(session, packet);
                        break;
                    case 0x11:
                        play_767_release::cookie_response(session, packet);
                        break;
                    case 0x12: {
                        auto tmp = play_767_release::plugin_message(session, packet, queriedPackets);
                        if (tmp.data.empty())
                            break;
                        else
                            return tmp;
                    }
                    case 0x13:
                        play_767_release::subscribe_to_debug_sample(session, packet);
                        break;
                    case 0x14:
                        play_767_release::edit_book(session, packet);
                        break;
                    case 0x15:
                        play_767_release::query_entity_tag(session, packet);
                        break;
                    case 0x16:
                        play_767_release::interact(session, packet);
                        break;
                    case 0x17:
                        play_767_release::jigsaw_generate(session, packet);
                        break;
                    case 0x18: { //keep alive
                        log::debug("play", "Keep alive");
                        int64_t keep_alive_packet_response = ReadValue<int64_t>(packet);
                        if (keep_alive_packet == keep_alive_packet_response)
                            session->sharedData().packets_state.keep_alive_ping_ms = std::min<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(keep_alive_solution->got_valid_keep_alive()).count(), INT32_MAX);
                        api::protocol::on_keep_alive.async_notify({keep_alive_packet_response, *session, session->sharedDataRef()});
                        break;
                    }
                    case 0x19:
                        play_767_release::lock_difficulty(session, packet);
                        break;
                    case 0x1A:
                        play_767_release::set_player_position(session, packet);
                        break;
                    case 0x1B:
                        play_767_release::set_player_position_and_rotation(session, packet);
                        break;
                    case 0x1C:
                        play_767_release::set_player_rotation(session, packet);
                        break;
                    case 0x1D:
                        play_767_release::set_player_on_ground(session, packet);
                        break;
                    case 0x1E:
                        play_767_release::move_vehicle(session, packet);
                        break;
                    case 0x1F:
                        play_767_release::paddle_boat(session, packet);
                        break;
                    case 0x20:
                        play_767_release::pick_item(session, packet);
                        break;
                    case 0x21:
                        play_767_release::ping_request(session, packet);
                        break;
                    case 0x22:
                        play_767_release::place_recipe(session, packet);
                        break;
                    case 0x23:
                        play_767_release::player_abilities(session, packet);
                        break;
                    case 0x24:
                        play_767_release::player_action(session, packet);
                        break;
                    case 0x25:
                        play_767_release::player_command(session, packet);
                        break;
                    case 0x26:
                        play_767_release::player_input(session, packet);
                        break;
                    case 0x27: { //pong
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
                    case 0x28:
                        play_767_release::change_recipe_book_settings(session, packet);
                        break;
                    case 0x29:
                        play_767_release::set_seen_recipe(session, packet);
                        break;
                    case 0x2A:
                        play_767_release::rename_item(session, packet);
                        break;
                    case 0x2B:
                        play_767_release::resource_pack_response(session, packet);
                        break;
                    case 0x2C:
                        play_767_release::seen_advancements(session, packet);
                        break;
                    case 0x2D:
                        play_767_release::select_trade(session, packet);
                        break;
                    case 0x2E:
                        play_767_release::set_beacon_effect(session, packet);
                        break;
                    case 0x2F:
                        play_767_release::set_held_item(session, packet);
                        break;
                    case 0x30:
                        play_767_release::program_command_block(session, packet);
                        break;
                    case 0x31:
                        play_767_release::program_command_cart(session, packet);
                        break;
                    case 0x32:
                        play_767_release::set_creative_slot(session, packet);
                        break;
                    case 0x33:
                        play_767_release::program_jigsaw_block(session, packet);
                        break;
                    case 0x34:
                        play_767_release::program_structure_block(session, packet);
                        break;
                    case 0x35:
                        play_767_release::update_sign(session, packet);
                        break;
                    case 0x36:
                        play_767_release::swing_arm(session, packet);
                        break;
                    case 0x37:
                        play_767_release::spectator_teleport(session, packet);
                        break;
                    case 0x38:
                        play_767_release::use_item_on(session, packet);
                        break;
                    case 0x39:
                        play_767_release::use_item(session, packet);
                        break;
                    default: {
                        auto error = "Unknown packet id: " + std::to_string(packet_id);
                        log::debug_error("play", error);
                        return packets::release_767::play::kick(error);
                    }
                    }
                    return IdleActions();
                }

                Response TooLargePacket() override {
                    return packets::release_767::play::kick("Packet too large");
                }

                Response Exception(const std::exception& ex) override {
                    return packets::release_767::play::kick("Internal server error: " + std::string(ex.what()) + "\nPlease report this to the server owner!");
                }

                Response UnexpectedException() override {
                    return packets::release_767::play::kick("Internal server error\nPlease report this to the server owner!");
                }

                Response OnSwitching() override {
                    pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
                        queriedPackets.push_back(plugin->OnPlay_initialize(session->sharedDataRef()));
                    });

                    for (auto& plugin : session->sharedData().compatible_plugins)
                        queriedPackets.push_back(pluginManagement.getPlugin(plugin)->OnPlay_initialize(session->sharedDataRef()));
                    return IdleActions();
                }

            public:
                HandlePlay(TCPsession* session)
                    : TCPClientHandle(session) {
                    keep_alive_solution->set_callback(
                        [this]() {
                            log::debug("play", "Send keep alive");
                            keep_alive_packet = generate_random_int();
                            return packets::release_767::play::keepAlive(keep_alive_packet);
                        }
                    );
                }

                ~HandlePlay() override {
                    pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
                        queriedPackets.push_back(plugin->OnPlay_uninitialized(session->sharedDataRef()));
                    });

                    for (auto& plugin : session->sharedData().compatible_plugins)
                        queriedPackets.push_back(pluginManagement.getPlugin(plugin)->OnPlay_uninitialized(session->sharedDataRef()));
                }

                TCPclient* DefineOurself(TCPsession* sock) override {
                    return nullptr;
                }
            };
        }
    }
}

#endif /* SRC_PROTOCOLHELPER_CLIENT_HANDLER_767_PLAY */
