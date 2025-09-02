/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/block_state_provider.hpp>
#include <src/api/registers.hpp>

namespace copper_server::api::block_state_provider {
    std::unordered_map<std::string, handler> handlers;
    std::function<base_objects::block()> process_provider(const enbt::compound_const_ref& provider_config) {
        auto& handler = handlers.at(registers::normalize_entry((std::string)provider_config["type"]));
        return {[handler = handler, provider_config = provider_config, state = enbt::compound()]() mutable {
            return handler(provider_config, state);
        }};
    }

    void register_handler(const std::string& name, handler handler) {
        handlers[registers::normalize_entry(name)] = std::move(handler);
    }

    void unregister_handler(const std::string& name) {
        handlers.erase(registers::normalize_entry(name));
    }

    const handler& get_handler(const std::string& name) {
        return handlers.at(registers::normalize_entry(name));
    }

    void reset_handlers() {
        handlers.clear();
    }

    bool has_handler(const std::string& name) {
        return handlers.find(registers::normalize_entry(name)) != handlers.end();
    }
}