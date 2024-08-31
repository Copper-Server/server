#ifndef SRC_API_PROTOCOL
#define SRC_API_PROTOCOL
#include "../base_objects/event.hpp"
#include "../base_objects/position.hpp"
#include "../base_objects/slot.hpp"
#include "../library/enbt.hpp"
#include "../library/list_array.hpp"
#include "../protocolHelper/util.hpp"

namespace crafted_craft {
    namespace api {
        namespace protocol {
            namespace data {
                struct teleport_request_completion {
                    int32_t teleport_id;
                    bool success;
                };

                struct block_nbt_request {
                    int32_t transaction_id;
                    Position position;
                };

                using chat_command = std::string;

                struct signed_chat_command {
                    std::string command;
                    int64_t timestamp;
                    int64_t salt;

                    struct argument_signature {
                        std::string argument_name;
                        uint8_t signature[256];
                    };

                    list_array<argument_signature> arguments_signature;
                    int32_t message_count;
                    bit_list_array<> acknowledged;
                };

                struct chat_message_unsigned {
                    std::string message;
                    int64_t timestamp;
                    int64_t salt;
                    int32_t message_count;
                    bit_list_array<> acknowledged;
                };

                struct chat_message_signed : public chat_message_unsigned {
                    uint8_t signature[256];
                };

                struct player_session {
                    ENBT::UUID session_id;

                    struct public_key_t {
                        list_array<uint8_t> public_key;
                        list_array<uint8_t> key_signature;
                        int64_t expiries_at;
                    } public_key;
                };

                struct client_information {
                    std::string locale;
                    uint8_t view_distance;
                    bool chat_colors;
                    uint8_t displayed_skin_parts;
                    bool enable_text_filtering;
                    bool allow_server_listings;
                    int32_t chat_mode;
                    int32_t main_hand;
                };

                struct command_suggestion {
                    int32_t transaction_id;
                    std::string text;
                };

                struct click_container_button {
                    int8_t window_id;
                    int8_t button_id;
                };

                struct click_container {
                    struct changed_slot {
                        int16_t slot;
                        base_objects::slot item;
                    };

                    uint8_t window_id;
                    int32_t state_id;
                    int16_t slot;
                    int8_t button;
                    int32_t mode;
                    list_array<changed_slot> changed_slots;
                    base_objects::slot carried_item;
                };

                struct change_container_slot_state {
                    int32_t slot_id;
                    int32_t window_id;
                    bool state;
                };

                struct plugin_message {
                    std::string channel;
                    list_array<uint8_t> data;
                };

                struct edit_book {
                    int32_t slot;
                    list_array<std::string> text;
                    std::optional<std::string> title;
                };

                struct query_entity_tag {
                    int32_t transaction_id;
                    int32_t entity_id;
                };

                struct interact_attack {
                    int32_t entity_id;
                    bool sneaking;
                };

                struct interact_at {
                    int32_t entity_id;
                    float target_x;
                    float target_y;
                    float target_z;
                    int8_t hand;
                    bool sneaking;
                };

                struct interact {
                    int32_t entity_id;
                    int8_t hand;
                    bool sneaking;
                };

                struct jigsaw_generate {
                    Position location;
                    int32_t levels;
                    bool keep_jigsaws;
                };

                struct set_player_position {
                    double x;
                    double y;
                    double z;
                    bool on_ground;
                };

                struct set_player_position_and_rotation {
                    double x;
                    double y;
                    double z;
                    float yaw;
                    float pitch;
                    bool on_ground;
                };

                struct set_player_rotation {
                    float yaw;
                    float pitch;
                    bool on_ground;
                };

                struct move_vehicle {
                    double x;
                    double y;
                    double z;
                    float yaw;
                    float pitch;
                };

                struct paddle_boat {
                    bool left_paddle;
                    bool right_paddle;
                };

                struct pick_item {
                    int32_t slot;
                };

                struct place_recipe {
                    int8_t window_id;
                    std::string recipe_id;
                    bool make_all;
                };

                struct player_action {
                    int32_t status;
                    Position location;
                    int8_t face;
                    int32_t sequence_id;
                };

                struct player_command {
                    int32_t entity_id;
                    int32_t action_id;
                    int32_t jump_boost;
                };

