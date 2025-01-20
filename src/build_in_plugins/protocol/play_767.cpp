#include <src/api/protocol.hpp>
#include <src/base_objects/network/accept_packet_registry.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>
#include <src/protocolHelper/client_handler/abstract.hpp>
#include <src/protocolHelper/packets/767/writers_readers.hpp>
#include <src/protocolHelper/util.hpp>
#include <src/registers.hpp>

namespace copper_server::build_in_plugins::protocol::play_767 {
    namespace play {
        void teleport_confirm(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::teleport_request_completion data;
            data.teleport_id = packet.read_var<int32_t>();
            data.success = session->sharedData().packets_state.play_data->pending_teleport_ids.front() != data.teleport_id;
            if (data.success)
                session->sharedData().packets_state.play_data->pending_teleport_ids.pop_front();
            api::protocol::on_teleport_request_completion.async_notify({data, *session, session->sharedDataRef()});
        }

        void query_block_nbt(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::block_nbt_request data;
            data.transaction_id = packet.read_var<int32_t>();
            data.position.raw = packet.read_value<int64_t>();
            api::protocol::on_block_nbt_request.async_notify({data, *session, session->sharedDataRef()});
        }

        void change_difficulty(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::on_change_difficulty.async_notify({packet.read_value<uint8_t>(), *session, session->sharedDataRef()});
        }

        void acknowledge_message(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::on_acknowledge_message.async_notify({packet.read_var<int32_t>(), *session, session->sharedDataRef()});
        }

        void chat_command(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::on_chat_command.async_notify({packet.read_string(32767), *session, session->sharedDataRef()});
        }

        void signed_chat_command(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::signed_chat_command data;
            data.command = packet.read_string(32767);
            data.timestamp = packet.read_value<int64_t>();
            data.salt = packet.read_value<int64_t>();
            int32_t arguments_count = packet.read_var<int32_t>();
            data.arguments_signature.reserve(arguments_count);
            for (int32_t i = 0; i < arguments_count; i++) {
                api::protocol::data::signed_chat_command::argument_signature arg;
                arg.argument_name = packet.read_string(16);
                for (int i = 0; i < 256; i++)
                    arg.signature[i] = packet.read();
                data.arguments_signature.push_back(arg);
            }
            data.message_count = packet.read_var<int32_t>();
            data.acknowledged.arr.push_back(packet.read());
            data.acknowledged.arr.push_back(packet.read());
            data.acknowledged.arr.push_back(packet.read());

            api::protocol::on_signed_chat_command.async_notify({data, *session, session->sharedDataRef()});
        }

        void chat_message(base_objects::network::tcp_session* session, ArrayStream& packet) {
            std::string message = packet.read_string(256);
            int64_t timestamp = packet.read_value<int64_t>();
            int64_t salt = packet.read_value<int64_t>();
            if (packet.read() == 0) {
                api::protocol::data::chat_message_unsigned data;
                data.message = message;
                data.timestamp = timestamp;
                data.salt = salt;
                data.message_count = packet.read_var<int32_t>();
                data.acknowledged.arr.push_back(packet.read());
                data.acknowledged.arr.push_back(packet.read());
                data.acknowledged.arr.push_back(packet.read());
                api::protocol::on_chat_message_unsigned.async_notify({data, *session, session->sharedDataRef()});
            } else {
                api::protocol::data::chat_message_signed data;
                for (int i = 0; i < 256; i++)
                    data.signature[i] = packet.read();
                data.message = message;
                data.timestamp = timestamp;
                data.salt = salt;
                data.message_count = packet.read_var<int32_t>();
                data.acknowledged.arr.push_back(packet.read());
                data.acknowledged.arr.push_back(packet.read());
                data.acknowledged.arr.push_back(packet.read());
                api::protocol::on_chat_message_signed.async_notify({data, *session, session->sharedDataRef()});
            }
        }

        void player_session(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::player_session data;
            data.session_id = packet.read_uuid();
            data.public_key.expiries_at = packet.read_value<int64_t>();
            data.public_key.public_key = packet.read_list_array();
            data.public_key.key_signature = packet.read_list_array();

            api::protocol::on_player_session.async_notify({data, *session, session->sharedDataRef()});
        }

