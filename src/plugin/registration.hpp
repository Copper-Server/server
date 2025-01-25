#ifndef SRC_PLUGIN_REGISTRATION
#define SRC_PLUGIN_REGISTRATION
#include <library/list_array.hpp>
#include <memory>
#include <src/base_objects/data_packs/known_pack.hpp>
#include <src/base_objects/events/event.hpp>
#include <src/base_objects/network/response.hpp>
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

    class PluginRegistration {
        struct event_auto_cleanup_t {
            base_objects::events::base_event* event_obj;
            base_objects::events::event_register_id id;
            base_objects::events::priority priority;
            bool async_mode;
        };

        list_array<event_auto_cleanup_t> cleanup_list;

    public:
        virtual void initializer(const std::shared_ptr<PluginRegistration>&) {};
        virtual void deinitializer(const std::shared_ptr<PluginRegistration>&) {};

        template <class T>
        void register_event(base_objects::events::event<T>& event_ref, base_objects::events::event<T>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(base_objects::events::priority::avg, false, fn), base_objects::events::priority::avg, false});
        }

        template <class T>
        void register_event(base_objects::events::event<T>& event_ref, base_objects::events::priority priority, base_objects::events::event<T>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(priority, false, fn), priority, false});
        }

        template <class T>
        void register_event(base_objects::events::event<T>& event_ref, base_objects::events::priority priority, bool async_mode, base_objects::events::event<T>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(priority, async_mode, fn), priority, async_mode});
        }

        void clean_up_registered_events() {
            cleanup_list.take().for_each([](event_auto_cleanup_t&& leave_data) {
                leave_data.event_obj->leave(leave_data.id, leave_data.priority, leave_data.async_mode);
            });
        }

        virtual ~PluginRegistration() {
            clean_up_registered_events();
        }

        //used only for login
        struct PluginResponse {
            list_array<uint8_t> data;
            std::string plugin_chanel;
        };

        using plugin_response_login = std::variant<base_objects::network::response, PluginResponse, bool>;

        using plugin_response = base_objects::network::plugin_response;
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

        //emergency unload to save important data, do not use any other api except configuration
        virtual void OnFaultUnload(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnCommandsLoad(const std::shared_ptr<PluginRegistration>&, base_objects::command_root_browser&) {}

        virtual void OnCommandsLoadComplete(const std::shared_ptr<PluginRegistration>&, base_objects::command_root_browser&) {}

        virtual void OnConfigReload(const std::shared_ptr<PluginRegistration>&) {}

#pragma endregion

#pragma region OnLogin

        //custom plugin handling
        virtual plugin_response_login OnLoginHandle(const std::shared_ptr<PluginRegistration>& self, const std::string& chanel, const list_array<uint8_t>& data, bool successful, base_objects::client_data_holder& client) {
            return false;
        }

        virtual plugin_response_login OnLoginStart(const std::shared_ptr<PluginRegistration>& self, const std::string& chanel, base_objects::client_data_holder& client) {
            return false;
        }

        virtual plugin_response_login OnLoginCookie(const std::shared_ptr<PluginRegistration>& self, const std::string& cookie_id, const list_array<uint8_t>& data, bool successful, base_objects::client_data_holder& client) {
            return false;
        }

#pragma endregion

#pragma region OnConfiguration

        virtual plugin_response OnConfiguration(base_objects::client_data_holder&) {
            return std::nullopt;
        }

        //custom plugin handling
        virtual plugin_response OnConfigurationHandle(const std::shared_ptr<PluginRegistration>& self, const std::string& chanel, const list_array<uint8_t>& data, base_objects::client_data_holder&) {
            return std::nullopt;
        }

        virtual plugin_response OnConfiguration_PlayerSettingsChanged(base_objects::client_data_holder&) {
            return std::nullopt;
        }

        virtual plugin_response OnConfiguration_gotKnownPacks(base_objects::client_data_holder&, const list_array<base_objects::data_packs::known_pack>& known_packs) {
            return std::nullopt;
        }

        virtual plugin_response OnConfigurationCookie(const std::shared_ptr<PluginRegistration>& self, const std::string& cookie_id, const list_array<uint8_t>& data, base_objects::client_data_holder&) {
            return std::nullopt;
        }

#pragma endregion

#pragma region OnPlay

        //custom plugin handling
        virtual plugin_response OnPlayHandle(const std::shared_ptr<PluginRegistration>& self, const std::string& chanel, const list_array<uint8_t>& data, base_objects::client_data_holder&) {
            return std::nullopt;
        }

        virtual plugin_response OnPlayCookie(const std::shared_ptr<PluginRegistration>& self, const std::string& cookie_id, const list_array<uint8_t>& data, base_objects::client_data_holder&) {
            return std::nullopt;
        }

        virtual plugin_response OnPlay_initialize(base_objects::client_data_holder& client) {
            return std::nullopt;
        }

        virtual plugin_response OnPlay_initialize_compatible(base_objects::client_data_holder& client) {
            return std::nullopt;
        }

        virtual plugin_response OnPlay_uninitialized(base_objects::client_data_holder& client) {
            return std::nullopt;
        }

        virtual plugin_response OnPlay_uninitialized_compatible(base_objects::client_data_holder& client) {
            return std::nullopt;
        }

        //player must be initialized for this call
        virtual plugin_response PlayerJoined(base_objects::client_data_holder& client) {
            return std::nullopt;
        }

        //player must be initialized for this call and uninitialized after
        virtual plugin_response PlayerLeave(base_objects::client_data_holder& client) {
            return std::nullopt;
        }

        //TODO add more events


#pragma endregion
    };

    using PluginRegistrationPtr = std::shared_ptr<PluginRegistration>;
}

#endif /* SRC_PLUGIN_REGISTRATION */
