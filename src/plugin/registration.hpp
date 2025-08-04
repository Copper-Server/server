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

    namespace api::new_packets::server_bound::configuration {
        struct select_known_packs;
    }

    class PluginRegistration {
        struct event_auto_cleanup_t {
            base_objects::events::base_event* event_obj;
            base_objects::events::event_register_id id;
            base_objects::events::priority priority;
            bool async_mode;
        };

        list_array<event_auto_cleanup_t> cleanup_list;
        bool is_loaded = false;
        friend class PluginManagement;

    public:
        virtual void initializer(const std::shared_ptr<PluginRegistration>&) {};
        virtual void deinitializer(const std::shared_ptr<PluginRegistration>&) {};

        template <class... Args>
        void register_event(base_objects::events::sync_event<Args...>& event_ref, base_objects::events::sync_event<Args...>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(base_objects::events::priority::avg, false, std::move(fn)), base_objects::events::priority::avg, false});
        }

        template <class... Args>
        void register_event(base_objects::events::sync_event<Args...>& event_ref, base_objects::events::priority priority, base_objects::events::sync_event<Args...>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(priority, false, std::move(fn)), priority, false});
        }

        template <class T>
        void register_event(base_objects::events::event<T>& event_ref, base_objects::events::event<T>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(base_objects::events::priority::avg, false, std::move(fn)), base_objects::events::priority::avg, false});
        }

        template <class T>
        void register_event(base_objects::events::event<T>& event_ref, base_objects::events::priority priority, base_objects::events::event<T>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(priority, false, std::move(fn)), priority, false});
        }

        template <class T>
        void register_event(base_objects::events::event<T>& event_ref, base_objects::events::priority priority, bool async_mode, base_objects::events::event<T>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(priority, async_mode, std::move(fn)), priority, async_mode});
        }

        void clean_up_registered_events() {
            cleanup_list.take().for_each([](event_auto_cleanup_t&& leave_data) {
                leave_data.event_obj->leave(leave_data.id, leave_data.priority, leave_data.async_mode);
            });
        }

        virtual ~PluginRegistration() noexcept {
            clean_up_registered_events();
        }

        struct login_response {
            struct none {};

            struct request_cookie {
                std::string identifier;
            };

            struct custom_query {
                std::string identifier;
                std::vector<uint8_t> data;
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
        virtual login_response OnLoginHandle(const std::shared_ptr<PluginRegistration>& self, const std::string& chanel, const list_array<uint8_t>& data, bool successful, base_objects::SharedClientData& client) {
            return {login_response::none{}};
        }

        virtual login_response OnLoginStart(const std::shared_ptr<PluginRegistration>& self, const std::string& chanel, base_objects::SharedClientData& client) {
            return {login_response::none{}};
        }

        virtual login_response OnLoginCookie(const std::shared_ptr<PluginRegistration>& self, const std::string& cookie_id, const list_array<uint8_t>& data, bool successful, base_objects::SharedClientData& client) {
            return {login_response::none{}};
        }

#pragma endregion

#pragma region OnConfiguration

        //returns true if the plugin completed its work in configuration
        virtual bool OnConfiguration(base_objects::SharedClientData&) {
            return false;
        }

        //returns true if the plugin completed its work in configuration
        virtual bool OnConfigurationHandle(const std::shared_ptr<PluginRegistration>& self, const std::string& chanel, const std::vector<uint8_t>& data, base_objects::SharedClientData&) {
            return true;
        }

        //returns true if the plugin completed its work in configuration
        virtual bool OnConfiguration_gotKnownPacks(base_objects::SharedClientData&, const api::new_packets::server_bound::configuration::select_known_packs&) {
            return true;
        }

        //returns true if the plugin completed its work in configuration
        virtual bool OnConfigurationCookie(const std::shared_ptr<PluginRegistration>& self, const std::string& cookie_id, const list_array<uint8_t>& data, base_objects::SharedClientData&) {}

#pragma endregion

#pragma region OnPlay

        //custom plugin handling
        virtual void OnPlayHandle(const std::shared_ptr<PluginRegistration>& self, const std::string& chanel, const list_array<uint8_t>& data, base_objects::SharedClientData&) {}

        virtual void OnPlayCookie(const std::shared_ptr<PluginRegistration>& self, const std::string& cookie_id, const list_array<uint8_t>& data, base_objects::SharedClientData&) {}

        virtual void OnPlay_initialize(base_objects::SharedClientData& client) {}

        virtual void OnPlay_initialize_compatible(base_objects::SharedClientData& client) {}

        virtual void OnPlay_post_initialize(base_objects::SharedClientData& client) {}

        virtual void OnPlay_post_initialize_compatible(base_objects::SharedClientData& client) {}

        virtual void OnPlay_uninitialized(base_objects::SharedClientData& client) {}

        virtual void OnPlay_uninitialized_compatible(base_objects::SharedClientData& client) {}

        //player must be initialized for this call
        virtual void PlayerJoined(base_objects::SharedClientData& client) {}

        //player data must be initialized for this call and uninitialized after
        virtual void PlayerLeave(base_objects::SharedClientData& client) {}

        //notifies when player fully left, the send operation is disabled
        virtual void PlayerLeft(base_objects::SharedClientData& client) {}

        //TODO add more events


#pragma endregion
    };

    using PluginRegistrationPtr = std::shared_ptr<PluginRegistration>;
}

#endif /* SRC_PLUGIN_REGISTRATION */