        void chunk_batch_received(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::on_chunk_batch_received.async_notify({packet.read_value<float>(), *session, session->sharedDataRef()});
        }

        void client_status(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::on_client_status.async_notify({packet.read_var<int32_t>(), *session, session->sharedDataRef()});
        }

        void client_information(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::client_information data;
            data.locale = packet.read_string(16);
            data.view_distance = packet.read();
            data.chat_mode = packet.read_var<int32_t>();
            data.chat_colors = packet.read();
            data.displayed_skin_parts = packet.read();
            data.main_hand = packet.read_var<int32_t>();
            data.enable_text_filtering = packet.read();
            data.allow_server_listings = packet.read();
            api::protocol::on_client_information.async_notify({data, *session, session->sharedDataRef()});
        }

        void command_suggestion(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::command_suggestion data;
            data.transaction_id = packet.read_var<int32_t>();
            data.text = packet.read_string(32500);
            api::protocol::on_command_suggestion.async_notify({data, *session, session->sharedDataRef()});
        }

        void switch_to_configuration(base_objects::network::tcp_session* session, ArrayStream& packet) {
            session->sharedData().packets_state.state = base_objects::SharedClientData::packets_state_t::protocol_state::configuration;
            session->sharedData().switchToHandler(client_handler::abstract::createhandle_configuration(session));
        }

        void click_container_button(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::click_container_button data;
            data.window_id = packet.read();
            data.button_id = packet.read();
            api::protocol::on_click_container_button.async_notify({data, *session, session->sharedDataRef()});
        }

        void click_container(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::click_container data;
            data.window_id = packet.read();
            data.state_id = packet.read_var<int32_t>();
            data.slot = packet.read_value<int16_t>();
            data.button = packet.read();
            data.mode = packet.read_var<int32_t>();
            int32_t changed_slots_count = packet.read_var<int32_t>();
            data.changed_slots.reserve(changed_slots_count);
            for (int32_t i = 0; i < changed_slots_count; i++) {
                api::protocol::data::click_container::changed_slot slot;
                slot.slot = packet.read_value<int16_t>();
                slot.item = packets::release_767::reader::ReadSlot(packet);
                data.changed_slots.push_back(slot);
            }
            data.carried_item = packets::release_767::reader::ReadSlot(packet);
            api::protocol::on_click_container.async_notify({data, *session, session->sharedDataRef()});
        }

        void close_container(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::on_close_container.async_notify({packet.read_var<int32_t>(), *session, session->sharedDataRef()});
        }

        void change_container_slot_state(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::change_container_slot_state data;
            data.slot_id = packet.read_var<int32_t>();
            data.window_id = packet.read_var<int32_t>();
            data.state = packet.read();
            api::protocol::on_change_container_slot_state.async_notify({data, *session, session->sharedDataRef()});
        }

        void cookie_response(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::cookie_response data;
            data.key = packet.read_identifier();
            if (packet.read())
                data.payload = packet.read_array<uint8_t>(5120);
            api::protocol::on_cookie_response.async_notify({data, *session, session->sharedDataRef()});
        }

        void plugin_message(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::plugin_message msg;
            msg.channel = packet.read_identifier();
            msg.data = packet.read_left(32767).to_vector();
            auto plugin = pluginManagement.get_bind_plugin(PluginManagement::registration_on::play, msg.channel);
            if (plugin)
                if (auto res = plugin->OnPlayHandle(plugin, msg.channel, msg.data, session->sharedDataRef()); res != std::nullopt)
                    session->sharedData().sendPacket(std::move(*res));
        }

        void subscribe_to_debug_sample(base_objects::network::tcp_session* session, ArrayStream& packet) {
            auto data = (api::protocol::data::debug_sample_subscription)packet.read_var<int32_t>();
            api::protocol::on_debug_sample_subscription.async_notify({data, *session, session->sharedDataRef()});
        }

