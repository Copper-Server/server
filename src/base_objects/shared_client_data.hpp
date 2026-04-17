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
#include <array>
#include <chrono>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <library/fast_task.hpp>
#include <library/list_array.hpp>
#include <src/api/mojang/session_server.hpp>
#include <src/base_objects/events/sync_event.hpp>
#include <src/base_objects/uuid.hpp>
#include <src/plugin/registration.hpp>

namespace copper_server {
    class plugin_registration;
    using plugin_registration_ptr = std::shared_ptr<plugin_registration>;

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
        struct slot;
        struct slot_data;

        struct shared_client_data {
            std::string name;
            std::string ip;
            std::shared_ptr<api::mojang::session_server::player_data> data;
            std::string client_brand;


            std::string locale; //max 16 chars
            list_array<plugin_registration_ptr> compatible_plugins;
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
            bool enable_tab_listings : 1 = true;
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

                struct play_data_t {
                    struct signature_t {
                        base_objects::uuid chat_session_id;
                        uint64_t pub_key_expiries_timestamp;
                        list_array<uint8_t> public_key;
                        list_array<uint8_t> public_signature;
                    };

                    struct last_seen_message {
                        std::array<uint8_t, 256> signature;
                    };

                    struct screen {
                        struct click_data {
                            struct hashed_slot_data {
                                int32_t item_id;
                                int32_t count;

                                struct component {
                                    int32_t type;
                                    int32_t crc32c_hash;
                                };

                                list_array<component> add_components;
                                list_array<int32_t> remove_components;
                            };

                            struct changed_slot {
                                short slot;
                                std::optional<hashed_slot_data> data;
                            };

                            int32_t state_id;
                            short slot;
                            int8_t button;
                            int32_t mode;
                            list_array<changed_slot> changed;
                            std::optional<hashed_slot_data> carry_item;
                        };

                        screen(base_objects::shared_client_data& client);
                        virtual ~screen() {};
                        void close();                                                //packet only
                        void set_data(int16_t prop, int16_t data);                   //packet only
                        void set_slot(int32_t slot, const base_objects::slot&);      //packet only
                        void set_slot(int32_t slot, const base_objects::slot_data&); //packet only
                        void update_content();                                       //packet only, uses max_size(), iterate(int32_t) and main_screen->get_carried_item()
                        void drop_item(const base_objects::slot&);                   //spawns item in the world
                        void set_held_item(const base_objects::slot_data&);          //packet only

                        virtual bool valid_slot(int32_t slot) const {
                            return slot <= max_size() && slot >= 0;
                        }

                        virtual bool has_item(int32_t) const = 0;
                        virtual int32_t max_size() const = 0;
                        virtual base_objects::slot& get_slot(int32_t) = 0;
                        virtual void iterate(std::move_only_function<void(base_objects::slot&, int32_t)>&& fn) = 0;

                        virtual void event_anvil_set_name(std::string& new_name) {}

                        virtual void event_place_recipe(int32_t recipe_id, bool make_all) = 0;
                        virtual void event_button_click(int32_t button_id) = 0;
                        virtual void event_click(click_data& data) = 0;
                        virtual void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) = 0;

                        virtual void event_close() {}

                        virtual void event_slot_state_changed(int32_t slot_id, bool state) {}

                        virtual void event_set_beacon(std::optional<int32_t> primary_potion, std::optional<int32_t> secondary_potion) {}

                        virtual void event_book_edit_request(int32_t slot_id, const list_array<std::string_view>& text, const std::optional<std::string_view>& name) {}

                        inline int32_t get_windows_id() const {
                            return current_id;
                        }

                        inline int32_t get_state_id() const {
                            return state_id;
                        }

                    protected:
                        friend struct play_data_t;
                        int32_t current_id;   //automatically assigned
                        int32_t state_id = 0; //incremented by one after each change
                        int32_t windows_type = 0;
                        base_objects::shared_client_data& client;
                    };

                    struct main_screen_i : public screen {
                        main_screen_i(base_objects::shared_client_data& client) : screen(client) {}

                        virtual base_objects::slot& get_carried_item() = 0;
                    };

                    std::unique_ptr<main_screen_i> main_screen;
                    std::unique_ptr<screen> current_screen;
                    std::unique_ptr<signature_t> signature;
                    list_array<last_seen_message> last_seen_messages;

                    struct shadow_movement_data_t {
                        bool using_elytra = false;

