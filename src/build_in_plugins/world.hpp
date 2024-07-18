#ifndef SRC_BUILD_IN_PLUGINS_WORLD
#define SRC_BUILD_IN_PLUGINS_WORLD
#include "../api/players.hpp"
#include "../base_objects/commands.hpp"
#include "../log.hpp"
#include "../plugin/main.hpp"
#include "../storage/world_data.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        class WorldManagementPlugin : public PluginAutoRegister<"world_manager", WorldManagementPlugin> {
            storage::worlds_data worlds_storage;

            void add_world_id_suggestion(base_objects::command_browser& browser);

        public:
            WorldManagementPlugin();
            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override;
        };
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_WORLD */