                struct player_input {
                    float sideways;
                    float forward;

                    union input_flags {
                        struct {
                            bool jump : 1;
                            bool sneaking : 1;
                        };

                        uint8_t raw;
                    } flags;
                };

                struct pong {
                    //body
                    int32_t id;

                    //calculated
                    std::chrono::system_clock::duration elapsed;
                };

                struct change_recipe_book_settings {
                    int32_t book_id;
                    bool book_open;
                    bool filter_active;
                };

                struct resource_pack_response {
                    ENBT::UUID uuid;
                    int32_t result;
                };

                struct seen_advancements {
                    int32_t action;
                    std::optional<std::string> tab_id;
                };

                struct set_beacon_effect {
                    std::optional<int32_t> primary_effect;
                    std::optional<int32_t> secondary_effect;
                };

                struct program_command_block {
                    Position location;
                    std::string command;
                    int32_t mode;
                    int8_t flags;
                };

                struct program_command_cart {
                    int32_t entity_id;
                    std::string command;
                    bool track_output;
                };

                struct set_creative_slot {
                    int16_t slot;
                    base_objects::slot item;
                };

                struct program_jigsaw_block {
                    Position location;
                    std::string name;
                    std::string target;
                    std::string pool;
                    std::string final_state;
                    std::string joint_type;
                    int32_t selection_priority;
                    int32_t placement_priority;
                };

                struct program_structure_block {
                    Position location;
                    int32_t action;
                    int32_t mode;
                    std::string name;
                    int8_t offset_x;
                    int8_t offset_y;
                    int8_t offset_z;
                    int8_t size_x;
                    int8_t size_y;
                    int8_t size_z;
                    int32_t mirror;
                    int32_t rotation;
                    std::string metadata;
                    float integrity;
                    int64_t seed;
                    int8_t flags;
                };

                struct update_sign {
                    Position location;
                    bool is_front_text;
                    std::string line1;
                    std::string line2;
                    std::string line3;
                    std::string line4;
                };

                struct swing_arm {
                    int32_t hand;
                };

                struct spectator_teleport {
                    ENBT::UUID target;
                };
                struct use_item_on{
                    int32_t hand;
                    Position location;
                    int32_t face;
                    float cursor_x;
                    float cursor_y;
                    float cursor_z;
                    bool inside_block;
                    int32_t sequence;
                };

                struct use_item{
                    int32_t hand;
                    int32_t sequence;
                    float yaw = 0;
                    float pitch = 0;
                };

                struct cookie_response {
                    std::string key;
                    std::optional<list_array<uint8_t>> payload;

                    cookie_response() = default;

                    cookie_response(const cookie_response& copy)
                        : key(copy.key), payload(copy.payload) {}

                    cookie_response(cookie_response&& move)
                        : key(std::move(move.key)), payload(std::move(move.payload)) {}

                    cookie_response& operator=(const cookie_response& copy) {
                        key = copy.key;
                        payload = copy.payload;
                        return *this;
                    }

                    cookie_response& operator=(cookie_response&& move) {
                        key = std::move(move.key);
                        payload = std::move(move.payload);
                        return *this;
                    }
                };

                enum class debug_sample_subscription {
                    tick_time = 0,
                };
            }

            template <class T>
            struct event_data {
                T data;
                TCPsession& client;
                base_objects::client_data_holder client_data;
            };

