#ifndef SRC_BUILD_IN_PLUGINS_BAN
#define SRC_BUILD_IN_PLUGINS_BAN
#include "../plugin/registration.hpp"
#include "../storage/enbt_list_storage.hpp"

namespace crafted_craft {
    class Server;
    namespace build_in_plugins {
        class BanPlugin : public PluginRegistration {
            storage::enbt_list_storage banned_players;
            storage::enbt_list_storage banned_ips;
            Server& server;

        public:
            BanPlugin();

            void OnLoad(const PluginRegistrationPtr& self) override;

            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override;
            plugin_response OnPlay_initialize(base_objects::client_data_holder& client) override;
        };
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_BAN */
