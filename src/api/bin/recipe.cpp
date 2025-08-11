/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/base_objects/recipe_processor.hpp>

namespace copper_server::api::recipe {
    base_objects::recipe_processor* processor = nullptr;

    void register_processor(base_objects::recipe_processor& to_register_processor) {
        if (processor)
            throw std::runtime_error("recipe_processor already registered");
        processor = &to_register_processor;
    }

    void unregister_processor() {
        if (!processor)
            throw std::runtime_error("recipe_processor already unregistered");
        processor = nullptr;
    }

    base_objects::slot process_recipe(const std::string& recipe_id, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
        if (processor)
            return processor->process_recipe(recipe_id, slots, dim_x, dim_z, context);
        else
            throw std::runtime_error("recipe_processor not registered");
    }

    base_objects::slot process_recipe(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
        if (processor)
            return processor->process_recipe(recipe, slots, dim_x, dim_z, context);
        else
            throw std::runtime_error("recipe_processor not registered");
    }

    void register_handler(const std::string& name, base_objects::recipe_processor::handler handler) {
        if (processor)
            processor->register_handler(name, handler);
        else
            throw std::runtime_error("recipe_processor not registered");
    }

    void unregister_handler(const std::string& name) {
        if (processor)
            processor->unregister_handler(name);
    }

    const base_objects::recipe_processor::handler& get_handler(const std::string& name) {
        if (processor)
            return processor->get_handler(name);
        else
            throw std::runtime_error("recipe_processor not registered");
    }

    void reset_handlers() {
        if (processor)
            return processor->reset_handlers();
    }

    bool has_handler(const std::string& name) {
        if (processor)
            return processor->has_handler(name);
        else
            return false;
    }

    void set_recipe(const std::string& name, const enbt::compound& recipe) {
        if (processor)
            return processor->set_recipe(name, recipe);
        else
            throw std::runtime_error("recipe_processor not registered");
    }

    void set_recipe(const std::string& name, enbt::compound&& recipe) {
        if (processor)
            return processor->set_recipe(name, std::move(recipe));
        else
            throw std::runtime_error("recipe_processor not registered");
    }

    void remove_recipe(const std::string& name) {
        if (processor)
            return processor->remove_recipe(name);
    }

    bool has_recipe(const std::string& name) {
        if (processor)
            return processor->has_recipe(name);
        else
            return false;
    }

    void remove_recipes() {
        if (processor)
            return processor->remove_recipes();
    }

    bool registered() {
        return processor;
    }
}