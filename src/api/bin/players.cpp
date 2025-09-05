/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <library/enbt/io.hpp>
#include <library/fast_task/include/files.hpp>
#include <library/list_array.hpp>
#include <src/api/configuration.hpp>
#include <src/api/players.hpp>
#include <src/api/selector.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>

namespace copper_server::api::network::tcp {
    class session;
}

namespace copper_server::base_objects::network {
    struct response;
}

namespace copper_server::api::players {
    namespace __internal__ {
        template <typename T>
        concept string_ = std::is_same<T, std::remove_cvref_t<std::string>>::value;
    }

    class online_player_storage {
        list_array<base_objects::client_data_holder> players;
        fast_task::task_mutex mutex;

        std::atomic_size_t online;

    public:
        void login_complete_to_cfg(base_objects::SharedClientData& player) {
            if (player.getAssignedData() != this)
                throw std::runtime_error("player not assigned to this storage");
            player.packets_state.state = base_objects::SharedClientData::packets_state_t::protocol_state::configuration;
            ++online;
        }

        size_t online_players() {
            return online;
        }

        base_objects::client_data_holder allocate_special_player(const std::function<void(base_objects::SharedClientData&, base_objects::network::response&&)>& callback) {
            std::unique_lock lock(mutex);
            players.push_back(new base_objects::SharedClientData((api::network::tcp::session*)nullptr, this, callback));
            return players.back();
        }

        base_objects::client_data_holder allocate_player(api::network::tcp::session* session) {
            std::unique_lock lock(mutex);
            players.push_back(new base_objects::SharedClientData(session, this));
            return players.back();
        }

        size_t size() const {
            return players.size();
        }

        size_t size(base_objects::SharedClientData::packets_state_t::protocol_state select_state) {
            std::unique_lock lock(mutex);
            size_t result = 0;
            for (auto& player : players)
                if (bool(player->packets_state.state & select_state))
                    ++result;
            return result;
        }

        bool has_player(const std::string& player) {
            std::unique_lock lock(mutex);
            for (auto& p : players)
                if (p->name == player)
                    return true;
            return false;
        }

        bool has_player_status(const std::string& player, base_objects::SharedClientData::packets_state_t::protocol_state select_state) {
            std::unique_lock lock(mutex);
            for (auto& p : players)
                if (bool(p->packets_state.state & select_state) && p->name == player)
                    return true;
            return false;
        }

        bool has_player_not_status(const std::string& player, base_objects::SharedClientData::packets_state_t::protocol_state select_state) {
            std::unique_lock lock(mutex);
            for (auto& p : players)
                if (!bool(p->packets_state.state & select_state) && p->name == player)
                    return true;
            return false;
        }

        void remove_player(const base_objects::client_data_holder& player) {
            if (!player)
                return;
            if (player->getAssignedData() != this)
                return;
            if (!player->canBeRemoved())
                return;

            std::unique_lock lock(mutex);
            size_t i = 0;
            for (auto& p : players) {
                if (p == player) {
                    if (!bool(p->packets_state.state & base_objects::SharedClientData::packets_state_t::protocol_state::initialization))
                        --online;
                    players.erase(i);
                    return;
                }
                i++;
            }
        }

        void remove_player(const std::string& player) {
            std::unique_lock lock(mutex);
            size_t i = 0;
            for (auto& p : players) {
                if (p->name == player) {
                    if (!bool(p->packets_state.state & base_objects::SharedClientData::packets_state_t::protocol_state::initialization) && p->canBeRemoved())
                        --online;
                    players.erase(i);
                    return;
                }
                i++;
            }
        }

        base_objects::client_data_holder get_player(base_objects::SharedClientData& player) {
            std::unique_lock lock(mutex);
            for (auto& p : players)
                if (&*p == &player)
                    return p;
            return nullptr;
        }

        base_objects::client_data_holder get_player(const std::string& player) {
            std::unique_lock lock(mutex);
            for (auto& p : players)
                if (p->name == player)
                    return p;
            return nullptr;
        }

        base_objects::client_data_holder get_player(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::string& player) {
            std::unique_lock lock(mutex);
            for (auto& p : players)
                if (bool(p->packets_state.state & select_state) && p->name == player)
                    return p;
            return nullptr;
        }

        base_objects::client_data_holder get_player_not_state(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::string& player) {
            std::unique_lock lock(mutex);
            for (auto& p : players)
                if (!bool(p->packets_state.state & select_state) && p->name == player)
                    return p;
            return nullptr;
        }

        list_array<base_objects::client_data_holder> get_players() {
            std::unique_lock lock(mutex);
            return players;
        }

