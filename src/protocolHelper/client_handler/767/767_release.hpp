#ifndef SRC_PROTOCOLHELPER_CLIENT_HANDLER_767_767_RELEASE
#define SRC_PROTOCOLHELPER_CLIENT_HANDLER_767_767_RELEASE
#include "../util.hpp"

namespace crafted_craft {
    namespace client_handler {
        namespace play_767_release {
            void teleport_confirm(TCPsession* session, ArrayStream& packet);

            void query_block_nbt(TCPsession* session, ArrayStream& packet);

            void change_difficulty(TCPsession* session, ArrayStream& packet);

            void acknowledge_message(TCPsession* session, ArrayStream& packet);

            void chat_command(TCPsession* session, ArrayStream& packet);

            void signed_chat_command(TCPsession* session, ArrayStream& packet);

            void chat_message(TCPsession* session, ArrayStream& packet);

            void player_session(TCPsession* session, ArrayStream& packet);

            void chunk_batch_received(TCPsession* session, ArrayStream& packet);

            void client_status(TCPsession* session, ArrayStream& packet);

            void client_information(TCPsession* session, ArrayStream& packet);

            void command_suggestion(TCPsession* session, ArrayStream& packet);

            void switch_to_configuration(TCPsession* session, ArrayStream& packet, TCPclient*& next_handler);

            void click_container_button(TCPsession* session, ArrayStream& packet);

            void click_container(TCPsession* session, ArrayStream& packet);

            void close_container(TCPsession* session, ArrayStream& packet);

            void change_container_slot_state(TCPsession* session, ArrayStream& packet);

            void cookie_response(TCPsession* session, ArrayStream& packet);

            Response plugin_message(TCPsession* session, ArrayStream& packet, std::list<PluginRegistration::plugin_response>& queriedPackets);

            void subscribe_to_debug_sample(TCPsession* session, ArrayStream& packet);

            void edit_book(TCPsession* session, ArrayStream& packet);

            void query_entity_tag(TCPsession* session, ArrayStream& packet);

            void interact(TCPsession* session, ArrayStream& packet);

            void jigsaw_generate(TCPsession* session, ArrayStream& packet);

            void lock_difficulty(TCPsession* session, ArrayStream& packet);

            void set_player_position(TCPsession* session, ArrayStream& packet);

            void set_player_position_and_rotation(TCPsession* session, ArrayStream& packet);

            void set_player_rotation(TCPsession* session, ArrayStream& packet);

            void set_player_on_ground(TCPsession* session, ArrayStream& packet);

            void move_vehicle(TCPsession* session, ArrayStream& packet);

            void paddle_boat(TCPsession* session, ArrayStream& packet);

            void pick_item(TCPsession* session, ArrayStream& packet);

            Response ping_request(TCPsession* session, ArrayStream& packet);

            void place_recipe(TCPsession* session, ArrayStream& packet);

            void player_abilities(TCPsession* session, ArrayStream& packet);

            void player_action(TCPsession* session, ArrayStream& packet);

            void player_command(TCPsession* session, ArrayStream& packet);

            void player_input(TCPsession* session, ArrayStream& packet);

            void change_recipe_book_settings(TCPsession* session, ArrayStream& packet);

            void set_seen_recipe(TCPsession* session, ArrayStream& packet);

            void rename_item(TCPsession* session, ArrayStream& packet);

            void resource_pack_response(TCPsession* session, ArrayStream& packet);

            void seen_advancements(TCPsession* session, ArrayStream& packet);

            void select_trade(TCPsession* session, ArrayStream& packet);

            void set_beacon_effect(TCPsession* session, ArrayStream& packet);

            void set_held_item(TCPsession* session, ArrayStream& packet);

            void program_command_block(TCPsession* session, ArrayStream& packet);

            void program_command_cart(TCPsession* session, ArrayStream& packet);

            void set_creative_slot(TCPsession* session, ArrayStream& packet);

            void program_jigsaw_block(TCPsession* session, ArrayStream& packet);

            void program_structure_block(TCPsession* session, ArrayStream& packet);

            void update_sign(TCPsession* session, ArrayStream& packet);

            void swing_arm(TCPsession* session, ArrayStream& packet);

            void spectator_teleport(TCPsession* session, ArrayStream& packet);

            void use_item_on(TCPsession* session, ArrayStream& packet);

            void use_item(TCPsession* session, ArrayStream& packet);
        }
    }
} // namespace crafted_craft


#endif /* SRC_PROTOCOLHELPER_CLIENT_HANDLER_767_767_RELEASE */
