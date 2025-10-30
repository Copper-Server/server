/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_PLUGIN_REGISTRATION
#define SRC_PLUGIN_REGISTRATION
#include <library/list_array.hpp>
#include <src/api/ecs.hpp>
#include <src/base_objects/data_packs/known_pack.hpp>
#include <src/base_objects/events/event.hpp>
#include <src/base_objects/events/sync_event.hpp>
#include <src/base_objects/network/response.hpp>

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace copper_server {
    namespace base_objects {
        class command_root_browser;
        struct shared_client_data;
        using client_data_holder = std::shared_ptr<shared_client_data>;
    }

    namespace api::packets::server_bound::config {
        struct select_known_packs;
    }

    namespace api::packets::events {
        template <class packet>
        base_objects::events::sync_event<packet&, base_objects::shared_client_data&>& send_viewer();

        template <class packet>
        base_objects::events::sync_event_no_cancel<packet&, base_objects::shared_client_data&>& post_send_viewer();

        template <class packet>
        base_objects::events::sync_event<packet&, base_objects::shared_client_data&>& receive_viewer();

        template <class packet>
        base_objects::events::sync_event_single<packet&&, base_objects::shared_client_data&>& processor();
    }

    namespace __internal {
        template <class Ret, class Arg0, class... Rest>
        Arg0 first_argument_helper(Ret (*)(Arg0, Rest...));

        template <class Ret, class Fn, class Arg0, class... Rest>
        Arg0 first_argument_helper(Ret (Fn::*)(Arg0, Rest...));

        template <class Ret, class Fn, class Arg0, class... Rest>
        Arg0 first_argument_helper(Ret (Fn::*)(Arg0, Rest...) const);

        template <class Fn>
        decltype(first_argument_helper(&Fn::operator())) first_argument_helper(Fn);

        template <class T>
        using first_argument_type = std::decay_t<decltype(first_argument_helper(std::declval<T>()))>;
    }

    class plugin_registration {
        struct event_auto_cleanup_t {
            base_objects::events::base_event* event_obj;
            base_objects::events::event_register_id id;
            base_objects::events::priority priority;
            bool async_mode;
            bool load_state;
        };

        list_array<event_auto_cleanup_t> cleanup_list;
        bool is_loaded = false;
        friend class plugin_management_system;

    public:
        virtual const std::string& get_name() const = 0;
        virtual void initializer(const std::shared_ptr<plugin_registration>&) {};
        virtual void deinitializer(const std::shared_ptr<plugin_registration>&) {};

        void register_packet_processor(auto&& fn) {
            register_event(api::packets::events::processor<__internal::first_argument_type<decltype(fn)>>(), std::move(fn));
        }

        void register_packet_post_send_viewer(auto&& fn) {
            register_event(api::packets::events::post_send_viewer<__internal::first_argument_type<decltype(fn)>>(), std::move(fn));
        }

        void register_packet_send_viewer(auto&& fn) {
            register_event(api::packets::events::send_viewer<__internal::first_argument_type<decltype(fn)>>(), std::move(fn));
        }

        void register_packet_receive_viewer(auto&& fn) {
            register_event(api::packets::events::receive_viewer<__internal::first_argument_type<decltype(fn)>>(), std::move(fn));
        }

        template <class... Args>
        void register_event(base_objects::events::sync_event<Args...>& event_ref, auto&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(std::move(fn), base_objects::events::priority::avg), base_objects::events::priority::avg, false, is_loaded});
        }

        template <class... Args>
        void register_event(base_objects::events::sync_event_no_cancel<Args...>& event_ref, auto&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(std::move(fn)), base_objects::events::priority::avg, false, is_loaded});
        }

        template <class... Args>
        void register_event(base_objects::events::sync_event_single<Args...>& event_ref, auto&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(std::move(fn)), base_objects::events::priority::avg, false, is_loaded});
        }


        template <class... Args>
        void register_event(base_objects::events::sync_event<Args...>& event_ref, base_objects::events::priority priority, base_objects::events::sync_event<Args...>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(std::move(fn), priority), priority, false, is_loaded});
        }

        template <class T>
        void register_event(base_objects::events::event<T>& event_ref, base_objects::events::event<T>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(base_objects::events::priority::avg, false, std::move(fn)), base_objects::events::priority::avg, false, is_loaded});
        }

        template <class T>
        void register_event(base_objects::events::event<T>& event_ref, base_objects::events::priority priority, base_objects::events::event<T>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(priority, false, std::move(fn)), priority, false, is_loaded});
        }

        template <class T>
        void register_event(base_objects::events::event<T>& event_ref, base_objects::events::priority priority, bool async_mode, base_objects::events::event<T>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(priority, async_mode, std::move(fn)), priority, async_mode, is_loaded});
        }

        void clean_up_registered_events() {
            cleanup_list.remove_if([](const event_auto_cleanup_t& leave_data) {
                if (leave_data.load_state)
                    leave_data.event_obj->leave(leave_data.id, leave_data.priority, leave_data.async_mode);
                return leave_data.load_state;
            });
        }

        void clean_up_all_events() {
            cleanup_list.take().for_each([](const event_auto_cleanup_t& leave_data) {
                leave_data.event_obj->leave(leave_data.id, leave_data.priority, leave_data.async_mode);
            });
        }

        virtual ~plugin_registration() noexcept {
            clean_up_all_events();
        }

        struct login_response {
            struct none {};

            struct request_cookie {
                std::string identifier;
            };

            struct custom_query {
                std::string identifier;
                list_array<uint8_t> data;
            };

            std::variant<none, request_cookie, custom_query> value;
        };

