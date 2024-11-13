#include <src/base_objects/loot_table_pool_entry_processor.hpp>

namespace copper_server::api::loot_table_pool_entry {
    base_objects::loot_table_pool_entry_processor* processor;

    void register_processor(base_objects::loot_table_pool_entry_processor& to_register_processor) {
        if (processor)
            throw std::runtime_error("loot_table_pool_entry_processor already registered");
        processor = &to_register_processor;
    }

    void unregister_processor() {
        if (!processor)
            throw std::runtime_error("loot_table_pool_entry_processor already unregistered");
        processor = nullptr;
    }

    std::optional<base_objects::slot> process_entry(const enbt::compound_ref& entry, const base_objects::command_context& context) {
        if (processor)
            return processor->process_entry(entry, context);
        else
            return std::nullopt;
    }

    void register_handler(const std::string& name, base_objects::loot_table_pool_entry_processor::handler handler) {
        if (processor)
            processor->register_handler(name, handler);
    }

    void unregister_handler(const std::string& name) {
        if (processor)
            processor->unregister_handler(name);
    }

    const base_objects::loot_table_pool_entry_processor::handler& get_handler(const std::string& name) {
        if (processor)
            return processor->get_handler(name);
        else
            throw std::runtime_error("predicate_processor not registered");
    }

    bool has_handler(const std::string& name) {
        return processor ? processor->has_handler(name) : false;
    }

    bool registered() {
        return processor;
    }
}