        void edit_book(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::edit_book data;
            data.slot = packet.read_var<int32_t>();
            int32_t text_count = packet.read_var<int32_t>();
            data.text.reserve(text_count);
            data.text.push_back(packet.read_string(8192));
            if (packet.read())
                data.title = packet.read_string(128);
            api::protocol::on_edit_book.async_notify({data, *session, session->sharedDataRef()});
        }

        void query_entity_tag(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::query_entity_tag data;
            data.transaction_id = packet.read_var<int32_t>();
            data.entity_id = packet.read_var<int32_t>();
            api::protocol::on_query_entity_tag.async_notify({data, *session, session->sharedDataRef()});
        }

        void interact(base_objects::network::tcp_session* session, ArrayStream& packet) {
            int32_t entity_id = packet.read_var<int32_t>();
            int32_t type = packet.read_var<int32_t>();
            switch (type) {
            case 0: {
                api::protocol::data::interact data;
                data.entity_id = entity_id;
                data.hand = packet.read();
                data.sneaking = packet.read();
                api::protocol::on_interact.async_notify({data, *session, session->sharedDataRef()});
            }
            case 1: {
                api::protocol::data::interact_attack data;
                data.entity_id = entity_id;
                data.sneaking = packet.read();
                api::protocol::on_interact_attack.async_notify({data, *session, session->sharedDataRef()});
            }
            case 2: {
                api::protocol::data::interact_at data;
                data.entity_id = entity_id;
                data.target_x = packet.read_value<float>();
                data.target_y = packet.read_value<float>();
                data.target_z = packet.read_value<float>();
                data.hand = packet.read();
                data.sneaking = packet.read();
                api::protocol::on_interact_at.async_notify({data, *session, session->sharedDataRef()});
            }
            default: {
                throw std::runtime_error("Unrecognized interact type.");
            }
            }
        }

        void jigsaw_generate(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::jigsaw_generate data;
            data.location.raw = packet.read_value<int64_t>();
            data.levels = packet.read_var<int32_t>();
            data.keep_jigsaws = packet.read();
            api::protocol::on_jigsaw_generate.async_notify({data, *session, session->sharedDataRef()});
        }

        void lock_difficulty(base_objects::network::tcp_session* session, ArrayStream& packet) {
            bool lock = packet.read();
            api::protocol::on_lock_difficulty.async_notify({lock, *session, session->sharedDataRef()});
        }

        void set_player_position(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::set_player_position data;
            data.x = packet.read_value<double>();
            data.y = packet.read_value<double>();
            data.z = packet.read_value<double>();
            data.on_ground = packet.read();
            api::protocol::on_set_player_position.async_notify({data, *session, session->sharedDataRef()});
        }

        void set_player_position_and_rotation(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::set_player_position_and_rotation data;
            data.x = packet.read_value<double>();
            data.y = packet.read_value<double>();
            data.z = packet.read_value<double>();
            data.yaw = packet.read_value<float>();
            data.pitch = packet.read_value<float>();
            data.on_ground = packet.read();
            api::protocol::on_set_player_position_and_rotation.async_notify({data, *session, session->sharedDataRef()});
        }

        void set_player_rotation(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::set_player_rotation data;
            data.yaw = packet.read_value<float>();
            data.pitch = packet.read_value<float>();
            data.on_ground = packet.read();
            api::protocol::on_set_player_rotation.async_notify({data, *session, session->sharedDataRef()});
        }

        void set_player_on_ground(base_objects::network::tcp_session* session, ArrayStream& packet) {
            uint8_t on_ground = packet.read();
            api::protocol::on_set_player_movement_flags.async_notify({on_ground, *session, session->sharedDataRef()});
        }

        void move_vehicle(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::move_vehicle data;
            data.x = packet.read_value<double>();
            data.y = packet.read_value<double>();
            data.z = packet.read_value<double>();
            data.yaw = packet.read_value<float>();
            data.pitch = packet.read_value<float>();
            api::protocol::on_move_vehicle.async_notify({data, *session, session->sharedDataRef()});
        }

        void paddle_boat(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::paddle_boat data;
            data.left_paddle = packet.read();
            data.right_paddle = packet.read();
            api::protocol::on_paddle_boat.async_notify({data, *session, session->sharedDataRef()});
        }

