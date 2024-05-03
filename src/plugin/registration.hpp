#ifndef SRC_PLUGIN_REGISTRATION
#define SRC_PLUGIN_REGISTRATION
#include "../ClientHandleHelper.hpp"
#include "../base_objects/commands.hpp"
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace crafted_craft {
    class PluginRegistration {
    public:
        //used only for login
        struct PluginResponse {
            list_array<uint8_t> data;
            std::string plugin_chanel;
        };
        using plugin_response = std::variant<TCPclient::Response, PluginResponse, bool>;

#pragma region Server

        virtual void OnLoad(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnLoadComplete(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnReload(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnReloadComplete(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnUnload(const std::shared_ptr<PluginRegistration>&) {}

        virtual void OnCommandsLoad(const std::shared_ptr<PluginRegistration>&, base_objects::command_root_browser&) {}

        virtual void OnCommandsLoadComplete(const std::shared_ptr<PluginRegistration>&, base_objects::command_root_browser&) {}

#pragma endregion

#pragma region OnLogin

        virtual plugin_response OnLoginInit(SharedClientData&) {
            return false;
        }

        virtual plugin_response OnLoginHandle(const std::string& chanel, const list_array<uint8_t>& data, bool successful, SharedClientData&) {
            return false;
        }

#pragma endregion

#pragma region OnConfiguration

        virtual plugin_response OnConfiguration(SharedClientData&) {
            return false;
        }

        virtual plugin_response OnConfigurationHandle(const std::string& chanel, const list_array<uint8_t>& data, SharedClientData&) {
            return false;
        }

        virtual plugin_response OnConfiguration_PlayerSettingsChanged(SharedClientData&) {
            return false;
        }

#pragma endregion

#pragma region OnPlay

        virtual plugin_response OnPlay_initialize(SharedClientData& client) {
            return false;
        }

        virtual plugin_response OnPlay_uninitialized(SharedClientData& client) {
            return false;
        }

#pragma endregion
    };

    using PluginRegistrationPtr = std::shared_ptr<PluginRegistration>;
}

#endif /* SRC_PLUGIN_REGISTRATION */
