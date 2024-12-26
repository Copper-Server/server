#ifndef SRC_BUILD_IN_PLUGINS_COMMUNICATION_CORE
#define SRC_BUILD_IN_PLUGINS_COMMUNICATION_CORE
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    //provides and manages chat system
    class CommunicationCorePlugin : public PluginAutoRegister<"communication_core", CommunicationCorePlugin> {
    public:
        CommunicationCorePlugin();
        void OnLoad(const PluginRegistrationPtr& self) override;
        void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override;
    };
}
#endif /* SRC_BUILD_IN_PLUGINS_COMMUNICATION_CORE */
