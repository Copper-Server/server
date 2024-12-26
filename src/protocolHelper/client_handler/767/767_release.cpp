

#include <src/api/protocol.hpp>
#include <src/base_objects/player.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/protocolHelper/client_handler/abstract.hpp>
#include <src/protocolHelper/packets/767/writers_readers.hpp>
#include <src/protocolHelper/util.hpp>

namespace copper_server::client_handler::play_767_release {
    void teleport_confirm(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Teleport confirm");
        api::protocol::data::teleport_request_completion data;
        data.teleport_id = ReadVar<int32_t>(packet);
        data.success = session->sharedData().packets_state.pending_teleport_ids.front() != data.teleport_id;
        if (data.success)
            session->sharedData().packets_state.pending_teleport_ids.pop_front();
        api::protocol::on_teleport_request_completion.async_notify({data, *session, session->sharedDataRef()});
    }

    void query_block_nbt(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Query block nbt");
        api::protocol::data::block_nbt_request data;
        data.transaction_id = ReadVar<int32_t>(packet);
        data.position.raw = ReadValue<int64_t>(packet);
        api::protocol::on_block_nbt_request.async_notify({data, *session, session->sharedDataRef()});
    }

    void change_difficulty(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Change difficulty");
        api::protocol::on_change_difficulty.async_notify({ReadValue<uint8_t>(packet), *session, session->sharedDataRef()});
    }

    void acknowledge_message(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Acknowledge message");
        api::protocol::on_acknowledge_message.async_notify({ReadVar<int32_t>(packet), *session, session->sharedDataRef()});
    }

    void chat_command(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Chat command");
        api::protocol::on_chat_command.async_notify({ReadString(packet, 32767), *session, session->sharedDataRef()});
    }

    void signed_chat_command(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Chat command");
        api::protocol::data::signed_chat_command data;
        data.command = ReadString(packet, 32767);
        data.timestamp = ReadValue<int64_t>(packet);
        data.salt = ReadValue<int64_t>(packet);
        int32_t arguments_count = ReadVar<int32_t>(packet);
        data.arguments_signature.reserve(arguments_count);
        for (int32_t i = 0; i < arguments_count; i++) {
            api::protocol::data::signed_chat_command::argument_signature arg;
            arg.argument_name = ReadString(packet, 16);
            for (int i = 0; i < 256; i++)
                arg.signature[i] = packet.read();
            data.arguments_signature.push_back(arg);
        }
        data.message_count = ReadVar<int32_t>(packet);
        data.acknowledged.arr.push_back(packet.read());
        data.acknowledged.arr.push_back(packet.read());
        data.acknowledged.arr.push_back(packet.read());

        api::protocol::on_signed_chat_command.async_notify({data, *session, session->sharedDataRef()});
    }

    void chat_message(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Chat message");
        std::string message = ReadString(packet, 256);
        int64_t timestamp = ReadValue<int64_t>(packet);
        int64_t salt = ReadValue<int64_t>(packet);
        if (packet.read() == 0) {
            api::protocol::data::chat_message_unsigned data;
            data.message = message;
            data.timestamp = timestamp;
            data.salt = salt;
            data.message_count = ReadVar<int32_t>(packet);
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
            data.message_count = ReadVar<int32_t>(packet);
            data.acknowledged.arr.push_back(packet.read());
            data.acknowledged.arr.push_back(packet.read());
            data.acknowledged.arr.push_back(packet.read());
            api::protocol::on_chat_message_signed.async_notify({data, *session, session->sharedDataRef()});
        }
    }

    void player_session(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Player session");
        api::protocol::data::player_session data;
        data.session_id = ReadUUID(packet);
        data.public_key.expiries_at = ReadValue<int64_t>(packet);
        data.public_key.public_key = ReadListArray(packet);
        data.public_key.key_signature = ReadListArray(packet);

        api::protocol::on_player_session.async_notify({data, *session, session->sharedDataRef()});
    }

    void chunk_batch_received(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Chunk batch received");
        api::protocol::on_chunk_batch_received.async_notify({ReadValue<float>(packet), *session, session->sharedDataRef()});
    }

    void client_status(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Client status");
        api::protocol::on_client_status.async_notify({ReadVar<int32_t>(packet), *session, session->sharedDataRef()});
    }

