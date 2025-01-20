#ifndef SRC_BUILD_IN_PLUGINS_CONSOLE
#define SRC_BUILD_IN_PLUGINS_CONSOLE
#include <src/base_objects/commands.hpp>
#include <src/base_objects/virtual_client.hpp>
#include <src/plugin/main.hpp>
#include <src/storage/players_data.hpp>

namespace copper_server::build_in_plugins {
    //provides access to admin console
    class ConsolePlugin : public PluginAutoRegister<"console_interface", ConsolePlugin> {
        base_objects::virtual_client console_data;

    public:
        ConsolePlugin();

        void OnLoad(const PluginRegistrationPtr& self) override;
        void OnUnload(const PluginRegistrationPtr& self) override;

        void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override;
    };
}
#endif /* SRC_BUILD_IN_PLUGINS_CONSOLE */
