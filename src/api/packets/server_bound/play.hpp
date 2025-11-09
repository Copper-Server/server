/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_SERVER_BOUND_PLAY
#define SRC_API_PACKETS_SERVER_BOUND_PLAY


#include <array>
#include <library/enbt/enbt.hpp>
#include <optional>
#include <src/api/packets/ops.hpp>
#include <src/api/packets/difficulty.hpp>
#include <src/api/packets/gamemode.hpp>
#include <src/api/packets/slot.hpp>
#include <src/api/packets/types.hpp>
#include <src/base_objects/position.hpp>

namespace copper_server::api::packets::server_bound::play {
    struct accept_teleportation : public packet<0x00> {
        ordered_id<var_int32, "sync/player_position"> teleport_id;
    };

    struct block_entity_tag_query : public packet<0x01> {
        var_int32 tag_query_id; //managed by client
        base_objects::position location;
    };

    struct bundle_item_selected : public packet<0x02> {
        var_int32 bundle_slot;
        var_int32 item_slot;
    };

    struct change_difficulty : public packet<0x03> {
        enum_as<difficulty_e, uint8_t> difficulty;
    };

    struct change_gamemode : public packet<0x04> {
        enum_as<gamemode_e, uint8_t> gamemode;
    };

    struct chat_ack : public packet<0x05> {
        var_int32 count;
    };

    struct chat_command : public packet<0x06> {
        string_sized<32767> command;
    };

    struct chat_command_signed : public packet<0x07> {
        struct argument_signature {
            string_sized<16> argument_name;
            std::array<uint8_t, 256> signature;
        };

        string_sized<32767> command;
        uint64_t timestamp;
        uint64_t salt;
        list_array_sized<argument_signature, 8> argument_signatures;
        var_int32 message_count;
        bitset_fixed<20> acknowledged;
        uint8_t check_sum;
    };

    struct chat : public packet<0x08> {
        string_sized<256> message;
        uint64_t timestamp;
        uint64_t salt;
        std::optional<std::array<uint8_t, 256>> signature = std::nullopt;
        var_int32 message_count;
        bitset_fixed<20> acknowledged;
        uint8_t check_sum;
    };

    struct chat_session_update : public packet<0x09> {
        base_objects::uuid uuid;
        uint64_t expiries_at;
        list_array_sized<uint8_t, 512> public_key;
        list_array_sized<uint8_t, 4096> key_signature;
    };

    struct chunk_batch_received : public packet<0x0A> {
        float chunks_per_tick;
    };

    struct client_command : public packet<0x0B> {
        enum class action_id_e : uint8_t {
            perform_respawn = 0,
            request_stats = 1,
        };
        using enum action_id_e;
        enum_as<action_id_e, var_int32> action_id;
    };

    struct client_tick_end : public packet<0x0C> {};

    struct client_information : public packet<0x0D> {
        enum class chat_mode_e : uint8_t {
            disabled = 0,
            commands_only = 1,
            hidden = 2,
        };
        enum class displayer_skin_parts_f : uint8_t {
            cape = 0x1,
            jacket = 0x2,
            left_sleeve = 0x4,
            right_sleeve = 0x8,
            left_pants = 0x10,
            right_pants = 0x20,
            hat = 0x40,
            _unused = 0x80
        };
        enum class main_hand_e : uint8_t {
            left = 0,
            right = 1
        };
        enum class particle_status_e : uint8_t {
            all = 0,
            decreased = 1,
            minimal = 2,
        };
        string_sized<16> locale;
        uint8_t view_distance;
        enum_as<chat_mode_e, var_int32> chat_mode;
        bool enable_chat_colors;
        enum_as_flag<displayer_skin_parts_f, uint8_t> displayer_skin_parts;
        enum_as<main_hand_e, var_int32> main_hand;
        bool enable_text_filtering;
        bool allow_server_listings;
        enum_as<particle_status_e, var_int32> particle_status;
    };

    struct command_suggestion : public packet<0x0E> {
        var_int32 suggestion_transaction_id; //managed by client
        string_sized<32500> command_text;
    };

    struct configuration_acknowledged : public packet<0x0F>, switches_to::config {};

    struct container_button_click : public packet<0x10> {
        var_int32 window_id;
        var_int32 button_id;
    };

    struct container_click : public packet<0x11> {
        struct hashed_slot_data {
            var_int32::item item_id;
            var_int32 count;

