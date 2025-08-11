/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/internal/block_state_provider.hpp>
#include <src/base_objects/block.hpp>
#include <src/base_objects/block_state_provider.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::processors_providers {
    //provides generator and registers default handles, custom handles can be added via api
    class block_state_generator : public PluginAutoRegister<"processors_provider/block_state_generator", block_state_generator> {
        base_objects::block_state_provider_generator generator;

    public:
        block_state_generator() {}

        void OnInitialization(const PluginRegistrationPtr&) override {
            generator.register_handler("simple_state_provider", [](const enbt::compound_const_ref& config, [[maybe_unused]] enbt::compound& local_state) {
                auto& state = config["state"];
                auto& full_states = base_objects::block::get_block((std::string)state["Name"]);
                auto states = full_states.assigned_states_to_properties->left.at(full_states.default_state);
                if (state.contains("Properties")) {
                    for (auto& [key, value] : state["Properties"].as_compound()) {
                        std::string as_string = value;
                        if (states.contains(key)) {
                            if (base_objects::static_block_data::get_allowed_property_values(key).contains(as_string)) {
                                states[key] = as_string;
                                continue;
                            }
                        }
                        throw std::runtime_error("Invalid property for block " + (std::string)state["Name"] + " \"" + key + "\": " + as_string);
                    }
                }
                return base_objects::block(full_states.assigned_states_to_properties->right.at(states));
            });
            generator.register_handler("rotated_block_provider", [](const enbt::compound_const_ref& config, [[maybe_unused]] enbt::compound& local_state) {
                auto& state = config["state"];
                auto& full_states = base_objects::block::get_block((std::string)state["Name"]);
                auto states = full_states.assigned_states_to_properties->left.at(full_states.default_state);
                return base_objects::block(full_states.assigned_states_to_properties->right.at(states));
                //TODO
            });
            //TODO
            api::block_state_provider::register_generator(generator);
        }
    };
}