        void apply_selector(base_objects::SharedClientData& caller, const std::string& selector, std::function<void(base_objects::SharedClientData&)>&& callback) {
            api::selector sel;
            sel.build_selector(selector);
            base_objects::command_context context(caller, true);
            sel.flags.only_players = true;
            sel.flags.only_entities = false;
            sel.select(context, [&callback](base_objects::entity& entity) {
                auto pl = entity.assigned_player;
                if (pl)
                    callback(*pl);
            });
        }

        void iterate_online(const std::function<bool(base_objects::SharedClientData&)>& callback) {
            iterate_players(base_objects::SharedClientData::packets_state_t::protocol_state::play, callback);
        }

        void iterate_players(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::function<bool(base_objects::SharedClientData&)>& callback) {
            std::unique_lock lock(mutex);
            for (auto& player : players)
                if (bool(player->packets_state.state & select_state))
                    if (callback(*player))
                        break;
        }

        void iterate_players_not_state(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::function<bool(base_objects::SharedClientData&)>& callback) {
            std::unique_lock lock(mutex);
            for (auto& player : players)
                if (bool(player->packets_state.state & select_state))
                    if (callback(*player))
                        break;
        }

        void iterate_players(const std::function<bool(base_objects::SharedClientData&)>& callback) {
            std::unique_lock lock(mutex);
            for (auto& player : players)
                if (callback(*player))
                    break;
        }

        template <__internal__::string_... Args>
        list_array<base_objects::client_data_holder> get_players(Args&&... args) {
            list_array<base_objects::client_data_holder> cache;
            cache.reserve(sizeof...(Args));
            list_array __players = {std::forward<Args>(args)...};


            std::unique_lock lock(mutex);
            for (auto& player : players)
                if (__players.contains(player->name))
                    cache.push_back(player);
            lock.unlock();


            list_array<base_objects::client_data_holder> result;
            result.resize(sizeof...(Args));

            //place result in same place as arguments
            for (size_t i = 0; i < sizeof...(Args); i++) {
                for (size_t j = 0; j < cache.size(); j++) {
                    if (cache[j]->name == __players[i]) {
                        result[i] = cache[j];
                        break;
                    }
                }
            }
            return result;
        }

        template <__internal__::string_... Args>
        list_array<base_objects::client_data_holder> get_players_state(base_objects::SharedClientData::packets_state_t::protocol_state select_state, Args&&... args) {
            list_array<base_objects::client_data_holder> cache;
            cache.reserve(sizeof...(Args));
            list_array __players = {std::forward<Args>(args)...};


            std::unique_lock lock(mutex);
            for (auto& player : players) {
                if (bool(player->packets_state.state & select_state))
                    if (__players.contains(player->name))
                        cache.push_back(player);
            }
            lock.unlock();


            list_array<base_objects::client_data_holder> result;
            result.resize(sizeof...(Args));

            //place result in same place as arguments
            for (size_t i = 0; i < sizeof...(Args); i++) {
                for (size_t j = 0; j < cache.size(); j++) {
                    if (cache[j]->name == __players[i]) {
                        result[i] = cache[j];
                        break;
                    }
                }
            }
            return result;
        }
    };

    namespace calls {
        base_objects::events::event<personal<Chat>> on_player_kick;
        base_objects::events::event<personal<Chat>> on_player_ban;
    }

    namespace handlers {
        base_objects::events::event<base_objects::client_data_holder> on_disconnect;
    }

