#ifndef SRC_BUILD_IN_PLUGINS_PERMISSIONS
#define SRC_BUILD_IN_PLUGINS_PERMISSIONS
#include <src/base_objects/commands.hpp>
#include <src/plugin/main.hpp>
#include <src/storage/list_storage.hpp>
#include <src/storage/permissions_manager.hpp>

namespace copper_server::build_in_plugins {
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
        base_objects::network::plugin_response PlayerJoined(base_objects::client_data_holder& client_ref) override;
    };
}
#endif /* SRC_BUILD_IN_PLUGINS_PERMISHIONS */
