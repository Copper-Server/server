#ifndef SRC_BUILD_IN_PLUGINS_PERMISSIONS
#define SRC_BUILD_IN_PLUGINS_PERMISSIONS
#include "../base_objects/commands.hpp"
#include "../plugin/main.hpp"
#include "../storage/list_storage.hpp"
#include "../storage/permissions_manager.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        //provides commands to manipulate permissions and implements OP list
        class PermissionsPlugin : public PluginAutoRegister<"permissions", PermissionsPlugin> {
            storage::list_storage op_list;
            storage::permissions_manager manager;
            void update_perm(base_objects::SharedClientData& client_ref);

        public:
            PermissionsPlugin();
            void OnInitialization(const PluginRegistrationPtr& self) override;

            void OnLoad(const PluginRegistrationPtr& self) override;
            void OnPostLoad(const PluginRegistrationPtr& self) override;

            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override;
            plugin_response OnPlay_initialize(base_objects::client_data_holder& client_ref) override;
        };
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_PERMISHIONS */