            struct component {
                var_int32::data_component_type type;
                int32_t crc32c_hash;
            };

            list_array<component> add_components;
            list_array<var_int32::data_component_type> remove_components;
        };

        struct changed_slot {
            short slot;
            std::optional<hashed_slot_data> data = std::nullopt;
        };

        var_int32 window_id;
        var_int32 state_id;
        short slot;
        int8_t button;
        var_int32 mode;
        list_array_sized<changed_slot, 128> changed;
        std::optional<hashed_slot_data> carry_item = std::nullopt;
    };

    struct container_close : public packet<0x12> {
        var_int32 window_id;
    };

    struct container_slot_state_changed : public packet<0x13> {
        var_int32 slot_id;
        var_int32 window_id;
        bool state = false;
    };

    struct cookie_response : public packet<0x14> {
        identifier key;
        std::optional<list_array_sized<uint8_t, 5120>> payload = std::nullopt;
    };

    struct custom_payload : public packet<0x15> {
        identifier channel;
        list_array_sized_siz_from_packet<uint8_t, 32767> payload;
    };

    struct debug_subscription_request : public packet<0x16> {
        list_array<var_int32::debug_subscription> sample_types;
    };

    struct edit_book : public packet<0x17> {
        var_int32 slot;
        list_array_sized<string_sized<1024>, 100> entries;
        std::optional<string_sized<32>> title = std::nullopt;
    };

    struct entity_tag_query : public packet<0x18> {
        var_int32 tag_query_id; //managed by client
        var_int32::entity_id id;
    };

    struct interact : public packet<0x19> {
        var_int32::entity_id id;
        enum class hand_e : uint8_t {
            main = 0,
            off = 1
        };

        struct interact_ : public enum_item<0> {
            enum_as<hand_e, var_int32> hand;
        };

        struct attack : public enum_item<1> {};

        struct interact_at : public enum_item<2> {
            float x;
            float y;
            float z;
            enum_as<hand_e, var_int32> hand;
        };

        enum_switch<var_int32, interact_, attack, interact_at> type;
        bool sneak_key_pressed = false;
    };

    struct jigsaw_generate : public packet<0x1A> {
        base_objects::position location;
        var_int32 levels;
        bool keep_jigsaws = false;
    };

    struct keep_alive : public packet<0x1B> {
        uint64_t id;
    };

    struct lock_difficulty : public packet<0x1C> {
        bool is_locked = false;
    };

    struct move_player_pos : public packet<0x1D> {
        enum class flags_f : uint8_t {
            on_ground = 1,
            push_against_wall = 2
        };
        using enum flags_f;

        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        enum_as_flag<flags_f, int8_t> flags;
    };

    struct move_player_pos_rot : public packet<0x1E> {
        enum class flags_f : uint8_t {
            on_ground = 1,
            push_against_wall = 2
        };
        using enum flags_f;

        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        float yaw = 0.0f;
        float pitch = 0.0f;
        enum_as_flag<flags_f, int8_t> flags;
    };

    struct move_player_rot : public packet<0x1F> {
        enum class flags_f : uint8_t {
            on_ground = 1,
            push_against_wall = 2
        };
        using enum flags_f;

        float yaw = 0.0f;
        float pitch = 0.0f;
        enum_as_flag<flags_f, int8_t> flags;
    };

    struct move_player_status_only : public packet<0x20> {
        enum class flags_f : uint8_t {
            on_ground = 1,
            push_against_wall = 2
        };
        using enum flags_f;

        enum_as_flag<flags_f, int8_t> flags;
    };

    struct move_vehicle : public packet<0x21> {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        float yaw = 0.0f;
        float pitch = 0.0f;
        bool on_ground = false;
    };

    struct paddle_boat : public packet<0x22> {
        bool left_paddle_turning = false;
        bool right_paddle_turning = false;
    };

    struct pick_item_from_block : public packet<0x23> {
        base_objects::position location;
        bool include_data = false;
    };

    struct pick_item_from_entity : public packet<0x24> {
        var_int32::entity_id id;
        bool include_data = false;
    };

    struct ping_request : public packet<0x25> {
        uint64_t payload;
    };

    struct place_recipe : public packet<0x26> {
        var_int32 windows_id;
        var_int32::recipe recipe_id;
        bool make_all = false;
    };

    struct player_abilities : public packet<0x27> {
        enum class flags_f : uint8_t {
            flying = 2
        };
        using enum flags_f;

