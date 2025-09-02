/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_BLOCK_STATE_PROVIDER
#define SRC_API_BLOCK_STATE_PROVIDER
#include <functional>
#include <library/enbt/enbt.hpp>
#include <src/base_objects/block.hpp>

namespace copper_server::api::block_state_provider {
    using handler = std::function<base_objects::block(const enbt::compound_const_ref& config, enbt::compound& local_state)>;

    std::function<base_objects::block()> process_provider(const enbt::compound_const_ref& provider_config);
    void register_handler(const std::string& name, handler handler);
    void unregister_handler(const std::string& name);
    const handler& get_handler(const std::string& name);
    void reset_handlers();
    bool has_handler(const std::string& name);
}

#endif /* SRC_API_BLOCK_STATE_PROVIDER */
