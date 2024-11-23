#ifndef SRC_BUILD_IN_PLUGINS_SERVER
#define SRC_BUILD_IN_PLUGINS_SERVER
#include <src/base_objects/commands.hpp>
#include <src/plugin/main.hpp>
#include <src/storage/players_data.hpp>

namespace copper_server {
    namespace build_in_plugins {
        //handles clients, commands, server and provides basic server commands
        class ServerPlugin : public PluginAutoRegister<"server", ServerPlugin> {
            storage::players_data players_data;

        public:
            base_objects::command_manager manager;

            ServerPlugin();

            void OnRegister(const PluginRegistrationPtr& self) override;
            void OnLoad(const PluginRegistrationPtr& self) override;
            void OnPostLoad(const std::shared_ptr<PluginRegistration>&) override;
            void OnUnload(const PluginRegistrationPtr& self) override;

            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override;
            plugin_response OnPlay_initialize(base_objects::client_data_holder& client_ref) override;
            plugin_response OnPlay_uninitialized(base_objects::client_data_holder& client) override;
        };
    }
}


#endif /* SRC_BUILD_IN_PLUGINS_SERVER */
