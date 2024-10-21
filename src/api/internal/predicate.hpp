#ifndef SRC_API_INTERNAL_PREDICATE
#define SRC_API_INTERNAL_PREDICATE
#include "../../base_objects/predicate_processor.hpp"

namespace crafted_craft::api::predicate {
    void register_processor(base_objects::predicate_processor& processor);
    void unregister_processor();
}

#endif /* SRC_API_INTERNAL_PREDICATE */