    void client_information(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Client information");
        api::protocol::data::client_information data;
        data.locale = ReadString(packet, 16);
        data.view_distance = packet.read();
        data.chat_mode = ReadVar<int32_t>(packet);
        data.chat_colors = packet.read();
        data.displayed_skin_parts = packet.read();
        data.main_hand = ReadVar<int32_t>(packet);
        data.enable_text_filtering = packet.read();
        data.allow_server_listings = packet.read();
        api::protocol::on_client_information.async_notify({data, *session, session->sharedDataRef()});
    }

    void command_suggestion(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Command suggestion");
        api::protocol::data::command_suggestion data;
        data.transaction_id = ReadVar<int32_t>(packet);
        data.text = ReadString(packet, 32500);
        api::protocol::on_command_suggestion.async_notify({data, *session, session->sharedDataRef()});
    }

    void switch_to_configuration(base_objects::network::tcp_session* session, ArrayStream& packet, base_objects::network::tcp_client*& next_handler) {
        session->sharedData().packets_state.state = base_objects::SharedClientData::packets_state_t::protocol_state::configuration;
        log::debug("configuration", "Switch to configuration");
        next_handler = abstract::createhandle_configuration(session);
    }

    void click_container_button(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Click container button");
        api::protocol::data::click_container_button data;
        data.window_id = packet.read();
        data.button_id = packet.read();
        api::protocol::on_click_container_button.async_notify({data, *session, session->sharedDataRef()});
    }

    void click_container(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Click container");
        api::protocol::data::click_container data;
        data.window_id = packet.read();
        data.state_id = ReadVar<int32_t>(packet);
        data.slot = ReadVar<int16_t>(packet);
        data.button = packet.read();
        data.mode = ReadVar<int32_t>(packet);
        int32_t changed_slots_count = ReadVar<int32_t>(packet);
        data.changed_slots.reserve(changed_slots_count);
        for (int32_t i = 0; i < changed_slots_count; i++) {
            api::protocol::data::click_container::changed_slot slot;
            slot.slot = ReadVar<int16_t>(packet);
            slot.item = packets::release_767::reader::ReadSlot(packet);
            data.changed_slots.push_back(slot);
        }
        data.carried_item = packets::release_767::reader::ReadSlot(packet);
        api::protocol::on_click_container.async_notify({data, *session, session->sharedDataRef()});
    }

    void close_container(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Close container");
        api::protocol::on_close_container.async_notify({packet.read(), *session, session->sharedDataRef()});
    }

    void change_container_slot_state(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Change container slot state");
        api::protocol::data::change_container_slot_state data;
        data.slot_id = ReadVar<int32_t>(packet);
        data.window_id = ReadVar<int32_t>(packet);
        data.state = packet.read();
        api::protocol::on_change_container_slot_state.async_notify({data, *session, session->sharedDataRef()});
    }

    void cookie_response(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Cookie response");
        api::protocol::data::cookie_response data;
        data.key = ReadIdentifier(packet);
        if (packet.read())
            data.payload = ReadArray<uint8_t>(packet);
        api::protocol::on_cookie_response.async_notify({data, *session, session->sharedDataRef()});
    }

    base_objects::network::response plugin_message(base_objects::network::tcp_session* session, ArrayStream& packet, std::list<base_objects::network::plugin_response>& queriedPackets) {
        log::debug("play", "Change container slot state");
        api::protocol::data::plugin_message msg;
        msg.channel = ReadIdentifier(packet);
        msg.data = packet.read_left().to_vector();
        auto plugin = pluginManagement.get_bind_plugin(PluginManagement::registration_on::play, msg.channel);
        if (plugin) {
            auto response = plugin->OnPlayHandle(plugin, msg.channel, msg.data, session->sharedDataRef());
            if (response)
                return *response;
        } else
            log::debug_error("play", "Unknown chanel: " + msg.channel);
        return base_objects::network::response::empty();
    }

    void subscribe_to_debug_sample(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Subscribe to debug sample");
        auto data = (api::protocol::data::debug_sample_subscription)ReadVar<int32_t>(packet);
        api::protocol::on_debug_sample_subscription.async_notify({data, *session, session->sharedDataRef()});
    }

