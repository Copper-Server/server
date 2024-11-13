#ifndef SRC_BUILD_IN_PLUGINS_ALLOWLIST
#define SRC_BUILD_IN_PLUGINS_ALLOWLIST
#include <src/api/allowlist.hpp>
#include <src/plugin/main.hpp>
#include <src/storage/list_storage.hpp>

namespace copper_server {
    namespace build_in_plugins {
        //provides and manages allow list system
        class AllowListPlugin : public PluginAutoRegister<"allow_list", AllowListPlugin> {
            storage::list_storage allow_list;
            api::allowlist::allowlist_mode mode = api::allowlist::allowlist_mode::off;

        public:
            AllowListPlugin();

            void OnPostLoad(const PluginRegistrationPtr& self) override;

            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override;
            plugin_response OnPlay_initialize(base_objects::client_data_holder& client) override;
        };
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_ALLOWLIST */
