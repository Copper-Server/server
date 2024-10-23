#include "block_state_provider_generator.hpp"
#include "../../api/internal/block_state_provider.hpp"
#include "../../base_objects/block.hpp"

namespace crafted_craft {
    namespace build_in_plugins {

        BlockStateProviderGenerator::BlockStateProviderGenerator() {
        }

        void BlockStateProviderGenerator::OnInitialization(const PluginRegistrationPtr& self) {
            generator.register_handler("simple_state_provider", [](const enbt::compound_const_ref& config, enbt::compound& local_state) {
                auto& state = config["state"];
                auto& full_states = base_objects::block::get_block((std::string)state["Name"]);
                auto states = full_states.assigned_states.left.at(full_states.default_state);
                if (state.contains("Properties")) {
                    for (auto& [key, value] : enbt::compound::make_ref(state["Properties"])) {
                        std::string as_string = value;
                        if (states.contains(key)) {
                            if (full_states.states[key].contains(as_string)) {
                                states[key] = as_string;
                                continue;
                            }
                        }
                        throw std::runtime_error("Invalid property for block " + (std::string)state["Name"] + " \"" + key + "\": " + as_string);
                    }
                }
                return base_objects::block(full_states.assigned_states.right.at(states));
            });
            generator.register_handler("rotated_block_provider", [](const enbt::compound_const_ref& config, enbt::compound& local_state) {
                auto& state = config["state"];
                auto& full_states = base_objects::block::get_block((std::string)state["Name"]);
                auto states = full_states.assigned_states.left.at(full_states.default_state);
                return base_objects::block(full_states.assigned_states.right.at(states));
                //TODO
            });
            //TODO
            api::block_state_provider::register_generator(generator);
        }
    }
}