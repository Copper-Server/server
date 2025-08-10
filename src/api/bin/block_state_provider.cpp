/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/base_objects/block_state_provider.hpp>

namespace copper_server::api::block_state_provider {
    base_objects::block_state_provider_generator* generator = nullptr;

    void register_generator(base_objects::block_state_provider_generator& processor) {
        if (generator)
            throw std::runtime_error("block_state_provider_generator already registered");
        generator = &processor;
    }

    void unregister_generator() {
        if (!generator)
            throw std::runtime_error("block_state_provider_generator already unregistered");
        generator = nullptr;
    }

    std::function<base_objects::block()> process_provider(const enbt::compound_const_ref& provider_config) {
        if (generator)
            return generator->process_provider(provider_config);
        else
            throw std::runtime_error("block_state_provider_generator not registered");
    }

    void register_handler(const std::string& name, base_objects::block_state_provider_generator::handler handler) {
        if (generator)
            generator->register_handler(name, handler);
    }

    void unregister_handler(const std::string& name) {
        if (generator)
            generator->unregister_handler(name);
    }

    const base_objects::block_state_provider_generator::handler& get_handler(const std::string& name) {
        if (generator)
            return generator->get_handler(name);
        else
            throw std::runtime_error("block_state_provider_generator not registered");
    }

    void reset_handlers() {
        if (generator)
            generator->reset_handlers();
    }

    bool has_handler(const std::string& name) {
        return generator ? generator->has_handler(name) : false;
    }

    bool registered() {
        return generator;
    }
}