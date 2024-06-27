#ifndef SRC_BUILD_IN_PLUGINS_ALLOWLIST
#define SRC_BUILD_IN_PLUGINS_ALLOWLIST
#include "../api/allowlist.hpp"
#include "../plugin/registration.hpp"
#include "../storage/list_storage.hpp"

namespace crafted_craft {
    class Server;
    namespace build_in_plugins {
        class AllowListPlugin : public PluginRegistration {
            storage::list_storage allow_list;
            Server& server;
            api::allowlist::allowlist_mode mode = api::allowlist::allowlist_mode::off;

        public:
            AllowListPlugin();

            void OnLoad(const PluginRegistrationPtr& self) override;

            void OnUnload(const PluginRegistrationPtr& self) override;

            void OnPostLoad(const PluginRegistrationPtr& self) override;

            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override;
            plugin_response OnPlay_initialize(base_objects::client_data_holder& client) override;
        };
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_ALLOWLIST */