        void pick_item(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::on_pick_item_old.async_notify({{packet.read_var<int32_t>()}, *session, session->sharedDataRef()});
        }

        void pong(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::pong data;
            data.id = packet.read_value<int32_t>();
            data.elapsed = std::chrono::system_clock::now() - session->sharedData().packets_state.pong_timer;
            if (data.id == session->sharedData().packets_state.excepted_pong) {
                session->sharedData().packets_state.excepted_pong = -1;
                api::protocol::on_pong.async_notify({data, *session, session->sharedDataRef()});
            }
        }

        void keep_alive(base_objects::network::tcp_session* session, ArrayStream& packet) {
            session->sharedData().gotKeepAlive(packet.read_value<int64_t>());
        }

        void ping_request(base_objects::network::tcp_session* session, ArrayStream& packet) {
            int64_t ping = packet.read_value<int64_t>();
            api::protocol::on_ping_request.async_notify({ping, *session, session->sharedDataRef()});
            list_array<uint8_t> result;
            result.reserve(9);
            result.push_back(0x34);
            WriteValue<int64_t>(ping, result);
            session->sharedData().sendPacket(base_objects::network::response::answer({std::move(result)}));
        }

        void place_recipe(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::place_recipe data;
            data.window_id = packet.read_var<int32_t>();
            data.recipe_id = registers::recipe_table.at(packet.read_identifier()).id;
            data.make_all = packet.read_value<bool>();
            api::protocol::on_place_recipe.async_notify({data, *session, session->sharedDataRef()});
        }

        void player_abilities(base_objects::network::tcp_session* session, ArrayStream& packet) {
            int8_t flags = packet.read_value<int8_t>();
            bool flying = flags & 0x02;
            api::protocol::on_player_abilities.async_notify({flags, *session, session->sharedDataRef()});
            session->sharedData().player_data.abilities.flags.flying = flying;
        }

        void player_action(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::player_action data;
            data.status = packet.read_var<int32_t>();
            data.location.raw = packet.read_value<int64_t>();
            data.face = packet.read_value<int8_t>();
            data.sequence_id = packet.read_var<int32_t>();
            api::protocol::on_player_action.async_notify({data, *session, session->sharedDataRef()});
        }

        void player_command(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::player_command data;
            data.entity_id = packet.read_var<int32_t>();
            data.action_id = packet.read_var<int32_t>();
            data.jump_boost = packet.read_var<int32_t>();
            api::protocol::on_player_command.async_notify({data, *session, session->sharedDataRef()});
        }

        void player_input(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::player_input data;
            data.sideways = packet.read_value<float>();
            data.forward = packet.read_value<float>();
            auto flags = packet.read_value<int8_t>();
            data.flags.jump = flags & 1;
            data.flags.sneaking = flags & 2;
            data.flags.forward = data.forward > 0;
            data.flags.backward = data.forward < 0;
            data.flags.right = data.sideways > 0;
            data.flags.left = data.sideways < 0;
            api::protocol::on_player_input.async_notify({data, *session, session->sharedDataRef()});
        }

        void change_recipe_book_settings(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::change_recipe_book_settings data;
            data.book_id = packet.read_var<int32_t>();
            data.book_open = packet.read_value<bool>();
            data.filter_active = packet.read_value<bool>();
            api::protocol::on_change_recipe_book_settings.async_notify({data, *session, session->sharedDataRef()});
        }

        void set_seen_recipe(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::on_set_seen_recipe.async_notify({packet.read_identifier(), *session, session->sharedDataRef()});
        }

        void rename_item(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::on_rename_item.async_notify({packet.read_identifier(), *session, session->sharedDataRef()});
        }

        void resource_pack_response(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::resource_pack_response data;
            data.uuid = packet.read_uuid();
            data.result = packet.read_var<int32_t>();
            api::protocol::on_resource_pack_response.async_notify({data, *session, session->sharedDataRef()});
        }

