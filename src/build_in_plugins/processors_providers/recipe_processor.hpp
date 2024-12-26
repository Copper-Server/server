#ifndef SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_RECIPE_PROCESSOR
#define SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_RECIPE_PROCESSOR
#include <src/base_objects/recipe_processor.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::processors_providers {
    //provides processor and registers default handles, custom handles can be added via api
    class RecipeProcessor : public PluginAutoRegister<"recipe_processor", RecipeProcessor> {
        base_objects::recipe_processor processor;

    public:
        RecipeProcessor();
        void OnInitialization(const PluginRegistrationPtr& self) override;
    };
}
#endif /* SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_RECIPE_PROCESSOR */