    void edit_book(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Edit book");
        api::protocol::data::edit_book data;
        data.slot = ReadVar<int32_t>(packet);
        int32_t text_count = ReadVar<int32_t>(packet);
        data.text.reserve(text_count);
        data.text.push_back(ReadString(packet, 8192));
        if (packet.read())
            data.title = ReadString(packet, 128);
        api::protocol::on_edit_book.async_notify({data, *session, session->sharedDataRef()});
    }

    void query_entity_tag(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Query entity tag");
        api::protocol::data::query_entity_tag data;
        data.transaction_id = ReadVar<int32_t>(packet);
        data.entity_id = ReadVar<int32_t>(packet);
        api::protocol::on_query_entity_tag.async_notify({data, *session, session->sharedDataRef()});
    }

    void interact(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Interact");
        int32_t entity_id = ReadVar<int32_t>(packet);
        int32_t type = ReadVar<int32_t>(packet);
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
            data.target_x = ReadValue<float>(packet);
            data.target_y = ReadValue<float>(packet);
            data.target_z = ReadValue<float>(packet);
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
        log::debug("play", "Jigsaw Generate");
        api::protocol::data::jigsaw_generate data;
        data.location.raw = ReadValue<int64_t>(packet);
        data.levels = ReadVar<int32_t>(packet);
        data.keep_jigsaws = packet.read();
        api::protocol::on_jigsaw_generate.async_notify({data, *session, session->sharedDataRef()});
    }

    void lock_difficulty(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Lock difficulty");
        bool lock = packet.read();
        api::protocol::on_lock_difficulty.async_notify({lock, *session, session->sharedDataRef()});
    }

    void set_player_position(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Set player position");
        api::protocol::data::set_player_position data;
        data.x = ReadValue<double>(packet);
        data.y = ReadValue<double>(packet);
        data.z = ReadValue<double>(packet);
        data.on_ground = packet.read();
        api::protocol::on_set_player_position.async_notify({data, *session, session->sharedDataRef()});
    }

    void set_player_position_and_rotation(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Set player position and rotation");
        api::protocol::data::set_player_position_and_rotation data;
        data.x = ReadValue<double>(packet);
        data.y = ReadValue<double>(packet);
        data.z = ReadValue<double>(packet);
        data.yaw = ReadValue<float>(packet);
        data.pitch = ReadValue<float>(packet);
        data.on_ground = packet.read();
        api::protocol::on_set_player_position_and_rotation.async_notify({data, *session, session->sharedDataRef()});
    }

    void set_player_rotation(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Set player rotation");
        api::protocol::data::set_player_rotation data;
        data.yaw = ReadValue<float>(packet);
        data.pitch = ReadValue<float>(packet);
        data.on_ground = packet.read();
        api::protocol::on_set_player_rotation.async_notify({data, *session, session->sharedDataRef()});
    }

    void set_player_on_ground(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Set player on ground state");
        bool on_ground = packet.read();
        api::protocol::on_set_player_on_ground.async_notify({on_ground, *session, session->sharedDataRef()});
    }

    void move_vehicle(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Move vehicle");
        api::protocol::data::move_vehicle data;
        data.x = ReadValue<double>(packet);
        data.y = ReadValue<double>(packet);
        data.z = ReadValue<double>(packet);
        data.yaw = ReadValue<float>(packet);
        data.pitch = ReadValue<float>(packet);
        api::protocol::on_move_vehicle.async_notify({data, *session, session->sharedDataRef()});
    }

    void paddle_boat(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Paddle boat");
        api::protocol::data::paddle_boat data;
        data.left_paddle = packet.read();
        data.right_paddle = packet.read();
        api::protocol::on_paddle_boat.async_notify({data, *session, session->sharedDataRef()});
    }

    void pick_item(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Pick item");
        api::protocol::on_pick_item.async_notify({{ReadVar<int32_t>(packet)}, *session, session->sharedDataRef()});
    }

    base_objects::network::response ping_request(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Ping request");
        int64_t ping = ReadValue<int64_t>(packet);
        api::protocol::on_ping_request.async_notify({ping, *session, session->sharedDataRef()});
        list_array<uint8_t> result;
        result.reserve(9);
        result.push_back(0x34);
        WriteValue<int64_t>(ping, result);
        return base_objects::network::response::answer({std::move(result)});
    }