        enum_as_flag<flags_f, int8_t> flags;
    };

    struct player_action : public packet<0x28> {
        enum class status_e : uint8_t {
            digging_start = 0,
            digging_canceled = 1,
            digging_finished = 2,
            drop_item_stack = 3,
            drop_item = 4,
            right_click_item = 5,
            swap_item_in_hand = 6,
        };
        enum class face_e : uint8_t {
            bottom = 0,
            top = 1,
            north = 2,
            south = 3,
            west = 4,
            east = 5,
        };
        enum_as<status_e, var_int32> status;
        base_objects::position location;
        enum_as<face_e, int8_t> face;
        var_int32 block_sequence_id;
    };

    struct player_command : public packet<0x29> {
        enum class action_e : uint8_t {
            leave_bed = 0,
            start_sprinting = 1,
            stop_sprinting = 2,
            horse_jump_start = 3,
            horse_jump_stop = 4,
            inventory_vehicle_open = 5,
            elytra_fly = 6,
        };
        var_int32::entity_id id;
        enum_as<action_e, var_int32> action;
        var_int32 jump_boost;
    };

    struct player_input : public packet<0x2A> {
        enum class status_f : uint8_t {
            forward = 1,
            backward = 2,
            left = 4,
            right = 8,
            jump = 16,
            sneak = 32,
            sprint = 64,
        };
        using enum status_f;
        enum_as_flag<status_f, uint8_t> face;
    };

    struct player_loaded : public packet<0x2B> {};

    struct pong : public packet<0x2C> {
        int32_t id;
    };

    struct recipe_book_change_settings : public packet<0x2D> {
        enum class book_type_e : uint8_t {
            crafting = 0,
            furnace = 1,
            blast_furnace = 2,
            smoker = 3,
        };
        using enum book_type_e;

        enum_as<book_type_e, var_int32> book_type;
        bool book_open = false;
        bool filter_active = false;
    };

    struct recipe_book_seen_recipe : public packet<0x2E> {
        var_int32::recipe recipe_id;
    };

    struct rename_item : public packet<0x2F> {
        string_sized<32767> new_name;
    };

    struct resource_pack : public packet<0x30> {
        enum class result_e : uint8_t {
            success = 0,
            declined = 1,
            download_failed = 2,
            accepted = 3,
            downloaded = 4,
            invalid_url = 5,
            reload_failed = 6,
            discarded = 7
        };
        using enum result_e;
        base_objects::uuid uuid;
        enum_as<result_e, var_int32> result;
    };

    struct seen_advancements : public packet<0x31> {
        struct opened_tab : public enum_item<0> {
            identifier tab_id;
        };

        struct closed_screen : public enum_item<1> {};

        enum_switch<var_int32, opened_tab, closed_screen> action;
    };

    struct select_trade : public packet<0x32> {
        var_int32 selected_slot;
    };

    struct set_beacon : public packet<0x33> {
        std::optional<var_int32::mob_effect> primary_effect = std::nullopt;
        std::optional<var_int32::mob_effect> secondary_effect = std::nullopt;
    };

    struct set_carried_item : public packet<0x34> {
        short slot;
    };

    struct set_command_block : public packet<0x35> {
        enum class mode_e : uint8_t {
            chain = 0,
            repeating = 1,
            impulse = 2,
        };

        enum class flags_f : uint8_t {
            track_output = 1,
            is_conditional = 2,
            automatic = 4,
        };
        using enum flags_f;

        base_objects::position location;
        string_sized<32767> command;
        enum_as<mode_e, var_int32> mode;
        enum_as_flag<flags_f, int8_t> flags;
    };

    struct set_command_minecart : public packet<0x36> {
        var_int32::entity_id id;
        string_sized<32767> command;
        bool track_output;
    };

    struct set_creative_mode_slot : public packet<0x37> {
        short slot;
        struct slot item;
    };

    struct set_jigsaw_block : public packet<0x38> {
        base_objects::position location;
        identifier name;
        identifier target;
        identifier pool;
        string_sized<32767> final_state;
        string_sized<32767> joint_state;
        var_int32 selection_priority;
        var_int32 placement_priority;
    };

    struct set_structure_block : public packet<0x39> {
        enum class mirror_side_e : uint8_t {
            none = 0,
            left_right = 1,
            front_back = 2,
        };
        enum class rotation_e : uint8_t {
            none = 0,
            clockwise_90 = 1,
            clockwise_180 = 2,
            counterclockwise_90 = 3,
        };