        void seen_advancements(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::seen_advancements data;
            data.action = packet.read_var<int32_t>();
            if (data.action == 1)
                data.tab_id = packet.read_identifier();
            api::protocol::on_seen_advancements.async_notify({data, *session, session->sharedDataRef()});
        }

        void select_trade(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::on_select_trade.async_notify({packet.read_var<int32_t>(), *session, session->sharedDataRef()});
        }

        void set_beacon_effect(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::set_beacon_effect data;
            if (packet.read_value<bool>())
                data.primary_effect = packet.read_var<int32_t>();
            if (packet.read_value<bool>())
                data.secondary_effect = packet.read_var<int32_t>();
            api::protocol::on_set_beacon_effect.async_notify({data, *session, session->sharedDataRef()});
        }

        void set_held_item(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::on_set_held_item.async_notify({packet.read_value<int16_t>(), *session, session->sharedDataRef()});
        }

        void program_command_block(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::program_command_block data;
            data.location.raw = packet.read_value<int64_t>();
            data.command = packet.read_identifier();
            data.mode = packet.read_var<int32_t>();
            data.flags = packet.read_value<int8_t>();
            api::protocol::on_program_command_block.async_notify({data, *session, session->sharedDataRef()});
        }

        void program_command_cart(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::program_command_cart data;
            data.entity_id = packet.read_var<int32_t>();
            data.command = packet.read_identifier();
            data.track_output = packet.read_value<bool>();
            api::protocol::on_program_command_cart.async_notify({data, *session, session->sharedDataRef()});
        }

        void set_creative_slot(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::set_creative_slot data;
            data.slot = packet.read_value<int16_t>();
            data.item = packets::release_767::reader::ReadSlot(packet);
            api::protocol::on_set_creative_slot.async_notify({data, *session, session->sharedDataRef()});
        }

        void program_jigsaw_block(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::program_jigsaw_block data;
            data.location.raw = packet.read_value<int64_t>();
            data.name = packet.read_identifier();
            data.target = packet.read_identifier();
            data.pool = packet.read_identifier();
            data.final_state = packet.read_identifier();
            data.joint_type = packet.read_identifier();
            data.selection_priority = packet.read_var<int32_t>();
            data.placement_priority = packet.read_var<int32_t>();
            api::protocol::on_program_jigsaw_block.async_notify({data, *session, session->sharedDataRef()});
        }

        void program_structure_block(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::program_structure_block data;
            data.location.raw = packet.read_value<int64_t>();
            data.action = packet.read_var<int32_t>();
            data.mode = packet.read_var<int32_t>();
            data.name = packet.read_identifier();
            data.offset_x = packet.read_value<int8_t>();
            data.offset_y = packet.read_value<int8_t>();
            data.offset_z = packet.read_value<int8_t>();
            data.size_x = packet.read_var<int8_t>();
            data.size_y = packet.read_var<int8_t>();
            data.size_z = packet.read_var<int8_t>();
            data.mirror = packet.read_var<int32_t>();
            data.rotation = packet.read_var<int32_t>();
            data.metadata = packet.read_string(128);
            data.integrity = packet.read_value<float>();
            data.seed = packet.read_value<int64_t>();
            data.flags = packet.read_value<int8_t>();
            api::protocol::on_program_structure_block.async_notify({data, *session, session->sharedDataRef()});
        }

        void update_sign(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::update_sign data;
            data.location.raw = packet.read_value<int64_t>();
            data.is_front_text = packet.read_value<bool>();
            data.line1 = packet.read_string(384);
            data.line2 = packet.read_string(384);
            data.line3 = packet.read_string(384);
            data.line4 = packet.read_string(384);
            api::protocol::on_update_sign.async_notify({data, *session, session->sharedDataRef()});
        }

        void swing_arm(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::on_swing_arm.async_notify({{packet.read_var<int32_t>()}, *session, session->sharedDataRef()});
        }

        void spectator_teleport(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::on_spectator_teleport.async_notify({{packet.read_uuid()}, *session, session->sharedDataRef()});
        }

