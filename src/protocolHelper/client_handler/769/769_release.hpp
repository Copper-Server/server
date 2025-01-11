#ifndef SRC_PROTOCOLHELPER_CLIENT_HANDLER_769_769_RELEASE
#define SRC_PROTOCOLHELPER_CLIENT_HANDLER_769_769_RELEASE
#include <src/protocolHelper/util.hpp>

namespace copper_server::client_handler::play_769_release {
    void teleport_confirm(base_objects::network::tcp_session* session, ArrayStream& packet);

    void query_block_nbt(base_objects::network::tcp_session* session, ArrayStream& packet);

    void bundle_item_selected(base_objects::network::tcp_session* session, ArrayStream& packet);

    void change_difficulty(base_objects::network::tcp_session* session, ArrayStream& packet);

    void acknowledge_message(base_objects::network::tcp_session* session, ArrayStream& packet);

    void chat_command(base_objects::network::tcp_session* session, ArrayStream& packet);

    void signed_chat_command(base_objects::network::tcp_session* session, ArrayStream& packet);

    void chat_message(base_objects::network::tcp_session* session, ArrayStream& packet);

    void player_session(base_objects::network::tcp_session* session, ArrayStream& packet);

    void chunk_batch_received(base_objects::network::tcp_session* session, ArrayStream& packet);

    void client_status(base_objects::network::tcp_session* session, ArrayStream& packet);

    void client_tick_end(base_objects::network::tcp_session* session, ArrayStream& packet);

    void client_information(base_objects::network::tcp_session* session, ArrayStream& packet);

    void command_suggestion(base_objects::network::tcp_session* session, ArrayStream& packet);

    void switch_to_configuration(base_objects::network::tcp_session* session, ArrayStream& packet, base_objects::network::tcp_client*& next_handler);

    void click_container_button(base_objects::network::tcp_session* session, ArrayStream& packet);

    void click_container(base_objects::network::tcp_session* session, ArrayStream& packet);

    void close_container(base_objects::network::tcp_session* session, ArrayStream& packet);

    void change_container_slot_state(base_objects::network::tcp_session* session, ArrayStream& packet);

    void cookie_response(base_objects::network::tcp_session* session, ArrayStream& packet);

    base_objects::network::response plugin_message(base_objects::network::tcp_session* session, ArrayStream& packet, std::list<base_objects::network::plugin_response>& queriedPackets);

    void subscribe_to_debug_sample(base_objects::network::tcp_session* session, ArrayStream& packet);

    void edit_book(base_objects::network::tcp_session* session, ArrayStream& packet);

    void query_entity_tag(base_objects::network::tcp_session* session, ArrayStream& packet);

    void interact(base_objects::network::tcp_session* session, ArrayStream& packet);

    void jigsaw_generate(base_objects::network::tcp_session* session, ArrayStream& packet);

    void lock_difficulty(base_objects::network::tcp_session* session, ArrayStream& packet);

    void set_player_position(base_objects::network::tcp_session* session, ArrayStream& packet);

    void set_player_position_and_rotation(base_objects::network::tcp_session* session, ArrayStream& packet);

    void set_player_rotation(base_objects::network::tcp_session* session, ArrayStream& packet);

    void set_player_movement_flags(base_objects::network::tcp_session* session, ArrayStream& packet);

    void move_vehicle(base_objects::network::tcp_session* session, ArrayStream& packet);

    void paddle_boat(base_objects::network::tcp_session* session, ArrayStream& packet);

    void pick_item_from_block(base_objects::network::tcp_session* session, ArrayStream& packet);

    void pick_item_from_entity(base_objects::network::tcp_session* session, ArrayStream& packet);

    base_objects::network::response ping_request(base_objects::network::tcp_session* session, ArrayStream& packet);

    void place_recipe(base_objects::network::tcp_session* session, ArrayStream& packet);

    void player_abilities(base_objects::network::tcp_session* session, ArrayStream& packet);

    void player_action(base_objects::network::tcp_session* session, ArrayStream& packet);

    void player_command(base_objects::network::tcp_session* session, ArrayStream& packet);

    void player_input(base_objects::network::tcp_session* session, ArrayStream& packet);

    void player_loaded(base_objects::network::tcp_session* session, ArrayStream& packet);

    void change_recipe_book_settings(base_objects::network::tcp_session* session, ArrayStream& packet);

    void set_seen_recipe(base_objects::network::tcp_session* session, ArrayStream& packet);

    void rename_item(base_objects::network::tcp_session* session, ArrayStream& packet);

    void resource_pack_response(base_objects::network::tcp_session* session, ArrayStream& packet);

    void seen_advancements(base_objects::network::tcp_session* session, ArrayStream& packet);

    void select_trade(base_objects::network::tcp_session* session, ArrayStream& packet);

    void set_beacon_effect(base_objects::network::tcp_session* session, ArrayStream& packet);

    void set_held_item(base_objects::network::tcp_session* session, ArrayStream& packet);

    void program_command_block(base_objects::network::tcp_session* session, ArrayStream& packet);

    void program_command_cart(base_objects::network::tcp_session* session, ArrayStream& packet);

    void set_creative_slot(base_objects::network::tcp_session* session, ArrayStream& packet);

    void program_jigsaw_block(base_objects::network::tcp_session* session, ArrayStream& packet);

    void program_structure_block(base_objects::network::tcp_session* session, ArrayStream& packet);

    void update_sign(base_objects::network::tcp_session* session, ArrayStream& packet);

    void swing_arm(base_objects::network::tcp_session* session, ArrayStream& packet);

    void spectator_teleport(base_objects::network::tcp_session* session, ArrayStream& packet);

    void use_item_on(base_objects::network::tcp_session* session, ArrayStream& packet);

    void use_item(base_objects::network::tcp_session* session, ArrayStream& packet);
}


#endif /* SRC_PROTOCOLHELPER_CLIENT_HANDLER_769_769_RELEASE */
