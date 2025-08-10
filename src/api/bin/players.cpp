/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/players.hpp>
#include <src/storage/memory/online_player.hpp>

namespace copper_server::api::players {
    namespace calls {
        base_objects::events::event<personal<Chat>> on_player_kick;
        base_objects::events::event<personal<Chat>> on_player_ban;
    }

    namespace handlers {
        base_objects::events::event<base_objects::client_data_holder> on_disconnect;
    }
    
    auto& get_storage() {
        static storage::memory::online_player_storage ops;
        return ops;
    }

    void login_complete_to_cfg(base_objects::SharedClientData& player) {
        get_storage().login_complete_to_cfg(player);
    }

    size_t online_players() {
        return get_storage().online_players();
    }

    base_objects::client_data_holder allocate_special_player(const std::function<void(base_objects::SharedClientData&, base_objects::network::response&&)>& callback) {
        return get_storage().allocate_special_player(callback);
    }

    base_objects::client_data_holder allocate_player(api::network::tcp::session* session) {
        return get_storage().allocate_player(session);
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

    base_objects::client_data_holder get_player(base_objects::SharedClientData& player) {
        return get_storage().get_player(player);
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

    void apply_selector(base_objects::SharedClientData& caller, const std::string& selector, std::function<void(base_objects::SharedClientData&)>&& callback) {
        get_storage().apply_selector(caller, selector, std::move(callback));
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
