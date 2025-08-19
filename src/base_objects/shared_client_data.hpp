/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_SHARED_CLIENT_DATA
#define SRC_BASE_OBJECTS_SHARED_CLIENT_DATA
#include <chrono>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <library/enbt/enbt.hpp>
#include <library/fast_task.hpp>
#include <library/list_array.hpp>
#include <src/base_objects/atomic_holder.hpp>
#include <src/base_objects/ptr_optional.hpp>
#include <src/mojang/api/session_server.hpp>
#include <src/plugin/registration.hpp>

namespace copper_server {
    class PluginRegistration;
    using PluginRegistrationPtr = std::shared_ptr<PluginRegistration>;

    namespace api::network::tcp {
        class session;
    }

    namespace base_objects::network {
        struct response;
    }

    namespace base_objects {
        namespace network::tcp {
            class client;
        }
        class player;

        struct SharedClientData {
            std::string name;
            std::string ip;
            std::shared_ptr<mojang::api::session_server::player_data> data;
            std::string client_brand;


            std::string locale; //max 16 chars
            list_array<PluginRegistrationPtr> compatible_plugins;
            uint8_t view_distance = 0;
            uint8_t simulation_distance = 0;
            enum class ChatMode : uint8_t {
                ENABLED = 0,
                COMMANDS_ONLY = 1,
                HIDDEN = 2
            } chat_mode
                = ChatMode::ENABLED;

            union {
                struct {
                    bool cape_enabled : 1;
                    bool jacket_enabled : 1;
                    bool left_sleeve_enabled : 1;
                    bool right_sleeve_enabled : 1;
                    bool left_pants_leg_enabled : 1;
                    bool right_pants_leg_enabled : 1;
                    bool hat_enabled : 1;
                    bool _unused : 1;
                } data;

                uint8_t mask = UINT8_MAX - 1;
            } skin_parts;

            enum class MainHand : uint8_t {
                LEFT = 0,
                RIGHT = 1
            } main_hand
                = MainHand::RIGHT;

            bool enable_filtering : 1 = false;
            bool allow_server_listings : 1 = false;
            bool enable_chat_colors : 1 = true;
            bool is_virtual : 1 = false;
            enum class ParticleStatus : uint8_t {
                ALL = 0,
                DECREASED = 1,
                MINIMAL = 2
            } particle_status : 2
                = ParticleStatus::ALL;
            player& player_data;

            //here all fields should not be modified by plugins, except plugins implementing protocol(read allowed for all)
            struct packets_state_t {
                struct unordered_track {
                    std::unordered_set<int32_t> valid_ids;
                    std::optional<int32_t> latest;
                };

                struct internal_data_t {
                    std::unordered_map<std::string, int32_t> id_tracker;
                    std::unordered_map<std::string, unordered_track> unordered_id_tracker;
                    std::shared_ptr<void> extra_data; //here stored custom handler data, cleared after switching to other state
                };

                fast_task::protected_value<internal_data_t> internal_data;

                std::unordered_set<enbt::raw_uuid> active_resource_packs;
                std::chrono::time_point<std::chrono::system_clock> pong_timer;
                std::atomic_int32_t keep_alive_ping_ms = 0;
                std::atomic_int32_t local_chat_counter = 0;
                int32_t protocol_version = -1;
                bool is_transferred = false;
                bool is_fully_initialized = false;
                bool is_play_fully_initialized = false;
                bool is_play_initialized = false;


                enum class protocol_state : uint8_t {
                    handshake = 0x1,
                    status = 0x2,
                    login = 0x4,
                    initialization = 0x7, //handshake or status or login
                    configuration = 0x8,
                    play = 0x10
                } state : 5
                    = protocol_state::handshake; //the valid values for this field is handshake, status, login, configuration and play
            } packets_state;

            std::chrono::milliseconds ping = std::chrono::milliseconds(0);

            void registerPlugin(PluginRegistrationPtr plugin) {
                compatible_plugins.push_back(plugin);
            }

            void unregisterPlugin(PluginRegistrationPtr plugin) {
                compatible_plugins.remove(plugin);
            }

            bool isCompatiblePlugin(PluginRegistrationPtr plugin) {
                return compatible_plugins.contains(plugin);
            }

            void sendPacket(base_objects::network::response&& packet) {
                if (special_callback)
                    special_callback(*this, std::move(packet));
                else if (ss)
                    send_indirect(std::move(packet));
                sent = true;
            }

            SharedClientData(api::network::tcp::session* ss = nullptr, void* assigned_data = nullptr, std::function<void(base_objects::SharedClientData& self, base_objects::network::response&&)> special_callback = nullptr);
            ~SharedClientData();

            void* getAssignedData() const {
                return assigned_data;
            }

            bool canBeRemoved() const {
                return !special_callback;
            }

            bool isSpecial() const {
                return (bool)special_callback;
            }

            //internal
            api::network::tcp::session* get_session() {
                return ss;
            }

            bool did_send_packet() {
                bool res = sent;
                sent = false;
                return res;
            }

        private:
            void send_indirect(base_objects::network::response&&);
            friend struct virtual_client;
            std::function<void(base_objects::SharedClientData& self, base_objects::network::response&&)> special_callback;
            void* assigned_data;
            api::network::tcp::session* ss;
            bool sent = false;
        };

        inline SharedClientData::packets_state_t::protocol_state operator|(SharedClientData::packets_state_t::protocol_state a, SharedClientData::packets_state_t::protocol_state b) {
            return SharedClientData::packets_state_t::protocol_state(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
        }

        inline SharedClientData::packets_state_t::protocol_state operator&(SharedClientData::packets_state_t::protocol_state a, SharedClientData::packets_state_t::protocol_state b) {
            return SharedClientData::packets_state_t::protocol_state(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
        }

        inline SharedClientData::packets_state_t::protocol_state operator^(SharedClientData::packets_state_t::protocol_state a, SharedClientData::packets_state_t::protocol_state b) {
            return SharedClientData::packets_state_t::protocol_state(static_cast<uint8_t>(a) ^ static_cast<uint8_t>(b));
        }

        using client_data_holder = atomic_holder<SharedClientData>;
    }
}
#endif /* SRC_BASE_OBJECTS_SHARED_CLIENT_DATA */
