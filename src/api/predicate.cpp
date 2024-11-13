#include <src/base_objects/predicate_processor.hpp>

namespace copper_server::api::predicate {
    base_objects::predicate_processor* processor = nullptr;

    void register_processor(base_objects::predicate_processor& to_register_processor) {
        if (processor)
            throw std::runtime_error("predicate_processor already registered");
        processor = &to_register_processor;
    }

    void unregister_processor() {
        if (!processor)
            throw std::runtime_error("predicate_processor already unregistered");
        processor = nullptr;
    }

    bool process_predicate(const enbt::compound_ref& predicate, const base_objects::command_context& context) {
        if (processor)
            return processor->process_predicate(predicate, context);
        else
            return false;
    }

    void register_handler(const std::string& name, base_objects::predicate_processor::handler handler) {
        if (processor)
            processor->register_handler(name, handler);
    }

    void unregister_handler(const std::string& name) {
        if (processor)
            processor->unregister_handler(name);
    }

    const base_objects::predicate_processor::handler& get_handler(const std::string& name) {
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
