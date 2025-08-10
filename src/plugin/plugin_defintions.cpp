/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/plugin/main.hpp>
#include <src/plugin/special.hpp>

namespace copper_server {
    SpecialPluginStatus* special_status;

    namespace __internal__ {
        std::vector<std::pair<std::string, std::shared_ptr<delayed_construct_base>>>& registration_list() {
            static std::vector<std::pair<std::string, std::shared_ptr<delayed_construct_base>>> list;
            return list;
        }

        void register_configuration(const PluginRegistrationPtr& self) {
            pluginManagement.registerPluginOn(self, PluginManagement::registration_on::configuration);
        }

        void register_play(const PluginRegistrationPtr& self) {
            pluginManagement.registerPluginOn(self, PluginManagement::registration_on::play);
        }
    }
}