                        int32_t last_movement_packet = 0;
                    };

                    shadow_movement_data_t shadow_movement;
                    int32_t screen_counter = 0;

                    void init_main_screen(std::unique_ptr<main_screen_i> _screen) {
                        _screen->current_id = 0;
                        main_screen = std::move(_screen);
                    }

                    void open_screen(std::unique_ptr<screen> _screen) {
                        auto id = ++screen_counter;
                        if (screen_counter == 100)
                            screen_counter = 0;
                        current_screen = std::move(_screen);
                        current_screen->current_id = id;
                    }

                    void add_seen_signed_message(const std::array<uint8_t, 256>& val) {
                        last_seen_messages.emplace_back(val);
                        if (last_seen_messages.size() > 20)
                            last_seen_messages.pop_front();
                    }
                };

                struct internal_data_t {
                    std::unordered_map<std::string, int32_t> id_tracker;
                    std::unordered_map<std::string, unordered_track> unordered_id_tracker;
                    std::shared_ptr<void> extra_data; //here stored custom handler data, cleared after switching to other state
                    std::unique_ptr<play_data_t> play_data;
                };

                fast_task::protected_value<internal_data_t> internal_data;

                std::unordered_set<base_objects::uuid> active_resource_packs;
                std::chrono::system_clock::time_point pong_timer;
                std::chrono::system_clock::time_point last_batch_check = std::chrono::system_clock::time_point::min();
                std::atomic_int32_t keep_alive_ping_ms = 0;
                std::atomic_int32_t local_chat_counter = 0;
                std::atomic_uint32_t await_ack_chunk_batches = 0;
                int32_t chunk_batch_size = 25; //used only for world processing and thread unsafe. The writes should be synced
                int32_t chunks_sent = 0;       //used only for world processing and thread unsafe. The writes should be synced
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

                template <class FN>
                decltype(auto) get_play_data(FN&& fn) {
                    return internal_data.set([&fn](auto& internal_) {
                        if (!internal_.play_data)
                            internal_.play_data = std::make_unique<play_data_t>();
                        return fn(*internal_.play_data);
                    });
                }
            } packets_state;

            std::chrono::milliseconds ping = std::chrono::milliseconds(0);

            void register_plugin(plugin_registration_ptr plugin) {
                compatible_plugins.push_back(plugin);
            }

            void unregisterPlugin(plugin_registration_ptr plugin) {
                compatible_plugins.remove(plugin);
            }

            bool isCompatiblePlugin(plugin_registration_ptr plugin) {
                return compatible_plugins.contains(plugin);
            }

            void sendPacket(base_objects::network::response&& packet) {
                if (special_callback)
                    special_callback(*this, std::move(packet));
                else if (ss)
                    send_indirect(std::move(packet));
                sent = true;
            }

            shared_client_data(api::network::tcp::session* ss = nullptr, void* assigned_data = nullptr, std::function<void(base_objects::shared_client_data& self, base_objects::network::response&&)> special_callback = nullptr);
            ~shared_client_data();

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

            //returns true if client could accept packets (works for virtual and real)
            bool is_active() const;

            void deactivate(); //internal

        private:
            void send_indirect(base_objects::network::response&&);
            friend struct virtual_client;
            std::function<void(base_objects::shared_client_data& self, base_objects::network::response&&)> special_callback;
            void* assigned_data;
            api::network::tcp::session* ss;
            bool sent = false;
        };

        inline shared_client_data::packets_state_t::protocol_state operator|(shared_client_data::packets_state_t::protocol_state a, shared_client_data::packets_state_t::protocol_state b) {
            return shared_client_data::packets_state_t::protocol_state(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
        }

        inline shared_client_data::packets_state_t::protocol_state operator&(shared_client_data::packets_state_t::protocol_state a, shared_client_data::packets_state_t::protocol_state b) {
            return shared_client_data::packets_state_t::protocol_state(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
        }

        inline shared_client_data::packets_state_t::protocol_state operator^(shared_client_data::packets_state_t::protocol_state a, shared_client_data::packets_state_t::protocol_state b) {
            return shared_client_data::packets_state_t::protocol_state(static_cast<uint8_t>(a) ^ static_cast<uint8_t>(b));
        }

        using client_data_holder = std::shared_ptr<shared_client_data>;
    }
}
#endif /* SRC_BASE_OBJECTS_SHARED_CLIENT_DATA */
