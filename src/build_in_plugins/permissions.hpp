#ifndef SRC_BUILD_IN_PLUGINS_PERMISSIONS
#define SRC_BUILD_IN_PLUGINS_PERMISSIONS
#include "../base_objects/commands.hpp"
#include "../plugin/registration.hpp"
#include "../storage/list_storage.hpp"

namespace crafted_craft {
    class Server;

    namespace build_in_plugins {
        class PermissionsPlugin : public PluginRegistration {
            storage::list_storage op_list;
            Server& server;
            void update_perm(base_objects::SharedClientData& client_ref);

        public:
            PermissionsPlugin();

            void OnLoad(const PluginRegistrationPtr& self) override;
            void OnPostLoad(const PluginRegistrationPtr& self) override;

            void OnUnload(const PluginRegistrationPtr& self) override;

            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override;
            plugin_response OnPlay_initialize(base_objects::client_data_holder& client_ref) override;
        };
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_PERMISHIONS */