        void use_item_on(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::use_item_on data;
            data.hand = packet.read_var<int32_t>();
            data.location.raw = packet.read_value<int64_t>();
            data.face = packet.read_var<int32_t>();
            data.cursor_x = packet.read_value<float>();
            data.cursor_y = packet.read_value<float>();
            data.cursor_z = packet.read_value<float>();
            data.inside_block = packet.read_value<bool>();
            data.sequence = packet.read_var<int32_t>();
            api::protocol::on_use_item_on.async_notify({data, *session, session->sharedDataRef()});
        }

        void use_item(base_objects::network::tcp_session* session, ArrayStream& packet) {
            api::protocol::data::use_item data;
            data.hand = packet.read_var<int32_t>();
            data.sequence = packet.read_var<int32_t>();
            data.yaw = packet.read_value<float>();
            data.pitch = packet.read_value<float>();

            api::protocol::on_use_item.async_notify({data, *session, session->sharedDataRef()});
        }
    }

    class ProtocolSupport_767_play : public PluginAutoRegister<"protocol_support_for_767_play", ProtocolSupport_767_play> {
    public:
        void OnRegister(const PluginRegistrationPtr& self) override {
            base_objects::network::packet_registry.serverbound.play.register_seq(
                767,
                {
                    {"teleport_confirm", play::teleport_confirm},
                    {"query_block_nbt", play::query_block_nbt},
                    {"change_difficulty", play::change_difficulty},
                    {"acknowledge_message", play::acknowledge_message},
                    {"chat_command", play::chat_command},
                    {"signed_chat_command", play::signed_chat_command},
                    {"chat_message", play::chat_message},
                    {"player_session", play::player_session},
                    {"chunk_batch_received", play::chunk_batch_received},
                    {"client_status", play::client_status},
                    {"client_information", play::client_information},
                    {"command_suggestion", play::command_suggestion},
                    {"switch_to_configuration", play::switch_to_configuration},
                    {"click_container_button", play::click_container_button},
                    {"click_container", play::click_container},
                    {"close_container", play::close_container},
                    {"change_container_slot_state", play::change_container_slot_state},
                    {"cookie_response", play::cookie_response},
                    {"plugin_message", play::plugin_message},
                    {"subscribe_to_debug_sample", play::subscribe_to_debug_sample},
                    {"edit_book", play::edit_book},
                    {"query_entity_tag", play::query_entity_tag},
                    {"interact", play::interact},
                    {"jigsaw_generate", play::jigsaw_generate},
                    {"keep_alive", play::keep_alive},
                    {"lock_difficulty", play::lock_difficulty},
                    {"set_player_position", play::set_player_position},
                    {"set_player_position_and_rotation", play::set_player_position_and_rotation},
                    {"set_player_rotation", play::set_player_rotation},
                    {"set_player_on_ground", play::set_player_on_ground},
                    {"move_vehicle", play::move_vehicle},
                    {"paddle_boat", play::paddle_boat},
                    {"pick_item", play::pick_item},
                    {"ping_request", play::ping_request},
                    {"place_recipe", play::place_recipe},
                    {"player_abilities", play::player_abilities},
                    {"player_action", play::player_action},
                    {"player_command", play::player_command},
                    {"player_input", play::player_input},
                    {"pong", play::pong},
                    {"change_recipe_book_settings", play::change_recipe_book_settings},
                    {"set_seen_recipe", play::set_seen_recipe},
                    {"rename_item", play::rename_item},
                    {"resource_pack_response", play::resource_pack_response},
                    {"seen_advancements", play::seen_advancements},
                    {"select_trade", play::select_trade},
                    {"set_beacon_effect", play::set_beacon_effect},
                    {"set_held_item", play::set_held_item},
                    {"program_command_block", play::program_command_block},
                    {"program_command_cart", play::program_command_cart},
                    {"set_creative_slot", play::set_creative_slot},
                    {"program_jigsaw_block", play::program_jigsaw_block},
                    {"program_structure_block", play::program_structure_block},
                    {"update_sign", play::update_sign},
                    {"swing_arm", play::swing_arm},
                    {"spectator_teleport", play::spectator_teleport},
                    {"use_item_on", play::use_item_on},
                    {"use_item", play::use_item},
                }
            );
        }
    };
}