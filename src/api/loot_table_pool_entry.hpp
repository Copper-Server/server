#ifndef SRC_API_LOOT_TABLE_POOL_ENTRY
#define SRC_API_LOOT_TABLE_POOL_ENTRY
#include "../base_objects/loot_table_pool_entry_processor.hpp"

namespace crafted_craft {
    namespace api {
        namespace loot_table_pool_entry {
            std::optional<base_objects::slot> process_entry(const enbt::compound_ref& predicate, const base_objects::command_context& context);
            void register_handler(const std::string& name, base_objects::loot_table_pool_entry_processor::handler handler);
            void unregister_handler(const std::string& name);
            const base_objects::loot_table_pool_entry_processor::handler& get_handler(const std::string& name);
            bool has_handler(const std::string& name);

            bool registered();
        }
    }
}
#endif /* SRC_API_LOOT_TABLE_POOL_ENTRY */
