#ifndef SRC_BUILD_IN_PLUGINS_SERVER
#define SRC_BUILD_IN_PLUGINS_SERVER
#include "../base_objects/commands.hpp"
#include "../base_objects/virtual_client.hpp"
#include "../plugin/registration.hpp"
#include "../storage/permissions_manager.hpp"
#include "../storage/players_data.hpp"

namespace crafted_craft {
    class Server;
    namespace build_in_plugins {
        class ServerPlugin : public PluginRegistration {
            storage::players_data players_data;
            Server& server;
            base_objects::virtual_client console_data;

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
