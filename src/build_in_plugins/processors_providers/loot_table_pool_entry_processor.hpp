#ifndef SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_LOOT_TABLE_POOL_ENTRY_PROCESSOR
#define SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_LOOT_TABLE_POOL_ENTRY_PROCESSOR
#include <src/base_objects/loot_table_pool_entry_processor.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::processors_providers {
    //provides processor and registers default handles, custom handles can be added via api
    class LootTablePoolEntryProcessor : public PluginAutoRegister<"loot_table_pool_entry_processor", LootTablePoolEntryProcessor> {
        base_objects::loot_table_pool_entry_processor processor;

    public:
        LootTablePoolEntryProcessor();
        void OnInitialization(const PluginRegistrationPtr& self) override;
    };
}
#endif /* SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_LOOT_TABLE_POOL_ENTRY_PROCESSOR */
