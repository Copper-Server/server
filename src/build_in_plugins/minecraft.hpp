#ifndef SRC_BUILD_IN_PLUGINS_MINECRAFT
#define SRC_BUILD_IN_PLUGINS_MINECRAFT
#include <src/plugin/main.hpp>

namespace copper_server {
    namespace build_in_plugins {
        //handles notchain plugin chanels
        class MinecraftPlugin : public PluginAutoRegister<"minecraft", MinecraftPlugin> {
        public:
            void OnLoad(const PluginRegistrationPtr& self) override;
            void OnUnload(const PluginRegistrationPtr& self) override;
            plugin_response OnConfiguration(base_objects::client_data_holder& client) override;
            plugin_response OnConfigurationHandle(const PluginRegistrationPtr& self, const std::string& chanel, const list_array<uint8_t>& data, base_objects::client_data_holder& client) override;
        };
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_MINECRAFT */
