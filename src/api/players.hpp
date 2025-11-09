/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
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
#include <src/base_objects/uuid.hpp>
#include <string>

namespace copper_server::api::network::tcp {
    class session;
}

namespace copper_server::base_objects::network {
    struct response;
}

namespace copper_server::base_objects {
    class player;
}

namespace copper_server::api::players {
    template <class T>
    struct personal {
        base_objects::client_data_holder player;
        T data;
    };

    namespace calls {
        extern base_objects::events::event<personal<base_objects::chat>> on_player_kick;
        extern base_objects::events::event<personal<base_objects::chat>> on_player_ban;
    }

    namespace handlers {
        extern base_objects::events::event<base_objects::client_data_holder> on_disconnect;
        extern base_objects::events::sync_event_no_cancel<base_objects::shared_client_data&> on_tab_listing_changed;
        extern base_objects::events::sync_event_no_cancel<base_objects::shared_client_data&> on_skin_parts_changed;
        extern base_objects::events::sync_event_no_cancel<base_objects::shared_client_data&> on_gamemode_changed;
    }

    void login_complete_to_cfg(base_objects::shared_client_data& player);
    size_t online_players();
    void set_gamemode(uint8_t gamemode, base_objects::shared_client_data&);
    void set_tab_listing(bool enable, base_objects::shared_client_data&);
    base_objects::client_data_holder allocate_special_player(const std::function<void(base_objects::shared_client_data&, base_objects::network::response&&)>& callback);
    base_objects::client_data_holder allocate_player(api::network::tcp::session* session = nullptr);
    size_t size();
    size_t size(base_objects::shared_client_data::packets_state_t::protocol_state select_state);
    bool has_player(const std::string& player);
    bool has_player_status(const std::string& player, base_objects::shared_client_data::packets_state_t::protocol_state select_state);
    bool has_player_not_status(const std::string& player, base_objects::shared_client_data::packets_state_t::protocol_state select_state);
    void remove_player(const base_objects::client_data_holder& player);
    void remove_player(const std::string& player);
    base_objects::client_data_holder get_player(base_objects::shared_client_data& player);
    base_objects::client_data_holder get_player(const std::string& player);
    base_objects::client_data_holder get_player(base_objects::shared_client_data::packets_state_t::protocol_state select_state, const std::string& player);
    base_objects::client_data_holder get_player_not_state(base_objects::shared_client_data::packets_state_t::protocol_state select_state, const std::string& player);
    list_array<base_objects::client_data_holder> get_players();
    void apply_selector(base_objects::shared_client_data& caller, const std::string& selector, std::function<void(base_objects::shared_client_data&)>&& callback);
    void iterate_online(const std::function<bool(base_objects::shared_client_data&)>& callback);
    void iterate_players(base_objects::shared_client_data::packets_state_t::protocol_state select_state, const std::function<bool(base_objects::shared_client_data&)>& callback);
    void iterate_players_not_state(base_objects::shared_client_data::packets_state_t::protocol_state select_state, const std::function<bool(base_objects::shared_client_data&)>& callback);
    void iterate_players(const std::function<bool(base_objects::shared_client_data&)>& callback);


    void save_player(base_objects::player&& sav, base_objects::uuid uuid);
    base_objects::player load_player(base_objects::uuid uuid);
}

#endif /* SRC_API_PLAYERS */