        struct ignore_entities : public flag_item<1, 1, 1> {};

        struct show_air : public flag_item<2, 2, 2> {};

        struct show_bounding_block : public flag_item<4, 4, 3> {};

        struct strict_placement : public flag_item<8, 8, 4> {};

        base_objects::position location;
        var_int32 action;
        var_int32 mode;
        string_sized<32767> name;
        limited_num<int8_t, -48, 48> offset_x;
        limited_num<int8_t, -48, 48> offset_y;
        limited_num<int8_t, -48, 48> offset_z;
        limited_num<int8_t, 0, 48> size_x;
        limited_num<int8_t, 0, 48> size_y;
        limited_num<int8_t, 0, 48> size_z;
        enum_as<mirror_side_e, var_int32> mirror_side;
        enum_as<rotation_e, var_int32> rotation;
        string_sized<128> metadata;
        limited_num<float, 0.0f, 1.0f> integrity;
        var_int64 seed;
        flags_list<
            int8_t,
            ignore_entities,
            show_air,
            show_bounding_block,
            strict_placement>
            flags;
    };

    struct set_test_block : public packet<0x3A> {
        enum class mode_e : uint8_t {
            start = 0,
            log = 1,
            fail = 2,
            accept = 3,
        };
        base_objects::position location;
        enum_as<mode_e, var_int32> mode;
        std::string message;
    };

    struct sign_update : public packet<0x3B> {
        base_objects::position location;
        bool is_front_text;
        std::array<string_sized<384>, 4> lines;
    };

    struct swing : public packet<0x3C> {
        enum class hand_e : uint8_t {
            main = 0,
            off = 1,
        };
        enum_as<hand_e, var_int32> hand;
    };

    struct teleport_to_entity : public packet<0x3D> {
        base_objects::uuid uuid;
    };

    struct test_instance_block_action : public packet<0x3E> {
        enum class action_e : uint8_t {
            init = 0,
            query = 1,
            set = 2,
            reset = 3,
            save = 4,
            export_ = 5,
            run = 6,
        };
        enum class rotation_e : uint8_t {
            none = 0,
            clockwise_90 = 1,
            clockwise_180 = 2,
            counterclockwise_90 = 3,
        };
        enum class status_e : uint8_t {
            cleared = 0,
            running = 1,
            finished = 2,
        };
        base_objects::position location;
        enum_as<action_e, var_int32> action;
        std::optional<var_int32::test_instance_type> test_id = std::nullopt;
        var_int32 size_x;
        var_int32 size_y;
        var_int32 size_z;
        enum_as<rotation_e, var_int32> rotation;
        bool ignore_entities = false;
        enum_as<status_e, var_int32> status;
        std::optional<base_objects::chat> error_message = std::nullopt;
    };

    struct use_item_on : public packet<0x3F> {
        enum class hand_e : uint8_t {
            main = 0,
            off = 1,
        };
        enum_as<hand_e, var_int32> hand;
        base_objects::position location;
        var_int32 face;
        limited_num<float, 0.0f, 1.0f> cursor_x;
        limited_num<float, 0.0f, 1.0f> cursor_y;
        limited_num<float, 0.0f, 1.0f> cursor_z;
        bool inside_block = false;
        bool world_border_hit = false;
        var_int32 block_sequence_id;
    };

    struct use_item : public packet<0x40> {
        enum class hand_e : uint8_t {
            main = 0,
            off = 1,
        };
        enum_as<hand_e, var_int32> hand;
        var_int32 block_sequence_id;
        float yaw = 0.0f;
        float pitch = 0.0f;
    };

