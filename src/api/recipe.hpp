#ifndef SRC_API_RECIPE
#define SRC_API_RECIPE
#include <src/base_objects/recipe_processor.hpp>

namespace copper_server::api::recipe {
    base_objects::slot process_recipe(const std::string& recipe_id, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context);
    base_objects::slot process_recipe(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context);
    void register_handler(const std::string& name, base_objects::recipe_processor::handler handler);
    void unregister_handler(const std::string& name);
    const base_objects::recipe_processor::handler& get_handler(const std::string& name);
    void reset_handlers();
    bool has_handler(const std::string& name);
    void set_recipe(const std::string& name, const enbt::compound& recipe);
    void set_recipe(const std::string& name, enbt::compound&& recipe);
    void remove_recipe(const std::string& name);
    bool has_recipe(const std::string& name);
    void remove_recipes();

    bool registered();
}

#endif /* SRC_API_RECIPE */
