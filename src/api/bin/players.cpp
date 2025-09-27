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
        auto path = api::configuration::get().server.get_storage_path() / "players";
        std::filesystem::create_directories(path);
        fast_task::files::atomic_async_ofstream file(path / (uuid.to_string() + ".enbt"));
        if (!file.is_open())
            throw std::runtime_error("Failed to open file: " + (path / (uuid.to_string() + ".enbt")).string());
        enbt::io_helper::value_write_stream write(file);
        auto compound = write.write_compound();
        compound
            .write("abilities", [&player](auto& writer) {
                writer
                    .write_compound()
                    .write("flags", [&player](auto& flags) {
                        flags
                            .write_compound()
                            .write("invulnerable", player.abilities.flags.invulnerable)
                            .write("flying", player.abilities.flags.flying)
                            .write("allow_flying", player.abilities.flags.allow_flying)
                            .write("creative_mode", player.abilities.flags.creative_mode)
                            .write("flying_speed", player.abilities.flags.flying_speed)
                            .write("walking_speed", player.abilities.flags.walking_speed);
                    })
                    .write("flying_speed", player.abilities.flying_speed)
                    .write("field_of_view_modifier", player.abilities.field_of_view_modifier);
            })
            .write("flags", [&player](auto& writer) {
                writer
                    .write_compound()
                    .write("hardcore_hearts", player.hardcore_hearts)
                    .write("reduced_debug_info", player.reduced_debug_info)
                    .write("show_death_screen", player.show_death_screen);
            })
            .write("gamemode", [&player](auto& writer) {
                writer
                    .write_compound()
                    .write("op_level", player.op_level)
                    .write("gamemode", player.gamemode)
                    .write("prev_gamemode", player.prev_gamemode);
            })
            .write("permission_groups", [&player](auto& writer) {
                writer.write_array(player.permission_groups.size()).iterable(player.permission_groups);
            })
            .write("local_data", [&player](auto& writer) {
                writer.write(player.local_data);
            });

        if (player.last_death_location.has_value()) {
            compound.write("death_location", [&player](auto& writer) {
                writer
                    .write_compound()
                    .write("x", player.last_death_location->x)
                    .write("y", player.last_death_location->y)
                    .write("z", player.last_death_location->z)
                    .write("world_id", player.last_death_location->world_id);
            });
        }
        if (player.assigned_entity)
            compound.write("assigned_entity", [&player](auto& stream) {
                base_objects::entity::store_to_file(player.assigned_entity, stream);
            });
        file.flush();
    }

    base_objects::player load_player(enbt::raw_uuid uuid) {
        auto path = api::configuration::get().server.get_storage_path() / "players";
        auto file_path = path / (uuid.to_string() + ".enbt");
        std::filesystem::create_directories(path);
        base_objects::player player;

        fast_task::files::async_iofstream file(
            file_path,
            fast_task::files::open_mode::read,
            fast_task::files::on_open_action::open_exists,
            fast_task::files::_sync_flags{}
        );
        if (!file.is_open()) {
            if (std::filesystem::exists(file_path))
                throw std::runtime_error("Failed to open file: " + file_path.string());
            else {
                player.assigned_entity = base_objects::entity::create("minecraft:player");
                save_player(std::move(player), uuid);
            }
            return load_player(uuid);
        }
        enbt::io_helper::value_read_stream(file)
            .read_compound()
            .collect("abilities", [&](enbt::io_helper::value_read_stream& stream) {
                stream
                    .read_compound()
                    .collect("flags", [&](enbt::io_helper::value_read_stream& stream) {
                        stream
                            .read_compound()
                            .collect("creative_mode", [&](enbt::io_helper::value_read_stream& stream) { player.abilities.flags.creative_mode = stream.read(); })
                            .collect("walking_speed", [&](enbt::io_helper::value_read_stream& stream) { player.abilities.flags.walking_speed = stream.read(); })
                            .collect("flying_speed", [&](enbt::io_helper::value_read_stream& stream) { player.abilities.flags.flying_speed = stream.read(); })
                            .collect("invulnerable", [&](enbt::io_helper::value_read_stream& stream) { player.abilities.flags.invulnerable = stream.read(); })
                            .collect("allow_flying", [&](enbt::io_helper::value_read_stream& stream) { player.abilities.flags.allow_flying = stream.read(); })
                            .collect("flying", [&](enbt::io_helper::value_read_stream& stream) { player.abilities.flags.flying = stream.read(); })
                            .force_all_collect();
                    })
                    .collect_as("flying_speed", player.abilities.flying_speed)
                    .collect_as("field_of_view_modifier", player.abilities.field_of_view_modifier)
                    .make_collect();
            })
            .collect("flags", [&](enbt::io_helper::value_read_stream& stream) {
                stream
                    .read_compound()
                    .collect("reduced_debug_info", [&](enbt::io_helper::value_read_stream& stream) { player.reduced_debug_info = stream.read(); })
                    .collect("show_death_screen", [&](enbt::io_helper::value_read_stream& stream) { player.show_death_screen = stream.read(); })
                    .collect("hardcore_hearts", [&](enbt::io_helper::value_read_stream& stream) { player.hardcore_hearts = stream.read(); })
                    .force_all_collect();
            })
            .collect("gamemode", [&](enbt::io_helper::value_read_stream& stream) {
                stream
                    .read_compound()
                    .collect_as("prev_gamemode", player.prev_gamemode)
                    .collect_as("gamemode", player.gamemode)
                    .collect_as("op_level", player.op_level)
                    .force_all_collect();
            })
            .collect_iterate( //
                "permission_groups",
                [&player](auto size) { player.permission_groups.reserve(size); },
                [&player](enbt::io_helper::value_read_stream& stream) { player.permission_groups.push_back(stream.read()); }
            )
            .collect("death_location", [&player](enbt::io_helper::value_read_stream& stream) {
                player.last_death_location.emplace();
                stream
                    .read_compound()
                    .collect_as("x", player.last_death_location->x)
                    .collect_as("y", player.last_death_location->y)
                    .collect_as("z", player.last_death_location->z)
                    .collect_as("world_id", player.last_death_location->world_id)
                    .force_all_collect();
            })
            .collect("assigned_entity", [&player](enbt::io_helper::value_read_stream& stream) {
                player.assigned_entity = base_objects::entity::load_from_file(stream);
            })
            .collect("local_data", [&player](enbt::io_helper::value_read_stream& stream) {
                player.local_data = stream.read();
            })
            .make_collect();
        if (!player.assigned_entity)
            player.assigned_entity = base_objects::entity::create("minecraft:player");
        return player;
    }
}
