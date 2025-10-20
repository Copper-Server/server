/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/log.hpp>
#include <src/api/packets/client_bound/config.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/plugin/main.hpp>
#include <src/util/readers.hpp>

namespace copper_server::build_in_plugins::base::minecraft {
    struct brand : public plugin_auto_register<"base/minecraft/brand", brand> {
        void on_load(const plugin_registration_ptr& self) override {
            plugin_management.bind_plugin_on("minecraft:brand", self, plugin_management_system::registration_on::configuration);
        }

        bool on_configuration(base_objects::shared_client_data& client) override {
            base_objects::network::response::item r;
            r.write_string("CopperServer");
            client << api::packets::client_bound::config::custom_payload{
                .channel = "minecraft:brand",
                .payload = r.data
            };
            return client.client_brand.size();//end configuration if client already sent brand
        }

        bool on_configuration_handle(const plugin_registration_ptr& _, const std::string& chanel, const list_array<uint8_t>& data, base_objects::shared_client_data& client) override {
            if (chanel == "minecraft:brand") {
                ArrayStream stream(data.data(), data.size());
                int32_t len = stream.read_var<int32_t>();
                std::string brand((char*)stream.data_read(), len);
                stream.r += len;
                client.client_brand = brand;
            }
            return true;
        }
    };
}
