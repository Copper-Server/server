/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_INTERNAL_BLOCK_STATE_PROVIDER
#define SRC_API_INTERNAL_BLOCK_STATE_PROVIDER
#include <src/base_objects/block_state_provider.hpp>

namespace copper_server::api::block_state_provider {
    void register_generator(base_objects::block_state_provider_generator& processor);
    void unregister_generator();
}

#endif /* SRC_API_INTERNAL_BLOCK_STATE_PROVIDER */
