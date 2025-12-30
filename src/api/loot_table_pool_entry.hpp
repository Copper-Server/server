/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_LOOT_TABLE_POOL_ENTRY
#define SRC_API_LOOT_TABLE_POOL_ENTRY
#include <src/base_objects/commands.hpp>
#include <src/base_objects/slot.hpp>
#include <src/util/nbt.hpp>

namespace copper_server::api::loot_table_pool_entry {
    using handler = std::function<std::optional<base_objects::slot>(const util::nbt_compound&, const base_objects::command_context&)>;

    std::optional<base_objects::slot> process_entry(const util::nbt_compound& predicate, const base_objects::command_context& context);
    void register_handler(const std::string& name, handler handler);
    void unregister_handler(const std::string& name);
    const handler& get_handler(const std::string& name);
    void reset_handlers();
    bool has_handler(const std::string& name);
}
#endif /* SRC_API_LOOT_TABLE_POOL_ENTRY */