#pragma region Server

        //first initialisation
        virtual void on_register(const std::shared_ptr<plugin_registration>&) {}

        //called on initialization, default resources allocated, plugins registered
        virtual void on_initialization(const std::shared_ptr<plugin_registration>&) {}

        //called after initialization
        virtual void on_load(const std::shared_ptr<plugin_registration>&) {}

        //called after on_load, api is ready
        virtual void on_post_load(const std::shared_ptr<plugin_registration>&) {}

        virtual void on_load_complete(const std::shared_ptr<plugin_registration>&) {}

        virtual void on_unload(const std::shared_ptr<plugin_registration>&) {}

        virtual void on_post_unload(const std::shared_ptr<plugin_registration>&) {}

        virtual void on_unload_complete(const std::shared_ptr<plugin_registration>&) {}

        virtual void on_unregister(const std::shared_ptr<plugin_registration>&) {}

        //emergency unload to save important data, do not use any other api except configuration
        virtual void on_fault_unload(const std::shared_ptr<plugin_registration>&) {}

        virtual void on_commands_load(const std::shared_ptr<plugin_registration>&, base_objects::command_root_browser&) {}

        virtual void on_commands_load_complete(const std::shared_ptr<plugin_registration>&, base_objects::command_root_browser&) {}

        virtual void on_config_reload(const std::shared_ptr<plugin_registration>&) {}

#pragma endregion

#pragma region OnLogin
        //custom plugin handling
        //args: self, chanel, is_successful, client
        virtual login_response on_login_handle(const std::shared_ptr<plugin_registration>&, const std::string&, const list_array<uint8_t>&, bool, base_objects::shared_client_data&) {
            return {login_response::none{}};
        }

        //args: self, chanel, client
        virtual login_response on_login_start(const std::shared_ptr<plugin_registration>&, const std::string&, base_objects::shared_client_data&) {
            return {login_response::none{}};
        }

        //args: self, chanel, is_successful, client
        virtual login_response on_login_cookie(const std::shared_ptr<plugin_registration>&, const std::string&, const list_array<uint8_t>&, bool, base_objects::shared_client_data&) {
            return {login_response::none{}};
        }

#pragma endregion

#pragma region on_configuration

        //returns true if the plugin completed its work in configuration
        virtual bool on_configuration(base_objects::shared_client_data&) {
            return false;
        }

        //returns true if the plugin completed its work in configuration
        //args: self, chanel, data, client
        virtual bool on_configuration_handle(const std::shared_ptr<plugin_registration>&, const std::string&, const list_array<uint8_t>&, base_objects::shared_client_data&) {
            return true;
        }

        //returns true if the plugin completed its work in configuration
        virtual bool on_configuration_got_known_packs(base_objects::shared_client_data&, const api::packets::server_bound::config::select_known_packs&) {
            return true;
        }

        //returns true if the plugin completed its work in configuration
        //args: self, cookie_id, data, client
        virtual bool on_configuration_cookie(const std::shared_ptr<plugin_registration>&, const std::string&, const list_array<uint8_t>&, base_objects::shared_client_data&) {
            return true;
        }

#pragma endregion

#pragma region OnPlay

        //custom plugin handling
        virtual void on_play_handle(const std::shared_ptr<plugin_registration>&, const std::string&, const list_array<uint8_t>&, base_objects::shared_client_data&) {}

        virtual void on_play_cookie(const std::shared_ptr<plugin_registration>&, const std::string&, const list_array<uint8_t>&, base_objects::shared_client_data&) {}

        virtual void on_play_pre_initialize(base_objects::shared_client_data&) {} //world is not available

        virtual void on_play_initialize(base_objects::shared_client_data&) {}

        virtual void on_play_initialize_compatible(base_objects::shared_client_data&) {}

        virtual void on_play_post_initialize(base_objects::shared_client_data&) {}

        virtual void on_play_post_initialize_compatible(base_objects::shared_client_data&) {}

        virtual void on_play_uninitialized(base_objects::shared_client_data&) {}

        virtual void on_play_uninitialized_compatible(base_objects::shared_client_data&) {}

        //player must be initialized for this call
        virtual void player_joined(base_objects::shared_client_data&) {}

        //player data must be initialized for this call and uninitialized after
        virtual void player_leave(base_objects::shared_client_data&) {}

        //notifies when player fully left, the send operation is disabled
        virtual void player_left(base_objects::shared_client_data&) {}

        //TODO add more events


#pragma endregion

#pragma region ECS

        virtual void register_systems(api::ecs::scheduler&) {}

#pragma endregion
    };

    using plugin_registration_ptr = std::shared_ptr<plugin_registration>;
}

#endif /* SRC_PLUGIN_REGISTRATION */
