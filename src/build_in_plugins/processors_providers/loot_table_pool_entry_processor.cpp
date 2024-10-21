#include "loot_table_pool_entry_processor.hpp"
#include "../../api/internal/loot_table_pool_entry.hpp"
#include "../../api/loot_table_pool_entry.hpp"
#include "../../api/world.hpp"

namespace crafted_craft {
    namespace build_in_plugins {

        LootTablePoolEntryProcessor::LootTablePoolEntryProcessor() {}

        void LootTablePoolEntryProcessor::OnInitialization(const PluginRegistrationPtr& self) {

            api::loot_table_pool_entry::register_processor(processor);
        }
    }
}
