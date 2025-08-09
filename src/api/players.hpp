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

namespace copper_server::api::network::tcp {
    class session;
}

namespace copper_server::base_objects::network {
    struct response;
}

namespace copper_server::api::players {
    template <class T>
    struct personal {
        base_objects::client_data_holder player;
        T data;
    };

    namespace calls {
        extern base_objects::events::event<personal<Chat>> on_player_kick;
        extern base_objects::events::event<personal<Chat>> on_player_ban;
    }

    namespace handlers {
        extern base_objects::events::event<base_objects::client_data_holder> on_disconnect;
    }

    void login_complete_to_cfg(base_objects::SharedClientData& player);
    size_t online_players();
    base_objects::client_data_holder allocate_special_player(const std::function<void(base_objects::SharedClientData&, base_objects::network::response&&)>& callback);
    base_objects::client_data_holder allocate_player(api::network::tcp::session* session = nullptr);
    size_t size();
    size_t size(base_objects::SharedClientData::packets_state_t::protocol_state select_state);
    bool has_player(const std::string& player);
    bool has_player_status(const std::string& player, base_objects::SharedClientData::packets_state_t::protocol_state select_state);
    bool has_player_not_status(const std::string& player, base_objects::SharedClientData::packets_state_t::protocol_state select_state);
    void remove_player(const base_objects::client_data_holder& player);
    void remove_player(const std::string& player);
    base_objects::client_data_holder get_player(base_objects::SharedClientData& player);
    base_objects::client_data_holder get_player(const std::string& player);
    base_objects::client_data_holder get_player(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::string& player);
    base_objects::client_data_holder get_player_not_state(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::string& player);
    list_array<base_objects::client_data_holder> get_players();
    void apply_selector(base_objects::SharedClientData& caller, const std::string& selector, std::function<void(base_objects::SharedClientData&)>&& callback);
    void iterate_online(const std::function<bool(base_objects::SharedClientData&)>& callback);
    void iterate_players(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::function<bool(base_objects::SharedClientData&)>& callback);
    void iterate_players_not_state(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::function<bool(base_objects::SharedClientData&)>& callback);
    void iterate_players(const std::function<bool(base_objects::SharedClientData&)>& callback);
}

#endif /* SRC_API_PLAYERS */
