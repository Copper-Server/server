#ifndef SRC_API_INTERNAL_LOOT_TABLE_POOL_ENTRY
#define SRC_API_INTERNAL_LOOT_TABLE_POOL_ENTRY
#include <src/base_objects/loot_table_pool_entry_processor.hpp>

namespace copper_server::api::loot_table_pool_entry {
    void register_processor(base_objects::loot_table_pool_entry_processor& processor);
    void unregister_processor();
}

#endif /* SRC_API_INTERNAL_LOOT_TABLE_POOL_ENTRY */
