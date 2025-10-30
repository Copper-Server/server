/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/loot_table_pool_entry.hpp>
#include <src/api/world.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::processors_providers {
    struct loot_table_pool_entry : public plugin_auto_register<"processors_provider/loot_table_pool_entry", loot_table_pool_entry> {
        void on_initialization(const plugin_registration_ptr&) override {
        }
    };
}
