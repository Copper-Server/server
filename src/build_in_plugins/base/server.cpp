#include <src/api/client.hpp>
#include <src/api/configuration.hpp>
#include <src/api/console.hpp>
#include <src/api/permissions.hpp>
#include <src/api/players.hpp>
#include <src/api/server.hpp>
#include <src/base_objects/commands.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    struct ServerPlugin : public PluginAutoRegister<"base/server", ServerPlugin> {
        void OnPostLoad(const std::shared_ptr<PluginRegistration>&) override {
            if (api::console::console_enabled())
                api::console::on_command("version");
        }

        void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override {
            using predicate = base_objects::parser;
            using pred_string = base_objects::parsers::string;
            using cmd_pred_string = base_objects::parsers::command::string;

            browser.add_child({"stop", "stop server", "/stop"})
                .set_callback("command.stop", [this](const list_array<predicate>&, base_objects::command_context& context) {
                    api::server::shutdown();
                    pluginManagement.callUnload();
                });
        }
    };
}
