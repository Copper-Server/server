#include <src/api/players.hpp>
#include <src/storage/memory/online_player.hpp>

namespace copper_server::api::players {
    namespace handlers {
        base_objects::events::event<base_objects::client_data_holder> on_player_join;
        base_objects::events::event<base_objects::client_data_holder> on_player_leave;
    }

    namespace calls {
        base_objects::events::event<teleport_request> on_teleport_request;


        base_objects::events::event<player_chat> on_player_chat;
        base_objects::events::event<player_personal_chat> on_player_personal_chat;

        base_objects::events::event<Chat> on_system_message_broadcast;
        base_objects::events::event<personal<Chat>> on_system_message;

        base_objects::events::event<Chat> on_system_message_overlay_broadcast;
        base_objects::events::event<personal<Chat>> on_system_message_overlay;

        base_objects::events::event<personal<Chat>> on_player_kick;
        base_objects::events::event<personal<Chat>> on_player_ban;

        base_objects::events::event<Chat> on_action_bar_message_broadcast;
        base_objects::events::event<personal<Chat>> on_action_bar_message;

        base_objects::events::event<Chat> on_title_message_broadcast;
        base_objects::events::event<personal<Chat>> on_title_message;

        base_objects::events::event<bool> on_title_clear_broadcast;
        base_objects::events::event<personal<bool>> on_title_clear;

        base_objects::events::event<Chat> on_subtitle_message_broadcast;
        base_objects::events::event<personal<Chat>> on_subtitle_message;

        base_objects::events::event<titles_times> on_title_times_broadcast;
        base_objects::events::event<personal<titles_times>> on_title_times;

        base_objects::events::event<unsigned_chat> on_unsigned_message_broadcast;
        base_objects::events::event<personal<unsigned_chat>> on_unsigned_message;
    }

    auto& get_storage() {
        static storage::memory::online_player_storage ops;
        return ops;
    }

    void login_complete_to_cfg(base_objects::client_data_holder& player) {
        get_storage().login_complete_to_cfg(player);
    }

    size_t online_players() {
        return get_storage().online_players();
    }

    base_objects::client_data_holder allocate_special_player(const std::function<void(base_objects::SharedClientData&)>& callback) {
        return get_storage().allocate_special_player(callback);
    }

    base_objects::client_data_holder allocate_player() {
        return get_storage().allocate_player();
    }

    size_t size() {
        return get_storage().size();
    }

    size_t size(base_objects::SharedClientData::packets_state_t::protocol_state select_state) {
        return get_storage().size(select_state);
    }

    bool has_player(const std::string& player) {
        return get_storage().has_player(player);
    }

    bool has_player_status(const std::string& player, base_objects::SharedClientData::packets_state_t::protocol_state select_state) {
        return get_storage().has_player_status(player, select_state);
    }

    bool has_player_not_status(const std::string& player, base_objects::SharedClientData::packets_state_t::protocol_state select_state) {
        return get_storage().has_player_not_status(player, select_state);
    }

    void remove_player(const base_objects::client_data_holder& player) {
        get_storage().remove_player(player);
    }

    void remove_player(const std::string& player) {
        get_storage().remove_player(player);
    }

    base_objects::client_data_holder get_player(const std::string& player) {
        return get_storage().get_player(player);
    }

    base_objects::client_data_holder get_player(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::string& player) {
        return get_storage().get_player(select_state, player);
    }

    base_objects::client_data_holder get_player_not_state(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::string& player) {
        return get_storage().get_player_not_state(select_state, player);
    }

    list_array<base_objects::client_data_holder> get_players() {
        return get_storage().get_players();
    }

    void apply_selector(base_objects::client_data_holder& caller, const std::string& selector, const std::function<void(base_objects::SharedClientData&)>& callback, const std::function<void()>& not_found) {
        get_storage().apply_selector(caller, selector, callback, not_found);
    }

    void iterate_online(const std::function<bool(base_objects::SharedClientData&)>& callback) {
        get_storage().iterate_online(callback);
    }

    void iterate_players(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::function<bool(base_objects::SharedClientData&)>& callback) {
        get_storage().iterate_players(select_state, callback);
    }

    void iterate_players_not_state(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::function<bool(base_objects::SharedClientData&)>& callback) {
        get_storage().iterate_players_not_state(select_state, callback);
    }

    void iterate_players(const std::function<bool(base_objects::SharedClientData&)>& callback) {
        get_storage().iterate_players(callback);
    }
}
