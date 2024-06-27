#include "main.hpp"
#include "special.hpp"

namespace crafted_craft {
    PluginManagement pluginManagement;
    SpecialPluginHandshake* special_handshake;
    SpecialPluginStatus* special_status;

    namespace ___internal__ {
        void register_OnLoginInit(const std::shared_ptr<PluginRegistration>& self) {
        }

        void register_OnLoginHandle(const std::shared_ptr<PluginRegistration>& self);

        void register_OnConfiguration(const std::shared_ptr<PluginRegistration>& self);
        void register_OnConfigurationHandle(const std::shared_ptr<PluginRegistration>& self);
        void register_OnConfiguration_PlayerSettingsChanged(const std::shared_ptr<PluginRegistration>& self);

        void register_OnPlayHandle(const std::shared_ptr<PluginRegistration>& self);
        void register_OnPlay_initialize(const std::shared_ptr<PluginRegistration>& self);
        void register_OnPlay_uninitialized(const std::shared_ptr<PluginRegistration>& self);


        void unregister_OnLoginInit(const std::shared_ptr<PluginRegistration>& self);
        void unregister_OnLoginHandle(const std::shared_ptr<PluginRegistration>& self);

        void unregister_OnConfiguration(const std::shared_ptr<PluginRegistration>& self);
        void unregister_OnConfigurationHandle(const std::shared_ptr<PluginRegistration>& self);
        void unregister_OnConfiguration_PlayerSettingsChanged(const std::shared_ptr<PluginRegistration>& self);

        void unregister_OnPlayHandle(const std::shared_ptr<PluginRegistration>& self);
        void unregister_OnPlay_initialize(const std::shared_ptr<PluginRegistration>& self);
        void unregister_OnPlay_uninitialized(const std::shared_ptr<PluginRegistration>& self);
    }
}