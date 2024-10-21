#ifndef SRC_API_INTERNAL_RECIPE
#define SRC_API_INTERNAL_RECIPE
#include "../../base_objects/recipe_processor.hpp"

namespace crafted_craft::api::recipe {
    void register_processor(base_objects::recipe_processor& processor);
    void unregister_processor();
}

#endif /* SRC_API_INTERNAL_RECIPE */
