#include "allowlist.hpp"
#include "ban.hpp"
#include "players.hpp"
#include "protocol.hpp"

namespace crafted_craft {
    namespace api {
        namespace allowlist {
            base_objects::event<allowlist_mode> on_mode_change;
            base_objects::event<std::string> on_kick;
            base_objects::event<std::string> on_add;
            base_objects::event<std::string> on_remove;
        }

        namespace ban {
            base_objects::event<ban_data> on_ban;
            base_objects::event<ban_data> on_pardon;
            base_objects::event<ban_data> on_ban_ip;
            base_objects::event<ban_data> on_pardon_ip;
        }

        namespace players {
            namespace handlers {
                base_objects::event<base_objects::client_data_holder> on_player_join;
                base_objects::event<base_objects::client_data_holder> on_player_leave;
            }

            namespace calls {
                base_objects::event<teleport_request> on_teleport_request;
                base_objects::event<player_chat> on_player_chat;
                base_objects::event<player_personal_chat> on_player_personal_chat;
                base_objects::event<Chat> on_system_message_broadcast;
                base_objects::event<personal<Chat>> on_system_message;
                base_objects::event<Chat> on_system_message_overlay_broadcast;
                base_objects::event<personal<Chat>> on_system_message_overlay;
                base_objects::event<personal<Chat>> on_player_kick;
                base_objects::event<personal<Chat>> on_player_ban;
                base_objects::event<Chat> on_action_bar_message_broadcast;
                base_objects::event<personal<Chat>> on_action_bar_message;
                base_objects::event<Chat> on_title_message_broadcast;
                base_objects::event<personal<Chat>> on_title_message;
                base_objects::event<Chat> on_subtitle_message_broadcast;
                base_objects::event<personal<Chat>> on_subtitle_message;
                base_objects::event<titles_times> on_title_times_broadcast;
                base_objects::event<personal<titles_times>> on_title_times;
                base_objects::event<unsigned_chat> on_unsigned_message_broadcast;
                base_objects::event<personal<unsigned_chat>> on_unsigned_message;
            }
        }

        namespace protocol {
            base_objects::event<event_data<data::teleport_request_completion>> on_teleport_request_completion;
            base_objects::event<event_data<data::block_nbt_request>> on_block_nbt_request;
            base_objects::event<event_data<uint8_t>> on_change_difficulty;
            base_objects::event<event_data<int32_t>> on_acknowledge_message;
            base_objects::event<event_data<data::chat_command>> on_chat_command;
            base_objects::event<event_data<data::chat_message_unsigned>> on_chat_message_unsigned;
            base_objects::event<event_data<data::chat_message_signed>> on_chat_message_signed;
            base_objects::event<event_data<data::player_session>> on_player_session;
            base_objects::event<event_data<float>> on_chunk_batch_received;
            base_objects::event<event_data<int32_t>> on_client_status;
            base_objects::event<event_data<data::client_information>> on_client_information;
            base_objects::event<event_data<data::command_suggestion>> on_command_suggestion;
            base_objects::event<event_data<data::click_container_button>> on_click_container_button;
            base_objects::event<event_data<data::click_container>> on_click_container;
            base_objects::event<event_data<uint8_t>> on_close_container;
            base_objects::event<event_data<data::change_container_slot_state>> on_change_container_slot_state;
            base_objects::event<event_data<data::plugin_message>> on_plugin_message;
            base_objects::event<event_data<data::edit_book>> on_edit_book;
            base_objects::event<event_data<data::query_entity_tag>> on_query_entity_tag;
            base_objects::event<event_data<data::interact_attack>> on_interact_attack;
            base_objects::event<event_data<data::interact_at>> on_interact_at;
            base_objects::event<event_data<data::interact>> on_interact;
            base_objects::event<event_data<data::jigsaw_generate>> on_jigsaw_generate;
            base_objects::event<event_data<int64_t>> on_keep_alive;
            base_objects::event<event_data<bool>> on_lock_difficulty;
            base_objects::event<event_data<data::set_player_position>> on_set_player_position;
            base_objects::event<event_data<data::set_player_position_and_rotation>> on_set_player_position_and_rotation;
            base_objects::event<event_data<data::set_player_rotation>> on_set_player_rotation;
            base_objects::event<event_data<bool>> on_set_player_on_ground;
            base_objects::event<event_data<data::move_vehicle>> on_move_vehicle;
            base_objects::event<event_data<data::paddle_boat>> on_paddle_boat;
            base_objects::event<event_data<data::pick_item>> on_pick_item;
            base_objects::event<event_data<int64_t>> on_ping_request;
            base_objects::event<event_data<data::place_recipe>> on_place_recipe;
            base_objects::event<event_data<int8_t>> on_player_abilities;
            base_objects::event<event_data<data::player_action>> on_player_action;
            base_objects::event<event_data<data::player_command>> on_player_command;
            base_objects::event<event_data<data::player_input>> on_player_input;
            base_objects::event<event_data<data::pong>> on_pong;
            base_objects::event<event_data<data::change_recipe_book_settings>> on_change_recipe_book_settings;
            base_objects::event<event_data<std::string>> on_set_seen_recipe;
            base_objects::event<event_data<std::string>> on_rename_item;
            base_objects::event<event_data<data::resource_pack_response>> on_resource_pack_response;
            base_objects::event<event_data<data::seen_advancements>> on_seen_advancements;
            base_objects::event<event_data<int32_t>> on_select_trade;
            base_objects::event<event_data<data::set_beacon_effect>> on_set_beacon_effect;
            base_objects::event<event_data<int16_t>> on_set_held_item;
            base_objects::event<event_data<data::program_command_block>> on_program_command_block;
            base_objects::event<event_data<data::program_command_cart>> on_program_command_cart;
            base_objects::event<event_data<data::set_creative_slot>> on_set_creative_slot;
            base_objects::event<event_data<data::program_jigsaw_block>> on_program_jigsaw_block;
            base_objects::event<event_data<data::program_structure_block>> on_program_structure_block;
            base_objects::event<event_data<data::update_sign>> on_update_sign;
            base_objects::event<event_data<data::swing_arm>> on_swing_arm;
            base_objects::event<event_data<data::spectator_teleport>> on_spectator_teleport;
            base_objects::event<event_data<data::use_item_on>> on_use_item_on;
            base_objects::event<event_data<data::use_item>> on_use_item;
        } // namespace protocol
    }
}
