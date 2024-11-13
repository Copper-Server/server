
#include <src/api/internal/recipe.hpp>
#include <src/build_in_plugins/processors_providers/recipe_processor.hpp>

namespace copper_server {
    namespace build_in_plugins {
        using namespace base_objects;

        //TODO
        slot handle_furnace(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_shaped(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_shapeless(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_transmute(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_special_armordye(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_special_bannerduplicate(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_special_bookcloning(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_special_firework_rocket(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_special_firework_star(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_special_firework_star_fade(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_special_mapcloning(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_special_mapextending(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_special_repairitem(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_special_shielddecoration(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_special_shulkerboxcoloring(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_special_tippedarrow(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_special_suspiciousstew(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_crafting_decorated_pot(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_smithing_transform(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_smithing_trim(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        slot handle_stonecutting(const enbt::compound_const_ref& recipe, list_array<base_objects::slot*>& slots, uint32_t dim_x, uint32_t dim_z, const base_objects::command_context& context) {
            return std::nullopt;
        }

        RecipeProcessor::RecipeProcessor() {}

        void RecipeProcessor::OnInitialization(const PluginRegistrationPtr& self) {
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
    }
}