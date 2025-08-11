/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/internal/recipe.hpp>
#include <src/base_objects/recipe_processor.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::processors_providers {
    using namespace base_objects;

    //TODO
    slot handle_furnace([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_shaped([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_shapeless([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_transmute([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_special_armordye([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_special_bannerduplicate([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_special_bookcloning([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_special_firework_rocket([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_special_firework_star([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_special_firework_star_fade([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_special_mapcloning([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_special_mapextending([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_special_repairitem([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_special_shielddecoration([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_special_shulkerboxcoloring([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_special_tippedarrow([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_special_suspiciousstew([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_crafting_decorated_pot([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_smithing_transform([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_smithing_trim([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    slot handle_stonecutting([[maybe_unused]] const enbt::compound_const_ref& recipe, [[maybe_unused]] list_array<base_objects::slot*>& slots, [[maybe_unused]] uint32_t dim_x, [[maybe_unused]] uint32_t dim_z, [[maybe_unused]] const base_objects::command_context& context) {
        return std::nullopt;
    }

    //provides processor and registers default handles, custom handles can be added via api
    class recipe : public PluginAutoRegister<"processors_provider/recipe", recipe> {
        base_objects::recipe_processor processor;

    public:
        recipe() {}

        void OnInitialization(const PluginRegistrationPtr&) override {
            processor.register_handler("minecraft:blasting", handle_furnace);
            processor.register_handler("minecraft:smelting", handle_furnace);
            processor.register_handler("minecraft:smoking", handle_furnace);
            processor.register_handler("minecraft:campfire_cooking", handle_furnace);
            processor.register_handler("minecraft:crafting_shaped", handle_crafting_shaped);
            processor.register_handler("minecraft:crafting_transmute", handle_crafting_transmute);
            processor.register_handler("minecraft:crafting_special_armordye", handle_crafting_special_armordye);
            processor.register_handler("minecraft:crafting_special_bannerduplicate", handle_crafting_special_bannerduplicate);
            processor.register_handler("minecraft:crafting_special_bookcloning", handle_crafting_special_bookcloning);
            processor.register_handler("minecraft:crafting_special_firework_rocket", handle_crafting_special_firework_rocket);
            processor.register_handler("minecraft:crafting_special_firework_star", handle_crafting_special_firework_star);
            processor.register_handler("minecraft:crafting_special_firework_star_fade", handle_crafting_special_firework_star_fade);
            processor.register_handler("minecraft:crafting_special_mapcloning", handle_crafting_special_mapcloning);
            processor.register_handler("minecraft:crafting_special_mapextending", handle_crafting_special_mapextending);
            processor.register_handler("minecraft:crafting_special_repairitem", handle_crafting_special_repairitem);
            processor.register_handler("minecraft:crafting_special_shielddecoration", handle_crafting_special_shielddecoration);
            processor.register_handler("minecraft:crafting_special_shulkerboxcoloring‌", handle_crafting_special_shulkerboxcoloring);
            processor.register_handler("minecraft:crafting_special_tippedarrow", handle_crafting_special_tippedarrow);
            processor.register_handler("minecraft:crafting_special_suspiciousstew", handle_crafting_special_suspiciousstew);
            processor.register_handler("minecraft:smithing_transform", handle_smithing_transform);
            processor.register_handler("minecraft:smithing_trim", handle_smithing_trim);
            processor.register_handler("minecraft:stonecutting", handle_stonecutting);
            api::recipe::register_processor(processor);
        }
    };
}
