#ifndef SRC_BUILD_IN_PLUGINS_WORLD
#define SRC_BUILD_IN_PLUGINS_WORLD
#include "../library/fast_task.hpp"
#include "../plugin/main.hpp"
#include "../storage/world_data.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        class WorldManagementPlugin : public PluginAutoRegister<"world_manager", WorldManagementPlugin> {
            storage::worlds_data worlds_storage;
            std::shared_ptr<fast_task::task> world_ticking;


            void add_world_id_suggestion(base_objects::command_browser& browser);
            void add_world_name_suggestion(base_objects::command_browser& browser);

        public:
            WorldManagementPlugin();

            void OnRegister(const PluginRegistrationPtr& self);
            void OnLoad(const PluginRegistrationPtr& self) override;
            void OnUnload(const PluginRegistrationPtr& self) override;
            void OnFaultUnload(const PluginRegistrationPtr& self) override;


            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override;
        };
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_WORLD */