    void place_recipe(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Place recipe");
        api::protocol::data::place_recipe data;
        data.window_id = ReadVar<int32_t>(packet);
        data.recipe_id = ReadIdentifier(packet);
        data.make_all = ReadValue<bool>(packet);
        api::protocol::on_place_recipe.async_notify({data, *session, session->sharedDataRef()});
    }

    void player_abilities(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Player abilities");
        int8_t flags = ReadValue<int8_t>(packet);
        bool flying = flags & 0x02;
        api::protocol::on_player_abilities.async_notify({flags, *session, session->sharedDataRef()});
        session->sharedData().player_data.abilities.flags.flying = flying;
    }

    void player_action(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Player action");
        api::protocol::data::player_action data;
        data.status = ReadVar<int32_t>(packet);
        data.location.raw = ReadValue<int64_t>(packet);
        data.face = ReadValue<int8_t>(packet);
        data.sequence_id = ReadVar<int32_t>(packet);
        api::protocol::on_player_action.async_notify({data, *session, session->sharedDataRef()});
    }

    void player_command(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Player command");
        api::protocol::data::player_command data;
        data.entity_id = ReadVar<int32_t>(packet);
        data.action_id = ReadVar<int32_t>(packet);
        data.jump_boost = ReadVar<int32_t>(packet);
        api::protocol::on_player_command.async_notify({data, *session, session->sharedDataRef()});
    }

    void player_input(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Player input");
        api::protocol::data::player_input data;
        data.sideways = ReadValue<float>(packet);
        data.forward = ReadValue<float>(packet);
        data.flags.raw = ReadValue<int8_t>(packet);
        api::protocol::on_player_input.async_notify({data, *session, session->sharedDataRef()});
    }

    void change_recipe_book_settings(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Change recipe book settings");
        api::protocol::data::change_recipe_book_settings data;
        data.book_id = ReadVar<int32_t>(packet);
        data.book_open = ReadValue<bool>(packet);
        data.filter_active = ReadValue<bool>(packet);
        api::protocol::on_change_recipe_book_settings.async_notify({data, *session, session->sharedDataRef()});
    }

    void set_seen_recipe(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Set seen recipe");
        api::protocol::on_set_seen_recipe.async_notify({ReadIdentifier(packet), *session, session->sharedDataRef()});
    }

    void rename_item(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Rename item");
        api::protocol::on_rename_item.async_notify({ReadIdentifier(packet), *session, session->sharedDataRef()});
    }

    void resource_pack_response(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Resource pack response");
        api::protocol::data::resource_pack_response data;
        data.uuid = ReadUUID(packet);
        data.result = ReadVar<int32_t>(packet);
        api::protocol::on_resource_pack_response.async_notify({data, *session, session->sharedDataRef()});
    }

    void seen_advancements(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Seen advancements");
        api::protocol::data::seen_advancements data;
        data.action = ReadVar<int32_t>(packet);
        if (data.action == 1)
            data.tab_id = ReadIdentifier(packet);
        api::protocol::on_seen_advancements.async_notify({data, *session, session->sharedDataRef()});
    }

    void select_trade(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Select trade");
        api::protocol::on_select_trade.async_notify({ReadVar<int32_t>(packet), *session, session->sharedDataRef()});
    }

    void set_beacon_effect(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Set beacon effect");
        api::protocol::data::set_beacon_effect data;
        if (ReadValue<bool>(packet))
            data.primary_effect = ReadVar<int32_t>(packet);
        if (ReadValue<bool>(packet))
            data.secondary_effect = ReadVar<int32_t>(packet);
        api::protocol::on_set_beacon_effect.async_notify({data, *session, session->sharedDataRef()});
    }

    void set_held_item(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Set held item");
        api::protocol::on_set_held_item.async_notify({ReadVar<int16_t>(packet), *session, session->sharedDataRef()});
    }

    void program_command_block(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Program command block");
        api::protocol::data::program_command_block data;
        data.location.raw = ReadValue<int64_t>(packet);
        data.command = ReadIdentifier(packet);
        data.mode = ReadVar<int32_t>(packet);
        data.flags = ReadValue<int8_t>(packet);
        api::protocol::on_program_command_block.async_notify({data, *session, session->sharedDataRef()});
    }

