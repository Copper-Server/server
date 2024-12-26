#ifndef SRC_BUILD_IN_PLUGINS_BAN
#define SRC_BUILD_IN_PLUGINS_BAN
#include <src/plugin/main.hpp>
#include <src/storage/enbt_list_storage.hpp>

namespace copper_server::build_in_plugins {
    //provides and manages ban system
    class BanPlugin : public PluginAutoRegister<"ban", BanPlugin> {
        storage::enbt_list_storage banned_players;
        storage::enbt_list_storage banned_ips;

    public:
        BanPlugin();

        void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override;
        plugin_response OnPlay_initialize(base_objects::client_data_holder& client) override;
    };
}
#endif /* SRC_BUILD_IN_PLUGINS_BAN */