    struct custom_click_action : public packet<0x41> {
        identifier id;
        enbt::value payload;
    };
}
namespace copper_server::api::packets {
        extern template packet_ops<server_bound::play::accept_teleportation>;
        extern template packet_ops<server_bound::play::block_entity_tag_query>;
        extern template packet_ops<server_bound::play::bundle_item_selected>;
        extern template packet_ops<server_bound::play::change_difficulty>;
        extern template packet_ops<server_bound::play::change_gamemode>;
        extern template packet_ops<server_bound::play::chat_ack>;
        extern template packet_ops<server_bound::play::chat_command>;
        extern template packet_ops<server_bound::play::chat_command_signed>;
        extern template packet_ops<server_bound::play::chat>;
        extern template packet_ops<server_bound::play::chat_session_update>;
        extern template packet_ops<server_bound::play::chunk_batch_received>;
        extern template packet_ops<server_bound::play::client_command>;
        extern template packet_ops<server_bound::play::client_tick_end>;
        extern template packet_ops<server_bound::play::client_information>;
        extern template packet_ops<server_bound::play::command_suggestion>;
        extern template packet_ops<server_bound::play::configuration_acknowledged>;
        extern template packet_ops<server_bound::play::container_button_click>;
        extern template packet_ops<server_bound::play::container_click>;
        extern template packet_ops<server_bound::play::container_close>;
        extern template packet_ops<server_bound::play::container_slot_state_changed>;
        extern template packet_ops<server_bound::play::cookie_response>;
        extern template packet_ops<server_bound::play::custom_payload>;
        extern template packet_ops<server_bound::play::debug_subscription_request>;
        extern template packet_ops<server_bound::play::edit_book>;
        extern template packet_ops<server_bound::play::entity_tag_query>;
        extern template packet_ops<server_bound::play::interact>;
        extern template packet_ops<server_bound::play::jigsaw_generate>;
        extern template packet_ops<server_bound::play::keep_alive>;
        extern template packet_ops<server_bound::play::lock_difficulty>;
        extern template packet_ops<server_bound::play::move_player_pos>;
        extern template packet_ops<server_bound::play::move_player_pos_rot>;
        extern template packet_ops<server_bound::play::move_player_rot>;
        extern template packet_ops<server_bound::play::move_player_status_only>;
        extern template packet_ops<server_bound::play::move_vehicle>;
        extern template packet_ops<server_bound::play::paddle_boat>;
        extern template packet_ops<server_bound::play::pick_item_from_block>;
        extern template packet_ops<server_bound::play::pick_item_from_entity>;
        extern template packet_ops<server_bound::play::ping_request>;
        extern template packet_ops<server_bound::play::place_recipe>;
        extern template packet_ops<server_bound::play::player_abilities>;
        extern template packet_ops<server_bound::play::player_action>;
        extern template packet_ops<server_bound::play::player_command>;
        extern template packet_ops<server_bound::play::player_input>;
        extern template packet_ops<server_bound::play::player_loaded>;
        extern template packet_ops<server_bound::play::pong>;
        extern template packet_ops<server_bound::play::recipe_book_change_settings>;
        extern template packet_ops<server_bound::play::recipe_book_seen_recipe>;
        extern template packet_ops<server_bound::play::rename_item>;
        extern template packet_ops<server_bound::play::resource_pack>;
        extern template packet_ops<server_bound::play::seen_advancements>;
        extern template packet_ops<server_bound::play::select_trade>;
        extern template packet_ops<server_bound::play::set_beacon>;
        extern template packet_ops<server_bound::play::set_carried_item>;
        extern template packet_ops<server_bound::play::set_command_block>;
        extern template packet_ops<server_bound::play::set_command_minecart>;
        extern template packet_ops<server_bound::play::set_creative_mode_slot>;
        extern template packet_ops<server_bound::play::set_jigsaw_block>;
        extern template packet_ops<server_bound::play::set_structure_block>;
        extern template packet_ops<server_bound::play::set_test_block>;
        extern template packet_ops<server_bound::play::sign_update>;
        extern template packet_ops<server_bound::play::swing>;
        extern template packet_ops<server_bound::play::teleport_to_entity>;
        extern template packet_ops<server_bound::play::test_instance_block_action>;
        extern template packet_ops<server_bound::play::use_item_on>;
        extern template packet_ops<server_bound::play::use_item>;
        extern template packet_ops<server_bound::play::custom_click_action>;


