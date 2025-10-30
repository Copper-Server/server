/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/recipe.hpp>
#include <src/api/registers.hpp>

namespace copper_server::api::recipe {
    std::unordered_map<std::string, std::pair<handler, handler_placement>> handlers;

    base_objects::slot process_recipe(const std::string& recipe_id, std::unordered_map<uint32_t, base_objects::slot_data*>& slots, uint32_t dim_x, uint32_t dim_y, const base_objects::command_context& context) {
        auto& recipe = api::registers::recipe_table.at(recipe_id);
        return handlers.at((std::string)recipe.get_data_name()).first(recipe, slots, dim_x, dim_y, context);
    }

    base_objects::slot process_recipe(const base_objects::recipe& recipe, std::unordered_map<uint32_t, base_objects::slot_data*>& slots, uint32_t dim_x, uint32_t dim_y, const base_objects::command_context& context) {
        return handlers.at((std::string)recipe.get_data_name()).first(recipe, slots, dim_x, dim_y, context);
    }

    std::unordered_map<uint32_t, api::id::set::item> process_placement(const std::string& recipe_id, uint32_t dim_x, uint32_t dim_y, const base_objects::command_context& context) {
        auto& recipe = api::registers::recipe_table.at(recipe_id);
        return handlers.at((std::string)recipe.get_data_name()).second(recipe, dim_x, dim_y, context);
    }

    std::unordered_map<uint32_t, api::id::set::item> process_placement(const base_objects::recipe& recipe, uint32_t dim_x, uint32_t dim_y, const base_objects::command_context& context) {
        return handlers.at((std::string)recipe.get_data_name()).second(recipe, dim_x, dim_y, context);
    }

    void register_handler(const std::string& name, handler&& handler, handler_placement&& placement) {
        handlers[api::registers::normalize_entry(name)] = std::make_pair(std::move(handler), std::move(placement));
    }

    void unregister_handler(const std::string& name) {
        handlers.erase(api::registers::normalize_entry(name));
    }

    const handler& get_handler(const std::string& name) {
        return handlers.at(api::registers::normalize_entry(name)).first;
    }

    const handler_placement& get_handler_placement(const std::string& name) {
        return handlers.at(api::registers::normalize_entry(name)).second;
    }

    void reset_handlers() {
        handlers.clear();
    }

    bool has_handler(const std::string& name) {
        return handlers.find(api::registers::normalize_entry(name)) != handlers.end();
    }
}