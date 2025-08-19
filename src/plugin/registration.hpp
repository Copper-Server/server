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
#include <src/base_objects/atomic_holder.hpp>
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
        struct SharedClientData;
        template <typename T>
        class atomic_holder;
        using client_data_holder = atomic_holder<SharedClientData>;
    }

    namespace api::packets::server_bound::configuration {
        struct select_known_packs;
    }

    namespace api::packets::events {
        template <class Packet>
        auto& viewer();
        template <class Packet>
        auto& viewer_post_send();
        template <class Packet>
        auto& processor();
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

    class PluginRegistration {
        struct event_auto_cleanup_t {
            base_objects::events::base_event* event_obj;
            base_objects::events::event_register_id id;
            base_objects::events::priority priority;
            bool async_mode;
            bool load_state;
        };

        list_array<event_auto_cleanup_t> cleanup_list;
        bool is_loaded = false;
        friend class PluginManagement;

    public:
        virtual void initializer(const std::shared_ptr<PluginRegistration>&) {};
        virtual void deinitializer(const std::shared_ptr<PluginRegistration>&) {};

        void register_packet_processor(auto&& fn) {
            register_event(api::packets::events::processor<__internal::first_argument_type<decltype(fn)>>(), std::move(fn));
        }

        void register_packet_post_send_viewer(auto&& fn) {
            register_event(api::packets::events::viewer_post_send<__internal::first_argument_type<decltype(fn)>>(), std::move(fn));
        }

        void register_packet_viewer(auto&& fn) {
            register_event(api::packets::events::viewer<__internal::first_argument_type<decltype(fn)>>(), std::move(fn));
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

        virtual ~PluginRegistration() noexcept {
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
        virtual void OnRegister(const std::shared_ptr<PluginRegistration>&) {}

        //called on initialization, default resources allocated, plugins registered
        virtual void OnInitialization(const std::shared_ptr<PluginRegistration>&) {}

        //called after initialization
        virtual void OnLoad(const std::shared_ptr<PluginRegistration>&) {}

        //called after OnLoad, api is ready
        virtual void OnPostLoad(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnLoadComplete(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnUnload(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnPostUnload(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnUnloadComplete(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnUnregister(const std::shared_ptr<PluginRegistration>&) {}

        //emergency unload to save important data, do not use any other api except configuration
        virtual void OnFaultUnload(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnCommandsLoad(const std::shared_ptr<PluginRegistration>&, base_objects::command_root_browser&) {}

        virtual void OnCommandsLoadComplete(const std::shared_ptr<PluginRegistration>&, base_objects::command_root_browser&) {}

        virtual void OnConfigReload(const std::shared_ptr<PluginRegistration>&) {}

#pragma endregion

#pragma region OnLogin
        //custom plugin handling
        //args: self, chanel, is_successful, client
        virtual login_response OnLoginHandle(const std::shared_ptr<PluginRegistration>&, const std::string&, const list_array<uint8_t>&, bool, base_objects::SharedClientData&) {
            return {login_response::none{}};
        }

        //args: self, chanel, client
        virtual login_response OnLoginStart(const std::shared_ptr<PluginRegistration>&, const std::string&, base_objects::SharedClientData&) {

            return {login_response::none{}};
        }

        //args: self, chanel, is_successful, client
        virtual login_response OnLoginCookie(const std::shared_ptr<PluginRegistration>&, const std::string&, const list_array<uint8_t>&, bool, base_objects::SharedClientData&) {
            return {login_response::none{}};
        }

#pragma endregion

#pragma region OnConfiguration

        //returns true if the plugin completed its work in configuration
        virtual bool OnConfiguration(base_objects::SharedClientData&) {
            return false;
        }

        //returns true if the plugin completed its work in configuration
        //args: self, chanel, data, client
        virtual bool OnConfigurationHandle(const std::shared_ptr<PluginRegistration>&, const std::string&, const list_array<uint8_t>&, base_objects::SharedClientData&) {
            return true;
        }

        //returns true if the plugin completed its work in configuration
        virtual bool OnConfiguration_gotKnownPacks(base_objects::SharedClientData&, const api::packets::server_bound::configuration::select_known_packs&) {
            return true;
        }

        //returns true if the plugin completed its work in configuration
        //args: self, cookie_id, data, client
        virtual bool OnConfigurationCookie(const std::shared_ptr<PluginRegistration>&, const std::string&, const list_array<uint8_t>&, base_objects::SharedClientData&) {
            return true;
        }

#pragma endregion

#pragma region OnPlay

        //custom plugin handling
        virtual void OnPlayHandle(const std::shared_ptr<PluginRegistration>&, const std::string&, const list_array<uint8_t>&, base_objects::SharedClientData&) {}

        virtual void OnPlayCookie(const std::shared_ptr<PluginRegistration>&, const std::string&, const list_array<uint8_t>&, base_objects::SharedClientData&) {}

        virtual void OnPlay_initialize(base_objects::SharedClientData&) {}

        virtual void OnPlay_initialize_compatible(base_objects::SharedClientData&) {}

        virtual void OnPlay_post_initialize(base_objects::SharedClientData&) {}

        virtual void OnPlay_post_initialize_compatible(base_objects::SharedClientData&) {}

        virtual void OnPlay_uninitialized(base_objects::SharedClientData&) {}

        virtual void OnPlay_uninitialized_compatible(base_objects::SharedClientData&) {}

        //player must be initialized for this call
        virtual void PlayerJoined(base_objects::SharedClientData&) {}

        //player data must be initialized for this call and uninitialized after
        virtual void PlayerLeave(base_objects::SharedClientData&) {}

        //notifies when player fully left, the send operation is disabled
        virtual void PlayerLeft(base_objects::SharedClientData&) {}

        //TODO add more events


#pragma endregion
    };

    using PluginRegistrationPtr = std::shared_ptr<PluginRegistration>;
}

#endif /* SRC_PLUGIN_REGISTRATION */
