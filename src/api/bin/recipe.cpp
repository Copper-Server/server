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
    std::unordered_map<std::string, enbt::compound> recipes;
    std::unordered_map<std::string, handler> handlers;

    base_objects::slot process_recipe(const std::string& recipe_id, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
        auto& recipe = recipes.at(recipe_id);
        return handlers.at(api::registers::normalize_entry((std::string)recipe.at("type")))(recipe, slots, dim_x, dim_z, context);
    }

    base_objects::slot process_recipe(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
        return handlers.at(api::registers::normalize_entry((std::string)recipe.at("type")))(recipe, slots, dim_x, dim_z, context);
    }

    void register_handler(const std::string& name, handler handler) {
        handlers[api::registers::normalize_entry(name)] = std::move(handler);
    }

    void unregister_handler(const std::string& name) {
        handlers.erase(api::registers::normalize_entry(name));
    }

    const handler& get_handler(const std::string& name) {
        return handlers.at(api::registers::normalize_entry(name));
    }

    void reset_handlers() {
        handlers.clear();
    }

    bool has_handler(const std::string& name) {
        return handlers.find(api::registers::normalize_entry(name)) != handlers.end();
    }

    void set_recipe(const std::string& name, const enbt::compound& recipe) {
        recipes[api::registers::normalize_entry(name)] = recipe;
    }

    void set_recipe(const std::string& name, enbt::compound&& recipe) {
        recipes[api::registers::normalize_entry(name)] = std::move(recipe);
    }

    void remove_recipe(const std::string& name) {
        recipes.erase(api::registers::normalize_entry(name));
    }

    bool has_recipe(const std::string& name) {
        return recipes.find(api::registers::normalize_entry(name)) != recipes.end();
    }

    void remove_recipes() {
        recipes.clear();
    }
}