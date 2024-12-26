#ifndef SRC_API_PLAYERS
#define SRC_API_PLAYERS
#include <array>
#include <library/list_array.hpp>
#include <optional>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/events/event.hpp>
#include <src/base_objects/events/sync_event.hpp>
#include <src/base_objects/position.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <string>

namespace copper_server::api::players {
    namespace handlers {
        extern base_objects::events::event<base_objects::client_data_holder> on_player_join;
        extern base_objects::events::event<base_objects::client_data_holder> on_player_leave;
    }

    template <class T>
    struct personal {
        base_objects::client_data_holder player;
        T data;
    };

    struct titles_times {
        //all in ticks
        int32_t fade_in;
        int32_t stay;
        int32_t fade_out;
    };

    struct unsigned_chat {
        Chat message;
        int32_t chat_type_id; //get id from registry
        Chat sender_name;
        std::optional<Chat> receiver_name;
    };

    struct player_chat {
        base_objects::client_data_holder sender;
        std::string message;

        std::optional<std::array<uint8_t, 256>> signature;
        uint64_t salt;
        list_array<std::array<uint8_t, 256>> previous_messages;
        int32_t chat_type_id; //get id from registry

        Chat sender_decorated_name;
    };

    struct player_personal_chat : public player_chat {
        base_objects::client_data_holder receiver;
        int32_t receiver_chat_type_id; //get id from registry
        Chat receiver_decorated_name;
    };

    namespace calls {
        struct teleport_request {
            base_objects::client_data_holder player;
            base_objects::position position;
        };

        extern base_objects::events::event<teleport_request> on_teleport_request;


        extern base_objects::events::event<player_chat> on_player_chat;
        extern base_objects::events::event<player_personal_chat> on_player_personal_chat;

        extern base_objects::events::event<Chat> on_system_message_broadcast;
        extern base_objects::events::event<personal<Chat>> on_system_message;

        extern base_objects::events::event<Chat> on_system_message_overlay_broadcast;
        extern base_objects::events::event<personal<Chat>> on_system_message_overlay;

        extern base_objects::events::event<personal<Chat>> on_player_kick;
        extern base_objects::events::event<personal<Chat>> on_player_ban;

        extern base_objects::events::event<Chat> on_action_bar_message_broadcast;
        extern base_objects::events::event<personal<Chat>> on_action_bar_message;

        extern base_objects::events::event<Chat> on_title_message_broadcast;
        extern base_objects::events::event<personal<Chat>> on_title_message;

        //pass true to reset
        extern base_objects::events::event<bool> on_title_clear_broadcast;
        //pass true to reset
        extern base_objects::events::event<personal<bool>> on_title_clear;

        extern base_objects::events::event<Chat> on_subtitle_message_broadcast;
        extern base_objects::events::event<personal<Chat>> on_subtitle_message;

        extern base_objects::events::event<titles_times> on_title_times_broadcast;
        extern base_objects::events::event<personal<titles_times>> on_title_times;

        extern base_objects::events::event<unsigned_chat> on_unsigned_message_broadcast;
        extern base_objects::events::event<personal<unsigned_chat>> on_unsigned_message;
    }

    void login_complete_to_cfg(base_objects::client_data_holder& player);
    size_t online_players();
    base_objects::client_data_holder allocate_special_player(const std::function<void(base_objects::SharedClientData&)>& callback);
    base_objects::client_data_holder allocate_player();
    size_t size();
    size_t size(base_objects::SharedClientData::packets_state_t::protocol_state select_state);
    bool has_player(const std::string& player);
    bool has_player_status(const std::string& player, base_objects::SharedClientData::packets_state_t::protocol_state select_state);
    bool has_player_not_status(const std::string& player, base_objects::SharedClientData::packets_state_t::protocol_state select_state);
    void remove_player(const base_objects::client_data_holder& player);
    void remove_player(const std::string& player);
    base_objects::client_data_holder get_player(const std::string& player);
    base_objects::client_data_holder get_player(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::string& player);
    base_objects::client_data_holder get_player_not_state(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::string& player);
    list_array<base_objects::client_data_holder> get_players();
    void apply_selector(base_objects::client_data_holder& caller, const std::string& selector, const std::function<void(base_objects::SharedClientData&)>& callback, const std::function<void()>& not_found);
    void iterate_online(const std::function<bool(base_objects::SharedClientData&)>& callback);
    void iterate_players(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::function<bool(base_objects::SharedClientData&)>& callback);
    void iterate_players_not_state(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::function<bool(base_objects::SharedClientData&)>& callback);
    void iterate_players(const std::function<bool(base_objects::SharedClientData&)>& callback);
}

#endif /* SRC_API_PLAYERS */
