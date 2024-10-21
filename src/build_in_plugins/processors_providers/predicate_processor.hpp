#ifndef SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_PREDICATE_PROCESSOR
#define SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_PREDICATE_PROCESSOR
#include "../../base_objects/predicate_processor.hpp"
#include "../../plugin/main.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        //provides processor and registers default handles, custom handles can be added via api
        //also provides custom predicates:
        //`crafted_craft:__adventure_block_`  //for minecraft:can_break and minecraft:can_place_on components uses loot_context to extract `block_state` and `origin` values, so this predicate can be used in normal predicates and uses same format for `minecraft:can_break` or `minecraft:can_place_on` components
        class PredicateProcessor : public PluginAutoRegister<"predicate_processor", PredicateProcessor> {
            base_objects::predicate_processor processor;

        public:
            PredicateProcessor();
            void OnInitialization(const PluginRegistrationPtr& self) override;
        };
    }
}


#endif /* SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_PREDICATE_PROCESSOR */
