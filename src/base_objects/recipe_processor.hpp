#ifndef SRC_BASE_OBJECTS_RECIPE_PROCESSOR
#define SRC_BASE_OBJECTS_RECIPE_PROCESSOR
#include <functional>
#include <library/enbt.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/slot.hpp>

namespace copper_server::base_objects {
    struct recipe_processor {
        //returns empty slot if slots not suits recipe, do not modify `slots` argument if recipe not suits
        //`slots` is refrence to slots and handler must not deallocate them, they can't be nullptr
        using handler = std::function<slot(const enbt::compound_const_ref& recipe, list_array<slot*>& slots, uint32_t dim_x, uint32_t dim_z, const command_context& context)>;

        slot process_recipe(const std::string& recipe_id, list_array<slot*>& slots, uint32_t dim_x, uint32_t dim_z, const command_context& context) const {
            auto& recipe = recipes.at(recipe_id);
            return handlers.at(normalize_name((std::string)recipe.at("type")))(recipe, slots, dim_x, dim_z, context);
        }

        slot process_recipe(const enbt::compound_const_ref& recipe, list_array<slot*>& slots, uint32_t dim_x, uint32_t dim_z, const command_context& context) const {
            return handlers.at(normalize_name((std::string)recipe.at("type")))(recipe, slots, dim_x, dim_z, context);
        }

        void register_handler(const std::string& name, handler handler) {
            handlers[normalize_name(name)] = std::move(handler);
        }

        void unregister_handler(const std::string& name) {
            handlers.erase(normalize_name(name));
        }

        const handler& get_handler(const std::string& name) const {
            return handlers.at(normalize_name(name));
        }

        void reset_handlers() {
            handlers.clear();
        }

        bool has_handler(const std::string& name) const {
            return handlers.find(normalize_name(name)) != handlers.end();
        }

        void set_recipe(const std::string& name, const enbt::compound& recipe) {
            recipes[normalize_name(name)] = recipe;
        }

        void set_recipe(const std::string& name, enbt::compound&& recipe) {
            recipes[normalize_name(name)] = std::move(recipe);
        }

        void remove_recipe(const std::string& name) {
            recipes.erase(normalize_name(name));
        }

        bool has_recipe(const std::string& name) const {
            return recipes.find(normalize_name(name)) != recipes.end();
        }

        void remove_recipes() {
            recipes.clear();
        }

    private:
        static std::string normalize_name(const std::string& name) {
            if (name.find(':') != std::string::npos)
                return name;
            else
                return "minecraft:" + name;
        }

        std::unordered_map<std::string, enbt::compound> recipes;
        std::unordered_map<std::string, handler> handlers;
    };
}
#endif /* SRC_BASE_OBJECTS_RECIPE_PROCESSOR */
