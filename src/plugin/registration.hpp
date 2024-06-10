#ifndef SRC_PLUGIN_REGISTRATION
#define SRC_PLUGIN_REGISTRATION
#include "../base_objects/commands.hpp"
#include "../base_objects/event.hpp"
#include "../base_objects/response.hpp"
#include "../base_objects/server_configuaration.hpp"
#include "../base_objects/shared_client_data.hpp"
#include "../library/list_array.hpp"
#include <memory>
#include <string>
#include <variant>
#include <vector>


namespace crafted_craft {
    class PluginRegistration {
        struct event_auto_cleanup_t {
            base_objects::base_event* event_obj;
            base_objects::event_register_id id;
            base_objects::event_priority priority;
            bool async_mode;
        };

        list_array<event_auto_cleanup_t> cleanup_list;

    public:
        template <class T>
        void register_event(base_objects::event<T>& event_ref, base_objects::event<T>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(base_objects::event_priority::avg, false, fn), base_objects::event_priority::avg, false});
        }

        template <class T>
        void register_event(base_objects::event<T>& event_ref, base_objects::event_priority priority, base_objects::event<T>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(priority, false, fn), priority, false});
        }

        template <class T>
        void register_event(base_objects::event<T>& event_ref, base_objects::event_priority priority, bool async_mode, base_objects::event<T>::function&& fn) {
            cleanup_list.push_back({&event_ref, event_ref.join(priority, async_mode, fn), priority, async_mode});
        }

        void clean_up_registered_events() {
            cleanup_list.take().forEach([](event_auto_cleanup_t&& leave_data) {
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

        using plugin_response = std::variant<Response, PluginResponse, bool>;

#pragma region Server

        //first initialisation
        virtual void OnRegister(const std::shared_ptr<PluginRegistration>&) {}

        //called after initialization
        virtual void OnLoad(const std::shared_ptr<PluginRegistration>&) {}

        //called after OnLoad, api is ready
        virtual void OnPostLoad(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnLoadComplete(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnUnload(const std::shared_ptr<PluginRegistration>&) {
            clean_up_registered_events();
        }

        virtual void OnCommandsLoad(const std::shared_ptr<PluginRegistration>&, base_objects::command_root_browser&) {}

        virtual void OnCommandsLoadComplete(const std::shared_ptr<PluginRegistration>&, base_objects::command_root_browser&) {}

        virtual void OnConfigReload(const std::shared_ptr<PluginRegistration>&, const base_objects::ServerConfiguration&) {}

#pragma endregion

#pragma region OnLogin

        virtual plugin_response OnLoginInit(base_objects::client_data_holder&) {
            return false;
        }

        virtual plugin_response OnLoginHandle(const std::shared_ptr<PluginRegistration>& self, const std::string& chanel, const list_array<uint8_t>& data, bool successful, base_objects::client_data_holder& client) {
            return false;
        }

#pragma endregion

#pragma region OnConfiguration

        virtual plugin_response OnConfiguration(base_objects::client_data_holder&) {
            return false;
        }

        virtual plugin_response OnConfigurationHandle(const std::shared_ptr<PluginRegistration>& self, const std::string& chanel, const list_array<uint8_t>& data, base_objects::client_data_holder&) {
            return false;
        }

        virtual plugin_response OnConfiguration_PlayerSettingsChanged(base_objects::client_data_holder&) {
            return false;
        }

#pragma endregion

#pragma region OnPlay

        virtual plugin_response OnPlayHandle(const std::shared_ptr<PluginRegistration>& self, const std::string& chanel, const list_array<uint8_t>& data, base_objects::client_data_holder&) {
            return false;
        }

        virtual plugin_response OnPlay_initialize(base_objects::client_data_holder& client) {
            return false;
        }

        virtual plugin_response OnPlay_uninitialized(base_objects::client_data_holder& client) {
            return false;
        }

#pragma endregion
    };

    using PluginRegistrationPtr = std::shared_ptr<PluginRegistration>;
}

#endif /* SRC_PLUGIN_REGISTRATION */
