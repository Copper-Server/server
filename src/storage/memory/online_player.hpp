#ifndef SRC_STORAGE_MEMORY_ONLINE_PLAYER
#define SRC_STORAGE_MEMORY_ONLINE_PLAYER
#include <shared_mutex>


#include "../../base_objects/shared_client_data.hpp"
#include "../../library/fast_task.hpp"
#include "../../library/list_array.hpp"

namespace crafted_craft {
    namespace storage {
        namespace memory {
            namespace __internal__ {
                template <typename T>
                concept string_ = std::is_same<T, std::remove_cvref_t<std::string>>::value;
            }

            class online_player_storage {
                list_array<base_objects::client_data_holder> players;
                fast_task::task_mutex mutex;

                std::atomic_size_t online;

            public:
                void login_complete_to_cfg(base_objects::client_data_holder& player) {
                    if (player->getAssignedData() != this)
                        throw std::runtime_error("player not assigned to this storage");
                    player->packets_state.state = base_objects::SharedClientData::packets_state_t::protocol_state::configuration;
                    ++online;
                }

                size_t online_players() {
                    return online;
                }

                base_objects::client_data_holder allocate_special_player(const std::function<void(SharedClientData&)>& callback) {
                    std::unique_lock lock(mutex);
                    players.push_back(new base_objects::SharedClientData(this, callback));
                    return players.back();
                }

                base_objects::client_data_holder allocate_player() {
                    std::unique_lock lock(mutex);
                    players.push_back(new base_objects::SharedClientData(this));
                    return players.back();
                }

                size_t size() const {
                    return players.size();
                }

                size_t size(base_objects::SharedClientData::packets_state_t::protocol_state select_state) {
                    std::unique_lock lock(mutex);
                    size_t result = 0;
                    for (auto& player : players)
                        if (player->packets_state.state == select_state)
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
                        if (p->name == player && p->packets_state.state == select_state)
                            return true;
                    return false;
                }

                bool has_player_not_status(const std::string& player, base_objects::SharedClientData::packets_state_t::protocol_state select_state) {
                    std::unique_lock lock(mutex);
                    for (auto& p : players)
                        if (p->name == player && p->packets_state.state != select_state)
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
                            if (p->packets_state.state != base_objects::SharedClientData::packets_state_t::protocol_state::initialization)
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
                            if (p->packets_state.state != base_objects::SharedClientData::packets_state_t::protocol_state::initialization && p->canBeRemoved())
                                --online;
                            players.erase(i);
                            return;
                        }
                        i++;
                    }
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
                        if (p->name == player && p->packets_state.state == select_state)
                            return p;
                    return nullptr;
                }

                base_objects::client_data_holder get_player_not_state(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::string& player) {
                    std::unique_lock lock(mutex);
                    for (auto& p : players)
                        if (p->name == player && p->packets_state.state != select_state)
                            return p;
                    return nullptr;
                }

                list_array<base_objects::client_data_holder> get_players() {
                    std::unique_lock lock(mutex);
                    return players;
                }

                void apply_selector(base_objects::client_data_holder& caller, const std::string& selector, const std::function<void(base_objects::SharedClientData&)>& callback, const std::function<void()>& not_found) {
                    std::unique_lock lock(mutex);
                }

                void iterate_online(const std::function<bool(base_objects::SharedClientData&)>& callback) {
                    iterate_players(base_objects::SharedClientData::packets_state_t::protocol_state::play, callback);
                }
                void iterate_players(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::function<bool(base_objects::SharedClientData&)>& callback) {
                    std::unique_lock lock(mutex);
                    for (auto& player : players)
                        if (player->packets_state.state == select_state)
                            if (callback(*player))
                                break;
                }

                void iterate_players_not_state(base_objects::SharedClientData::packets_state_t::protocol_state select_state, const std::function<bool(base_objects::SharedClientData&)>& callback) {
                    std::unique_lock lock(mutex);
                    for (auto& player : players)
                        if (player->packets_state.state != select_state)
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
                        if (player->packets_state.state == select_state)
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
        }
    }
}


#endif /* SRC_STORAGE_MEMORY_ONLINE_PLAYER */
