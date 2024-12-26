#include <src/api/allowlist.hpp>
#include <src/api/ban.hpp>
#include <src/api/protocol.hpp>
#include <src/api/statistics.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/virtual_client.hpp>
#include <src/log.hpp>

namespace copper_server::api {
    namespace allowlist {
        base_objects::events::event<allowlist_mode> on_mode_change;
        base_objects::events::event<std::string> on_kick;
        base_objects::events::event<std::string> on_add;
        base_objects::events::event<std::string> on_remove;
    }

    namespace ban {
        base_objects::events::event<ban_data> on_ban;
        base_objects::events::event<ban_data> on_pardon;
        base_objects::events::event<ban_data> on_ban_ip;
        base_objects::events::event<ban_data> on_pardon_ip;
    }
    namespace protocol {
        base_objects::events::event<event_data<data::teleport_request_completion>> on_teleport_request_completion;
        base_objects::events::event<event_data<data::block_nbt_request>> on_block_nbt_request;
        base_objects::events::event<event_data<uint8_t>> on_change_difficulty;
        base_objects::events::event<event_data<int32_t>> on_acknowledge_message;
        base_objects::events::event<event_data<data::chat_command>> on_chat_command;
        base_objects::events::event<event_data<data::signed_chat_command>> on_signed_chat_command;
        base_objects::events::event<event_data<data::chat_message_unsigned>> on_chat_message_unsigned;
        base_objects::events::event<event_data<data::chat_message_signed>> on_chat_message_signed;
        base_objects::events::event<event_data<data::player_session>> on_player_session;
        base_objects::events::event<event_data<float>> on_chunk_batch_received;
        base_objects::events::event<event_data<int32_t>> on_client_status;
        base_objects::events::event<event_data<data::client_information>> on_client_information;
        base_objects::events::event<event_data<data::command_suggestion>> on_command_suggestion;
        base_objects::events::event<event_data<data::click_container_button>> on_click_container_button;
        base_objects::events::event<event_data<data::click_container>> on_click_container;
        base_objects::events::event<event_data<uint8_t>> on_close_container;
        base_objects::events::event<event_data<data::change_container_slot_state>> on_change_container_slot_state;
        base_objects::events::event<event_data<data::cookie_response>> on_cookie_response;
        base_objects::events::event<event_data<data::plugin_message>> on_plugin_message;
        base_objects::events::event<event_data<data::debug_sample_subscription>> on_debug_sample_subscription;
        base_objects::events::event<event_data<data::edit_book>> on_edit_book;
        base_objects::events::event<event_data<data::query_entity_tag>> on_query_entity_tag;
        base_objects::events::event<event_data<data::interact_attack>> on_interact_attack;
        base_objects::events::event<event_data<data::interact_at>> on_interact_at;
        base_objects::events::event<event_data<data::interact>> on_interact;
        base_objects::events::event<event_data<data::jigsaw_generate>> on_jigsaw_generate;
        base_objects::events::event<event_data<int64_t>> on_keep_alive;
        base_objects::events::event<event_data<bool>> on_lock_difficulty;
        base_objects::events::event<event_data<data::set_player_position>> on_set_player_position;
        base_objects::events::event<event_data<data::set_player_position_and_rotation>> on_set_player_position_and_rotation;
        base_objects::events::event<event_data<data::set_player_rotation>> on_set_player_rotation;
        base_objects::events::event<event_data<bool>> on_set_player_on_ground;
        base_objects::events::event<event_data<data::move_vehicle>> on_move_vehicle;
        base_objects::events::event<event_data<data::paddle_boat>> on_paddle_boat;
        base_objects::events::event<event_data<data::pick_item>> on_pick_item;
        base_objects::events::event<event_data<int64_t>> on_ping_request;
        base_objects::events::event<event_data<data::place_recipe>> on_place_recipe;
        base_objects::events::event<event_data<int8_t>> on_player_abilities;
        base_objects::events::event<event_data<data::player_action>> on_player_action;
        base_objects::events::event<event_data<data::player_command>> on_player_command;
        base_objects::events::event<event_data<data::player_input>> on_player_input;
        base_objects::events::event<event_data<data::pong>> on_pong;
        base_objects::events::event<event_data<data::change_recipe_book_settings>> on_change_recipe_book_settings;
        base_objects::events::event<event_data<std::string>> on_set_seen_recipe;
        base_objects::events::event<event_data<std::string>> on_rename_item;
        base_objects::events::event<event_data<data::resource_pack_response>> on_resource_pack_response;
        base_objects::events::event<event_data<data::seen_advancements>> on_seen_advancements;
        base_objects::events::event<event_data<int32_t>> on_select_trade;
        base_objects::events::event<event_data<data::set_beacon_effect>> on_set_beacon_effect;
        base_objects::events::event<event_data<int16_t>> on_set_held_item;
        base_objects::events::event<event_data<data::program_command_block>> on_program_command_block;
        base_objects::events::event<event_data<data::program_command_cart>> on_program_command_cart;
        base_objects::events::event<event_data<data::set_creative_slot>> on_set_creative_slot;
        base_objects::events::event<event_data<data::program_jigsaw_block>> on_program_jigsaw_block;
        base_objects::events::event<event_data<data::program_structure_block>> on_program_structure_block;
        base_objects::events::event<event_data<data::update_sign>> on_update_sign;
        base_objects::events::event<event_data<data::swing_arm>> on_swing_arm;
        base_objects::events::event<event_data<data::spectator_teleport>> on_spectator_teleport;
        base_objects::events::event<event_data<data::use_item_on>> on_use_item_on;
        base_objects::events::event<event_data<data::use_item>> on_use_item;
    } // namespace protocol

    namespace command {
        base_objects::command_manager* global_manager = nullptr;

        void register_manager(base_objects::command_manager& manager) {
            if (!global_manager)
                global_manager = &manager;
            else
                throw std::runtime_error("Command manager already registered");
        }

        void unregister_manager() {
            if (global_manager)
                global_manager = nullptr;
            else
                throw std::runtime_error("Command manager not yet registered");
        }

        base_objects::command_manager& get_manager() {
            if (global_manager)
                return *global_manager;
            else
                throw std::runtime_error("Command manager not yet registered");
        }
    }

    namespace console {
        base_objects::virtual_client* console_data;

        void register_virtual_client(base_objects::virtual_client& client) {
            if (!console_data)
                console_data = &client;
            else
                throw std::runtime_error("Console's virtual client already registered");
        }

        void unregister_virtual_client() {
            if (console_data)
                console_data = nullptr;
            else
                throw std::runtime_error("Console's virtual client not yet registered");
        }

        void execute_as_console(const std::string& command) {
            if (!console_data)
                throw std::runtime_error("Console's virtual client not yet registered");

            base_objects::command_context context(console_data->client);
            api::command::get_manager().execute_command(command, context);
        }

        bool console_enabled() {
            if (!console_data)
                return false;
            return log::commands::is_inited();
        }
    }

    namespace statistics {
        namespace minecraft {
            base_objects::events::event<statistic_event> custom;
            base_objects::events::event<statistic_event> mined;
            base_objects::events::event<statistic_event> broken;
            base_objects::events::event<statistic_event> crafted;
            base_objects::events::event<statistic_event> used;
            base_objects::events::event<statistic_event> picked_up;
            base_objects::events::event<statistic_event> dropped;
            base_objects::events::event<statistic_event> killed;
            base_objects::events::event<statistic_event> killed_by;
        }
    }
}