    void program_command_cart(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Program command block mineCart");
        api::protocol::data::program_command_cart data;
        data.entity_id = ReadVar<int32_t>(packet);
        data.command = ReadIdentifier(packet);
        data.track_output = ReadValue<bool>(packet);
        api::protocol::on_program_command_cart.async_notify({data, *session, session->sharedDataRef()});
    }

    void set_creative_slot(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Set creative slot");
        api::protocol::data::set_creative_slot data;
        data.slot = ReadVar<int16_t>(packet);
        data.item = packets::release_767::reader::ReadSlot(packet);
        api::protocol::on_set_creative_slot.async_notify({data, *session, session->sharedDataRef()});
    }

    void program_jigsaw_block(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Program jigsaw block");
        api::protocol::data::program_jigsaw_block data;
        data.location.raw = ReadValue<int64_t>(packet);
        data.name = ReadIdentifier(packet);
        data.target = ReadIdentifier(packet);
        data.pool = ReadIdentifier(packet);
        data.final_state = ReadIdentifier(packet);
        data.joint_type = ReadIdentifier(packet);
        data.selection_priority = ReadVar<int32_t>(packet);
        data.placement_priority = ReadVar<int32_t>(packet);
        api::protocol::on_program_jigsaw_block.async_notify({data, *session, session->sharedDataRef()});
    }

    void program_structure_block(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Program structure block");
        api::protocol::data::program_structure_block data;
        data.location.raw = ReadValue<int64_t>(packet);
        data.action = ReadVar<int32_t>(packet);
        data.mode = ReadVar<int32_t>(packet);
        data.name = ReadIdentifier(packet);
        data.offset_x = ReadValue<int8_t>(packet);
        data.offset_y = ReadValue<int8_t>(packet);
        data.offset_z = ReadValue<int8_t>(packet);
        data.size_x = ReadVar<int8_t>(packet);
        data.size_y = ReadVar<int8_t>(packet);
        data.size_z = ReadVar<int8_t>(packet);
        data.mirror = ReadVar<int32_t>(packet);
        data.rotation = ReadVar<int32_t>(packet);
        data.metadata = ReadString(packet, 128);
        data.integrity = ReadValue<float>(packet);
        data.seed = ReadValue<int64_t>(packet);
        data.flags = ReadValue<int8_t>(packet);
        api::protocol::on_program_structure_block.async_notify({data, *session, session->sharedDataRef()});
    }

    void update_sign(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Update sign");
        api::protocol::data::update_sign data;
        data.location.raw = ReadValue<int64_t>(packet);
        data.is_front_text = ReadValue<bool>(packet);
        data.line1 = ReadString(packet, 384);
        data.line2 = ReadString(packet, 384);
        data.line3 = ReadString(packet, 384);
        data.line4 = ReadString(packet, 384);
        api::protocol::on_update_sign.async_notify({data, *session, session->sharedDataRef()});
    }

    void swing_arm(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Swing arm");
        api::protocol::on_swing_arm.async_notify({{ReadVar<int32_t>(packet)}, *session, session->sharedDataRef()});
    }

    void spectator_teleport(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Spectator request to teleport");
        api::protocol::on_spectator_teleport.async_notify({{ReadUUID(packet)}, *session, session->sharedDataRef()});
    }

    void use_item_on(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Use item on");
        api::protocol::data::use_item_on data;
        data.hand = ReadVar<int32_t>(packet);
        data.location.raw = ReadValue<int64_t>(packet);
        data.face = ReadVar<int32_t>(packet);
        data.cursor_x = ReadValue<float>(packet);
        data.cursor_y = ReadValue<float>(packet);
        data.cursor_z = ReadValue<float>(packet);
        data.inside_block = ReadValue<bool>(packet);
        data.sequence = ReadVar<int32_t>(packet);
        api::protocol::on_use_item_on.async_notify({data, *session, session->sharedDataRef()});
    }

    void use_item(base_objects::network::tcp_session* session, ArrayStream& packet) {
        log::debug("play", "Use item");
        api::protocol::data::use_item data;
        data.hand = ReadVar<int32_t>(packet);
        data.sequence = ReadVar<int32_t>(packet);
        data.yaw = ReadValue<float>(packet);
        data.pitch = ReadValue<float>(packet);

        api::protocol::on_use_item.async_notify({data, *session, session->sharedDataRef()});
    }
}