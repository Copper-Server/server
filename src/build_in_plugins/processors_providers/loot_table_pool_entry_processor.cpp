#include <src/api/internal/loot_table_pool_entry.hpp>
#include <src/api/loot_table_pool_entry.hpp>
#include <src/api/world.hpp>
#include <src/build_in_plugins/processors_providers/loot_table_pool_entry_processor.hpp>

namespace copper_server::build_in_plugins::processors_providers {

    LootTablePoolEntryProcessor::LootTablePoolEntryProcessor() {}

    void LootTablePoolEntryProcessor::OnInitialization(const PluginRegistrationPtr& self) {

        api::loot_table_pool_entry::register_processor(processor);
    }
}
