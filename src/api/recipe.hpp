/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_RECIPE
#define SRC_API_RECIPE
#include <functional>
#include <library/enbt/enbt.hpp>
#include <src/api/id.hpp>
#include <src/base_objects/recipe.hpp>
#include <src/base_objects/slot.hpp>

namespace copper_server::base_objects {
    struct command_context;
}

namespace copper_server::api::recipe {
    //returns empty slot if slots not suits recipe, do not modify `slots` argument if recipe not suits
    //`slots` is refrence to slots and handler must not deallocate them, they can't be nullptr
    using handler = std::function<base_objects::slot(const base_objects::recipe& recipe, std::unordered_map<uint32_t, base_objects::slot_data*>& slots, uint32_t dim_x, uint32_t dim_y, const base_objects::command_context& context)>;
    //returns map of the item set for each crafting postion from 0,0 to dim_x, dim_y
    using handler_placement = std::function<std::unordered_map<uint32_t, api::id::set::item>(const base_objects::recipe& recipe, uint32_t dim_x, uint32_t dim_y, const base_objects::command_context& context)>;
    base_objects::slot process_recipe(const std::string& recipe_id, std::unordered_map<uint32_t, base_objects::slot_data*>& slots, uint32_t dim_x, uint32_t dim_y, const base_objects::command_context& context);
    base_objects::slot process_recipe(const base_objects::recipe& recipe, std::unordered_map<uint32_t, base_objects::slot_data*>& slots, uint32_t dim_x, uint32_t dim_y, const base_objects::command_context& context);
    std::unordered_map<uint32_t, api::id::set::item> process_placement(const std::string& recipe_id, uint32_t dim_x, uint32_t dim_y, const base_objects::command_context& context);
    std::unordered_map<uint32_t, api::id::set::item> process_placement(const base_objects::recipe& recipe, uint32_t dim_x, uint32_t dim_y, const base_objects::command_context& context);
    void register_handler(const std::string& name, handler&& handler, handler_placement&& placement);
    void unregister_handler(const std::string& name);
    const handler& get_handler(const std::string& name);
    const handler_placement& get_handler_placement(const std::string& name);
    void reset_handlers();
    bool has_handler(const std::string& name);
}

#endif /* SRC_API_RECIPE */
