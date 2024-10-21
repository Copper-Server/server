
#include "../../base_objects/loot_table_pool_entry_processor.hpp"
#include "../../plugin/main.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        //provides processor and registers default handles, custom handles can be added via api
        class LootTablePoolEntryProcessor : public PluginAutoRegister<"loot_table_pool_entry_processor", LootTablePoolEntryProcessor> {
            base_objects::loot_table_pool_entry_processor processor;

        public:
            LootTablePoolEntryProcessor();
            void OnInitialization(const PluginRegistrationPtr& self) override;
        };
    }
}