            extern base_objects::event<event_data<data::teleport_request_completion>> on_teleport_request_completion;
            extern base_objects::event<event_data<data::block_nbt_request>> on_block_nbt_request;
            extern base_objects::event<event_data<uint8_t>> on_change_difficulty;
            extern base_objects::event<event_data<int32_t>> on_acknowledge_message;
            extern base_objects::event<event_data<data::chat_command>> on_chat_command;
            extern base_objects::event<event_data<data::signed_chat_command>> on_signed_chat_command;
            extern base_objects::event<event_data<data::chat_message_unsigned>> on_chat_message_unsigned;
            extern base_objects::event<event_data<data::chat_message_signed>> on_chat_message_signed;
            extern base_objects::event<event_data<data::player_session>> on_player_session;
            extern base_objects::event<event_data<float>> on_chunk_batch_received;
            extern base_objects::event<event_data<int32_t>> on_client_status;
            extern base_objects::event<event_data<data::client_information>> on_client_information;
            extern base_objects::event<event_data<data::command_suggestion>> on_command_suggestion;
            extern base_objects::event<event_data<data::click_container_button>> on_click_container_button;
            extern base_objects::event<event_data<data::click_container>> on_click_container;
            extern base_objects::event<event_data<uint8_t>> on_close_container;
            extern base_objects::event<event_data<data::change_container_slot_state>> on_change_container_slot_state;
            extern base_objects::event<event_data<data::cookie_response>> on_cookie_response;
            extern base_objects::event<event_data<data::plugin_message>> on_plugin_message;
            extern base_objects::event<event_data<data::debug_sample_subscription>> on_debug_sample_subscription;
            extern base_objects::event<event_data<data::edit_book>> on_edit_book;
            extern base_objects::event<event_data<data::query_entity_tag>> on_query_entity_tag;
            extern base_objects::event<event_data<data::interact_attack>> on_interact_attack;
            extern base_objects::event<event_data<data::interact_at>> on_interact_at;
            extern base_objects::event<event_data<data::interact>> on_interact;
            extern base_objects::event<event_data<data::jigsaw_generate>> on_jigsaw_generate;
            extern base_objects::event<event_data<int64_t>> on_keep_alive;
            extern base_objects::event<event_data<bool>> on_lock_difficulty;
            extern base_objects::event<event_data<data::set_player_position>> on_set_player_position;
            extern base_objects::event<event_data<data::set_player_position_and_rotation>> on_set_player_position_and_rotation;
            extern base_objects::event<event_data<data::set_player_rotation>> on_set_player_rotation;
            extern base_objects::event<event_data<bool>> on_set_player_on_ground;
            extern base_objects::event<event_data<data::move_vehicle>> on_move_vehicle;
            extern base_objects::event<event_data<data::paddle_boat>> on_paddle_boat;
            extern base_objects::event<event_data<data::pick_item>> on_pick_item;
            extern base_objects::event<event_data<int64_t>> on_ping_request;
            extern base_objects::event<event_data<data::place_recipe>> on_place_recipe;
            extern base_objects::event<event_data<int8_t>> on_player_abilities;
            extern base_objects::event<event_data<data::player_action>> on_player_action;
            extern base_objects::event<event_data<data::player_command>> on_player_command;
            extern base_objects::event<event_data<data::player_input>> on_player_input;
            extern base_objects::event<event_data<data::pong>> on_pong;
            extern base_objects::event<event_data<data::change_recipe_book_settings>> on_change_recipe_book_settings;
            extern base_objects::event<event_data<std::string>> on_set_seen_recipe;
            extern base_objects::event<event_data<std::string>> on_rename_item;
            extern base_objects::event<event_data<data::resource_pack_response>> on_resource_pack_response;
            extern base_objects::event<event_data<data::seen_advancements>> on_seen_advancements;
            extern base_objects::event<event_data<int32_t>> on_select_trade;
            extern base_objects::event<event_data<data::set_beacon_effect>> on_set_beacon_effect;
            extern base_objects::event<event_data<int16_t>> on_set_held_item;
            extern base_objects::event<event_data<data::program_command_block>> on_program_command_block;
            extern base_objects::event<event_data<data::program_command_cart>> on_program_command_cart;
            extern base_objects::event<event_data<data::set_creative_slot>> on_set_creative_slot;
            extern base_objects::event<event_data<data::program_jigsaw_block>> on_program_jigsaw_block;
            extern base_objects::event<event_data<data::program_structure_block>> on_program_structure_block;
            extern base_objects::event<event_data<data::update_sign>> on_update_sign;
            extern base_objects::event<event_data<data::swing_arm>> on_swing_arm;
            extern base_objects::event<event_data<data::spectator_teleport>> on_spectator_teleport;
            extern base_objects::event<event_data<data::use_item_on>> on_use_item_on;
            extern base_objects::event<event_data<data::use_item>> on_use_item;
        } // namespace protocol

    } // namespace api


} // namespace crafted_craft


#endif /* SRC_API_PROTOCOL */
