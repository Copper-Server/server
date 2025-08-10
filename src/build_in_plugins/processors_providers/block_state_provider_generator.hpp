/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_BLOCK_STATE_PROVIDER_GENERATOR
#define SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_BLOCK_STATE_PROVIDER_GENERATOR
#include <src/base_objects/block_state_provider.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::processors_providers {
    //provides generator and registers default handles, custom handles can be added via api
    class BlockStateProviderGenerator : public PluginAutoRegister<"block_state_provider_generator", BlockStateProviderGenerator> {
        base_objects::block_state_provider_generator generator;

    public:
        BlockStateProviderGenerator();
        void OnInitialization(const PluginRegistrationPtr& self) override;
    };
}
#endif /* SRC_BUILD_IN_PLUGINS_PROCESSORS_PROVIDERS_BLOCK_STATE_PROVIDER_GENERATOR */
