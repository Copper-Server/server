/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_PREDICATE_PROCESSOR
#define SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_PREDICATE_PROCESSOR
#include <src/base_objects/predicate_processor.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::processors_providers {
    //provides processor and registers default handles, custom handles can be added via api
    //also provides custom predicates:
    //`copper_server:__adventure_block_`  //for minecraft:can_break and minecraft:can_place_on components uses loot_context to extract `block_state` and `origin` values, so this predicate can be used in normal predicates and uses same format for `minecraft:can_break` or `minecraft:can_place_on` components
    class PredicateProcessor : public PluginAutoRegister<"predicate_processor", PredicateProcessor> {
        base_objects::predicate_processor processor;

    public:
        PredicateProcessor();
        void OnInitialization(const PluginRegistrationPtr& self) override;
    };
}
#endif /* SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_PREDICATE_PROCESSOR */
