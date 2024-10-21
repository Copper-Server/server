#ifndef SRC_BASE_OBJECTS_LOOT_TABLE_POOL_ENTRY_PROCESSOR
#define SRC_BASE_OBJECTS_LOOT_TABLE_POOL_ENTRY_PROCESSOR
#include "../library/enbt.hpp"
#include "commands.hpp"
#include "slot.hpp"

namespace crafted_craft::base_objects {
    struct loot_table_pool_entry_processor {
        using handler = std::function<std::optional<slot>(const enbt::compound_const_ref&, const command_context&)>;

        std::optional<slot> process_entry(const enbt::compound_const_ref& predicate, const command_context& context) const {
            return handlers.at(normalize_name((std::string)predicate["type"]))(predicate, context);
        }

        void register_handler(const std::string& name, handler handler) {
            handlers[normalize_name(name)] = std::move(handler);
        }

        void unregister_handler(const std::string& name) {
            handlers.erase(normalize_name(name));
        }

        const handler& get_handler(const std::string& name) const {
            return handlers.at(normalize_name(name));
        }

        void reset_handlers() {
            handlers.clear();
        }
        
        bool has_handler(const std::string& name) const {
            return handlers.find(normalize_name(name)) != handlers.end();
        }


    private:
        static std::string normalize_name(const std::string& name) {
            if (name.find(':') != std::string::npos)
                return name;
            else
                return "minecraft:" + name;
        }

        std::unordered_map<std::string, handler> handlers;
    };
}

#endif /* SRC_BASE_OBJECTS_LOOT_TABLE_POOL_ENTRY_PROCESSOR */
