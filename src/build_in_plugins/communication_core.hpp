#ifndef SRC_BUILD_IN_PLUGINS_COMMUNICATION_CORE
#define SRC_BUILD_IN_PLUGINS_COMMUNICATION_CORE
#include "../plugin/main.hpp"

namespace crafted_craft {
    class Server;
    namespace build_in_plugins {
        class CommunicationCorePlugin : public PluginAutoRegister<"communication_core", CommunicationCorePlugin> {
        public:
            CommunicationCorePlugin();
            void OnLoad(const PluginRegistrationPtr& self) override;
            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override;
        };
    } // namespace build_in_plugins

} // namespace crafted_craft


#endif /* SRC_BUILD_IN_PLUGINS_COMMUNICATION_CORE */