    auto& get_storage() {
        static online_player_storage ops;
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

    void save_player(base_objects::player&& player, enbt::raw_uuid uuid) {
        auto path = api::configuration::get().server.base_path / "players" / (uuid.to_string() + ".enbt");
        enbt::compound as_file_data;
        {
            enbt::compound abilities;
            {
                enbt::compound flags;
                flags["invulnerable"] = player.abilities.flags.invulnerable;
                flags["flying"] = player.abilities.flags.flying;
                flags["allow_flying"] = player.abilities.flags.allow_flying;
                flags["creative_mode"] = player.abilities.flags.creative_mode;
                flags["flying_speed"] = player.abilities.flags.flying_speed;
                flags["walking_speed"] = player.abilities.flags.walking_speed;
                abilities["flags"] = flags;
            }
            abilities["flying_speed"] = player.abilities.flying_speed;
            abilities["field_of_view_modifier"] = player.abilities.field_of_view_modifier;
            as_file_data["abilities"] = abilities;
        }
        {
            enbt::compound flags;
            flags["hardcore_hearts"] = player.hardcore_hearts;
            flags["reduced_debug_info"] = player.reduced_debug_info;
            flags["show_death_screen"] = player.show_death_screen;
            as_file_data["flags"] = flags;
        }
        {
            enbt::compound gamemode;
            gamemode["op_level"] = player.op_level;
            gamemode["gamemode"] = player.gamemode;
            gamemode["prev_gamemode"] = player.prev_gamemode;
            as_file_data["gamemode"] = gamemode;
        }
        {
            enbt::fixed_array permissions(player.permission_groups.size());
            for (size_t i = 0; i < player.permission_groups.size(); i++)
                permissions.set(i, player.permission_groups[i]);
            as_file_data["permission_groups"] = permissions;
        }
        if (player.last_death_location.has_value()) {
            enbt::compound death_location;
            death_location["x"] = player.last_death_location->x;
            death_location["y"] = player.last_death_location->y;
            death_location["z"] = player.last_death_location->z;
            death_location["world_id"] = player.last_death_location->world_id;
            as_file_data["death_location"] = death_location;
        }

        as_file_data["local_data"] = player.local_data;
        if (player.assigned_entity)
            as_file_data["assigned_entity"] = player.assigned_entity->copy_to_enbt();

        fast_task::files::async_iofstream file(
            path,
            fast_task::files::open_mode::write,
            fast_task::files::on_open_action::always_new,
            fast_task::files::_sync_flags{}
        );
        if (!file.is_open())
            throw std::runtime_error("Failed to open file: " + path.string());
        enbt::io_helper::write_token(file, as_file_data);
        file.flush();
    }

    base_objects::player load_player(enbt::raw_uuid uuid) {
        auto path = api::configuration::get().server.base_path / "players" / (uuid.to_string() + ".enbt");
        base_objects::player player;

        fast_task::files::async_iofstream file(
            path,
            fast_task::files::open_mode::read,
            fast_task::files::on_open_action::open_exists,
            fast_task::files::_sync_flags{}
        );
        if (!file.is_open()) {
            if (std::filesystem::exists(path))
                throw std::runtime_error("Failed to open file: " + path.string());
            else {
                player.assigned_entity = base_objects::entity::create("minecraft:player");
                save_player(std::move(player), uuid);
            }
            return load_player(uuid);
        }
        enbt::io_helper::value_read_stream stream(file);
        stream.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& stream) {
            if (name == "abilities") {
                stream.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& stream) {
                    if (name == "flags") {
                        stream.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& stream) {
                            if (name == "invulnerable")
                                player.abilities.flags.invulnerable = stream.read();
                            else if (name == "flying")
                                player.abilities.flags.flying = stream.read();
                            else if (name == "allow_flying")
                                player.abilities.flags.allow_flying = stream.read();
                            else if (name == "creative_mode")
                                player.abilities.flags.creative_mode = stream.read();
                            else if (name == "flying_speed")
                                player.abilities.flags.flying_speed = stream.read();
                            else if (name == "walking_speed")
                                player.abilities.flags.walking_speed = stream.read();
                        });
                    } else if (name == "flying_speed")
                        player.abilities.flying_speed = stream.read();
                    else if (name == "field_of_view_modifier")
                        player.abilities.field_of_view_modifier = stream.read();
                });
            } else if (name == "flags") {
                stream.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& stream) {
                    if (name == "hardcore_hearts")
                        player.hardcore_hearts = stream.read();
                    else if (name == "reduced_debug_info")
                        player.reduced_debug_info = stream.read();
                    else if (name == "show_death_screen")
                        player.show_death_screen = stream.read();
                });
            } else if (name == "gamemode") {
                stream.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& stream) {
                    if (name == "op_level")
                        player.op_level = stream.read();
                    else if (name == "gamemode")
                        player.gamemode = stream.read();
                    else if (name == "prev_gamemode")
                        player.prev_gamemode = stream.read();
                });
            } else if (name == "permission_groups") {
                stream.iterate([&](enbt::io_helper::value_read_stream& stream) {
                    player.permission_groups.push_back(stream.read());
                });
                player.permission_groups.commit();
            } else if (name == "death_location") {
                stream.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& stream) {
                    if (name == "x")
                        player.last_death_location->x = stream.read();
                    else if (name == "y")
                        player.last_death_location->y = stream.read();
                    else if (name == "z")
                        player.last_death_location->z = stream.read();
                    else if (name == "world_id")
                        player.last_death_location->world_id = stream.read().as_string();
                });
            } else if (name == "local_data")
                player.local_data = stream.read();
            else if (name == "assigned_entity") {
                auto entity = stream.read();
                auto entity_comp = entity.as_compound();
                entity_comp.erase("bound_world");
                player.assigned_entity = base_objects::entity::load_from_enbt(entity_comp);
            }
        });

        if (!player.assigned_entity)
            player.assigned_entity = base_objects::entity::create("minecraft:player");
        return player;
    }
}
