#ifndef SRC_API_PREDICATE
#define SRC_API_PREDICATE
#include "../base_objects/predicate_processor.hpp"

namespace crafted_craft::api::predicate {
    bool process_predicate(const enbt::compound_ref& predicate, const base_objects::command_context& context);
    void register_handler(const std::string& name, base_objects::predicate_processor::handler handler);
    void unregister_handler(const std::string& name);
    const base_objects::predicate_processor::handler& get_handler(const std::string& name);
    bool has_handler(const std::string& name);

    bool registered();
}
#endif /* SRC_API_PREDICATE */