    using server_bound_play_ops = state_ops<
        server_bound::play::accept_teleportation,
        server_bound::play::block_entity_tag_query,
        server_bound::play::bundle_item_selected,
        server_bound::play::change_difficulty,
        server_bound::play::change_gamemode,
        server_bound::play::chat_ack,
        server_bound::play::chat_command,
        server_bound::play::chat_command_signed,
        server_bound::play::chat,
        server_bound::play::chat_session_update,
        server_bound::play::chunk_batch_received,
        server_bound::play::client_command,
        server_bound::play::client_tick_end,
        server_bound::play::client_information,
        server_bound::play::command_suggestion,
        server_bound::play::configuration_acknowledged,
        server_bound::play::container_button_click,
        server_bound::play::container_click,
        server_bound::play::container_close,
        server_bound::play::container_slot_state_changed,
        server_bound::play::cookie_response,
        server_bound::play::custom_payload,
        server_bound::play::debug_subscription_request,
        server_bound::play::edit_book,
        server_bound::play::entity_tag_query,
        server_bound::play::interact,
        server_bound::play::jigsaw_generate,
        server_bound::play::keep_alive,
        server_bound::play::lock_difficulty,
        server_bound::play::move_player_pos,
        server_bound::play::move_player_pos_rot,
        server_bound::play::move_player_rot,
        server_bound::play::move_player_status_only,
        server_bound::play::move_vehicle,
        server_bound::play::paddle_boat,
        server_bound::play::pick_item_from_block,
        server_bound::play::pick_item_from_entity,
        server_bound::play::ping_request,
        server_bound::play::place_recipe,
        server_bound::play::player_abilities,
        server_bound::play::player_action,
        server_bound::play::player_command,
        server_bound::play::player_input,
        server_bound::play::player_loaded,
        server_bound::play::pong,
        server_bound::play::recipe_book_change_settings,
        server_bound::play::recipe_book_seen_recipe,
        server_bound::play::rename_item,
        server_bound::play::resource_pack,
        server_bound::play::seen_advancements,
        server_bound::play::select_trade,
        server_bound::play::set_beacon,
        server_bound::play::set_carried_item,
        server_bound::play::set_command_block,
        server_bound::play::set_command_minecart,
        server_bound::play::set_creative_mode_slot,
        server_bound::play::set_jigsaw_block,
        server_bound::play::set_structure_block,
        server_bound::play::set_test_block,
        server_bound::play::sign_update,
        server_bound::play::swing,
        server_bound::play::teleport_to_entity,
        server_bound::play::test_instance_block_action,
        server_bound::play::use_item_on,
        server_bound::play::use_item,
        server_bound::play::custom_click_action
    >;
}

inline copper_server::api::packets::server_bound::play::client_information::displayer_skin_parts_f operator|(copper_server::api::packets::server_bound::play::client_information::displayer_skin_parts_f a, copper_server::api::packets::server_bound::play::client_information::displayer_skin_parts_f b) {
    return copper_server::api::packets::server_bound::play::client_information::displayer_skin_parts_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::server_bound::play::move_player_pos::flags_f operator|(copper_server::api::packets::server_bound::play::move_player_pos::flags_f a, copper_server::api::packets::server_bound::play::move_player_pos::flags_f b) {
    return copper_server::api::packets::server_bound::play::move_player_pos::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::server_bound::play::move_player_pos_rot::flags_f operator|(copper_server::api::packets::server_bound::play::move_player_pos_rot::flags_f a, copper_server::api::packets::server_bound::play::move_player_pos_rot::flags_f b) {
    return copper_server::api::packets::server_bound::play::move_player_pos_rot::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::server_bound::play::move_player_rot::flags_f operator|(copper_server::api::packets::server_bound::play::move_player_rot::flags_f a, copper_server::api::packets::server_bound::play::move_player_rot::flags_f b) {
    return copper_server::api::packets::server_bound::play::move_player_rot::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::server_bound::play::move_player_status_only::flags_f operator|(copper_server::api::packets::server_bound::play::move_player_status_only::flags_f a, copper_server::api::packets::server_bound::play::move_player_status_only::flags_f b) {
    return copper_server::api::packets::server_bound::play::move_player_status_only::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::server_bound::play::player_input::status_f operator|(copper_server::api::packets::server_bound::play::player_input::status_f a, copper_server::api::packets::server_bound::play::player_input::status_f b) {
    return copper_server::api::packets::server_bound::play::player_input::status_f(static_cast<int>(a) | static_cast<int>(b));
}

inline copper_server::api::packets::server_bound::play::set_command_block::flags_f operator|(copper_server::api::packets::server_bound::play::set_command_block::flags_f a, copper_server::api::packets::server_bound::play::set_command_block::flags_f b) {
    return copper_server::api::packets::server_bound::play::set_command_block::flags_f(static_cast<int>(a) | static_cast<int>(b));
}

#endif /* SRC_API_PACKETS_SERVER_BOUND_PLAY */
